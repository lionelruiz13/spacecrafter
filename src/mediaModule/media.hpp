/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2014-2018 of the LSS Team & Association Sirius
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */


#ifndef _MEDIA_HPP_
#define _MEDIA_HPP_

#include "mediaModule/audio.hpp"
#include "mediaModule/image_mgr.hpp"
#include "mediaModule/video_player.hpp"
#include "mediaModule/external_mplayer.hpp"
#include "mediaModule/vr360.hpp"
#include "mediaModule/viewport.hpp"
#include "tools/app_settings.hpp"

class Projector;
class Navigator;

class Media {

public:
	Media(unsigned int _width, unsigned int _height);
	~Media();
	Media(Media const &) = delete;
	Media& operator = (Media const &) = delete;

	////////////////////////////////////////////////////////////////////////////
	//
	// other
	//
	////////////////////////////////////////////////////////////////////////////
	void createVR360();
	void createViewPort();
	void createImageShader();

	//! affiche une image du player video à destination du VR360
	void drawVR360(const Projector* prj, const Navigator* nav) {
		vr360->draw(prj, nav);
	}

	//! affiche une image de player video à destination du ViewPort
	void drawViewPort() {
		viewPort->draw();
	}

	void faderUpdate(float delta_time) {
		vr360->update(delta_time);
		viewPort->update(delta_time);
	}
	
	void disableFader() {
		viewPort->disableFader();
	}

	//! permet d'indiquer au shader la couleur à effacer
	//! \param color représente les 3 composantes RGB
	//! \param intensity représente -+ un delta de différence de couleurs
	void setKeyColor(const Vec3f &color, float intensity = 0.05) {
		viewPort->setKeyColor(color,intensity);
	}

	//! active la transparence lors de la lecture des vidéos
	//! \param v représente le booléan .
	void setKeyColor(bool v) {
		viewPort->setTransparency(v);
	}

	////////////////////////////////////////////////////////////////////////////
	//
	//interface audio
	//
	////////////////////////////////////////////////////////////////////////////
	void audioSetVolume(int _value) {
		audio->setVolume(_value);
	}

	void audioVolumeIncrement(int value=5) {
		audio->incrementVolume(value);
	}

	void audioVolumeDecrement(int value=5) {
		audio->decrementVolume(value);
	}

	void audioMusicPlay(const std::string &filename,bool loop);

	void audioMusicPause() {
		if (!audioNoPause)
			audio->musicPause();
	}

	void audioMusicResume() {
		audio->musicResume();
	}

	void audioMusicRewind() {
		audio->musicRewind();
	}

	void audioMusicHalt() {
		if (mediaState.video_state != V_STATE::V_PLAY) {
			audio->musicHalt();
		}
	}

	void audioMusicSync() {
		audio->musicSync();
	}

	void audioMusicDrop() {
		audio->musicDrop();
	}

	void audioUpdate(int delta_time) {
		audio->update(delta_time);
	}

	void audioMusicJump(float deltaTime) {
		audio->musicJump(deltaTime);
	}

	//! Permet de fixer le comportement 
	//! @param true: la musique continue à jouer si le script entre en pause
	//! @param false: la musique s'arrète de jouer si le script entre en pause
	void audioSetMusicToPause(bool value) {
		audioNoPause = value;
	}
	////////////////////////////////////////////////////////////////////////////
	//
	//interface image
	//
	////////////////////////////////////////////////////////////////////////////
	int imageLoad(const std::string &filename, const  std::string &name, const std::string &coordinate, bool mipmap) {
		return imageMgr->loadImage( filename,  name, coordinate, mipmap);
	}

	void imageClone(const std::string &name, int i) {
		imageMgr->clone(name,i);
	}

	void imageDrop(const std::string &name) {
		return imageMgr->drop_image(name);
	}

	void imageDropAll() {
		return imageMgr->dropAllImages();
	}

	void imageDropAllNoPersistent() {
		return imageMgr->dropAllNoPersistent();
	}

	bool imageSet(const std::string &name) {
		return imageMgr->setImage(name);
	}

	void imageSetAlpha( float alpha, float duration) {
		imageMgr->setAlpha(alpha, duration);
	}

	void imageSetScale( float scale, float duration) {
		imageMgr->setScale(scale, duration);
	}

	void imageSetRotation( float rotation, float duration) {
		imageMgr->setRotation(rotation, duration);
	}

