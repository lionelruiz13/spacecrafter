#include "tools/Renderer.hpp"
#include "tools/shader.hpp"
#include "tools/OpenGL.hpp"


void Renderer::clearColor()
{
    glClear(GL_COLOR_BUFFER_BIT);
}


void Renderer::drawArrays(shaderProgram* shader, VertexArray* va, GLenum mode, unsigned int first, unsigned int count )
{
    shader->use();
	va->bind();
	glDrawArrays(mode, first, count);
	va->unBind();
	shader->unuse();
}