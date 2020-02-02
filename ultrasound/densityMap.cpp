#include "densityMap.h"
#include <iostream>

DensityMap::DensityMap(int dim) {
	this->dim = dim;

	radius = 6;

	for (int i = 0; i < dim; i++) {
		cells.push_back(std::vector<std::vector<float>>{});

		for (int j = 0; j < dim; j++) {
			cells.back().push_back(std::vector<float>{});

			for (int k = 0; k < dim; k++) {
				cells.back().back().push_back(0);
			}
		}
	}
}

void DensityMap::addLine(glm::vec3 p1, glm::vec3 p2, std::vector<float> vals) {
	int numVals = vals.size();

	float x = p1.x;
	float y = p1.y;
	float z = p1.z;

	float dx = (p2.x - p1.x) / numVals;
	float dy = (p2.y - p1.y) / numVals;
	float dz = (p2.z - p1.z) / numVals;

	for (int i = 0; i < numVals; i++) {
		// sphere around the current segment

		int ix = x * dim;
		int iy = y * dim;
		int iz = z * dim;

		for (int rx = -radius; rx <= radius; rx++) {
			for (int ry = -radius; ry <= radius; ry++) {
				for (int rz = -radius; rz <= radius; rz++) {
					if (ix + rx < 0 || ix + rx >= dim ||
						iy + ry < 0 || iy + ry >= dim ||
						iz + rz < 0 || iz + rz >= dim) {
						
						continue;
					}

					if (sqrt(rx * rx + ry * ry + rz * rz) > radius) {
						continue;
					}

					float absx = x * dim + float(rx);
					float absy = y * dim + float(ry);
					float absz = z * dim + float(rz);

					float distance = sqrt(pow(absx - ix, 2) + pow(absy - iy, 2) + pow(absz - iz, 2));

					int px = ix + rx;
					int py = iy + ry;
					int pz = iz + rz;

					float n = pow(1.25, -distance);

					if (cells[px][py][pz] < n) {
						cells[px][py][pz] = n;
					}
				}
			}
		}

		x += dx;
		y += dy;
		z += dz;
	}
}

std::vector<float> DensityMap::getVertices() {
	std::vector<float> vertices;

	for (int i = 0; i < dim - 1; i++) {
		for (int j = 0; j < dim - 1; j++) {
			for (int k = 0; k < dim; k++) {
				float v1[4] = { i, j, k,			cells[i][j][k] };
				float v2[4] = { i + 1, j, k,		cells[i + 1][j][k] };
				float v3[4] = { i, j + 1, k,		cells[i][j + 1][k] };
				float v4[4] = { i + 1, j + 1, k,	cells[i + 1][j + 1][k] };

				vertices.insert(vertices.end(), v1, v1 + 4);
				vertices.insert(vertices.end(), v2, v2 + 4);
				vertices.insert(vertices.end(), v4, v4 + 4);

				vertices.insert(vertices.end(), v1, v1 + 4);
				vertices.insert(vertices.end(), v3, v3 + 4);
				vertices.insert(vertices.end(), v4, v4 + 4);
			}
		}
	}

	for (int i = 0; i < dim - 1; i++) {
		for (int j = 0; j < dim; j++) {
			for (int k = 0; k < dim - 1; k++) {
				float v1[4] = { i,j, k,				cells[i][j][k] };
				float v2[4] = { i + 1, j, k,		cells[i + 1][j][k] };
				float v3[4] = { i, j, k + 1,		cells[i][j][k + 1] };
				float v4[4] = { i + 1, j, k + 1,	cells[i + 1][j][k + 1] };

				vertices.insert(vertices.end(), v1, v1 + 4);
				vertices.insert(vertices.end(), v2, v2 + 4);
				vertices.insert(vertices.end(), v4, v4 + 4);

				vertices.insert(vertices.end(), v1, v1 + 4);
				vertices.insert(vertices.end(), v3, v3 + 4);
				vertices.insert(vertices.end(), v4, v4 + 4);
			}
		}
	}

	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim - 1; j++) {
			for (int k = 0; k < dim - 1; k++) {
				float v1[4] = { i, j, k,			cells[i][j][k] };
				float v2[4] = { i, j + 1, k,		cells[i][j + 1][k] };
				float v3[4] = { i, j, k + 1,		cells[i][j][k + 1] };
				float v4[4] = { i, j + 1, k + 1,	cells[i][j + 1][k + 1] };

				vertices.insert(vertices.end(), v1, v1 + 4);
				vertices.insert(vertices.end(), v2, v2 + 4);
				vertices.insert(vertices.end(), v4, v4 + 4);

				vertices.insert(vertices.end(), v1, v1 + 4);
				vertices.insert(vertices.end(), v3, v3 + 4);
				vertices.insert(vertices.end(), v4, v4 + 4);
			}
		}
	}

	return vertices;
}

// Not being used right now, but maybe in the future
// to get smoother shading
float pointLineDistance(glm::vec3 a, glm::vec3 b, glm::vec3 v) {
	glm::vec3 ab = b - a;
	glm::vec3 av = v - a;

	if (glm::dot(av, ab) <= 0.0) {
		return glm::length(av);
	}

	glm::vec3 bv = v - b;

	if (glm::dot(bv, ab) >= 0.0) {
		return glm::length(bv);
	}

	return glm::length(glm::cross(ab, av)) / glm::length(ab);
}