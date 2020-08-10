/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright 2017 Immersive Adventure
 *
 *
 */

#ifndef _VIDEOPLAYER_HPP_
#define _VIDEOPLAYER_HPP_

#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <SDL2/SDL.h>

#include "renderGL/shader.hpp"
#include "renderGL/stateGL.hpp"
#include "yuv_wrapper.hpp"

#ifndef WIN32
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#endif

class s_texture;
class Media;

//! \class VideoPlayer
//! \brief Classe qui gere toute les fonctions de la ffmpeg pour le player vidéo.
//! Son but est de rendre un pointeur sur texture utilisable dans le reste du logiciel
//! La classe gère d'elle même le bon fps
class VideoPlayer {
public:
	//! \fn VideoPlayer
	//! \brief Constructeur: initialise les états de la ffmpeg
	VideoPlayer(Media* _media);

	//! \fn ~VideoPlayer
	//! \brief Destructeur, ferme les états de la ffmpeg
	~VideoPlayer();

	//! recherche la frame suivante
	void update();

	//! initialise la ffmpeg avec le nom du fichier passé en argument
	int play(const std::string& fileName, bool convertToRBG);

	//! termine la lecture d'une vidéo en cours
	void playStop();

	//! met la video en pause
	void pause();

	//! affiche sur la console les informations liées au fichier vidéo
	void getInfo();

	//! \fn RestartVideo
	//! \brief Redemarre la vidéo au début
	bool RestartVideo();

	bool Invertflow(float &reallyDeltaTime);

	//! \fn JumpVideo
	//! \param seconde temps a sauter (en secondes)
	//! \brief Permet de faire un saut relatif dans le flux video
	//! \param reallyDeltaTime : indique à la classe Media de combien on s'est déplacé au final.
	bool JumpVideo(float seconde, float &reallyDeltaTime);

	//! Renvoie l'état du player
	//! @return true si un fichier est en cours de lecture, false sinon
	bool isVideoPlayed() {
		return m_isVideoPlayed;
	}

	//! Renvoie l'ID de la texture dans le GPU représentant la frame lue du fichier vidéo
	GLuint getVideoTexture() {
		return RGBtexture;
	}

	YUV_WRAPPER getYUV_VideoTexture() {
		return YUV_Texture;
	}

private:
	// renvoie la nouvelle frame vidéo et la convertit dans la mémoire de la CG.
	void getNextVideoFrame();
	// récupère la nouvelle frame vidéo avant conversion
	void getNextFrame();
	// initialisation de la classe
	void init();
	// fonction interne de saut dans la vidéo
	bool seekVideo(int64_t frameToSkeep, float &reallyDeltaTime);
	//! initialise une texture à la taille de la vidéo
	void initTexture();
	 //! texture représentant la frame actuelle

	Media* media=nullptr;
	GLuint RGBtexture;
	YUV_WRAPPER YUV_Texture;
	GLuint YUVtexture[3];

	bool isDisplayRVB;		//!< indique si le rendu doit être converti en RVG ou pas
	std::string fileName; 	//!< nom de la vidéo à lire
	int video_w;			
	int video_h;
	bool m_isVideoPlayed;	//!< indique si une vidéo est en cours de lecture
	bool isInPause;			//!< indique si la video est en pause
	bool isSeeking;			//!< indique si on est entrain de sauter de frame

	int64_t nbFrames;		 //!< compteur de frames
	int64_t nbTotalFrame;	 //!< nombre de frames prévues par la vidéo

	//Gestion frameRate
	int64_t TickCount ;
	int64_t firstCount;
	int64_t lastCount;
	double d_lastCount;

	double frameRate;
	double frameRateDuration;
	double elapsedTime;

	//gestion de la pause
	Uint32 startPause;
	Uint32 endPause;

#ifndef WIN32
	//parametres liés à ffmpeg
	AVFormatContext	*pFormatCtx;
	int				videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame	*pFrameIn,*pFrameOut;
	AVStream *video_st;
	AVPacket *packet;
	struct SwsContext *img_convert_ctx;
#endif
};


#endif // VIDEOPLAYER_HPP
