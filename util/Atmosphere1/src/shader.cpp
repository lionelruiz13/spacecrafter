#include "shader.hpp"
//~ #include "stateGL.hpp"
#include <iostream>
#include <string.h>
#include <cstdio>
#include <cstdlib>

#include <fstream>

//~ #include "stateGL.hpp"

using namespace std;

string shaderProgram::shaderDir = "";

shaderProgram::shaderProgram()
{
}

shaderProgram::~shaderProgram()
{
	if(vshader!=NOSHADER) {
		glDetachShader(program,vshader);
		glDeleteShader(vshader);
	}
	if(tcshader!=NOSHADER) {
		glDetachShader(program,tcshader);
		glDeleteShader(tcshader);
	}
	if(teshader!=NOSHADER) {
		glDetachShader(program,teshader);
		glDeleteShader(teshader);
	}
	if(gshader!=NOSHADER) {
		glDetachShader(program,gshader);
		glDeleteShader(gshader);
	}
	if(fshader!=NOSHADER) {
		glDetachShader(program,fshader);
		glDeleteShader(fshader);
	}
	uniformLocations.clear();
	glDeleteProgram(program);
}


void shaderProgram::debugShader( GLuint shader)
{
	char* log= nullptr;
	GLsizei logsize = 0;
	GLint compile_status = GL_TRUE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if(compile_status != GL_TRUE) {
		// on recupere la taille du message d'erreur
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);

		// on alloue un espace memoire dans lequel OpenGL ecrira le message
		log = (char*)malloc(logsize + 1);
		if(log == nullptr) {
			cout<<"impossible d'allouer de la memoire !\n"<<endl;
			exit(EXIT_FAILURE);
		}
		memset(log, '\0', logsize + 1);
		glGetShaderInfoLog(shader, logsize, &logsize, log);
		cout<<"impossible de compiler le shader : "<< shader << endl<<log<<endl;
		free(log);
		glDeleteShader(shader);
		exit(EXIT_FAILURE);
	}
}

const char * shaderProgram::getTypeString( GLenum type )
{
	// There are many more types than are covered here, but
	// these are the most common in these examples.
	switch(type) {
		case GL_FLOAT:
			return "float";
		case GL_FLOAT_VEC2:
			return "vec2";
		case GL_FLOAT_VEC3:
			return "vec3";
		case GL_FLOAT_VEC4:
			return "vec4";
		case GL_DOUBLE:
			return "double";
		case GL_INT:
			return "int";
		case GL_INT_VEC2:
			return "vec2i";
		case GL_INT_VEC3:
			return "vec3i";
		case GL_INT_VEC4:
			return "vec4i";
		case GL_UNSIGNED_INT:
			return "unsigned int";

		case GL_BOOL:
			return "bool";
		case GL_FLOAT_MAT2:
			return "mat2";
		case GL_FLOAT_MAT3:
			return "mat3";
		case GL_FLOAT_MAT4:
			return "mat4";
		case GL_SAMPLER_2D:
			return "texture2D";
		default:
			//~ printf("%x\n",type);
			return "?";
	}
}

void shaderProgram::printActiveAttribs()
{
	// >= OpenGL 4.3, use glGetProgramResource
	GLint numAttribs;
	glGetProgramInterfaceiv( program, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttribs);

	GLenum properties[] = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION};

	printf("%i: Active attributes:\n", program);
	for( int i = 0; i < numAttribs; ++i ) {
		GLint results[3];
		glGetProgramResourceiv(program, GL_PROGRAM_INPUT, i, 3, properties, 3, NULL, results);

		GLint nameBufSize = results[0] + 1;
		char * name = new char[nameBufSize];
		glGetProgramResourceName(program, GL_PROGRAM_INPUT, i, nameBufSize, NULL, name);
		printf("%-5d %s (%s)\n", results[2], name, getTypeString(results[1]));
		delete [] name;
	}
}

