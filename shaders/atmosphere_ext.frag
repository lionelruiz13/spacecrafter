//
// atmosphere exterieure
//

#version 430 core
#pragma debug(on)
#pragma optimize(off)


// out vec4 color;

uniform sampler2D atmGradient;

// here you need to define your earth and atmosphere's radius.
uniform float planetRadius;
uniform float atmRadius;
uniform float atmAlpha;

uniform vec3 planetPos;
uniform vec3 camPos;
uniform vec3  sunPos;

in vec3 pos; //from vertex shader

out vec4 FragColor;

//~ void main(void)
//~ {
	//~ FragColor = vec4(0.0,0.0,1.0,0.4);
//~ }



void main(void)
{
    vec3 earthPos = planetPos;

    vec3 earthToPxl =  pos - earthPos;

    // if (earthToPxl == earthPos) {
    //     FragColor = vec4(0.0,1.0,0.0, 1.0);
    //     return;
    // }

   float transparency = dot(normalize(earthToPxl),normalize(-earthPos));

    FragColor = vec4(vec3(1.0,0.0,0.0)*transparency+vec3(0.0,0.0,1.0)*(-transparency),atmAlpha);
}

// {
//     vec3 earthPos = vec3(UBOData.model[3][0],
//                          UBOData.model[3][1],
//                          UBOData.model[3][2]);

//     vec3 earthToPxl =  fs_in.pos - earthPos;

//     float transparency=dot(normalize(earthToPxl),normalize(-earthPos));

//     color=vec4(vec3(0.0,1.0,1.0)*transparency+vec3(1.0,0.0,0.0)*(-transparency),0.5);
// }



/*
void main(void)
{
    // position of the pixel in the 3D space
    vec3 pxlPos = pos;

    // camera to current pixel vector
    vec3 camToPxl = pos - camPos;
    vec3 camToPxlN = normalize(camToPxl);
    vec3 sunToPxl = pos - sunPos;

    // So first step is know whether the light hit the ground or
    // join the other side of the atmosphere
    // line/sphere collision:
    //https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
    // delta = (F.(C-O))²-||C-O||²+r²
    vec3 OC = camPos - planetPos;
    float b = dot(camToPxlN,OC);
    float delta = b*b -dot(OC,OC)+ planetRadius*planetRadius;
    float deltaAtm;

    float result;
    if(delta>0) {
        // terre touchée
        // r = -(F.CO)+-sqrt((F.CO)²-CO.CO+r²)
        float deltasqrt = sqrt(delta);
        result = - b - deltasqrt;
    }
    else {
        // atmosphère touchée
        deltaAtm = delta + atmRadius*atmRadius - planetRadius*planetRadius;

        float deltasqrt = sqrt(deltaAtm);
        result =  -b + deltasqrt;
    }
    //get the intersection point
    vec3 intersect = camPos + camToPxlN*result;
    // Now we now the distance traveled by the light in the atmosphere
    vec3 atmVec= intersect - pos;
    float atmLength = length(atmVec);

    // we got enough to get the lowest point reached by our atmosphere vector
    // and get the height, so the y we need on the gradient
    vec2 gradUV;
    if(delta>0)
        gradUV.y = 0.9;
    else
        gradUV.y = (length((pos+ atmVec*0.5)-planetPos)-planetRadius)/(atmRadius-planetRadius);


    // Now we'll make some ray tracing to get an accurate shadow in the atmosphere.
    // The idea is to compute some point on the traced line equally spaced,
    // and for each to test if it is possible to reach the sun from it with a straight line
    // without encountering the earth.
    // This gives an idea of the enlightened part of our atmosphere segment
    // as a percentage (scatteredLight)
    
    #define NBPOINTS 20
    #define NBPOINTSINV 0.05
    vec3 point;
    OC = sunPos - planetPos;
    float scatteredLight=0.0;
    vec3 sunToPoint [NBPOINTS];
    vec4 sunColor = vec4(1.0,1.0,0.3,1.0);

    // we compute the first point separately
    sunToPoint[0] = normalize(pos - sunPos);
    b = dot(sunToPoint[0],OC);
    delta = b*b -dot(OC,OC)+ planetRadius*planetRadius;
    if(delta<0.0)
    {
        scatteredLight+=NBPOINTSINV*atmAlpha;
        if(dot(pos-planetPos,sunPos-planetPos)>0)
            scatteredLight+=NBPOINTSINV*atmAlpha;
    }

    for(int i=1;i<NBPOINTS;i++)
    {
        float floati = i;
        point = pos+ atmVec*NBPOINTSINV*floati;

        // we first test if the point face the sun to improve performances
        if(dot(point-planetPos,sunPos-planetPos)>0)
            scatteredLight+=NBPOINTSINV*atmAlpha;
        else
        {
            // in the other case we need to test the collision with the earth
            sunToPoint[i] = normalize(point - sunPos);
            b = dot(sunToPoint[i],OC);
            delta = b*b -dot(OC,OC)+ planetRadius*planetRadius;
            if(delta<0.0) {
                scatteredLight+=NBPOINTSINV*atmAlpha;
            }
        }
    }

    // Now to compute x for the gradient, we need some angles
    // and then copy this:
    // http://petrocket.blogspot.fr/2010/04/atmosphere-shader-update-and-treegrass.html
    float StretchAmount = 0.5;
    float ln,lnn,lc;
    ln  = dot(-sunToPoint[0], normalize(pos - planetPos));
    lnn = dot(-sunToPoint[0], -camToPxlN);
    lc  = dot(-sunToPoint[0], normalize(camPos - planetPos));

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

    FragColor = skyColor;
}
*/





