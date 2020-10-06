vec3 convertToRGB(in sampler2D t_y, in sampler2D t_u, in sampler2D t_v, in vec2 coord)
{
    float y = texture(t_y, coord).r;  
    float u = texture(t_u, coord).r - 0.5;  
    float v = texture(t_v, coord).r - 0.5;  

    float r = clamp (y +             1.402 * v, 0, 1);
    float g = clamp (y - 0.344 * u - 0.714 * v, 0, 1);  
    float b = clamp (y + 1.772 * u            , 0, 1);

    return vec3(r,g,b);
}
