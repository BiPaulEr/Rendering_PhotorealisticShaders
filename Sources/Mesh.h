#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Transform.h"

class Mesh : public Transform {
public:
	virtual ~Mesh ();

	inline const std::vector<glm::vec3> & vertexPositions () const { return m_vertexPositions; }
	inline std::vector<glm::vec3> & vertexPositions () { return m_vertexPositions; }
	inline const std::vector<glm::vec3> & vertexNormals () const { return m_vertexNormals; }
	inline std::vector<glm::vec3> & vertexNormals () { return m_vertexNormals; }
	inline const std::vector<glm::vec2> & vertexTexCoords () const { return m_vertexTexCoords; }
	inline std::vector<glm::vec2> & vertexTexCoords () { return m_vertexTexCoords; }
	inline const std::vector<glm::uvec3> & triangleIndices () const { return m_triangleIndices; }
	inline std::vector<glm::uvec3> & triangleIndices () { return m_triangleIndices; }

	/// Compute the parameters of a sphere which bounds the mesh
	void computeBoundingSphere (glm::vec3 & center, float & radius) const;

	void recomputePerVertexNormals (bool angleBased = false);

	void init ();
	void render ();
	void clear ();

	void computePlanarParameterization();

private:
	std::vector<glm::vec3> m_vertexPositions;
	std::vector<glm::vec3> m_vertexNormals;
	std::vector<glm::vec2> m_vertexTexCoords;
	std::vector<glm::uvec3> m_triangleIndices;
	GLuint m_vao = 0;
	GLuint m_posVbo = 0;
	GLuint m_normalVbo = 0;
	GLuint m_texCoordVbo = 0;
	GLuint m_ibo = 0;
};

#endif // MESH_H
