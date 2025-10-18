#define GLM_ENABLE_EXPERIMENTAL

//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <SOIL2/soil2.h>
//#include <string>
//#include <iostream>
//#include <fstream>
//#include <cmath>

//#include <glm/glm.hpp>
//#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
//#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtx/euler_angles.hpp>

#include "Utils.h"
using namespace std;

Utils::Utils() {}

string Utils::readShaderFile(const char *filePath) 
{
	string content;

	ifstream fileStream(filePath, ios::in);
	if (!fileStream.is_open())
	{
		throw "Unable to open file.";
	}
	string line = "";
	while (!fileStream.eof()) 
	{
		getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}

bool Utils::checkOpenGLError() 
{
	bool foundError = false;
	int glErr = glGetError();
	while (glErr != GL_NO_ERROR) 
	{
		cout << "glError: " << glErr << endl;
		foundError = true;
		glErr = glGetError();
	}
	return foundError;
}

void Utils::printShaderLog(GLuint shader) 
{
	int len = 0;
	int chWrittn = 0;
	char *log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) 
	{
		log = (char *)malloc(len);
		glGetShaderInfoLog(shader, len, &chWrittn, log);
		cout << "Shader Info Log: " << log << endl;
		free(log);
	}
}

GLuint Utils::prepareShader(int shaderTYPE, const char *shaderPath)
{
	GLint shaderCompiled;
	string shaderStr = readShaderFile(shaderPath);
	const char *shaderSrc = shaderStr.c_str();
	GLuint shaderRef = glCreateShader(shaderTYPE);

	if (shaderRef == 0 || shaderRef == GL_INVALID_ENUM)
	{
		printf("Error: Could not create shader \"%s\" of type:%d\n", shaderPath, shaderTYPE);
		return 0;
	}

	glShaderSource(shaderRef, 1, &shaderSrc, NULL);
	glCompileShader(shaderRef);
	checkOpenGLError();
	
	glGetShaderiv(shaderRef, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled != GL_TRUE)
	{
		if (shaderTYPE == GL_VERTEX_SHADER) cout << "Vertex ";
		if (shaderTYPE == GL_TESS_CONTROL_SHADER) cout << "Tess Control ";
		if (shaderTYPE == GL_TESS_EVALUATION_SHADER) cout << "Tess Eval ";
		if (shaderTYPE == GL_GEOMETRY_SHADER) cout << "Geometry ";
		if (shaderTYPE == GL_FRAGMENT_SHADER) cout << "Fragment ";
		if (shaderTYPE == GL_COMPUTE_SHADER) cout << "Compute ";
		cout << "shader compilation error for shader: '" << shaderPath << "'." << endl;
		printShaderLog(shaderRef);
	}
	return shaderRef;
}

void Utils::printProgramLog(int prog) 
{
	int len = 0;
	int chWrittn = 0;
	char *log;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char *)malloc(len);
		glGetProgramInfoLog(prog, len, &chWrittn, log);
		cout << "Program Info Log: " << log << endl;
		free(log);
	}
}

int Utils::finalizeShaderProgram(GLuint sprogram)
{
	GLint linked;
	glLinkProgram(sprogram);
	checkOpenGLError();
	glGetProgramiv(sprogram, GL_LINK_STATUS, &linked);
	if (linked != 1)
	{
		cout << "linking failed" << endl;
		printProgramLog(sprogram);
	}
	return sprogram;
}

GLuint Utils::createShaderProgram(const char *vp, const char *fp) 
{
	GLuint vShader = prepareShader(GL_VERTEX_SHADER, vp);
	GLuint fShader = prepareShader(GL_FRAGMENT_SHADER, fp);
	GLuint vfprogram = glCreateProgram();
	glAttachShader(vfprogram, vShader);
	glAttachShader(vfprogram, fShader);
	finalizeShaderProgram(vfprogram);
	return vfprogram;
}

