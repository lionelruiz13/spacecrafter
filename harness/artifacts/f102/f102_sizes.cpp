// F102 - the uniform-pool arithmetic, MEASURED rather than computed by hand.
// Prints sizeof() for every struct a persistent acquireBuffer() on
// context.uniformMgr asks for, and the size BufferMgr::acquireBuffer actually
// carves out of the 1 MiB pool: ((size-1)/A + 1)*A with A =
// minUniformBufferOffsetAlignment (BufferMgr.cpp:39-41 <- VulkanMgr.cpp:82).
//
// Build (from claude/harness/artifacts/f102):
//   g++ -O0 -std=c++20 -I../../../../src -I../../../../src/EntityCore \
//       -o <outdir>/f102_sizes f102_sizes.cpp
// Run: ./f102_sizes [alignment] [--tsv]     (alignment default 64)
//
// It links nothing. bodyShaderInterface.hpp is included for real; the few
// declarations that live in headers pulling the whole engine in are COPIED
// verbatim below with their file:line, and f102_pool.py re-greps each copy
// against its header before it uses a number (I2: the header is the authority,
// this file is a checked mirror).
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string>
#include "tools/vecmath.hpp"
#include "experimentalModule/meshModules/bodyShaderInterface.hpp"

typedef Vec3f vec3;
typedef Mat4f mat4;

// ojm_mgr.hpp:92-95 - OjmContainer::uniformData (the in-galaxy OJM body)
struct ojmContainerUniformData { Mat4f ModelViewMatrix; Mat4f NormalMatrix; };
// AtmExtModule.hpp:76-86
struct atmExtUBO {
    Mat4f ModelViewMatrix; Vec3f sunPos; float planetRadius; Vec3f bodyPos;
    float planetOneMinusOblateness; Vec3f clipping_fov; float atmRadius;
    Vec2i TesParam; float atmAlpha;
};
// OortModule.hpp:80
struct oortFrag { Vec3f color; float fader; };
// ring.hpp:113-123 - Ring::RingUniform (old path, per ringed body)
struct RingUniform {
    Mat4f ModelViewMatrix; Mat4f ModelViewMatrixInverse; Vec3f clipping_fov;
    float RingScale; Vec3f PlanetPosition; float PlanetRadius;
    Vec3f LightDirection; float SunnySideUp; float fadingFactor;
};
// bodyShader.hpp:206-214 (globalFrag's moon sibling), :216-230
struct moonFrag { vec3 MoonPosition1; float MoonRadius1; vec3 UmbraColor; float SunHalfAngle; };
struct artGeom { mat4 ModelViewMatrix; vec3 clipping_fov; };
struct artVert { float normal[12]; float radius; };
struct LightInfo { vec3 Position; float fixAlignment; vec3 Intensity; };
// bodyShader.hpp:232-241, :251-254, :256-269 (ShadowFrag is a flexible-array
// tail: the acquire asks sizeof(ShadowFrag) + sizeof(UShadowingBody)*maxShadowCast)
struct ShadowVert { mat4 ModelViewMatrix; float WorldToModelMatrix[12]; float zNear, zRange, fov, radius; };
struct UShadowingBody { vec3 posRadius; int idx; };
struct ShadowFragHead {
    float ShadowMatrix[12]; vec3 lightDirection; float sinSunAngle;
    float heightMapDepthLevel, heightMapDepth, squaredHeightMapDepthLevel;
    float sunDeviation, atmDeviation; vec3 _padding; vec3 atmColor; int nbShadowingBodies;
};

static int A = 64;
static bool tsv = false;
static unsigned rounded(unsigned s) { return ((s - 1) / A + 1) * A; }

static void row(const char *key, const char *site, unsigned size)
{
    if (tsv) printf("%s\t%u\t%u\n", key, size, rounded(size));
    else printf("  %-26s %-38s %6u %6u\n", key, site, size, rounded(size));
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--tsv") tsv = true;
        else A = atoi(argv[i]);
    }
    if (!tsv) {
        printf("alignment (minUniformBufferOffsetAlignment) = %d\n", A);
        printf("pool bufferBlocSize (app.cpp:274)           = %d\n", 1 * 1024 * 1024);
        printf("  %-26s %-38s %6s %6s\n", "struct", "acquire site", "sizeof", "carved");
    } else {
        printf("ALIGNMENT\t%d\t%d\n", A, A);
        printf("POOL\t%d\t%d\n", 1 * 1024 * 1024, 1 * 1024 * 1024);
    }
    // ---- NEW path, per body, eager at load (ModularSystem::loadBody 1458) ---
    row("globalVertProj", "BasicMesh.cpp:20 / LayeredMesh.cpp:45 vert", sizeof(globalVertProj));
    row("meshFrag", "BasicMesh.cpp:20 / LayeredMesh.cpp:45 frag", sizeof(meshFrag));
    row("meshTescGeom", "LayeredMesh.cpp:53 tescGeom (tessellated)", sizeof(meshTescGeom));
    row("rayMarchVert", "LayeredMesh.cpp:64 rayVert (rayCapable)", sizeof(rayMarchVert));
    row("rayMarchFrag", "LayeredMesh.cpp:65 rayFrag (rayCapable)", sizeof(rayMarchFrag));
    row("bodyRingVert", "RingModule.cpp:74 uVert", sizeof(bodyRingVert));
    row("bodyRingFrag", "RingModule.cpp:75 uFrag", sizeof(bodyRingFrag));
    row("atmExtUBO", "AtmExtModule.cpp:73 uniform", sizeof(atmExtUBO));
    row("ojmVert", "OjmModule.cpp:97 uVert", sizeof(ojmVert));
    row("ojmGeom", "OjmModule.cpp:97 uGeom", sizeof(ojmGeom));
    row("ojmLight", "OjmModule.cpp:98 uLight", sizeof(ojmLight));
    row("ojmShadowBlock", "OjmModule.cpp:98 uShadow", sizeof(ojmShadowBlock));
    row("oortMat", "OortModule.cpp:76 uMat (Mat4f)", sizeof(Mat4f));
    row("oortFrag", "OortModule.cpp:77 uFrag", sizeof(oortFrag));
    // ---- OLD path, per body ------------------------------------------------
    row("globalFrag", "body_*.cpp uGlobalFrag", sizeof(globalFrag));
    row("moonFrag", "body_moon.cpp:257 uMoonFrag (heightmap row)", sizeof(moonFrag));
    row("Vec3f", "uUmbraColor / uColor", sizeof(Vec3f));
    row("Mat4f", "uModelViewMatrixInverse / uModelViewMatrix", sizeof(Mat4f));
    row("floatUniform", "body_sun uRmag/uCmag/uRadius", sizeof(float));
    row("RingUniform", "ring.cpp:146 uniform (per ringed body)", sizeof(RingUniform));
    row("artGeom", "body_artificial.cpp:84 uProj", sizeof(artGeom));
    row("LightInfo", "body_artificial.cpp:84 uLight", sizeof(LightInfo));
    row("artVert", "body_artificial.cpp:84 uVert", sizeof(artVert));
    row("ShadowVert", "body_*.cpp uShadowVert", sizeof(ShadowVert));
    row("ShadowFrag_mc4", "body_*.cpp uShadowFrag (maxShadowCast=4)",
        (unsigned)(sizeof(ShadowFragHead) + sizeof(UShadowingBody) * 4));
    // ---- the in-galaxy OJM body (14.sts), ojm_mgr.cpp:77 -------------------
    row("ojmContainerUniformData", "ojm_mgr.cpp:77 uniform", sizeof(ojmContainerUniformData));
    return 0;
}
