#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <glad/glad.h>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class ShaderProgram {
public:
	/// Create the program. A valid OpenGL context must be active.
	ShaderProgram ();

	virtual ~ShaderProgram ();

	/// Generate a minimal shader program, made of one vertex shader and one fragment shader
	static std::shared_ptr<ShaderProgram> genBasicShaderProgram (const std::string & vertexShaderFilename,
															 	 const std::string & fragmentShaderFilename);

	/// OpenGL identifier of the program
	inline GLuint id () { return m_id; }

	/// Loads and compile a shader from a text file, before attaching it to a program
	void loadShader (GLenum type, const std::string & shaderFilename);

	/// The main GPU program is ready to be handle streams of polygons
	inline void link () { glLinkProgram (m_id); }

	/// Activate the program
	inline void use () { glUseProgram (m_id); }

	/// Desactivate the current program
	inline static void stop () { glUseProgram (0); }

	inline GLuint getLocation (const std::string & name) { return glGetUniformLocation (m_id, name.c_str ()); }

	inline void set(const std::string & name, bool value) { glUniform1f(getLocation(name.c_str()), value); }

	inline void set (const std::string & name, float value) { glUniform1f (getLocation (name.c_str ()), value); }

	inline void set (const std::string & name, const glm::vec2 & value) { glUniform2fv (getLocation (name.c_str ()), 1, glm::value_ptr(value)); }

	inline void set (const std::string & name, const glm::vec3 & value) { glUniform3fv (getLocation (name.c_str ()), 1, glm::value_ptr(value)); }

	inline void set (const std::string & name, const glm::vec4 & value) { glUniform4fv (getLocation (name.c_str ()), 1, glm::value_ptr(value)); }

	inline void set (const std::string & name, const glm::mat4 & value) { glUniformMatrix4fv (getLocation (name.c_str ()), 1, GL_FALSE, glm::value_ptr(value)); }

	inline void set (const std::string & name, GLuint value) { glUniform1i (getLocation (name.c_str ()), value); }

private:
	/// Loads the content of an ASCII file in a standard C++ string
	std::string file2String (const std::string & filename);

	GLuint m_id = 0;
};

#endif // SHADER_PROGRAM_H
