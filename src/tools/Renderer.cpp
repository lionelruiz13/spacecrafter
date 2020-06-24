#include "tools/Renderer.hpp"
#include "tools/shader.hpp"
#include "tools/OpenGL.hpp"


void Renderer::clearColor()
{
    glClear(GL_COLOR_BUFFER_BIT);
}


void Renderer::clearDepthBuffer()
{
    glClear(GL_DEPTH_BUFFER_BIT);
}


void Renderer::drawArrays(shaderProgram* shader, VertexArray* va, GLenum mode, unsigned int first, unsigned int count )
{
    shader->use();
	va->bind();
	glDrawArrays(mode, first, count);
	va->unBind();
	shader->unuse();
}

void Renderer::drawArraysWithoutShader(VertexArray* va, GLenum mode, unsigned int first, unsigned int count )
{
	va->bind();
	glDrawArrays(mode, first, count);
	va->unBind();
}

void Renderer::viewport(int x, int y, int w, int h)
{
	glViewport(x,y,w,h);
}



void Renderer::drawMultiArrays(shaderProgram* shader, VertexArray* va, GLenum mode, unsigned int boucle, unsigned int count )
{
    shader->use();
	va->bind();
	// glDrawArrays(mode, boucle, count);
	for (unsigned int y=0; y<boucle; y++) {
		glDrawArrays(GL_TRIANGLE_STRIP,y*count , count);
	}
	va->unBind();
	shader->unuse();
}
