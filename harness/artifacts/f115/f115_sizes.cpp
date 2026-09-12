// F115 - the uniform-pool census, EXTENDED from F102's f102_sizes.cpp.
//
// Contract with F102 (discriminating check (a) of the F115 section): the first
// 28 --tsv lines of this program are BYTE-IDENTICAL to
// claude/harness/artifacts/f102/sizes.tsv, keys and order included. Everything
// this task adds comes after them, so the landed table is reproduced rather
// than restated. Verify with:
//   ./f115_sizes 64 --tsv | head -28 | diff - ../f102/sizes.tsv
//
// I2 IMPROVEMENT OVER F102: F102 copied the old path's declarations into its
// own source because "headers pulling the whole engine in". Measured here:
// src/bodyModule/bodyShader.hpp includes only <list> <string> <memory>,
// tools/vecmath.hpp and bodyShaderInterface.hpp plus forward declarations, and
// compiles standalone. So globalTescGeom, ringFrag, moonFrag, artGeom, artVert,
// LightInfo, ShadowVert, ShadowFrag, UShadowingBody and OjmShadowFrag are now
// taken from the REAL header, not from a mirror - and the fact that the 28 rows
// still reproduce byte for byte is an independent check on F102's copies.
// Three declarations still cannot be included (their headers pull
// SharedBuffer.hpp -> BufferMgr.hpp -> vulkan): AtmExt::_uniform,
// Ring::RingUniform, OjmContainer::uniformData. Those stay mirrored WITH their
// file:line, and f115_mirrors.py re-greps each field list against its header
// before any number here is used.
//
// Build (from claude/harness/artifacts/f115):
//   g++ -O0 -std=c++20 -I../../../../src -I../../../../src/EntityCore \
//       -o <outdir>/f115_sizes f115_sizes.cpp
// Run: ./f115_sizes [alignment] [--tsv]     (alignment default 64)
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string>
#include "tools/vecmath.hpp"
#include "experimentalModule/meshModules/bodyShaderInterface.hpp"
#include "bodyModule/bodyShader.hpp"   // the REAL old-path UBO header

// ---- the three that cannot be included, mirrored WITH their file:line -------
// ring.hpp:113-123 - Ring::RingUniform (old path, per ringed body)
struct RingUniform {
    Mat4f ModelViewMatrix; Mat4f ModelViewMatrixInverse; Vec3f clipping_fov;
    float RingScale; Vec3f PlanetPosition; float PlanetRadius;
    Vec3f LightDirection; float SunnySideUp; float fadingFactor;
};
// atm_ext.hpp:20-30 (old) == AtmExtModule.hpp:76-86 (new, "exact copy of the
// old AtmExt::_uniform" says its own comment) - ONE layout, TWO declarations.
struct atmExtUBO {
    Mat4f ModelViewMatrix; Vec3f sunPos; float planetRadius; Vec3f bodyPos;
    float planetOneMinusOblateness; Vec3f clipping_fov; float atmRadius;
    Vec2i TesParam; float atmAlpha;
};
// ojm_mgr.hpp:89-92 - OjmContainer::uniformData (the in-galaxy OJM body)
struct ojmContainerUniformData { Mat4f ModelViewMatrix; Mat4f NormalMatrix; };
// OortModule.hpp:80 - per-feature, kept for F102-row parity
struct oortFrag { Vec3f color; float fader; };

static int A = 64;
static bool tsv = false;
static unsigned rounded(unsigned s) { return ((s - 1) / A + 1) * A; }

static void row(const char *key, const char *site, unsigned size)
{
    if (tsv) printf("%s\t%u\t%u\n", key, size, rounded(size));
    else printf("  %-26s %-42s %6u %6u\n", key, site, size, rounded(size));
}
static void sec(const char *title)
{
    if (!tsv) printf("\n-- %s\n", title);
}
// a derived total: already-carved bytes, printed with carved == sizeof so the
// column stays readable and f115_pool.py can sum the third column per section.
static void total(const char *key, const char *site, unsigned carved)
{
    if (tsv) printf("%s\t%u\t%u\n", key, carved, carved);
    else printf("  %-26s %-42s %6s %6u\n", key, site, "", carved);
}

