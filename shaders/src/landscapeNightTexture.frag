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
	vec2 sampleCoord1 = TexCoord;
	vec2 sampleCoord2 = TexCoord;
	if (fisheye > 0.5) {
		if (HorizonAltitude < 0.0) {
			// The PNG is flipped on upload: v=0 reads its bottom row.
			// Linear filtering blends the two center texels for even widths.
			vec4 day = textureLod(texunit0, vec2(0.5, 0.0), 0.0);
			vec4 night = textureLod(texunit1, vec2(0.5, 0.0), 0.0);
			// Fully transparent PNG texels may still contain RGB values; discard them.
			if (day.a == 0.0) day.rgb = vec3(0.0);
			if (night.a == 0.0) night.rgb = vec3(0.0);
			// Reuse the other texture's color when only one bottom-center texel is visible.
			// If both are transparent, both colors remain black.
			if (day.a == 0.0) day.rgb = night.rgb;
			if (night.a == 0.0) night.rgb = day.rgb;
			vec3 horizonColor = sky_brightness * day.rgb + 4.0 * (0.25 - sky_brightness) * night.rgb;
			FragColor = vec4(horizonColor, fader);
			return;
		}
		vec2 texSize1 = vec2(textureSize(texunit0, 0));
		vec2 texSize2 = vec2(textureSize(texunit1, 0));
		// Each texture gets a two-texel inset even if their sizes differ.
		float scale1 = max(0.0, 1.0 - 4.0 / min(texSize1.x, texSize1.y));
		float scale2 = max(0.0, 1.0 - 4.0 / min(texSize2.x, texSize2.y));
		sampleCoord1 = vec2(0.5) + (TexCoord - vec2(0.5)) * scale1;
		sampleCoord2 = vec2(0.5) + (TexCoord - vec2(0.5)) * scale2;
	}
	vec4 tex_color1 = vec4(texture(texunit0,sampleCoord1)).rgba;
	vec4 tex_color2 = vec4(texture(texunit1,sampleCoord2)).rgba;
	vec4 tex_color;
	tex_color.rgb = sky_brightness * tex_color1.rgb + 4*(0.25-sky_brightness)* tex_color2.rgb;
	tex_color.a = max (tex_color1.a , tex_color2.a);

	if (tex_color.a == 0.0)
		discard;

	tex_color.a = tex_color.a * fader;
	FragColor = tex_color;
}
