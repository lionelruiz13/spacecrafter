#include "tools/Renderer.hpp"
#include "tools/shader.hpp"
#include "tools/OpenGL.hpp"


void Renderer::clearColor()
{
    glClear(GL_COLOR_BUFFER_BIT);
}


void Renderer::drawArrays(shaderProgram* shader, VertexArray* va, unsigned int first, unsigned int count )
{
    shader->use();
	va->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, first, count);
	va->unBind();
	shader->unuse();
}