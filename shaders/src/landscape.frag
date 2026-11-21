//
// landscape
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0, set=1) uniform sampler2D texunit0;
layout (binding=1, set=1) uniform sampler2D texunit1;

//entrée
layout (binding=3, set=1) uniform uFrag {
	float sky_brightness;
	float fader;
	float fisheye;
	float padding;
};
//uniform bool haveNightTex;

//sortie
layout (location=0) out vec4 FragColor;

// global
//vec4 tex_color;

// structure
layout (location=0) in vec2 TexCoord;
layout (location=1) in float HorizonAltitude;

void main(void)
{
	vec2 sampleCoord = TexCoord;
	if (fisheye > 0.5) {
		if (HorizonAltitude < 0.0) {
			// The PNG is flipped on upload: v=0 reads its bottom row.
			// Linear filtering blends the two center texels for even widths.
			vec4 southColor = textureLod(texunit0, vec2(0.5, 0.0), 0.0);
			// Fully transparent PNG texels may still contain RGB values; discard them.
			if (southColor.a == 0.0) southColor.rgb = vec3(0.0);
			FragColor = vec4(sky_brightness * southColor.rgb, fader);
			return;
		}
		vec2 texSize = vec2(textureSize(texunit0, 0));
		// Sample two texels inside the circular edge, independent of screen zoom.
		float scale = max(0.0, 1.0 - 4.0 / min(texSize.x, texSize.y));
		sampleCoord = vec2(0.5) + (TexCoord - vec2(0.5)) * scale;
	}
	vec4 tex_color = vec4(texture(texunit0,sampleCoord)).rgba;
	tex_color.rgb = sky_brightness * tex_color.rgb;

	if (tex_color.a == 0.0)
		discard;

	tex_color.a = tex_color.a * fader;
	FragColor = tex_color;
}