void shaderProgram::printActiveUniforms()
{
	GLint numUniforms = 0;
	glGetProgramInterfaceiv( program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

	GLenum properties[] = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX};

	printf("%i: Active uniforms:\n", program);
	for( int i = 0; i < numUniforms; ++i ) {
		GLint results[4];
		glGetProgramResourceiv(program, GL_UNIFORM, i, 4, properties, 4, NULL, results);

		if( results[3] != -1 ) continue;  // Skip uniforms in blocks
		GLint nameBufSize = results[0] + 1;
		char * name = new char[nameBufSize];
		glGetProgramResourceName(program, GL_UNIFORM, i, nameBufSize, NULL, name);
		printf("%-5d %s (%s)\n", results[2], name, getTypeString(results[1]));
		delete [] name;
	}
}

void shaderProgram::printActiveUniformBlocks()
{

	GLint numBlocks = 0;

	glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);
	GLenum blockProps[] = {GL_NUM_ACTIVE_VARIABLES, GL_NAME_LENGTH};
	GLenum blockIndex[] = {GL_ACTIVE_VARIABLES};
	GLenum props[] = {GL_NAME_LENGTH, GL_TYPE, GL_BLOCK_INDEX};

	for(int block = 0; block < numBlocks; ++block) {
		GLint blockInfo[2];
		glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, block, 2, blockProps, 2, NULL, blockInfo);
		GLint numUnis = blockInfo[0];

		char * blockName = new char[blockInfo[1]+1];
		glGetProgramResourceName(program, GL_UNIFORM_BLOCK, block, blockInfo[1]+1, NULL, blockName);
		printf("%i: uniform block \"%s\":\n", program, blockName);
		delete [] blockName;

		GLint * unifIndexes = new GLint[numUnis];
		glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, block, 1, blockIndex, numUnis, NULL, unifIndexes);

		for( int unif = 0; unif < numUnis; ++unif ) {
			GLint uniIndex = unifIndexes[unif];
			GLint results[3];
			glGetProgramResourceiv(program, GL_UNIFORM, uniIndex, 3, props, 3, NULL, results);

			GLint nameBufSize = results[0] + 1;
			char * name = new char[nameBufSize];
			glGetProgramResourceName(program, GL_UNIFORM, uniIndex, nameBufSize, NULL, name);
			printf("    %s (%s)\n", name, getTypeString(results[1]));
			delete [] name;
		}

		delete [] unifIndexes;
	}
}


GLuint shaderProgram::makeShader(GLenum ShaderType, GLint numberOfLines,const GLchar**code,const GLint*linesLengths)
{
	GLuint shader = glCreateShader(ShaderType);
	if(shader == 0) {
		cout<< "impossible de creer le shader:"<<endl<<"glCreateShader failed."<<endl;
		exit(EXIT_FAILURE);
	}
	glShaderSource(shader, numberOfLines, code, linesLengths);
	glCompileShader(shader);
	debugShader(shader);
	return shader;
}


std::string shaderProgram::loadFileToString(const char * fname)
{
	std::ifstream ifile(fname);
	std::string filetext;

	if (ifile.is_open()) {
		while( ifile.good() ) {
			std::string line;
			std::getline(ifile, line);
			filetext.append(line + "\n");
		}
	} else {
		// show message:
		std::cout << "Error opening file " << fname << endl;
		exit(3);
	}

	return filetext;
}


GLuint shaderProgram::makeShader(string str,GLenum ShaderType)
{
	str = shaderDir + str;
	string shader_string = loadFileToString(str.c_str());

	GLuint shader = glCreateShader(ShaderType);
	if(shader == 0) {
		fprintf(stderr, "impossible de creer le shader %s\n", str.c_str());
		exit(EXIT_FAILURE);
	}

	GLchar const *shader_source = shader_string.c_str();
	GLint const shader_length = shader_string.size();

	glShaderSource(shader, 1, &shader_source, &shader_length);
	glCompileShader(shader);
	debugShader(shader);

	return shader;
}


void shaderProgram::printInformations()
{
	printActiveUniforms();
	printActiveUniformBlocks();
	printActiveAttribs();
}


