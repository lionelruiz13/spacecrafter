//We need to use the projection fisheye, assume to give vec3 clipping_fov
layout (binding=0) uniform uModelViewMatrix {mat4 ModelViewMatrix;};
#include <fisheye_noMV.glsl>

