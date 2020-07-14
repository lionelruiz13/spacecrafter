#include <iostream>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <initializer_list>

#include "tools/shader.hpp"
#include "tools/stateGL.hpp"
#include "tools/log.hpp"

std::string shaderProgram::shaderDir = "./";
GLuint shaderProgram::currentProgram= 0;

static std::string fixProgramName(const std::string &vs) 
{
	size_t lastdot = vs.find_last_of(".");
    if (lastdot == std::string::npos)
		return vs;
	else
    	return vs.substr(0, lastdot); 
}


shaderProgram::shaderProgram()
{
	cLog::get()->mark(LOG_FILE::SHADER);
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
	subroutineLocations.clear();
	glDeleteProgram(program);
}


void shaderProgram::debugShader( GLuint shader, const std::string &str)
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
			std::cout<<"impossible d'allouer de la memoire !\n"<<std::endl;
			exit(EXIT_FAILURE);
		}
		memset(log, '\0', logsize + 1);
		glGetShaderInfoLog(shader, logsize, &logsize, log);
		std::ostringstream out;
		out << "unable to compile shader : "<< shader << " " << str << std::endl << log;
		cLog::get()->write(out.str(), LOG_TYPE::L_ERROR, LOG_FILE::SHADER);
		free(log);
		glDeleteShader(shader);
		exit(EXIT_FAILURE);
	}
	else {
		std::ostringstream out;
		out << "shader "<< shader << " name " << str << " compiling succes";
		cLog::get()->write(out.str(), LOG_TYPE::L_INFO, LOG_FILE::SHADER);
	}
}


void shaderProgram::debugProgram()
{
	GLint isLinked = 0;
	char* log= nullptr;
	glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		// on recupere la taille du message d'erreur
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		// on alloue un espace memoire dans lequel OpenGL ecrira le message
		log = (char*)malloc(maxLength + 1);
		if(log == nullptr) {
			std::cout<<"impossible d'allouer de la memoire !\n"<<std::endl;
			exit(EXIT_FAILURE);
		}
		memset(log, '\0', maxLength + 1);
		glGetProgramInfoLog(program, maxLength, &maxLength, log);
		std::ostringstream out;
		out << "unable to compile program : "<< program << " " << programName << std::endl << log;
		cLog::get()->write(out.str(), LOG_TYPE::L_ERROR, LOG_FILE::SHADER);
		free(log);
		// The program is useless now. So delete it.
		glDeleteProgram(program);
		// Provide the infolog in whatever manner you deem best.
		// Exit with failure.
		return;
	}
	else {
		std::ostringstream out;
		out << "program : "<< program << " name " << programName << " compiling succes";
		cLog::get()->write(out.str(), LOG_TYPE::L_INFO, LOG_FILE::SHADER);	
	}
}


std::string shaderProgram::getTypeString( GLenum type )
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

	std::ostringstream out;
	out << programName << " - " << program << " :  Active attributes";
	cLog::get()->write(out.str(), LOG_TYPE::L_OTHER, LOG_FILE::SHADER);

	for( int i = 0; i < numAttribs; ++i ) {
		GLint results[3];
		glGetProgramResourceiv(program, GL_PROGRAM_INPUT, i, 3, properties, 3, NULL, results);

		GLint nameBufSize = results[0] + 1;
		char * name = new char[nameBufSize];
		glGetProgramResourceName(program, GL_PROGRAM_INPUT, i, nameBufSize, NULL, name);
		{
			std::ostringstream out;
			out << "\t" << results[2] << " - " << name << " : " << getTypeString(results[1]);
			cLog::get()->write(out.str(), LOG_TYPE::L_OTHER, LOG_FILE::SHADER);
		}
		delete [] name;
	}
}


