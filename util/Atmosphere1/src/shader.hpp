#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include <GL/glew.h>
#include <string>
#include "vecmath.hpp"
#include <map>
//~ #include "stateGL.hpp"

/**
 * @file    shader.h shader.c
 * @author  Jérôme Lartillot modified by Olivier Nivoix
 * @version 1.0
 *
 * @section REQUIREMENTS
 * -stdlib.h
 * -stdio.h
 * -the GLEW library
 * -using namespace std;
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

	prog.getUniformLocation("texunit0");
	prog.setUniform("texunit0",0);

    prog.use();


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,ntexture);

    //... some drawings...

    glUseProgram(0);
 *
 */

struct DataGL {
	GLuint vao = 0, pos = 0, tex = 0, norm = 0, color = 0, mag = 0, scale=0, elementBuffer=0;
};


const unsigned int NOSHADER = ~0;

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

	void init(std::string vs, std::string tcs, std::string tes, std::string gs, std::string fs);

	void init(GLuint vs,GLuint tcs=NOSHADER,GLuint tes=NOSHADER,GLuint gs=NOSHADER,GLuint fs=NOSHADER);

	/**
	*   \fn setShaderDir
	*   set the Directory path to all shader source
	*   \param shader: the directory path
	*/
	static void setShaderDir(const std::string& _shaderDir) {
		shaderProgram::shaderDir = _shaderDir;
	}

	/**
	*   \fn makeShader Creates shader from a file.
	*   This function call debugShader.
	*   \param str:        must contain a filename whichcontain the shader code or the shader code.
	*   \param ShaderType: cf glCreateShader: GL_COMPUTE_SHADER, GL_VERTEX_SHADER,
	*                      GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER,
	*                      or GL_FRAGMENT_SHADER
	*   \return Return the shader id as GLuint. You must assign it (or be stupid).
	*/
	GLuint makeShader(std::string str, GLenum ShaderType);


	void use();

	//! Program object getter.
	//! @return the program object handle.
	//! @deprecated
	//! You should consider it as read-only.
	GLuint getProgram() {
		return program;
	};

	/**
	*   \fn debugShader
	*   Print compilation errors of a shader in cout(if they exists).
	*   Example: debugShader(myShaderId);
	*   \param shader: the shader to debug.
	*/
	void debugShader(GLuint shader);

	/**
	*   \fn printInformations
	*   Print informations about uniforms, uniforms block, attributes
	*/
	void printInformations();

	void setUniformLocation(const char * name );

	void setUniform( const char *name, const Vec2f & v);
	void setUniform( const char *name, const Vec3f & v);
	void setUniform( const char *name, const Vec2i & v);
	void setUniform( const char *name, const Vec3i & v);
	void setUniform( const char *name, const Vec4f & v);
	void setUniform( const char *name, const Vec4i & v);
	void setUniform( const char *name, const Mat4f & m);
	void setUniform( const char *name, float val);
	void setUniform( const char *name, double val);
	void setUniform( const char *name, int val );
	void setUniform( const char *name, bool val );
	void setUniform( const char *name, GLuint val );

private:
	GLuint program;
	GLuint vshader; // vertex shader
	GLuint fshader; // fragment shader
	GLuint tcshader; // tesselation control
	GLuint teshader; // tesselation evaluation shader
	GLuint gshader; // geometry shader
	static  std::string shaderDir;
	std::string loadFileToString(const char * fname);
	std::map<std::string, GLuint> uniformLocations;
	GLint getUniformLocation(const char * name);

	void printActiveUniforms();
	void printActiveUniformBlocks();
	void printActiveAttribs();
	const char * getTypeString( GLenum type );
	/**
	*  \fn makeShader    Creates a shader from a source code in a two dimensions array.
	*  This may help: https://www.opengl.org/sdk/docs/man/html/glShaderSource.xhtml
	*  This function call debugShader.
	*  \param ShaderType: cf glCreateShader: GL_COMPUTE_SHADER, GL_VERTEX_SHADER,
	*                     GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER,
	*                     or GL_FRAGMENT_SHADER
	*  \param numberOfLines: of the shader source code.
	*  \param code: the source code of the shader.
	*  \param linesLengths: explicit parameter.
	*  \return Return the shader id as GLuint. You must assign it (or be stupid).
	*/
	GLuint makeShader(GLenum ShaderType, GLint numberOfLines, const GLchar**code,const GLint*linesLengths);
};

#endif // SHADER_H_INCLUDED