	void imageSetPersistent(bool value) {
		imageMgr->setPersistent(value);
	}

	void imageSetLocation(float xpos, bool deltax, float ypos, bool deltay, float duration, bool accelerate_x = false, bool decelerate_x = false, bool accelerate_y = false, bool decelerate_y = false) {
		imageMgr->setLocation(xpos, deltax, ypos, deltay, duration, accelerate_x, decelerate_x, accelerate_y, decelerate_y);
	}

	void imageSetRatio(float ratio, float duration) {
		imageMgr->setRatio(ratio, duration);
	}

	void imageUpdate(int delta_time) {
		imageMgr->update(delta_time);
	}

	void imageDraw(const Navigator * nav, Projector * prj) {
		imageMgr->draw(nav, prj);
	}

	//! permet d'indiquer au shader la couleur à effacer
	//! \param color représente les 3 composantes RGB
	//! \param intensity représente -+ un delta de différence de couleurs
	void imageSetKeyColor(const Vec3f &color, float intensity = 0.05) {
		imageMgr->setKeyColor(color,intensity);
	}

	//! active la transparence lors de la lecture des vidéos
	//! \param v représente le booléan .
	void imageSetKeyColor(bool v) {
		imageMgr->setTransparency(v);
	}

	////////////////////////////////////////////////////////////////////////////
	//
	//interface video
	//
	////////////////////////////////////////////////////////////////////////////
	void playerUpdate() {
		player->update();
	}

	void playerPause() {
		player->pause();
		audio->musicPause();
	}

	int playerPlay(const std::string &type, const std::string &filename, const std::string& _name, const std::string& _position);

	int playerPlay(const std::string &type, const std::string &videoname, const std::string &audioname, const std::string& _name, const std::string& _position);

	void playerStop();

	void playerRestart();

	void playerJump(float deltaTime);

	void playerInvertflow();

	// int getDerive() {
	// 	return player->getDerive();
	// }

	// int playerGetFps() {
	// 	return player->getFps();
	// }

	bool playerGetAlive() {
		return player->getAlive();
	}

	////////////////////////////////////////////////////////////////////////////
	//
	//interface external
	//
	////////////////////////////////////////////////////////////////////////////
	void externalReset();

	void externalPlay(const std::string &filename) {
		external->play(filename);
	}

	void externalExecute(const std::string &msg) {
		external->execute(msg);
	}

	void externalStop() {
		external->stop();
	}

	void externalPause() {
		external->pause();
	}

	void externalSpeed(double value) {
		external->speed(value);
	}

	void externalVolume(double value) {
		external->volume(value);
	}

	void externalJumpRelative(int secondes) {
		external->jumpRelative(secondes);
	}

	void externalJumpAbsolute(int secondes) {
		external->jumpAbsolute(secondes);
	}

	void externalUpdate(int delta_time) {
		external->update(delta_time);
	}

	void externalInit(const std::string &_mplayerFilename, const std::string &_mplayerMkfifoName, bool _mplayerEnable);

	bool externalMplayerIsAlive() {
		return mplayerEnable;
	}

private:
	Audio * audio = nullptr;
	ImageMgr* imageMgr = nullptr;
	VideoPlayer* player = nullptr;
	ExternalMplayer* external = nullptr;
	VR360* vr360 = nullptr;
	ViewPort* viewPort = nullptr;

	// utilisé pour reset de External_Viewer
	unsigned int width;
	unsigned int height;
	std::string mplayerFilename;
	std::string mplayerMkfifoName;
	std::string skyLanguage;
	bool mplayerEnable;
	bool audioNoPause=false;

	std::string imageVideoName;

	//etat de media, concernant video et audio
	enum class V_TYPE : char { V_NO, V_VIEWPORT, V_IMAGE, V_VR360 , V_VRCUBE};
	enum class V_STATE: char { V_OFF, V_PAUSE, V_PLAY };
	enum class A_TYPE : char { A_NO, A_VIDEO, A_AUDIO} ;
	enum class A_STATE: char { A_OFF, A_PAUSE, A_PLAY};

	struct MediaState {
		V_TYPE video_type;
		V_STATE video_state;
		A_TYPE audio_type;
		A_STATE audio_state;
	};

	MediaState mediaState;
};

#endif //MEDIA_HPP
