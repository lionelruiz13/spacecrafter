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

Media::Media(/*unsigned int _width, unsigned int _height*/)
{
	// width = _width;
	// height = _height;
	audio = new Audio();
	imageMgr = new ImageMgr();
	player = new VideoPlayer();
	//external = new ExternalMplayer(width, height);
	mediaState = {V_TYPE::V_NO, V_STATE::V_OFF, A_TYPE::A_NO, A_STATE::A_OFF};
}


Media::~Media()
{
	if (audio)	delete audio;
	if (imageMgr) delete imageMgr;
	if (player) delete player;
	//if (external) delete external;
	if (vr360) delete vr360;
	if (viewPort) delete viewPort;
}

////////////////////////////////////////////////////////////////////////////////

void Media::audioMusicLoad(const std::string &filename)
{
	audio->musicLoad(filename);
	audioMusicPlay(false);
}

////////////////////////////////////////////////////////////////////////////////

int Media::playerPlay(const std::string &type, const std::string &filename, const std::string& _name, const std::string& _position)
{
	//~ printf("media->playerPlay sans son\n");
	//~ printf("Fichier video : %s\n", filename.c_str());
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

	mediaState.video_state=V_STATE::V_PLAY;

	audioMusicHalt();
	vr360->displayStop();
	viewPort->displayStop();

	if (playerGetAlive()) {
		if (type == "VR360") {
			vr360->setTexture(player->getYUV_VideoTexture());
			vr360->modeSphere();
			vr360->display(true);
			mediaState.video_type=V_TYPE::V_VR360;
			return 1;
		} else if (type == "VRCUBE") {
			vr360->setTexture(player->getYUV_VideoTexture());
			vr360->modeCube();
			vr360->display(true);
			mediaState.video_type=V_TYPE::V_VRCUBE;
			return 1;
		} else if (type == "VIEWPORT") {
			viewPort->setTexture(player->getYUV_VideoTexture());
			viewPort->displayFullScreen(true);
			viewPort->display(true);
			mediaState.video_type=V_TYPE::V_VIEWPORT;
			return 2;
		} else if (type == "DUAL_VIEWPORT") {
			viewPort->setTexture(player->getYUV_VideoTexture());
			viewPort->displayFullScreen(false);
			viewPort->display(true);
			mediaState.video_type=V_TYPE::V_VIEWPORT;
			return 2;
		} else if (type == "IMAGE") {
			mediaState.video_type=V_TYPE::V_IMAGE;
			imageVideoName = _name;
			imageMgr->loadImage(player->getVideoTexture(),_name, _position);
			return 3;
		} else {//no type -> stop playing
			playerStop();
			mediaState.video_state=V_STATE::V_OFF;
			mediaState.video_type=V_TYPE::V_NO;
			return -1;
		}
	} else {
		mediaState.video_state=V_STATE::V_OFF;
		mediaState.video_type=V_TYPE::V_NO;
		cLog::get()->write("Media::playerPlay error playerVideo with "+filename, LOG_TYPE::L_ERROR);
		printf("playerVideo est en erreur\n");
		return -1;
	}
}

int Media::playerPlay(const std::string &type,const  std::string &videoname, const std::string &audioname, const std::string& _name, const std::string& _position)
{
	//~ printf("media->playerPlay avec son\n");
	cLog::get()->write("Media::playerPlay trying to play videofilename "+videoname, LOG_TYPE::L_DEBUG);
	cLog::get()->write("Media::playerPlay trying to play audiofilename "+audioname, LOG_TYPE::L_DEBUG);
	int tmp = playerPlay(type, videoname, _name, _position);
	if (tmp >0) {
		audioMusicHalt();
		audioMusicLoad(audioname);
		audioMusicPlay(0);
		return 0;
	}
	return tmp;
}

void Media::playerStop()
{
	//~ printf("media->playerStop\n");
	cLog::get()->write("Media::playerPlayStop", LOG_TYPE::L_INFO);
	player->playStop();
	mediaState.video_state=V_STATE::V_OFF;
	audio->musicDrop();
	if (mediaState.video_type==V_TYPE::V_VR360)
		vr360->display(false);
	if (mediaState.video_type==V_TYPE::V_VRCUBE)
		vr360->display(false);
	if (mediaState.video_type==V_TYPE::V_VIEWPORT)
		viewPort->display(false);
	if ((mediaState.video_type==V_TYPE::V_IMAGE) && !imageVideoName.empty())
		imageMgr->drop_image(imageVideoName);
	mediaState.video_type=V_TYPE::V_NO;
}

void Media::playerRestart()
{
	//~ printf("media->playerRestart\n");
	cLog::get()->write("Media::playerRestart", LOG_TYPE::L_INFO);
	player->RestartVideo();
	audio->musicRewind();
}

void Media::playerJump(float deltaTime)
{
	float realDelta=0.0f;
	player->JumpVideo(deltaTime, realDelta);
	//~ printf("Le saut est de %f\n", realDelta);
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
 	//~ printf("Le saut est de %f\n", realDelta);
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

void Media::createVR360()
{
	vr360 = new VR360();
}

////////////////////////////////////////////////////////////////////////////////

void Media::createImageShader()
{
	imageMgr->createImageShader();
}
////////////////////////////////////////////////////////////////////////////////

void Media::createViewPort()
{
	viewPort = new ViewPort();
	viewPort-> createShader();
	viewPort-> createSC_context();
}

////////////////////////////////////////////////////////////////////////////////

// void Media::externalInit(const std::string &_mplayerFilename, const std::string &_mplayerMkfifoName, bool _mplayerEnable)
// {
// 	mplayerFilename = _mplayerFilename;
// 	mplayerMkfifoName = _mplayerMkfifoName;
// 	mplayerEnable = _mplayerEnable;
// 	external->init(mplayerFilename, mplayerMkfifoName, mplayerEnable);
// }

// void Media::externalReset()
// {
// 	if (external)
// 		delete external;

// 	external = new ExternalMplayer(width,height);
// 	external->init(mplayerFilename, mplayerMkfifoName, mplayerEnable);
// }
