#include "obj_file_loading.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <array>
#include <stdexcept>

#include <glm/gtx/string_cast.hpp>

using VertexFingerprint = std::array<uint64_t, 3>;

ObjMesh loadOBJ(const fs::path& aObjPath) {
	if (!fs::exists(aObjPath) || !fs::is_regular_file(aObjPath)) {
		throw std::runtime_error("File does not exist or is not a regular file: " + aObjPath.string());
	}

	std::ifstream file(aObjPath);

	std::map<VertexFingerprint, int> vertexIndices;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;
	ObjMesh mesh;

	std::string line;
	uint64_t lineNumber = 0;
	while (std::getline(file, line)) {
		++lineNumber;
		std::istringstream iss(line);
		std::string prefix;
		iss >> prefix;

		if (prefix == "v") {
			glm::vec3 position;
			if (!(iss >> position.x >> position.y >> position.z)) {
				throw std::runtime_error(
						"Invalid vertex position format in file: "
						+ aObjPath.string() + " on line " + std::to_string(lineNumber));
			}
			positions.push_back(position);
		} else if (prefix == "vt") {
			glm::vec2 texCoord;
			if (!(iss >> texCoord.x >> texCoord.y)) {
				throw std::runtime_error(
						"Invalid texture coordinate format in file: "
						+ aObjPath.string() + " on line " + std::to_string(lineNumber));
			}
			texCoords.push_back(texCoord);
		} else if (prefix == "vn") {
			glm::vec3 normal;
			if (!(iss >> normal.x >> normal.y >> normal.z)) {
				throw std::runtime_error(
						"Invalid normal format in file: "
						+ aObjPath.string() + " on line " + std::to_string(lineNumber));
			}
			normals.push_back(normal);
		} else if (prefix == "f") {
			std::vector<std::array<uint64_t, 3>> faceVertices;
			std::string vertexStr;
			
			// Read all vertices of the face
			while (iss >> vertexStr) {
				std::istringstream vertexStream(vertexStr);
				std::string indexStr;
				std::array<uint64_t, 3> indices = {0, 0, 0};
				
				// Read vertex/texture/normal indices
				for (int i = 0; i < 3; ++i) {
					if (std::getline(vertexStream, indexStr, '/')) {
						if (!indexStr.empty()) {
							indices[i] = std::stoull(indexStr);
						}
					}
				}
				
				// Check indices
				if (indices[0] > positions.size() || 
					(indices[1] > 0 && indices[1] > texCoords.size()) || 
					(indices[2] > 0 && indices[2] > normals.size())) {
					throw std::runtime_error(
						"Face index out of bounds in file: "
						+ aObjPath.string() + " on line " + std::to_string(lineNumber));
				}
				
				faceVertices.push_back(indices);
			}
			
			// If it's a quad, split it into two triangles
			if (faceVertices.size() == 4) {
				// First triangle
				for (int i = 0; i < 3; ++i) {
					VertexFingerprint fp = {
						faceVertices[i][0] - 1,
						faceVertices[i][1] - 1,
						faceVertices[i][2] - 1
					};
					auto it = vertexIndices.find(fp);
					if (it == vertexIndices.end()) {
						VertexNormTex vertex = {
							positions[fp[0]],
							normals[fp[2]],
							texCoords[fp[1]]
						};
						mesh.vertices.push_back(vertex);
						vertexIndices[fp] = unsigned(mesh.vertices.size() - 1);
						mesh.indices.push_back(unsigned(mesh.vertices.size() - 1));
					} else {
						mesh.indices.push_back(it->second);
					}
				}
				
				// Second triangle
				for (int i = 0; i < 3; ++i) {
					int idx = (i == 0) ? 0 : (i == 1) ? 2 : 3;
					VertexFingerprint fp = {
						faceVertices[idx][0] - 1,
						faceVertices[idx][1] - 1,
						faceVertices[idx][2] - 1
					};
					auto it = vertexIndices.find(fp);
					if (it == vertexIndices.end()) {
						VertexNormTex vertex = {
							positions[fp[0]],
							normals[fp[2]],
							texCoords[fp[1]]
						};
						mesh.vertices.push_back(vertex);
						vertexIndices[fp] = unsigned(mesh.vertices.size() - 1);
						mesh.indices.push_back(unsigned(mesh.vertices.size() - 1));
					} else {
						mesh.indices.push_back(it->second);
					}
				}
			} else if (faceVertices.size() == 3) {
				// Process triangle
				for (const auto& indices : faceVertices) {
					VertexFingerprint fp = {
						indices[0] - 1,
						indices[1] - 1,
						indices[2] - 1
					};
					auto it = vertexIndices.find(fp);
					if (it == vertexIndices.end()) {
						VertexNormTex vertex = {
							positions[fp[0]],
							normals[fp[2]],
							texCoords[fp[1]]
						};
						mesh.vertices.push_back(vertex);
						vertexIndices[fp] = unsigned(mesh.vertices.size() - 1);
						mesh.indices.push_back(unsigned(mesh.vertices.size() - 1));
					} else {
						mesh.indices.push_back(it->second);
					}
				}
			}
		}
	}

	if (mesh.vertices.empty() || mesh.indices.empty()) {
		throw std::runtime_error("Empty mesh or missing data in file: " + aObjPath.string());
	}

	return mesh;
}
