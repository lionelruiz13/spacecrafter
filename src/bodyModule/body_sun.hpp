/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 Immersive Adventure
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include <memory>
#include "bodyModule/body.hpp"
#include "vulkanModule/Context.hpp"

class VertexArray;
class Uniform;
class Pipeline;
class Buffer;

class Sun : public Body {

public:
	Sun(Body *parent,
	    const std::string& englishName,
	    bool flagHalo,
	    double radius,
	    double oblateness,
	    BodyColor * myColor,
	    float _sol_local_day,
	    float albedo,
	    std::shared_ptr<Orbit> orbit,
	    bool close_orbit,
	    ObjL* _currentObj,
	    double orbit_bounding_radius,
		BodyTexture* _bodyTexture,
		ThreadContext *context);
	~Sun();

	void setFlagOrbit(bool b) {
	}

	// Get the magnitude for an observer at pos obs_pos in the heliocentric coordinate (in AU)
	virtual float computeMagnitude(const Vec3d obs_pos) const;

	virtual void computeDraw(const Projector* prj, const Navigator * nav) override;


	virtual bool drawGL(Projector* prj, const Navigator* nav, const Observer* observatory, const ToneReproductor* eye,
	                    bool depthTest, bool drawHomePlanet) override;

	void setBigHalo(const std::string& halotexfile, const std::string& path);

	void setHaloSize(float s) {
		big_halo_size = s;
	}

	//big_halo_size
	void setBigHaloSize(int bhs);

	int getBigHaloSize() {
		return big_halo_size;
	}

protected:
	//params
	float big_halo_size;
	s_texture * tex_big_halo;		// Big halo texture

	// Draw the big halo
	void drawBigHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye);

	virtual void drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz);

	void createHaloShader(ThreadContext *context, float viewport_y);
	void createSunShader(ThreadContext *context);

	void selectShader() {};

	//variables
	// shaderProgram *shaderSun;
	SHADER_USE myShader;  			// the name of the shader used for his display
	//shaderProgram *myShaderProg;	// Shader moderne
	CommandMgr *cmdMgr = nullptr;
	s_texture *last_tex_current = nullptr;
	int commandIndexBigHalo, commandIndexSun = -2;
	std::unique_ptr<VertexArray> m_bigHaloGL;
	std::unique_ptr<Pipeline> pipelineBigHalo, pipelineSun;
	std::unique_ptr<PipelineLayout> layoutBigHalo, layoutSun;
	std::unique_ptr<Set> descriptorSetBigHalo, descriptorSetSun;
	std::unique_ptr<Uniform> uRmag, uCmag, uRadius, uColor;
	float *pRmag, *pCmag, *pRadius;
	Vec3f *pColor;
	std::unique_ptr<Uniform> uModelViewMatrix, uclipping_fov, uPlanetScaledRadius;
	Mat4f *pModelViewMatrix;
	Vec3f *pclipping_fov;
	float *pPlanetScaledRadius;
	std::unique_ptr<Buffer> drawData; // for Sun
	//std::unique_ptr<shaderProgram> shaderSun, shaderBigHalo;
};
