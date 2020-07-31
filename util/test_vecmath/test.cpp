#include "vecmath.hpp"
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/euler_angles.hpp> 
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
    float t = abs(vmTest[0]-glmTest.x) + abs(vmTest[1]-glmTest.y) + abs(vmTest[2]-glmTest.z);
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
    float t = abs(vmTest[0]-glmTest.x) + abs(vmTest[1]-glmTest.y) + abs(vmTest[2]-glmTest.z) + abs(vmTest[3]-glmTest.w);
    if (t< EPSILON)
        return true;
    else {
        std::cout << vmTest[0] << " " << vmTest[1] << " " << vmTest[2] <<  " " << vmTest[3] << std::endl;
        std::cout << glmTest.x << " " << glmTest.y << " " << glmTest.z << " " << glmTest.w << std::endl;
        return false;
    }
}

bool compareMat(Mat4f vmM, glm::mat4 glmM)
{
    bool test = true;
    for(int i=0; i< 4; i++)
        test = test && compareVec4(vmM[i],glmM[i]);
    return test;
}

int main()
{
    // grands tests des vec3
    float a1,b1,c1;
    float a2,b2,c2;
    float a3,b3,c3;
    
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
        Vec3f vmT;
        assert(compareVec3(vmT, glm::vec3(0,0,0) ));

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

    // tests sur vec4f
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


    // tests sur mat4f
    Mat4f tm;
    assert(compareMat(tm, glm::mat4(0)));

    // compare ortho
    for (int i=0; i< nbSubTests; i++) {
        a1= randf(10.f);
        a2= a1+ fabs(randf(10.f));
        b1= randf(10.f);
        b2= b1+ fabs(randf(10.f));
        c1= randf(1.f);
        c2= c1+ fabs(randf(10.f));
        Mat4f vmTest = Mat4f::ortho(a1, a2, b1, b2, c1, c2);
        glm::mat4 glmTest = glm::ortho(a1, a2, b1, b2, c1, c2);
        assert(compareMat(vmTest, glmTest));
    }

    // compare ortho2D
    for (int i=0; i< nbSubTests; i++) {
        a1= randf(10.f);
        a2= a1+ fabs(randf(10.f));
        b1= randf(10.f);
        b2= b1+ fabs(randf(10.f));
        Mat4f vmTest = Mat4f::ortho2D(a1, a2, b1, b2);
        glm::mat4 glmTest = glm::ortho(a1, a2, b1, b2);
        assert(compareMat(vmTest, glmTest));
    }

    // compare frustum
    for (int i=0; i< nbSubTests; i++) {
        a1= randf(10.f);
        a2= a1+ fabs(randf(10.f));
        b1= randf(10.f);
        b2= b1+ fabs(randf(10.f));
        c1= randf(1.f);
        c2= c1+ fabs(randf(10.f));
        Mat4f vmTest = Mat4f::frustum(a1, a2, b1, b2, c1, c2);
        glm::mat4 glmTest = glm::frustum(a1, a2, b1, b2, c1, c2);
        assert(compareMat(vmTest, glmTest));
    }

    // compare perspective
    /*
    for (int i=0; i< nbSubTests; i++) {
        a1= 0.1f + fabs(randf(359.9f));
        a2= 1.f + fabs(randf(1.f));
        c1= 0.01f+fabs(randf(1.f));
        c2= c1+ fabs(randf(10.f));
        Mat4f vmTest = Mat4f::perspective(a1, a2, c1, c2);
        glm::mat4 glmTest = glm::perspective(float(a1/180*M_PI), a2, c1, c2);
        assert(compareMat(vmTest, glmTest));
    }
    */

    // compare lookAt
    for (int i=0; i< nbSubTests; i++) {
        a1= randf(10.f);
        a2= randf(10.f);
        a3= randf(10.f);
        b1= randf(10.f);
        b2= randf(10.f);
        b3= randf(10.f);
        c1= randf(10.f);
        c2= randf(10.f);
        c3= randf(10.f);
        Mat4f vmTest = Mat4f::lookAt(a1, a2, a3, b1, b2, b3, c1, c2, c3);
        glm::mat4 glmTest = glm::lookAt(glm::vec3(a1, a2, a3), glm::vec3(b1, b2, b3), glm::vec3(c1, c2, c3) );
        assert(compareMat(vmTest, glmTest));
    }

    // compare yawPitchRoll
    for (int i=0; i< nbSubTests; i++) {
        a1= randf(90.f);
        b1= randf(90.f);
        c1= randf(90.f);
        Mat4f vmTest = Mat4f::yawPitchRoll(a1, b1, c1);
        glm::mat4 glmTest = glm::yawPitchRoll(float(a1/180*M_PI), float(b1/180*M_PI), float(c1/180*M_PI));
        assert(compareMat(vmTest, glmTest));
    }


    //vecmath
    std::cout << "vecmath" << std::endl;
    std::cout << "vec3f " << sizeof(glm::vec3) << std::endl;
    std::cout << "vec3d " << sizeof(glm::dvec3) << std::endl;
    std::cout << "vec4f " << sizeof(glm::vec4) << std::endl;
    std::cout << "vec4d " << sizeof(glm::dvec4) << std::endl;
    std::cout << "mat4f " << sizeof(glm::mat4) << std::endl;
    std::cout << "mat4d " << sizeof(glm::dmat4) << std::endl;

    //glm
    std::cout << "glm" << std::endl;
    std::cout << "fvec3 " << sizeof(Vec3f) << std::endl;
    std::cout << "dvec3 " << sizeof(Vec3d) << std::endl;
    std::cout << "fvec4 " << sizeof(Vec4f) << std::endl;
    std::cout << "dvec4 " << sizeof(Vec4d) << std::endl;
    std::cout << "fmat4 " << sizeof(Mat4f) << std::endl;
    std::cout << "dmat4 " << sizeof(Mat4d) << std::endl;
}