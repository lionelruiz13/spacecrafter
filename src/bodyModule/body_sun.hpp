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
#include "EntityCore/Resource/SharedBuffer.hpp"

class Pipeline;

class Sun : public Body {

public:
	Sun(Body *parent,
	    const std::string& englishName,
	    bool flagHalo,
	    double radius,
	    double oblateness,
	    std::unique_ptr<BodyColor> myColor,
	    float _sol_local_day,
	    float albedo,
	    std::unique_ptr<Orbit> orbit,
	    bool close_orbit,
	    ObjL* _currentObj,
	    double orbit_bounding_radius,
		std::shared_ptr<BodyTexture> _bodyTexture);
	~Sun();

	void setFlagOrbit(bool b) {
	}

	// Get the magnitude for an observer at pos obs_pos in the heliocentric coordinate (in AU)
	virtual float computeMagnitude(const Vec3d obs_pos) const;

	virtual void computeDraw(const Projector* prj, const Navigator * nav) override;


	virtual bool drawGL(Projector* prj, const Navigator* nav, const Observer* observatory, const ToneReproductor* eye,
	                    bool depthTest, bool drawHomePlanet, bool needClearDepthBuffer) override;

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
	std::unique_ptr<s_texture> tex_big_halo;		// Big halo texture

	// Draw the big halo
	void drawBigHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye);

	virtual void drawBody(VkCommandBuffer &cmd, const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz);

	void createHaloShader(float viewport_y);
	void createSunShader();

	void selectShader() {};
	void defineSunSet();
	void buildHaloCmd();

	SHADER_USE myShader;  			// the name of the shader used for his display
	VkCommandBuffer haloCmds[3] {};
	int cmds[3] {-1, -1, -1}; // sun cmds
	std::unique_ptr<VertexArray> m_bigHaloGL;
	std::unique_ptr<VertexBuffer> haloBuffer;
	Vec2f *screenPosF = nullptr;
	std::unique_ptr<Pipeline> pipelineBigHalo, pipelineSun;
	std::unique_ptr<PipelineLayout> layoutBigHalo, layoutSun;
	std::unique_ptr<Set> descriptorSetBigHalo;
	std::unique_ptr<Set> descriptorSetSun;
	std::unique_ptr<SharedBuffer<float>> uRmag, uCmag, uRadius;
	std::unique_ptr<SharedBuffer<Vec3f>> uColor;
	std::unique_ptr<SharedBuffer<Mat4f>> uModelViewMatrix;
	std::unique_ptr<SharedBuffer<Vec3f>> uclipping_fov;
	std::unique_ptr<SharedBuffer<float>> uPlanetScaledRadius;
};
