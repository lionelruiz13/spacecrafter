#include "vecmath.hpp"
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <assert.h>

const double EPSILON = 0.001f;
int nbSubTests = 5000;

float randf(float a) {
    return ((float)rand()/(float)(RAND_MAX)) * 2*a-a;
}

bool compareFloat(float vmTest, float glmTest)
{
    float t = abs(vmTest -glmTest);
    if (t< EPSILON)
        return true;
    else {
        std::cout << vmTest << " " << glmTest << " " << vmTest -  glmTest << std::endl;
        return false;
    }
}

bool compareVec3(Vec3f vmTest, glm::vec3 glmTest)
{
    float t = abs(vmTest[0]-glmTest.x) + abs(vmTest[1]-glmTest.y) +abs(vmTest[2]-glmTest.z);
    if (t< EPSILON)
        return true;
    else {
        std::cout << vmTest[0] << " " << vmTest[1] << " " << vmTest[2] << std::endl;
        std::cout << glmTest.x << " " << glmTest.y << " " << glmTest.z << std::endl;
        return false;
    }
}

bool compareVec4(Vec4f vmTest, glm::vec4 glmTest)
{
    float t = abs(vmTest[0]-glmTest.x) + abs(vmTest[1]-glmTest.y) +abs(vmTest[2]-glmTest.z) +abs(vmTest[3]-glmTest.w);
    if (t< EPSILON)
        return true;
    else {
        std::cout << vmTest[0] << " " << vmTest[1] << " " << vmTest[2] <<  " " << vmTest[3] << std::endl;
        std::cout << glmTest.x << " " << glmTest.y << " " << glmTest.z << " " << glmTest.w << std::endl;
        return false;
    }
}


int main()
{
    // grands tests des vec3
    float a1,b1,c1;
    float a2,b2,c2;
    
    for (int i=0; i< nbSubTests; i++) {
        a1 = randf(10.f);
        b1 = randf(10.f);
        c1 = randf(10.f);
        //std::cout << a1 << " " << b1 << " " << c1 << std::endl;
        a2 = randf(10.f);
        b2 = randf(10.f);
        c2 = randf(10.f);
        //std::cout << a2 << " " << b2 << " " << c2 << std::endl;

        //initialisation
        Vec3f vm1{a1, b1, c1};
        glm::vec3 glmv1{a1, b1, c1};
        // égalité
        assert(compareVec3(vm1, glmv1));

        Vec3f vm2{a2, b2, c2};
        glm::vec3 glmv2{a2, b2, c2};
        assert(compareVec3(vm2, glmv2));

        //sum 
        Vec3f vm3 = vm1*2.f + vm2;
        glm::vec3 glmv3 = glmv1*2.f + glmv2;
        assert(compareVec3(vm3, glmv3));

        // length
        assert(compareFloat(vm1.length() , glm::length(glmv1)));
        assert(compareFloat(vm1.lengthSquared() , glm::length2(glmv1)));

        // dot    
        float vmDot = vm1.dot(vm2);
        float glmDot = glm::dot(glmv1, glmv2);
        assert(compareFloat(vmDot, glmDot));

        //cross
        Vec3f vmCross = vm1^vm2;
        glm::vec3 glmCross = glm::cross(glmv1, glmv2);
        assert(compareVec3(vmCross, glmCross));

        //angle
        float vmAngle = vm1.angle(vm2);
        float glmAngle = glm::angle(glmv1, glmv2);
        assert(compareFloat(vmDot, glmDot));

        // normalize
        glmv1 = glm::normalize(glmv1);
        vm1.normalize();
        assert(compareVec3(vm1, glmv1));
    }

    float d1, d2;

    // tests sur vec4
     for (int i=0; i< nbSubTests; i++) {
        a1 = randf(10.f);
        b1 = randf(10.f);
        c1 = randf(10.f);
        if (randf(10.f)<0) d1=0.f; 
        else d1=1.f;
        //std::cout << a1 << " " << b1 << " " << c1 << std::endl;
        a2 = randf(10.f);
        b2 = randf(10.f);
        c2 = randf(10.f);
        if (randf(10.f)<0) d2=0.f; 
        else d2=1.f;
        //std::cout << a2 << " " << b2 << " " << c2 << std::endl;

        //initialisation
        Vec4f vm1{a1, b1, c1, d1};
        glm::vec4 glmv1{a1, b1, c1, d1};
        // égalité
        assert(compareVec4(vm1, glmv1));

        Vec4f vm2{a2, b2, c2, d2};
        glm::vec4 glmv2{a2, b2, c2, d2};
        assert(compareVec4(vm2, glmv2));

        //sum 
        Vec4f vm3 = vm1*2.f + vm2;
        glm::vec4 glmv3 = glmv1*2.f + glmv2;
        assert(compareVec4(vm3, glmv3));

        // length
        assert(compareFloat(vm1.length() , glm::length(glmv1)));
        assert(compareFloat(vm1.lengthSquared() , glm::length2(glmv1)));

        // dot    
        float vmDot = vm1.dot(vm2);
        float glmDot = glm::dot(glmv1, glmv2);
        assert(compareFloat(vmDot, glmDot));

        //cross
        Vec4f vmCross = vm1.cross(vm2);
        glm::vec4 glmCross = glm::vec4( glm::cross( glm::vec3( glmv1), glm::vec3( glmv2) ), 1 );
        assert(compareVec4(vmCross, glmCross));

        // normalize
        glmv1 = glm::normalize(glmv1);
        vm1.normalize();
        assert(compareVec4(vm1, glmv1));
    }

    
}