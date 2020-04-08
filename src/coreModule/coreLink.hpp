#ifndef CORELINK_HPP
#define CORELINK_HPP

#include "coreModule/core.hpp"
#include "coreModule/starLines.hpp"

class CoreLink {
public: 

	////////////////////////////////////////////////////////////////////////////////
	// StarLines---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying
	void starLinesSetFlag(bool b) {
		core->starLines->setFlagShow(b);
	}

	//! Get flag for displaying
	bool starLinesGetFlag(void) const {
		return core->starLines->getFlagShow();
	}

	//! Vide tous les tampons de tracé
	void starLinesDrop(void) const {
		core->starLines->drop();
	}

	//! Charge un ensemble d'asterismes d'un fichier
	void starLinesLoadData(const std::string &fileName) {
		core->starLines->loadData(fileName);
	}

	//! Charge un asterisme à partir d'une ligne
	void starLinesLoadAsterism(std::string record) const {
		core->starLines->loadStringData(record);
	}

	//! supprime le catalogue complet des asterismes
	void starLinesClear() {
		core->starLines->clear();
	}

	void starLinesLoadCat(const std::string &fileName){
		core->starLines->loadHipCatalogue(fileName);
	}

	void starLinesLoadBinCat(const std::string &fileName){
		core->starLines->loadHipBinCatalogue(fileName);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Skyline et Skygrid---------------------------
	////////////////////////////////////////////////////////////////////////////////
    void skyLineMgrSetColor(SKYLINE_TYPE name, Vec3f a) {
	    core->skyLineMgr->setColor(name, a);
    };

    void skyGridMgrSetColor(SKYGRID_TYPE name, Vec3f a) {
		core->skyGridMgr->setColor(name, a);
	}

	const Vec3f& skyLineMgrGetColor(SKYLINE_TYPE name) {
		return core->skyLineMgr->getColor(name);
	}

	const Vec3f& skyGridMgrGetColor(SKYGRID_TYPE name) {
		return core->skyGridMgr->getColor(name);
	}

	void skyLineMgrFlipFlagShow(SKYLINE_TYPE name) {
		core->skyLineMgr->flipFlagShow(name);
	}

	void skyGridMgrFlipFlagShow(SKYGRID_TYPE name) {
		core->skyGridMgr->flipFlagShow(name);
	}

	void skyLineMgrSetFlagShow(SKYLINE_TYPE name, bool value) {
		core->skyLineMgr->setFlagShow(name, value);
	}

	void skyGridMgrSetFlagShow(SKYGRID_TYPE name, bool value) {
		core->skyGridMgr->setFlagShow(name, value);
	}

	bool skyLineMgrGetFlagShow(SKYLINE_TYPE name) {
		return core->skyLineMgr->getFlagShow(name);
	}

	bool skyGridMgrGetFlagShow(SKYGRID_TYPE name) {
		return core->skyGridMgr->getFlagShow(name);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Text_usr---------------------------
	////////////////////////////////////////////////////////////////////////////////

	bool textAdd(std::string name, std::string text, int altitude, int azimuth, std::string textSize, Vec3f &color, int duration) {
		return core->text_usr->add(name, text,altitude, azimuth, textSize, color, duration);
	}

	bool textAdd(std::string name, std::string text, int altitude, int azimuth, std::string textSize, int duration) {
		return core->text_usr->add(name, text,altitude, azimuth, textSize, duration);
	}

	void textDel(std::string name) {
		core->text_usr->del(name);
	}

	void textClear() {
		core->text_usr->clear();
	}

	void textNameUpdate(std::string name, std::string text) {
		core->text_usr->nameUpdate(name, text);
	}

	void textDisplay(std::string name , bool displ) {
		core->text_usr->textDisplay(name, displ);
	}

	void textFadingDuration(int a) {
		core->text_usr->setFadingDuration(a);
	}

	void textSetDefaultColor(const Vec3f& v) {
		core->text_usr->setColor(v);
	}

    CoreLink(Core* _core);
    ~CoreLink();

private: 
    Core *core = nullptr;
};

#endif