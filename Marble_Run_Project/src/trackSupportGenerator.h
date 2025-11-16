#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "structs.h"

class TrackSupportGenerator {
public:
	TrackSupportGenerator();
	void newTrack(float x, float y, float z , float angle, float innerR = 0.9f, float outerR = 1.0f);
	void forward(float distance, float dHeight = 0.0f, float innerR = 0.9f, float outerR = 1.0f);
	void turn(float angle, float radius, float dHeight = 0.0f, int sections = 4, float innerR = 0.9f, float outerR = 1.0f);
	std::vector<TrackSupport> getSupports();
	glm::vec3 nextModuleCenter(float moduleLength = 8.0f, float moduleTilt = 10.0f); // default plinko
	glm::vec3 getLastPos();

	std::vector<TrackSupport> mSupports;
};