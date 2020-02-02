#pragma once

#include <vector>

#include <glm/glm.hpp>

class DensityMap {
public:
	int dim;
	int radius;

	std::vector<std::vector<std::vector<float>>> cells;

	DensityMap(int dim);

	void addLine(glm::vec3 p1, glm::vec3 p2, std::vector<float> vals);
	std::vector<float> getVertices();
};

float pointLineDistance(glm::vec3 a, glm::vec3 b, glm::vec3 v);