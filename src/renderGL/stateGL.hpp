#ifndef STATE_GL_HPP
#define STATE_GL_HPP


#include <iostream>

#include <GL/glew.h>

class StateGL {
public:
	static void enable(GLenum gl_enum) {
		EnableDisable(gl_enum, true);
	};

	static void disable(GLenum gl_enum) {
		EnableDisable(gl_enum, false);
	};

	static void BlendFunc(GLenum gl_enum1, GLenum gl_enum2);
private:
	static void EnableDisable(GLenum gl_enum, bool value);
	static bool cull_face;
	static bool stencil_test;
	static bool depth_test;
	static bool blend;
	static GLenum blendFunc1, blendFunc2;
};

#endif //STATE_GL_HPP
