#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>
#include <map>


/**
 * @file    shader.h shader.c
 * @author  Jérôme Lartillot modified by Olivier Nivoix
 * @version 1.0
 *
 * @section REQUIREMENTS
 * -stdlib.h
 * -stdio.h
 * -the GLEW library
 *
 * @section DESCRIPTION
 *
 *  This allow to manage shaders easily.
 *
 *
    Example:
    --------
    Program prog;
	Prog.init("bump.vert", "", "", "", "bump.frag");

	//get Infos
	Prog.printinformations();

	//with uniforms
	prog.getUniformLocation("texunit0");

	// with subroutine
	prog.setSubroutineLocation(GL_VERTEX_SHADER,"fct1");
	prog.setSubroutineLocation(GL_VERTEX_SHADER,"fct2");

    prog.use();
	prog.setUniform("texunit0",0);
	prog.setSubroutine(GL_VERTEX_SHADER,"fct2");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,ntexture);

    //... some drawings...

    prog.unuse();
 *
 */

struct DataGL {
	GLuint vao = 0, pos = 0, tex = 0, norm = 0, color = 0, mag = 0, scale=0, elementBuffer=0;
};


/**
*   \class Program
*   Creates a program for the pipeline from a vertex shader and a fragment shader.
*   You must initialise this with Program(GLuint vs,GLuint fs)
*   or init(GLuint vs,GLuint fs) before calling use().
*
*   Use Program::program to set uniforms variables.
*/
class shaderProgram {
public:
	shaderProgram();
	~shaderProgram();

	void init(const std::string &vs, const std::string &tcs, const std::string &tes, const std::string &gs, const std::string &fs);
	void init(const std::string &vs, const std::string &gs, const std::string &fs);
	void init(const std::string &vs, const std::string &fs);

	/**
	*   \fn initLogFile
	*   set to empty the log file
	*/
	static void initLogFile();

	/**
	*   \fn use
	*   set current program to pipeline
	*/
	void use();

	/**
	*   \fn unuse
	*   clean pipeline from current program
	*/
	void unuse();

	void setUniformLocation(const char * name );

	void setSubroutineLocation(GLenum ShaderType, const char* name);
	void setSubroutine(GLenum ShaderType, const char * name );

	void setUniform( const char *name, const glm::vec2 & v);
	void setUniform( const char *name, const glm::vec3 & v);
	void setUniform( const char *name, const glm::ivec2 & v);
	void setUniform( const char *name, const glm::ivec3 & v);
	void setUniform( const char *name, const glm::vec4 & v);
	void setUniform( const char *name, const glm::ivec4 & v);
	void setUniform( const char *name, const glm::mat4 & m);
	void setUniform( const char *name, float val);
	void setUniform( const char *name, double val);
	void setUniform( const char *name, int val );
	void setUniform( const char *name, bool val );
	void setUniform( const char *name, GLuint val );

private:

	void printActiveUniforms();
	void printActiveUniformBlocks();
	void printActiveAttribs();

	void writeToLog(const std::string &cmd);

	GLint getUniformLocation(const char * name);

	void init(GLuint vs,GLuint tcs=NOSHADER,GLuint tes=NOSHADER,GLuint gs=NOSHADER,GLuint fs=NOSHADER);

	std::string loadFileToString(const char * fname);

	const char * getTypeString( GLenum type );

	/**
	*   \fn debugShader
	*   Print compilation errors of a shader in cout(if they exists).
	*/
	void debugShader(GLuint shader, const std::string &str);

	/**
	*   \fn debugProgram
	*   Print compilation errors of current program in cout(if they exists).
	*/
	void debugProgram();

	/**
	*   \fn makeShader Creates shader from a file.
	*   This function call debugShader.
	*   \param str:        must contain a filename whichcontain the shader code or the shader code.
	*   \param ShaderType: cf glCreateShader: GL_COMPUTE_SHADER, GL_VERTEX_SHADER,
	*                      GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER,
	*                      or GL_FRAGMENT_SHADER
	*   \return Return the shader id as GLuint. You must assign it (or be stupid).
	*/
	GLuint makeShader(const std::string &str, GLenum ShaderType);


	static const unsigned int NOSHADER = ~0;

	GLuint program;
	GLuint vshader; // vertex shader
	GLuint fshader; // fragment shader
	GLuint tcshader; // tesselation control
	GLuint teshader; // tesselation evaluation shader
	GLuint gshader; // geometry shader
	std::string programName;

	std::map<std::string, GLuint> uniformLocations;
	std::map<std::string, GLuint> subroutineLocations;
};

#endif // SHADER_H_INCLUDED