void shaderProgram::init(std::string vs, std::string tcs, std::string tes, std::string gs, std::string fs)
{
	GLuint _vs =  makeShader(vs, GL_VERTEX_SHADER);
	GLuint _tcs, _tes, _gs;

	if (tcs != "")
		_tcs =  makeShader(tcs, GL_TESS_CONTROL_SHADER);
	else
		_tcs = NOSHADER;

	if (tes != "")
		_tes =  makeShader(tes, GL_TESS_EVALUATION_SHADER);
	else
		_tes = NOSHADER;

	if (gs != "")
		_gs =  makeShader(gs, GL_GEOMETRY_SHADER);
	else
		_gs = NOSHADER;

	GLuint _fs =  makeShader(fs, GL_FRAGMENT_SHADER);

	init(_vs, _tcs, _tes, _gs, _fs);
}


void shaderProgram::init(GLuint vs,GLuint tcs,GLuint tes,GLuint gs,GLuint fs)
{
	vshader=vs;
	tcshader=tcs;
	teshader=tes;
	gshader=gs;
	fshader=fs;

	program=glCreateProgram();
	if(!glIsProgram(program)) {
		cout<<"Error: couldn't create a new shader program handle."<<endl;
		exit(EXIT_FAILURE);
	}

	if(vs !=NOSHADER) glAttachShader(program,vshader);
	if(tcs!=NOSHADER) glAttachShader(program,tcshader);
	if(tes!=NOSHADER) glAttachShader(program,teshader);
	if(gs !=NOSHADER) glAttachShader(program,gshader);
	if(fs !=NOSHADER) glAttachShader(program,fshader);

	glLinkProgram(program);
}


void shaderProgram::use()
{
	glUseProgram(program);
}


void shaderProgram::setUniformLocation(const char * name )
{
	std::map<string, GLuint>::iterator pos;
	pos = uniformLocations.find(name);

	if( pos == uniformLocations.end() ) {
		uniformLocations[name] = glGetUniformLocation(program, name);
	} else
		printf("%i : setUniformLocation name %s already found !!!\n", program, name);
	//todo retour
}


int shaderProgram::getUniformLocation(const char * name )
{
	std::map<string, GLuint>::iterator pos;
	pos = uniformLocations.find(name);

	if( pos == uniformLocations.end() ) {
		printf("%i : error with %s atribution uniformLocations\n", program, name);
		uniformLocations[name] = glGetUniformLocation(program, name);
	}
	return uniformLocations[name];
}

void shaderProgram::setUniform( const char *name, const Vec3f & v)
{
	GLint loc = getUniformLocation(name);
	glUniform3fv(loc,1,v);
}

void shaderProgram::setUniform( const char *name, const Vec4f & v)
{
	GLint loc = getUniformLocation(name);
	glUniform4fv(loc,1, v);
}

void shaderProgram::setUniform( const char *name, const Vec2f & v)
{
	GLint loc = getUniformLocation(name);
	glUniform2fv(loc,1, v);
}

void shaderProgram::setUniform( const char *name, const Vec3i & v)
{
	GLint loc = getUniformLocation(name);
	glUniform3iv(loc,1,v);
}

void shaderProgram::setUniform( const char *name, const Vec4i & v)
{
	GLint loc = getUniformLocation(name);
	glUniform4iv(loc,1, v);
}

void shaderProgram::setUniform( const char *name, const Vec2i & v)
{
	GLint loc = getUniformLocation(name);
	glUniform2iv(loc,1, v);
}

void shaderProgram::setUniform( const char *name, const Mat4f & m)
{
	GLint loc = getUniformLocation(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, m);
}


void shaderProgram::setUniform( const char *name, float val )
{
	GLint loc = getUniformLocation(name);
	glUniform1f(loc, val);
}


void shaderProgram::setUniform( const char *name, double val )
{
	float _val = val;
	GLint loc = getUniformLocation(name);
	glUniform1f(loc, _val);
}


void shaderProgram::setUniform( const char *name, int val )
{
	GLint loc = getUniformLocation(name);
	glUniform1i(loc, val);
}


void shaderProgram::setUniform( const char *name, GLuint val )
{
	GLint loc = getUniformLocation(name);
	glUniform1ui(loc, val);
}


void shaderProgram::setUniform( const char *name, bool val )
{
	int loc = getUniformLocation(name);
	glUniform1i(loc, val);
}

