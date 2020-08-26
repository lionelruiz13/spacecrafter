vec3 convertToRGB(sampler2D t_y, sampler2D t_u, sampler2D t_v, vec2 coord)
{
	float y = texture2D(t_y, coord).r;  
    float u = texture2D(t_u, coord).r - 0.5;  
    float v = texture2D(t_v, coord).r - 0.5;  

    float r = clamp (y +             1.402 * v, 0, 1);
    float g = clamp (y - 0.344 * u - 0.714 * v, 0, 1);  
    float b = clamp (y + 1.772 * u            , 0, 1);

    return vec3(r,g,b);
}