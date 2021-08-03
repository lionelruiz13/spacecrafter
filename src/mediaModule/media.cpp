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

#include <algorithm>
#include "mediaModule/media.hpp"
#include "tools/log.hpp"

#define STEP_VOLUME 5

Media::Media()
{
	audio = std::make_unique<Audio>();
	imageMgr = std::make_unique<ImageMgr>();
	player = std::make_unique<VideoPlayer>(this);
	viewPort = std::make_unique<ViewPort>();
	vr360 = std::make_unique<VR360>();

	strToVid["vrcube"] = VID_TYPE::V_VRCUBE;
	strToVid["vr360"] = VID_TYPE::V_VR360 ;
	strToVid["image"] = VID_TYPE::V_IMAGE;
	strToVid["viewport"] = VID_TYPE::V_FULLVIEWPORT;
	strToVid["dual_viewport"] = VID_TYPE::V_DUALVIEWPORT;

	m_videoState = {V_TYPE::V_NONE, V_STATE::V_NONE};
}

Media::~Media()
{
	// if (audio)	delete audio;
	// if (imageMgr) delete imageMgr;
	// if (player) delete player;
	// if (vr360) delete vr360;
	// if (viewPort) delete viewPort;
}

VID_TYPE Media::strToVideoType(const std::string& _value)
{
	std::string value = _value;
	transform(value.begin(), value.end(), value.begin(), ::tolower);
	auto it = strToVid.find(value);
	if (it ==strToVid.end())
		return VID_TYPE::V_NONE;
	else
		return it->second;
}
////////////////////////////////////////////////////////////////////////////////

void Media::audioMusicLoad(const std::string &filename, bool )
{
	audio->musicLoad(filename, false);
	audioMusicPlay();
}

void Media::audioFunction(const AudioFunction& audioFunction, const AudioParam& audioParam)
{
	switch (audioFunction) {
		case AudioFunction::AF_MUSICPLAY:
			audio->musicPlay();
			break;
		case AudioFunction::AF_MUSICHALT:
			if (m_videoState.state != V_STATE::V_PLAY) {
				audio->musicHalt();
			}
			break;
		case AudioFunction::AF_MUSICDROP:
			audio->musicDrop();
			break;
		case AudioFunction::AF_MUSICSYNC:
			audio->musicSync();
			break;
		case AudioFunction::AF_MUSICREWIND:
			audio->musicRewind();
			break;
		case AudioFunction::AF_MUSICRESUME:
			audio->musicResume();
			break;
		case AudioFunction::AF_MUSICPAUSE:
			if (!audioNoPause)
				audio->musicPause();
			break;
		case AudioFunction::AF_MUSICJUMP:
			audio->musicJump(audioParam.secondJump);
			break;
		case AudioFunction::AF_MUSICLOAD:
			audio->musicLoad(audioParam.filename, audioParam.loop);
			audio->musicPlay();
			break;
	}
}