GLuint Utils::loadCubeMap(const char *mapDir) 
{
	GLuint textureRef;
	string xp = mapDir; xp = xp + "/px.png";
	string xn = mapDir; xn = xn + "/nx.png";
	string yp = mapDir; yp = yp + "/py.png";
	string yn = mapDir; yn = yn + "/ny.png";
	string zp = mapDir; zp = zp + "/pz.png";
	string zn = mapDir; zn = zn + "/nz.png";
	textureRef = SOIL_load_OGL_cubemap(xp.c_str(), xn.c_str(), yp.c_str(), yn.c_str(), zp.c_str(), zn.c_str(),
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (textureRef == 0) cout << "didnt find cube map image file" << endl;
	//	glBindTexture(GL_TEXTURE_CUBE_MAP, textureRef);
	// reduce seams
	//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return textureRef;
}

GLuint Utils::loadTexture(const char *texImagePath)
{
	GLuint textureRef;
	textureRef = SOIL_load_OGL_texture(texImagePath, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	if (textureRef == 0) cout << "didnt find texture file " << texImagePath << endl;
	// ----- mipmap/anisotropic section
	glBindTexture(GL_TEXTURE_2D, textureRef);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	if (glewIsSupported("GL_EXT_texture_filter_anisotropic")) {
		GLfloat anisoset = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoset);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisoset);
	}
	// ----- end of mipmap/anisotropic section
	return textureRef;
}

std::vector<std::vector<float>> Utils::loadHeightMap(const char* texImagePath)
{
	int width, height, channels;

	// Load image as 1-channel grayscale
	unsigned char* data = SOIL_load_image(texImagePath, &width, &height, &channels, SOIL_LOAD_L);

	if (!data) {
		std::cerr << "Failed to load heightmap: " << texImagePath << std::endl;
		return {};
	}

	std::vector<std::vector<float>> heightmap(width, std::vector<float>(height));

	// SOIL2 loads from top-left corner, row-major order
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			int index = y * width + x;
			float value = static_cast<float>(data[index]) / 255.0f;
			// Some trickery to orient correctly
			heightmap[width-1-x][height-1-y] = value;
		}
	}
	// Free the image data from memory
	SOIL_free_image_data(data);

	return heightmap;

}

// GOLD material - ambient, diffuse, specular, and shininess
float* Utils::goldAmbient() { static float a[4] = { 0.2473f, 0.1995f, 0.0745f, 1 }; return (float*)a; }
float* Utils::goldDiffuse() { static float a[4] = { 0.7516f, 0.6065f, 0.2265f, 1 }; return (float*)a; }
float* Utils::goldSpecular() { static float a[4] = { 0.6283f, 0.5559f, 0.3661f, 1 }; return (float*)a; }
float Utils::goldShininess() { return 51.2f; }

// SILVER material - ambient, diffuse, specular, and shininess
float* Utils::silverAmbient() { static float a[4] = { 0.1923f, 0.1923f, 0.1923f, 1 }; return (float*)a; }
float* Utils::silverDiffuse() { static float a[4] = { 0.5075f, 0.5075f, 0.5075f, 1 }; return (float*)a; }
float* Utils::silverSpecular() { static float a[4] = { 0.5083f, 0.5083f, 0.5083f, 1 }; return (float*)a; }
float Utils::silverShininess() { return 51.2f; }

// BRONZE material - ambient, diffuse, specular, and shininess
float* Utils::bronzeAmbient() { static float a[4] = { 0.2125f, 0.1275f, 0.0540f, 1 }; return (float*)a; }
float* Utils::bronzeDiffuse() { static float a[4] = { 0.7140f, 0.4284f, 0.1814f, 1 }; return (float*)a; }
float* Utils::bronzeSpecular() { static float a[4] = { 0.3936f, 0.2719f, 0.1667f, 1 }; return (float*)a; }
float Utils::bronzeShininess() { return 25.6f; }

void shaderSetVec3(GLuint shaderProgram, const char* name, glm::vec3& value)
{
	glUniform3fv(glGetUniformLocation(shaderProgram, name), 1, &value[0]);
}


void shaderSetVec4(GLuint shaderProgram, const char* name, glm::vec4& value)
{
	glUniform4fv(glGetUniformLocation(shaderProgram, name), 1, &value[0]);
}


void shaderSetMat4(GLuint shaderProgram, const char* name, glm::mat4& value)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name), 1, GL_FALSE, &value[0][0]);
}


void shaderSetFloat(GLuint shaderProgram, const char* name, float value)
{
	glUniform1f(glGetUniformLocation(shaderProgram, name), value);
}


void shaderSetInt(GLuint shaderProgram, const char* name, int value)
{
	glUniform1i(glGetUniformLocation(shaderProgram, name), value);
}