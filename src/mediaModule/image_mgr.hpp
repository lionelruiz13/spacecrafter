/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2005 Robert Spearman
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014-2018 LSS Team & Immersive Adventure
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
#ifndef _IMAGE_MGR_H_
#define _IMAGE_MGR_H_

#include <list>
#include <map>
#include <string>
#include <memory>
#include <GL/glew.h>
#include "tools/vecmath.hpp"
#include "mediaModule/media_base.hpp"
#include "tools/no_copy.hpp"

class Image;
class Navigator;
class Projector;

/**
 * @class ImageMgr
 * @brief Cette classe s'ocupe de gérer toutes les entités Image utilisées dans les scripts. 
 * 
 * Le conteneur active_images contient toutes les images qui sont indépendantes.
 * Cette classe sert alors gestionnaire d'images.
 *
 * Certaines images sont permanantes, aussi elles ne sont pas affectées par la suppression
 * 
 * currentImg permet de modifier les images du conteneur grace à la fonction
 * setImage. Si currentImg est défini, alors setXXXX modifie l'image concernée.
 */


class ImageMgr: public NoCopy {
public:
	ImageMgr();
	virtual ~ImageMgr();

	//! tube pour la création des shaders pour le draw des images
	void createImageShader();

	//! charge une image dans le conteneur
	int loadImage(const std::string& filename, const std::string& name, const std::string& coordinate, IMG_PROJECT project, bool mipmap);
	//! charge une image venant directement de l'OpenGL
	int loadImage(GLuint imgTex, const std::string& name, const std::string& coordinate, IMG_PROJECT project);
	//! supprime du conteneur l'image name
	void drop_image(const std::string &name);
	//! supprime toutes les images du conteneur non persistantes
	void dropAllNoPersistent();
	//! supprime toutes les images du conteneur
	void dropAllImages();

	//! modifie la persistance de currentImg
	void setPersistent(bool value);
	//! change l'image ciblée par currentImg
	bool setImage(const std::string &name);
	//! modifie la transparence de currentImg
	void setAlpha(float alpha, float duration);
	//! modifie la taille de currentImg
	void setScale(float scale, float duration);
	//! modifie l'angle de rotation de currentImg
	void setRotation(float rotation, float duration);
	//! modifie la position de currentImg sur le dome
	void setLocation(float xpos, bool deltax, float ypos, bool deltay, float duration, bool accelerate_x = false, bool decelerate_x = false, bool accelerate_y = false, bool decelerate_y = false);
	//! modifie le ratio de currentImg
	void setRatio(float ratio, float duration);

	//! active la suppression de couleur
	void setTransparency(bool v);
	//! détermine la couleur de l'image à effacer au tracé
	void setKeyColor(const Vec3f& _color, float _intensity);

	//! mise à jour des fader des images
	void update(int delta_time);
	//! affiche toutes les images du conteneur
	void draw(const Navigator * nav, Projector * prj);

	//! clone une image afin de l'afficher en double ou en triple
	// void clone(const std::string& _name, int i);

	//! convertit une chaine de caractère en enum IMAGE_POSITIONING
	IMAGE_POSITIONING  convertStrToPosition( const std::string & coordinate) const;
private:
	Image * currentImg=nullptr;
	std::list<std::unique_ptr<Image>> active_images;
	std::map<std::string , IMAGE_POSITIONING> strToPos;
};


#endif // _IMAGE_MGR_H
