#include "renderGL/stateGL.hpp"

bool StateGL::cull_face = false;
bool StateGL::stencil_test = false;
bool StateGL::depth_test = false;
bool StateGL::blend = false;
GLenum StateGL::blendFunc1 = GL_ZERO;
GLenum StateGL::blendFunc2 = GL_ZERO;


void StateGL::EnableDisable(GLenum gl_enum, bool value)
{
	switch (gl_enum) {
		case GL_CULL_FACE :
			if (value == cull_face) //{ printf("op\n"); return;}
				return;
			cull_face = value;
			value ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
			break;

		case GL_STENCIL_TEST :
			if (value == stencil_test) // { printf("op\n"); return;}
				return;
			stencil_test = value;
			value ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);
			break;

		case GL_DEPTH_TEST :
			if (value == depth_test) //{ printf("op\n"); return;}
				return;
			depth_test = value;
			value ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
			break;

		case GL_BLEND :
			if (value == blend) //{ printf("op\n"); return;}
				return;
			blend = value;
			value ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
			break;

		default:
			break;
	}
}

void StateGL::BlendFunc(GLenum gl_enum1, GLenum gl_enum2)
{
	if ((gl_enum1 == blendFunc1) && (gl_enum2 == blendFunc2)) {
		//~ printf("mise en cache BlendFunc\n");
		return;
	}
	else {
		//~ printf("changement BlendFunc\n");
		blendFunc1 = gl_enum1;
		blendFunc2 = gl_enum2;
		glBlendFunc(gl_enum1, gl_enum2);
	}
}

void StateGL::bindTexture2D(unsigned int texNumber,GLuint texRef)
{
	glActiveTexture(GL_TEXTURE0+texNumber);
	glBindTexture(GL_TEXTURE_2D, texRef);
}