void shaderProgram::printActiveUniforms()
{
	GLint numUniforms = 0;
	glGetProgramInterfaceiv( program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

	GLenum properties[] = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX};

	std::ostringstream out;
	out << programName << " - " << program << " :  Active uniforms";
	cLog::get()->write(out.str(), LOG_TYPE::L_OTHER, LOG_FILE::SHADER);

	for( int i = 0; i < numUniforms; ++i ) {
		GLint results[4];
		glGetProgramResourceiv(program, GL_UNIFORM, i, 4, properties, 4, NULL, results);

		if( results[3] != -1 ) continue;  // Skip uniforms in blocks
		GLint nameBufSize = results[0] + 1;
		char * name = new char[nameBufSize];
		glGetProgramResourceName(program, GL_UNIFORM, i, nameBufSize, NULL, name);
		{
			std::ostringstream out;
			out << "\t" << results[2] << " - " << name << " : " << getTypeString(results[1]);
			cLog::get()->write(out.str(), LOG_TYPE::L_OTHER, LOG_FILE::SHADER);
		}
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
		{
			std::ostringstream out;
			out << program << " uniform block " << blockName;
			cLog::get()->write(out.str(), LOG_TYPE::L_OTHER, LOG_FILE::SHADER);
		}
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
			{
				std::ostringstream out;
				out << "\t" << name << " (" << getTypeString(results[1]) << ")";
				cLog::get()->write(out.str(), LOG_TYPE::L_OTHER, LOG_FILE::SHADER);
			}
			delete [] name;
		}

		delete [] unifIndexes;
	}
}


std::string shaderProgram::loadFileToString(const std::string& fname)
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
		std::ostringstream out;
		out << "Error opening file " << fname ;
		cLog::get()->write(out.str(), LOG_TYPE::L_ERROR, LOG_FILE::SHADER);
		exit(EXIT_FAILURE);
	}
	return filetext;
}


GLuint shaderProgram::makeShader(const std::string &str, GLenum ShaderType)
{
	std::string fullStr = shaderDir + str;
	std::string shader_string = loadFileToString(fullStr.c_str());

	GLuint shader = glCreateShader(ShaderType);
	if(shader == 0) {
		std::ostringstream out;
		out << "impossible de creer le shader " << fullStr.c_str() ;
		cLog::get()->write(out.str(), LOG_TYPE::L_ERROR, LOG_FILE::SHADER);
		exit(EXIT_FAILURE);
	}

	GLchar const *shader_source = shader_string.c_str();
	GLint const shader_length = shader_string.size();

	glShaderSource(shader, 1, &shader_source, &shader_length);
	glCompileShader(shader);
	debugShader(shader, str);

	return shader;
}


void shaderProgram::init(const std::string &vs, const std::string &tcs, const std::string &tes, const std::string &gs, const std::string &fs)
{
	programName = fixProgramName(vs);

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

void shaderProgram::init(const std::string &vs, const std::string &gs, const std::string &fs)
{
	programName = fixProgramName(vs);

	GLuint _vs =  makeShader(vs, GL_VERTEX_SHADER);
	GLuint _gs;

	if (gs != "")
		_gs =  makeShader(gs, GL_GEOMETRY_SHADER);
	else
		_gs = NOSHADER;

	GLuint _fs =  makeShader(fs, GL_FRAGMENT_SHADER);

	init(_vs, NOSHADER, NOSHADER, _gs, _fs);
}

void shaderProgram::init(const std::string &vs, const std::string &fs)
{
	programName = fixProgramName(vs);

	GLuint _vs =  makeShader(vs, GL_VERTEX_SHADER);
	GLuint _fs =  makeShader(fs, GL_FRAGMENT_SHADER);

	init(_vs, NOSHADER, NOSHADER, NOSHADER, _fs);
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
		cLog::get()->write("Error: couldn't create a new shader program handle", LOG_TYPE::L_ERROR, LOG_FILE::SHADER);		
		exit(EXIT_FAILURE);
	}

	if(vs !=NOSHADER) glAttachShader(program,vshader);
	if(tcs!=NOSHADER) glAttachShader(program,tcshader);
	if(tes!=NOSHADER) glAttachShader(program,teshader);
	if(gs !=NOSHADER) glAttachShader(program,gshader);
	if(fs !=NOSHADER) glAttachShader(program,fshader);

	glLinkProgram(program);
	debugProgram();
	printActiveUniforms();
	printActiveUniformBlocks();
	printActiveAttribs();
}


void shaderProgram::use()
{
	if (program!=currentProgram) {
		glUseProgram(program);
		currentProgram = program;
	}
}

void shaderProgram::unuse()
{
	glUseProgram(0);
	currentProgram = 0;
}