void Media::audioVolume(const AudioVolume& volumeOrder, float _value)
{
	switch (volumeOrder) {
		case AudioVolume::AV_SETVOLUME:
			audio->setVolume(_value);
			break;
		case AudioVolume::AV_INCREMENTVOLUME:
			if (_value!=0)
				audio->incrementVolume(_value);
			else
				audio->incrementVolume(STEP_VOLUME);
			break;
		case AudioVolume::AV_DECREMENTVOLUME:
			if (_value!=0)
				audio->decrementVolume(_value);
			else
				audio->decrementVolume(STEP_VOLUME);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////

bool Media::playerPlay(const VID_TYPE &type, const std::string &filename, const std::string& _name, const std::string& _position, IMG_PROJECT tmpProject)
{
	cLog::get()->write("Media::playerPlay trying to play videofilename "+filename, LOG_TYPE::L_DEBUG);
	if (player->playNewVideo(filename) ==false) {
		cLog::get()->write("Media::playerPlay error playing videofilename "+filename, LOG_TYPE::L_ERROR);
		return false;
	}

	m_videoState.state=V_STATE::V_PLAY;

	audioMusicHalt();
	vr360->displayStop();
	viewPort->displayStop();

	if (!playerIsVideoPlayed()) {
		m_videoState.state=V_STATE::V_NONE;
		m_videoState.type=V_TYPE::V_NONE;
		cLog::get()->write("Media::playerPlay error playerVideo with "+filename, LOG_TYPE::L_ERROR);
		return false;
	}
	switch(type) {
		case VID_TYPE::V_VR360 :
			vr360->setTexture(player->getYUV_VideoTexture());
			vr360->modeSphere();
			vr360->display(true);
			m_videoState.type=V_TYPE::V_VR360;
			break;
		case VID_TYPE::V_VRCUBE :
			vr360->setTexture(player->getYUV_VideoTexture());
			vr360->modeCube();
			vr360->display(true);
			m_videoState.type=V_TYPE::V_VRCUBE;
			break;
		case VID_TYPE::V_FULLVIEWPORT :
			viewPort->setTexture(player->getYUV_VideoTexture());
			viewPort->displayFullScreen(true);
			viewPort->display(true);
			m_videoState.type=V_TYPE::V_VIEWPORT;
			break;
		case VID_TYPE::V_DUALVIEWPORT :
			viewPort->setTexture(player->getYUV_VideoTexture());
			viewPort->displayFullScreen(false);
			viewPort->display(true);
			m_videoState.type=V_TYPE::V_VIEWPORT;
			break;
		case VID_TYPE::V_IMAGE :
			m_videoState.type=V_TYPE::V_IMAGE;
			imageVideoName = _name;
			imageMgr->loadImage(player->getYUV_VideoTexture(),_name, _position, tmpProject);
			break;
		case VID_TYPE::V_NONE :
			playerStop();
			m_videoState.state=V_STATE::V_NONE;
			m_videoState.type=V_TYPE::V_NONE;
			break;
	}
	return true;
}

bool Media::playerPlay(const VID_TYPE &type, const std::string &videoname, const std::string &audioname, const std::string& _name, const std::string& _position, IMG_PROJECT tmpProject)
{
	cLog::get()->write("Media::playerPlay trying to play videofilename "+videoname, LOG_TYPE::L_DEBUG);
	bool tmp = playerPlay(type, videoname, _name, _position, tmpProject);
	if (tmp && !audioname.empty()) {
		audioMusicHalt();
		audioMusicLoad(audioname, false);
		audioMusicPlay();
		cLog::get()->write("Media::playerPlay trying to play audiofilename "+audioname, LOG_TYPE::L_DEBUG);
		return true;
	}
	return tmp;
}

void Media::playerStop()
{
	cLog::get()->write("Media::playerPlayStop", LOG_TYPE::L_INFO);
	player->stopCurrentVideo();
	m_videoState.state=V_STATE::V_NONE;
	audio->musicDrop();
	if (m_videoState.type==V_TYPE::V_VR360)
		vr360->display(false);
	if (m_videoState.type==V_TYPE::V_VRCUBE)
		vr360->display(false);
	if (m_videoState.type==V_TYPE::V_VIEWPORT)
		viewPort->display(false);
	if ((m_videoState.type==V_TYPE::V_IMAGE) && !imageVideoName.empty())
		imageMgr->drop_image(imageVideoName);
	m_videoState.type=V_TYPE::V_NONE;
}

void Media::playerRestart()
{
	cLog::get()->write("Media::playerRestart", LOG_TYPE::L_INFO);
	player->restartCurrentVideo();
	audio->musicRewind();
}

void Media::playerJump(float deltaTime)
{
	float realDelta=0.0f;
	player->jumpInCurrentVideo(deltaTime, realDelta);
	if (realDelta==0.0) {
		audio->musicRewind();
		return;
	}
	if (realDelta==-1.0)
		audio->musicDrop();
	else {
		audio->musicResume();
		audio->musicJump(realDelta);
	}
}

void Media::playerInvertflow()
{
	float realDelta=0.0f;
	player->invertVideoFlow(realDelta);
	if (realDelta==0.0) {
		audio->musicRewind();
		return;
	}
	if (realDelta==-1.0)
		audio->musicDrop();
	else {
		audio->musicResume();
		audio->musicJump(realDelta);
	}
}

////////////////////////////////////////////////////////////////////////////////

void Media::init(ThreadContext *context)
{
	vr360->init(context);
}

void Media::createSC_context(ThreadContext *context)
{
	viewPort-> createSC_context(context);
	vr360-> createSC_context(context);
	imageMgr->createImageShader(context);
	player->createTextures(context);
}
////////////////////////////////////////////////////////////////////////////////
