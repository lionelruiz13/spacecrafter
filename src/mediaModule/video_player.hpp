/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - 2020 all rights reserved
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
#include "mediaModule/media_base.hpp"

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

/**
 * \class VideoPlayer
 * \brief Classe qui gere toute les fonctions de la ffmpeg pour le player vidéo.
 *
 * Son but est de rendre un pointeur sur texture(s) utilisable dans le reste du logiciel
 * Deux possibilités s'offrent à l'utilisateur :
 * - une texture en RBG24 classique (mais lente à obtenir à cause des conversions)
 * - 3 textures YUV420p directement (rapide à obtenir: directement du fichier à la carte graphique)
 * 
 * La classe gère d'elle même le bon fps c'est à dire qu'elle regarde via des appels à SDL_GetTicks si elle doit mettre la frame à jour.
 * Si c'est le cas, alors elle met à jour la/les texture(s). 
 * 
 */
class VideoPlayer {
public:
	//! Constructeur: initialise les états de la ffmpeg
	VideoPlayer(Media* _media);

	//! Destructeur, ferme les états de la ffmpeg
	~VideoPlayer();

	//! recherche la frame suivante
	void update();

	//! initialise la ffmpeg avec le nom du fichier passé en argument
	int playNewVideo(const std::string& fileName, bool convertToRBG);

	//! termine la lecture d'une vidéo en cours
	void stopCurrentVideo();

	//! met la video en pause
	void pauseCurrentVideo();

	//! Redémarre la vidéo actuelle au début
	bool restartCurrentVideo();

	bool invertVideoFlow(float &reallyDeltaTime);

	//! Permet de faire un saut relatif dans le flux video
	//! \param seconde temps a sauter (en secondes)
	//! \param reallyDeltaTime : indique à la classe Media de combien on s'est déplacé au final.
	bool jumpInCurrentVideo(float seconde, float &reallyDeltaTime);

	//! Renvoie l'état du player
	//! @return true si un fichier est en cours de lecture, false sinon
	bool isVideoPlayed() const {
		return m_isVideoPlayed;
	}

	//! Renvoie l'ID de la texture RGB dans le GPU représentant la frame lue du fichier vidéo
	GLuint getRBG_VideoTexture() const {
		return videoTexture.rgb;
	}
	//! Renvoie l'ID des textures YUV dans le GPU représentant la frame lue du fichier vidéo
	VideoTexture getYUV_VideoTexture() const {
		return videoTexture;
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

	Media* media=nullptr;
	VideoTexture videoTexture;	//!< renvoie les indices des textures pour les classes nécessitant
	bool isDisplayRVB;			//!< indique si le rendu doit être converti en RVG ou pas

	std::string fileName; 	//!< nom de la vidéo
	Resolution videoRes;	//!< int video_w, video_h;	//!< taille w,h  de la vidéo

	bool m_isVideoPlayed;	//!< indique si une vidéo est en cours de lecture
	bool m_isVideoInPause;	//!< indique si la video est en pause
	bool m_isVideoSeeking;	//!< indique si on est entrain de sauter de frame

	//gestion des durées
	int64_t TickCount;
	int64_t firstCount;
	int64_t lastCount;
	double d_lastCount;

	//Gestion frameRate
	int64_t currentFrame;	//!< numéro de la frame actuelle
	int64_t nbTotalFrame;	//!< nombre de frames prévues par la vidéo
	double frameRate;
	double frameRateDuration;

	//gestion de la pause
	Uint32 startPause;
	Uint32 endPause;

#ifndef WIN32
	//parametres liés à ffmpeg
	AVFormatContext	*pFormatCtx;
	int				videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame			*pFrameIn,*pFrameOut;
	AVStream		*video_st;
	AVPacket		*packet;
	struct SwsContext *img_convert_ctx;
#endif
};

#endif // VIDEOPLAYER_HPP