void shaderProgram::setUniformLocation(std::initializer_list<const std::string> list)
{
    for(const auto elem : list )
    	this->setUniformLocation(elem);
}

void shaderProgram::setUniformLocation(const std::string&  name )
{
	std::map<std::string, GLuint>::iterator pos;
	pos = uniformLocations.find(name);

	if( pos == uniformLocations.end() ) {
		uniformLocations[name] = glGetUniformLocation(program, name.c_str());
	} else {
		std::ostringstream out;
		out << program << " :  setUniformLocation name " << name << " already found.";
		cLog::get()->write(out.str(), LOG_TYPE::L_WARNING, LOG_FILE::SHADER);
	}
}


int shaderProgram::getUniformLocation(const std::string&  name )
{
	std::map<std::string, GLuint>::iterator pos;
	pos = uniformLocations.find(name);

	if( pos == uniformLocations.end() ) {
		std::ostringstream out;
		out << program << " : error with " << name << " atribution uniformLocations";
		cLog::get()->write(out.str(), LOG_TYPE::L_WARNING, LOG_FILE::SHADER);
		return 0;
	}
	return uniformLocations[name];
}

void shaderProgram::setUniform( const std::string& name, const Vec3f & v)
{
	GLint loc = getUniformLocation(name);
	glUniform3fv(loc,1,v);
}

void shaderProgram::setUniform( const std::string& name, const Vec4f & v)
{
	GLint loc = getUniformLocation(name);
	glUniform4fv(loc,1, v);
}

void shaderProgram::setUniform( const std::string& name, const Vec2f & v)
{
	GLint loc = getUniformLocation(name);
	glUniform2fv(loc,1, v);
}

void shaderProgram::setUniform( const std::string& name, const Vec3i & v)
{
	GLint loc = getUniformLocation(name);
	glUniform3iv(loc,1,v);
}

void shaderProgram::setUniform( const std::string& name, const Vec4i & v)
{
	GLint loc = getUniformLocation(name);
	glUniform4iv(loc,1, v);
}

void shaderProgram::setUniform( const std::string& name, const Vec2i & v)
{
	GLint loc = getUniformLocation(name);
	glUniform2iv(loc,1, v);
}

void shaderProgram::setUniform( const std::string& name, const Mat4f & m)
{
	GLint loc = getUniformLocation(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, m);
}


void shaderProgram::setUniform( const std::string& name, float val )
{
	GLint loc = getUniformLocation(name);
	glUniform1f(loc, val);
}


void shaderProgram::setUniform( const std::string& name, double val )
{
	float _val = val;
	GLint loc = getUniformLocation(name);
	glUniform1f(loc, _val);
}


void shaderProgram::setUniform( const std::string& name, int val )
{
	GLint loc = getUniformLocation(name);
	glUniform1i(loc, val);
}


void shaderProgram::setUniform( const std::string& name, GLuint val )
{
	GLint loc = getUniformLocation(name);
	glUniform1ui(loc, val);
}


void shaderProgram::setUniform( const std::string& name, bool val )
{
	int loc = getUniformLocation(name);
	glUniform1i(loc, val);
}


void shaderProgram::setSubroutineLocation(GLenum ShaderType, const std::string& name)
{
	std::map<std::string, GLuint>::iterator pos;
	pos = subroutineLocations.find(name);

	if( pos == subroutineLocations.end() ) {
		subroutineLocations[name] = glGetSubroutineIndex(program, ShaderType, name.c_str());
	} else {
		std::ostringstream out;
		out << program << " : subroutineLocations name " << name << " already found ";
		cLog::get()->write(out.str(), LOG_TYPE::L_WARNING, LOG_FILE::SHADER);
		assert(false);
	}
}

void shaderProgram::setSubroutine(GLenum ShaderType, const std::string& name)
{
	std::map<std::string, GLuint>::iterator pos;
	pos = subroutineLocations.find(name);

	if( pos == subroutineLocations.end() ) {
		std::ostringstream out;
		out << program << " : error with " << name << " atribution SubroutineLocations";
		cLog::get()->write(out.str(), LOG_TYPE::L_WARNING, LOG_FILE::SHADER);
		assert(false);
		return ;
	}
	GLuint sub = subroutineLocations[name];
	glUniformSubroutinesuiv(ShaderType, 1, &sub);
}
