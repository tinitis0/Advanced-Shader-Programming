#ifndef TERRAIN_H
#define TERRAIN_H


#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "PerlinNoise.h"
class Terrain
{
public:
	Terrain(int widthIn, int heightIn, int stepSizeIn);
	Terrain();
	unsigned int getVAO();
	int getSize();
	PerlinNoise perlin;
	
private:
	std::vector<float> vertices;
	unsigned int VAO, VBO;
	int width;
	int height;
	int stepSize;
	void makeVertices(std::vector<float> *vertices);
	void makeVertex(int x, int y, std::vector<float> *vertices);
	std::vector<float> getVertices();
	double cycleOctaves(glm::vec3 pos, int numOctaves);
};
#endif












