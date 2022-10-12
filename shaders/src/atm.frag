#version 430 core

layout (location=0) in vec3 pos;
layout (location=0) out vec4 color;

// All positions are relative to the camera (camPos is at origin) - without projection
layout (binding=0) uniform ubo {
    mat4 ModelViewMatrix;
    vec3 sunPos;
    float planetRadius; // 1.0  ;
    vec3 bodyPos;
    float planetOneMinusOblateness;
    vec3 clipping_fov;
    float atmRadius;    // 1.05 ;
    ivec2 TesParam;     // [min_tes_lvl, max_tes_lvl]
    float atmAlpha;     // 1.0  ; // this value is a scale for atmosphere transparency
};

layout (binding=1) uniform sampler2D atmGradient;

void main(void)
{
    // Normalized ray from the camera to the atmosphere entrypoint
    vec3 ray = normalize(pos);

    // may be used to make a sun throug atmosphere
    //vec3 sunCamPos   = (UBOData.view*vec4(0.0,0.0,0.0,1.0)).xyz;

    // Body position (not eye coordinnates)

    vec3 sunToPxl = pos - sunPos;


    // So first step is know whether the light hit the ground or
    // join the other side of the atmosphere
    // line/sphere collision:
    //https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
    // delta = (F.(C-O))²-||C-O||²+r²
    vec3 OC = -bodyPos;
    float b = dot(ray,OC);
    float delta = b*b -dot(OC,OC)+ planetRadius*planetRadius;
    float deltaAtm;

    float result;
    if (delta>0) {
        // terre touchée
        // r = -(F.CO)+-sqrt((F.CO)²-CO.CO+r²)
        float deltasqrt = sqrt(delta);
        result = - b - deltasqrt;

    } else {
        // atmosphère touchée
        deltaAtm = delta + atmRadius*atmRadius - planetRadius*planetRadius;

        float deltasqrt = sqrt(deltaAtm);
        result =  -b + deltasqrt;

    }
    // get the intersection point
    vec3 intersect = ray*result;
    // Now we now the distance traveled by the light in the atmosphere
    vec3 atmVec= intersect - pos;
    float atmLength = length(atmVec);

    // we got enough to get the lowest point reached by our atmosphere vector
    // and get the height, so the y we need on the gradient
    vec2 gradUV;
    if (delta>0)
        gradUV.y = 0.9;
    else
        gradUV.y = (length((pos+ atmVec*0.5)-bodyPos)-planetRadius)/(atmRadius-planetRadius);


    // Now we'll make some ray tracing to get an accurate shadow in the atmosphere.
    // The idea is to compute some point on the traced line equally spaced,
    // and for each to test if it is possible to reach the sun from it with a straight line
    // without encountering the earth.
    // This gives an idea of the enlightened part of our atmosphere segment
    // as a percentage (scatteredLight)
    //
    #define NBPOINTS 20
    #define NBPOINTSINV 0.05
    vec3 point;
    OC = sunPos - bodyPos;
    float scatteredLight=0.0;
    vec3 sunToPoint [NBPOINTS];
    vec4 sunColor = vec4(1.0,1.0,0.3,1.0);
    //float sunAmount = 0.0; // this is in case we nees a sun

    // we compute the first point separately
    sunToPoint[0] = normalize(pos - sunPos);
    b = dot(sunToPoint[0],OC);
    delta = b*b -dot(OC,OC)+ planetRadius*planetRadius;
    if (delta<0.0) {
        scatteredLight+=NBPOINTSINV*atmAlpha;
        if (dot(pos-bodyPos,sunPos-bodyPos)>0)
            scatteredLight+=NBPOINTSINV*atmAlpha;
    }

    for (int i=1; i<NBPOINTS; i++) {
        float floati = i;
        point = pos+ atmVec*NBPOINTSINV*floati;

        // we first test if the point face the sun to improve performances
        if (dot(point-bodyPos,sunPos-bodyPos)>0) {
            scatteredLight+=NBPOINTSINV*atmAlpha;
        } else {
            // in the other case we need to test the collision with the earth
            sunToPoint[i] = normalize(point - sunPos);
            b = dot(sunToPoint[i],OC);
            delta = b*b -dot(OC,OC)+ planetRadius*planetRadius;
            if (delta<0.0) {
                scatteredLight+=NBPOINTSINV*atmAlpha;

                // the following computes may be used to draw a sun with refraction, but this methods is bad
                // (depend on the distance between the camera and the sun, no high dynamic range scattering)

                /*vec3 newPxlPos = point + (bodyPos-point)*0.05;
                vec3 pxlToSun = normalize(sunPos - newPxlPos);
                vec3 pxlToCam = normalize(camPos - newPxlPos);
                vec3 pxlCross = cross(pxlToSun,pxlToCam);
                //float pxlSin = pxlCross.x + pxlCross.y + pxlCross.z;
                float pxlSin = clamp(-dot(pxlToSun,pxlToCam),0.0,1.0);
                pxlSin *= pxlSin;
                pxlSin *= pxlSin;
                pxlSin *= pxlSin;
                pxlSin *= pxlSin;
                pxlSin *= pxlSin;
                pxlSin *= pxlSin;
                pxlSin *= pxlSin;
                pxlSin *= pxlSin;
                pxlSin *= pxlSin;
                pxlSin *= pxlSin;
                pxlSin *= pxlSin;
                pxlSin *= pxlSin;
                pxlSin *= pxlSin;
                sunAmount= pxlSin;*/
            }
        }
    }

    // Now to compute x for the gradient, we need some angles
    // and then copy this:
    // http://petrocket.blogspot.fr/2010/04/atmosphere-shader-update-and-treegrass.html
    float StretchAmount = 0.5;
    float ln,lnn,lc;
    ln  = -dot(sunToPoint[0], normalize(pos - bodyPos));
    //lnn = dot(-sunToPoint[0], normalize(camPos - fs_in.pos));
    lnn = -dot(sunToPoint[0], -ray);
    lc  = -dot(sunToPoint[0], normalize(-bodyPos));

    ln  = max(0,ln  + StretchAmount);
    lnn = max(0,lnn + StretchAmount);


    // using clamp tu force values to belong to the texture, but it doesn't
    // work when they reach borders, so it's [0.1,0.9] instead of [0.0,1.0]
    gradUV.x = clamp(ln + (lnn * lc),0.1,0.9) * clamp(lc + 1.0 + StretchAmount,0.0,1.0);
    gradUV.y = clamp(gradUV.y,0.0,0.9);


    // retrieving the color from the gradient
    vec4 skyColor = texture(atmGradient,gradUV);
    // and adding our alpha value as
    // % of the segment in the atmosphere enlightened *
    // length of this segment * a custom value to get the result closer to [0.1]
    // because the length of this segment depends of your sphere's rayons.
    skyColor.a = scatteredLight*(atmLength*1.5);

    color = skyColor /* * (1.0-sunAmount) + sunColor*sunAmount*/;
}