// the shadow-caster array's per-entry cost, on each path
static const unsigned NEW_ENTRY = sizeof(meshFrag::ShadowingBody);   // 96
static const unsigned OLD_ENTRY = sizeof(UShadowingBody);            // 16
// the receive header of each new-path block = block minus its array
static unsigned newBlockCapped(unsigned full, unsigned cap)
{
    return full - NEW_ENTRY * MAX_SHADOW_CASTERS_PER_RECEIVER + NEW_ENTRY * cap;
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
        printf("  %-26s %-42s %6s %6s\n", "struct", "acquire site", "sizeof", "carved");
    } else {
        printf("ALIGNMENT\t%d\t%d\n", A, A);
        printf("POOL\t%d\t%d\n", 1 * 1024 * 1024, 1 * 1024 * 1024);
    }
    // ====================================================================
    // PART 1 - F102's 28 rows, keys and order UNCHANGED (check (a))
    // ====================================================================
    sec("NEW path, per body, eager at load (ModularSystem::loadBody :1458)");
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
    sec("OLD path, per body");
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
        (unsigned)(sizeof(ShadowFrag) + sizeof(UShadowingBody) * 4));
    sec("the in-galaxy OJM body (14.sts), ojm_mgr.cpp:77");
    row("ojmContainerUniformData", "ojm_mgr.cpp:77 uniform", sizeof(ojmContainerUniformData));
    // ==== end of F102's 28 lines. Everything below is F115's. ============

    // ====================================================================
    // PART 2 - the rows F102's table did not carry
    // ====================================================================
    sec("F115 - OLD path blocks absent from F102's table");
    row("globalTescGeom", "body_moon.cpp:258 / body_bigbody.cpp:149", sizeof(globalTescGeom));
    row("ringFrag", "body_bigbody.cpp:229 uRingFrag", sizeof(ringFrag));
    row("ShadowFrag_mc8", "body_moon.cpp:445 (FIELD config: 8)",
        (unsigned)(sizeof(ShadowFrag) + OLD_ENTRY * 8));
    row("OjmShadowFrag_mc4", "body_artificial.cpp:45 (default 4)",
        (unsigned)(sizeof(OjmShadowFrag) + OLD_ENTRY * 4));
    row("OjmShadowFrag_mc8", "body_artificial.cpp:45 (FIELD config: 8)",
        (unsigned)(sizeof(OjmShadowFrag) + OLD_ENTRY * 8));
    sec("F115 - NEW path, the same struct reused by a second module");
    row("photosphereVert", "PhotosphereModule.cpp:58 vert = globalVertProj", sizeof(globalVertProj));

    // ====================================================================
    // PART 3 - the shadow-caster array, priced per entry and per cap
    // ====================================================================
    sec("F115 - the receive array (MAX_SHADOW_CASTERS_PER_RECEIVER = 8)");
    row("ShadowingBody_entry", "bodyShaderInterface.hpp:56-71 (one entry)", NEW_ENTRY);
    row("newArray_x8", "8 entries, in EVERY new-path receive block",
        NEW_ENTRY * MAX_SHADOW_CASTERS_PER_RECEIVER);
    row("UShadowingBody_entry", "bodyShader.hpp:252-255 (old, one entry)", OLD_ENTRY);
    sec("F115 - the four new-path blocks that each embed the SAME array");
    row("meshFrag_cap1", "cap 1", newBlockCapped(sizeof(meshFrag), 1));
    row("meshFrag_cap2", "cap 2", newBlockCapped(sizeof(meshFrag), 2));
    row("meshFrag_cap4", "cap 4", newBlockCapped(sizeof(meshFrag), 4));
    row("meshFrag_cap0", "cap 0 (array out of the block entirely)", newBlockCapped(sizeof(meshFrag), 0));
    row("rayMarchFrag_cap1", "cap 1", newBlockCapped(sizeof(rayMarchFrag), 1));
    row("bodyRingFrag_cap1", "cap 1", newBlockCapped(sizeof(bodyRingFrag), 1));
    row("ojmShadowBlock_cap1", "cap 1", newBlockCapped(sizeof(ojmShadowBlock), 1));

    // ====================================================================
    // PART 4 - the per-body CARVED totals, by body class and path
    // ====================================================================
    const unsigned cGVP  = rounded(sizeof(globalVertProj));
    const unsigned cGF   = rounded(sizeof(globalFrag));
    const unsigned cMF   = rounded(sizeof(meshFrag));
    const unsigned cVec3 = rounded(sizeof(Vec3f));
    const unsigned cMat4 = rounded(sizeof(Mat4f));
    const unsigned cFl   = rounded(sizeof(float));
    const unsigned cMoon = rounded(sizeof(moonFrag));
    const unsigned cTesc = rounded(sizeof(globalTescGeom));
    const unsigned cSV   = rounded(sizeof(ShadowVert));
    const unsigned cSF8  = rounded(sizeof(ShadowFrag) + OLD_ENTRY * 8);
    const unsigned cRU   = rounded(sizeof(RingUniform));
    const unsigned cAtm  = rounded(sizeof(atmExtUBO));
    const unsigned cAG   = rounded(sizeof(artGeom));
    const unsigned cLI   = rounded(sizeof(LightInfo));
    const unsigned cAV   = rounded(sizeof(artVert));
    const unsigned cOSF8 = rounded(sizeof(OjmShadowFrag) + OLD_ENTRY * 8);
    const unsigned cRF   = rounded(sizeof(ringFrag));
    const unsigned cTG   = rounded(sizeof(meshTescGeom));
    const unsigned cRMV  = rounded(sizeof(rayMarchVert));
    const unsigned cRMF  = rounded(sizeof(rayMarchFrag));
    const unsigned cBRV  = rounded(sizeof(bodyRingVert));
    const unsigned cBRF  = rounded(sizeof(bodyRingFrag));
    const unsigned cOV   = rounded(sizeof(ojmVert));
    const unsigned cOG   = rounded(sizeof(ojmGeom));
    const unsigned cOL   = rounded(sizeof(ojmLight));
    const unsigned cOSB  = rounded(sizeof(ojmShadowBlock));
    const unsigned cOCU  = rounded(sizeof(ojmContainerUniformData));

    sec("F115 - CARVED per body, OLD path (eager unless said lazy)");
    total("OLD_moon_plain", "body_moon.cpp:284-285 (06.sts body)", cGVP + cGF);
    total("OLD_moon_night", "body_moon.cpp:266-267", cGVP + cGF);
    total("OLD_moon_bump", "body_moon.cpp:275-277", cGVP + cGF + cVec3);
    total("OLD_moon_tes", "body_moon.cpp:256-258 (heightmap)", cGVP + cMoon + cTesc);
    total("OLD_moon_shadowext", "body_moon.cpp:443,445 LAZY at first draw", cSV + cSF8);
    total("OLD_bigbody_plain", "body_bigbody.cpp:260-263 LAZY", cGVP + cGF);
    total("OLD_bigbody_ringed", "body_bigbody.cpp:220-229 LAZY", cGVP + cGF + cMat4 + cRF);
    total("OLD_bigbody_shadowext", "body_bigbody.cpp:126-128/186-188 LAZY", cSV + cSF8);
    total("OLD_smallbody_plain", "body_smallbody.cpp:176-177 LAZY", cGVP + cGF);
    total("OLD_smallbody_bump", "body_smallbody.cpp:169,176-177 LAZY", cGVP + cGF + cVec3);
    total("OLD_sun_eager", "body_sun.cpp:146-152 ctor", cFl * 3 + cVec3);
    total("OLD_sun_lazy", "body_sun.cpp:282-284 first draw", cMat4 + cVec3 + cFl);
    total("OLD_artificial", "body_artificial.cpp:84 ctor", cAG + cLI + cAV);
    total("OLD_artificial_shadowext", "body_artificial.cpp:45,171 LAZY", cOSF8);
    total("OLD_ring", "ring.cpp:146 (per ringed body)", cRU);
    total("OLD_atmext", "atm_ext.cpp:78 ctor (per atmosphere body)", cAtm);

    sec("F115 - CARVED per body, NEW path (all eager at load)");
    total("NEW_basicmesh", "BasicMesh.cpp:20 (06.sts body)", cGVP + cMF);
    total("NEW_layered_mid", "LayeredMesh.cpp:45", cGVP + cMF);
    total("NEW_layered_tes", "LayeredMesh.cpp:45,53", cGVP + cMF + cTG);
    total("NEW_layered_ray", "LayeredMesh.cpp:45,64,65", cGVP + cMF + cRMV + cRMF);
    total("NEW_layered_tes_ray", "LayeredMesh.cpp:45,53,64,65", cGVP + cMF + cTG + cRMV + cRMF);
    total("NEW_photosphere", "PhotosphereModule.cpp:58 (per star)", cGVP);
    total("NEW_ring", "RingModule.cpp:74-75 (per ringed body)", cBRV + cBRF);
    total("NEW_atmext", "AtmExtModule.cpp:73 (per atmosphere body)", cAtm);
    total("NEW_ojm", "OjmModule.cpp:97-98 (per artificial body)", cOV + cOG + cOL + cOSB);

    sec("F115 - the both-paths sum the owner asked about");
    total("BOTH_06sts_body", "OLD_moon_plain + NEW_basicmesh", cGVP + cGF + cGVP + cMF);
    total("INGALAXY_14sts_body", "ojm_mgr.cpp:77 (neither path)", cOCU);

    sec("F115 - the same bodies with the array capped at 1 (option O2)");
    total("NEW_basicmesh_cap1", "globalVertProj + meshFrag_cap1",
          cGVP + rounded(newBlockCapped(sizeof(meshFrag), 1)));
    total("BOTH_06sts_body_cap1", "OLD_moon_plain + the above",
          cGVP + cGF + cGVP + rounded(newBlockCapped(sizeof(meshFrag), 1)));
    total("NEW_basicmesh_cap0", "globalVertProj + meshFrag_cap0 (array shared)",
          cGVP + rounded(newBlockCapped(sizeof(meshFrag), 0)));
    return 0;
}
