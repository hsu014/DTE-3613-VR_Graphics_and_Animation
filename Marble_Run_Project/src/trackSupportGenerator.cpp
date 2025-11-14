#include "trackSupportGenerator.h"

TrackSupportGenerator::TrackSupportGenerator()
{ }

void TrackSupportGenerator::newTrack(
	float x, float y, float z, float angle, float innerR, float outerR)	
{
	mSupports.clear();
	mSupports.push_back({
		x, y, z,
		angle,
		innerR,
		outerR
		});
}

void TrackSupportGenerator::forward(
	float distance, float dHeight, float innerR, float outerR)
{
	TrackSupport& last = mSupports.back();

	float a = glm::radians(last.angle);
	float x = last.x + -sinf(a) * distance;
	float y = last.y + dHeight;
	float z = last.z + cosf(a) * distance;

	mSupports.push_back({
		x, y, z,
		last.angle,
		innerR,
		outerR
		});
}

void TrackSupportGenerator::turn(
	float angle, float radius, float dHeight, int sections, float innerR, float outerR)
{
	// Creates [sections] new supports. Previous support acting as first point in turn.
	TrackSupport last = mSupports.back();

	float x, y, z;
	float startAngle = glm::radians(last.angle);
	float turnAngle = glm::radians(angle);
	float sectionStep = turnAngle / sections;
	float sectionHeightStep = dHeight / (sections + 1);
	int turnCW = 1;		// Turn is clockwise

	glm::vec3 dir = glm::vec3(-sinf(startAngle), 0.0f, cosf(startAngle));
	glm::vec3 turnCenterDir = glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f));

	if (angle < 0.0f) {
		turnCenterDir *= -1;
		turnCW = -1;	// Turn is counter clockwise
	}

	glm::vec3 lastPos = glm::vec3(last.x, last.y, last.z);
	glm::vec3 turnCenter = lastPos + turnCenterDir * radius;

	for (int i = 1; i <= sections; i++) {
		float sectionAngle = startAngle + i * sectionStep;
		float sectionHeight = i * sectionHeightStep;

		x = turnCenter.x + cosf(sectionAngle) * radius * turnCW;
		y = last.y + sectionHeight;
		z = turnCenter.z + sinf(sectionAngle) * radius * turnCW;

		mSupports.push_back({
		x, y, z,
		glm::degrees(sectionAngle),
		innerR,
		outerR
			});
	}
}

std::vector<TrackSupport> TrackSupportGenerator::getSupports()
{
	return mSupports;
}

glm::vec3 TrackSupportGenerator::nextModuleCenter(float moduleLength, float moduleTilt)
{
	TrackSupport& last = mSupports.back();

	float a = glm::radians(last.angle);
	float len = moduleLength / 2.0f;
	float tilt = glm::radians(moduleTilt);

	float x = (-sinf(a) * len) * cosf(tilt);
	float y = -len * sinf(tilt);
	float z = (cosf(a) * len) * cosf(tilt);

	x += last.x;
	y += (last.y - last.outerRadius);
	z += last.z;

	return glm::vec3(x, y, z);
}
