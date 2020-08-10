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

#include "mediaModule/media.hpp"
#include "tools/log.hpp"

#define STEP_VOLUME 5

Media::Media()
{
	audio = new Audio();
	imageMgr = new ImageMgr();
	player = new VideoPlayer(this);
	viewPort = new ViewPort();
	vr360 = new VR360();
	m_videoState = {V_TYPE::V_NONE, V_STATE::V_NONE};
}


Media::~Media()
{
	if (audio)	delete audio;
	if (imageMgr) delete imageMgr;
	if (player) delete player;
	if (vr360) delete vr360;
	if (viewPort) delete viewPort;
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

int Media::playerPlay(const std::string &type, const std::string &filename, const std::string& _name, const std::string& _position)
{
	cLog::get()->write("Media::playerPlay trying to play videofilename "+filename, LOG_TYPE::L_DEBUG);
	int tmp;
	
	if (type == "IMAGE")
		tmp = player->play(filename, true);
	else
		tmp = player->play(filename, false);
	
	if (tmp !=0) {
		cLog::get()->write("Media::playerPlay error playing videofilename "+filename, LOG_TYPE::L_ERROR);
		return tmp;
	}

	m_videoState.state=V_STATE::V_PLAY;

	audioMusicHalt();
	vr360->displayStop();
	viewPort->displayStop();

	if (playerisVideoPlayed()) {
		if (type == "VR360") {
			vr360->setTexture(player->getYUV_VideoTexture());
			vr360->modeSphere();
			vr360->display(true);
			m_videoState.type=V_TYPE::V_VR360;
			return 1;
		} else if (type == "VRCUBE") {
			vr360->setTexture(player->getYUV_VideoTexture());
			vr360->modeCube();
			vr360->display(true);
			m_videoState.type=V_TYPE::V_VRCUBE;
			return 1;
		} else if (type == "VIEWPORT") {
			viewPort->setTexture(player->getYUV_VideoTexture());
			viewPort->displayFullScreen(true);
			viewPort->display(true);
			m_videoState.type=V_TYPE::V_VIEWPORT;
			return 2;
		} else if (type == "DUAL_VIEWPORT") {
			viewPort->setTexture(player->getYUV_VideoTexture());
			viewPort->displayFullScreen(false);
			viewPort->display(true);
			m_videoState.type=V_TYPE::V_VIEWPORT;
			return 2;
		} else if (type == "IMAGE") {
			m_videoState.type=V_TYPE::V_IMAGE;
			imageVideoName = _name;
			imageMgr->loadImage(player->getVideoTexture(),_name, _position);
			return 3;
		} else {//no type -> stop playing
			playerStop();
			m_videoState.state=V_STATE::V_NONE;
			m_videoState.type=V_TYPE::V_NONE;
			return -1;
		}
	} else {
		m_videoState.state=V_STATE::V_NONE;
		m_videoState.type=V_TYPE::V_NONE;
		cLog::get()->write("Media::playerPlay error playerVideo with "+filename, LOG_TYPE::L_ERROR);
		printf("playerVideo est en erreur\n");
		return -1;
	}
}

int Media::playerPlay(const std::string &type,const  std::string &videoname, const std::string &audioname, const std::string& _name, const std::string& _position)
{
	cLog::get()->write("Media::playerPlay trying to play videofilename "+videoname, LOG_TYPE::L_DEBUG);
	cLog::get()->write("Media::playerPlay trying to play audiofilename "+audioname, LOG_TYPE::L_DEBUG);
	int tmp = playerPlay(type, videoname, _name, _position);
	if (tmp >0) {
		audioMusicHalt();
		audioMusicLoad(audioname, false);
		audioMusicPlay();
		return 0;
	}
	return tmp;
}

void Media::playerStop()
{
	cLog::get()->write("Media::playerPlayStop", LOG_TYPE::L_INFO);
	player->playStop();
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
	player->RestartVideo();
	audio->musicRewind();
}

void Media::playerJump(float deltaTime)
{
	float realDelta=0.0f;
	player->JumpVideo(deltaTime, realDelta);
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
 	player->Invertflow(realDelta);
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

void Media::init()
{
	vr360->init();
}

void Media::createSC_context()
{
	viewPort-> createShader();
	viewPort-> createSC_context();
	vr360-> createShader();
	imageMgr->createImageShader();
}
////////////////////////////////////////////////////////////////////////////////
