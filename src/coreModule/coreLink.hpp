#ifndef CORELINK_HPP
#define CORELINK_HPP

#include "coreModule/core.hpp"

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

	////////////////////////////////////////////////////////////////////////////////
	// Time---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set time speed in JDay/sec
	void timeSetSpeed(double ts) {
		core->timeMgr->setTimeSpeed(ts);
	}

	void timeChangeSpeed(double ts, double duration) {
		core->timeMgr->changeTimeSpeed(ts, duration);
	}

	//! Get time speed in JDay/sec
	double timeGetSpeed(void) const {
		return core->timeMgr->getTimeSpeed();
	}

	void timeLoadSpeed(void) const {
		return core->timeMgr->loadTimeSpeed();
	}
	void timeSaveSpeed() const  {
		core->timeMgr-> saveTimeSpeed();
	}

	//! Set the current date in Julian Day
	void setJDay(double JD) {
		core->timeMgr->setJDay(JD);
	}
	//! Get the current date in Julian Day
	double getJDay(void) const {
		return core->timeMgr->getJDay();
	}

	bool timeGetFlagPause() const {
		return core->timeMgr->getTimePause();
	}

	void timeSetFlagPause(bool _value) const {
		core->timeMgr->setTimePause(_value);
	}

	double timeGetMultiplier() const {
		return core->timeMgr->getTimeMultiplier();
	}
	void timeSetMultiplier(double _value) {
		core->timeMgr->setTimeMultiplier(_value);
	}
	void timeResetMultiplier() {
		core->timeMgr->setTimeMultiplier(1.0);
	};

	////////////////////////////////////////////////////////////////////////////////
	// dateSun---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! return the JD time when the sun go down
	double dateSunRise(double _jd, double _longitude, double _latitude) {
		return core->timeMgr->dateSunRise(_jd,_longitude, _latitude);
	}

	//! return the JD time when the sun set up
	double dateSunSet(double _jd, double _longitude, double _latitude) {
		return core->timeMgr->dateSunSet(_jd,_longitude, _latitude);
	}

	//! return the JD time when the sun cross the meridian
	double dateSunMeridian(double _jd, double _longitude, double _latitude) {
		return core->timeMgr->dateSunMeridian(_jd,_longitude, _latitude);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Tully---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void tullySetFlagShow(bool v) {
		core->tully->setFlagShow(v);
	}

	bool tullyGetFlagShow() {
		return core->tully->getFlagShow();
	}

	void tullySetColor(const std::string &colorMode)
	{
		if (colorMode=="white")
			core->tully->setWhiteColor(true);
		if (colorMode=="custom")
			core->tully->setWhiteColor(false);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Stars---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void starSetFlag(bool b) {
		core->hip_stars->setFlagStars(b);
		core->starNav->setFlagStars(b);
	}

	bool starGetFlag(void) const {
		return core->hip_stars->getFlagStars();
	}

	void starSetTraceFlag(bool b) {
		core->hip_stars->setFlagTrace(b);
	}

	bool starGetTraceFlag(void) const {
		return core->hip_stars->getFlagTrace();
	}

	void starSetColorTable(int p, Vec3f a) {
		core->hip_stars->setColorStarTable(p,a);
	}

	void starSetDuration(float f) {
		return core->hip_stars->setFaderDuration(f);
	}

	void starSetFlagName(bool b) {
		core->hip_stars->setFlagNames(b);
	}
	bool starGetFlagName(void) const {
		return core->hip_stars->getFlagNames();
	}

	void starSetLimitingMag(float f) {
		core->hip_stars->setMagConverterMaxScaled60DegMag(f);
	}
	float starGetLimitingMag(void) const {
		return core->hip_stars->getMagConverterMaxScaled60DegMag();
	}

	// Fonctions non utilisée ?
	// -------------------------------
	// void starSetFlagSciName(bool b) {
	// 	core->hip_stars->setFlagSciNames(b);
	// }
	// bool starGetFlagSciName(void) const {
	// 	return core->hip_stars->getFlagSciNames();
	// }

	void starSetFlagTwinkle(bool b) {
		core->hip_stars->setFlagTwinkle(b);
	}
	bool starGetFlagTwinkle(void) const {
		return core->hip_stars->getFlagTwinkle();
	}

	void starSetMaxMagName(float f) {
		core->hip_stars->setMaxMagName(f);
	}
	float starGetMaxMagName(void) const {
		return core->hip_stars->getMaxMagName();
	}

	void starSetSizeLimit(float f) {
		core->starNav->setStarSizeLimit(f);
		core->setStarSizeLimit(f);
	}

	// Fonctions non utilisée ?
	// -------------------------------
	// void starSetMaxMagSciName(float f) {
	// 	core->hip_stars->setMaxMagName(f);
	// }
	// float starGetMaxMagSciName(void) const {
	// 	return core->hip_stars->getMaxMagName();
	// }

	void starSetScale(float f) {
		core->starNav->setScale(f);
		core->hip_stars->setScale(f);
	}
	float starGetScale(void) const {
		return core->hip_stars->getScale();
	}

	void starSetMagScale(float f) {
		core->starNav->setMagScale(f);
		core->hip_stars->setMagScale(f);
	}
	float starGetMagScale(void) const {
		return core->hip_stars->getMagScale();
	}

	void starSetTwinkleAmount(float f) {
		core->hip_stars->setTwinkleAmount(f);
	}
	float  starGetTwinkleAmount(void) const {
		return core->hip_stars->getTwinkleAmount();
	}

	////////////////////////////////////////////////////////////////////////////////
	// StarNavigator---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void starNavigatorClear(){
		core->starNav->clear();
	}

	void starNavigatorLoad(const std::string &fileName, bool binaryMode){
		core->starNav->loadData(fileName, binaryMode);
	}

	void starNavigatorLoadRaw(const std::string &fileName){
		core->starNav->loadRawData(fileName);
	}

	void starNavigatorLoadOther(const std::string &fileName){
		core->starNav->loadOtherData(fileName);
	}

	void starNavigatorSave(const std::string &fileName, bool binaryMode){
		core->starNav->saveData(fileName, binaryMode);
	}

	////////////////////////////////////////////////////////////////////////////////
	// SunTrace---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying SunTrace
	void bodyTraceSetFlag(bool b) const {
		core->bodytrace->setFlagShow(b);
	}
	//! Get flag for displaying SunTrace
	bool bodyTraceGetFlag(void) const {
		return core->bodytrace->getFlagShow();
	}

	void bodyPenUp() const {
		core->bodytrace->upPen();
	}

	void bodyPenDown() const {
		core->bodytrace->downPen();
	}

	void bodyPenToggle() const {
		core->bodytrace->togglePen();
	}

	void bodyTraceClear () const {
		core->bodytrace->clear();
	}

	void bodyTraceHide(std::string value) const {
		if (value=="all")
			core->bodytrace->hide(-1);
		else
			core->bodytrace->hide(Utility::strToInt(value));
	}

	void bodyTraceBodyChange(std::string bodyName) const {
		if (bodyName=="selected") 
			core->ssystem->bodyTraceBodyChange(core->selected_object.getEnglishName()); 
		else 
			core->ssystem->bodyTraceBodyChange(bodyName);
	}

	////////////////////////////////////////////////////////////////////////////////
	// UBO---------------------------
	////////////////////////////////////////////////////////////////////////////////

	void uboSetAmbientLight(float v) {
		core->ubo_cam->setAmbientLight(v);
	}

	float uboGetAmbientLight() {
		return core->ubo_cam->getAmbientLight();
	}

	////////////////////////////////////////////////////////////////////////////////
	// DSO---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! hide a particular DSO
	void dsoSelectName(std::string DSOName, bool hide) const {
		return core->nebulas->selectName(hide, DSOName);
	}

	//! hide all DSO
	void dsoHideAll() const {
		core->nebulas->hideAll();
	}

	//! show (unhide) all DSO
	void dsoShowAll() const {
		core->nebulas->showAll();
	}

	//! select all DSO in constellationName to be hidden or showed
	void dsoSelectConstellation(bool hide, std::string constellationName) const {
		core->nebulas->selectConstellation(hide, constellationName);
	}

	//! select all DSO with typeName to be hidden or showed
	void dsoSelectType(bool hide, std::string typeName) const {
		core->nebulas->selectType(hide, typeName);
	}

	////////////////////////////////////////////////////////////////////////////////
	// FOV ( projection )
	////////////////////////////////////////////////////////////////////////////////

	//! Zoom to the given FOV (in degree)
	void zoomTo(double aim_fov, float move_duration = 1.) {
		core->projection->zoomTo(aim_fov, move_duration);
	}

	//! Get current FOV (in degree)
	float getFov(void) const {
		return core->projection->getFov();
	}

	//! If is currently zooming, return the target FOV, otherwise return current FOV
	double getAimFov(void) const {
		return core->projection->getAimFov();
	}

	//! Set the current FOV (in degree)
	void setFov(double f) {
		core->projection->setFov(f);
	}

	//! Set the maximum FOV (in degree)
	void setMaxFov(double f) {
		core->projection->setMaxFov(f);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Body---------------------------
	////////////////////////////////////////////////////////////////////////////////

	void BodyOJMLoad(const std::string &mode, const std::string &name, const std::string &filename, const std::string &pathFile, const Vec3f &Position, const float multiplier) {
		core->ojmMgr->load(mode, name, filename, pathFile, Position, multiplier);
	}

	void BodyOJMRemove(const std::string &mode, const std::string &name){
		core->ojmMgr->remove(mode, name);
	}

	void BodyOJMRemoveAll(const std::string &mode){
		core->ojmMgr->removeAll(mode);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Camera---------------------------
	////////////////////////////////////////////////////////////////////////////////
	
	bool cameraAddAnchor(stringHash_t& param) {
		return core->anchorManager->addAnchor(param); 
	}

	bool cameraRemoveAnchor(const std::string &name) {
		return core->anchorManager->removeAnchor(name);
	}

	bool cameraSwitchToAnchor(const std::string &name) {
		return core->anchorManager->switchToAnchor(name);
	}

	bool cameraMoveToPoint(double x, double y, double z){
		return core->anchorManager->setCurrentAnchorPos(Vec3d(x,y,z));
	}
	
	bool cameraMoveToPoint(double x, double y, double z, double time){
		return core->anchorManager->moveTo(Vec3d(x,y,z),time);
	}
	
	bool cameraMoveToBody(const std::string& bodyName, double time, double alt = -1.0){

		if(bodyName == "selected"){
			return core->anchorManager->moveToBody(core->getSelectedPlanetEnglishName(), time, alt);
		}

		if(bodyName == "default"){
			return core->anchorManager->moveToBody(core->ssystem->getEarth()->getEnglishName(), time, alt);
		}

		return core->anchorManager->moveToBody(bodyName,time, alt);
	}
	
	bool cameraMoveRelativeXYZ( double x, double y, double z) {
		return core->anchorManager->moveRelativeXYZ(x,y,z);
	}
	
	bool cameraTransitionToPoint(const std::string& name){
		return core->anchorManager->transitionToPoint(name);
	}
	
	bool cameraTransitionToBody(const std::string& name){

		if(name == "selected"){
			return core->anchorManager->transitionToBody(core->getSelectedPlanetEnglishName());
		}

		return core->anchorManager->transitionToBody(name);
	}

	bool cameraSave(const std::string& name = "anchor"){
		return core->anchorManager->saveCameraPosition(core->settings->getUserDir() + "anchors/" + name);
	}
	
	bool loadCameraPosition(const std::string& filename){
		return core->anchorManager->loadCameraPosition(core->settings->getUserDir() + "anchors/" + filename);
	}
	
	bool lookAt(double az, double alt, double time = 1.){
		return core->navigation->lookAt(az, alt, time);
	}
	
	bool cameraSetFollowRotation(const std::string& name, bool value){
		return core->anchorManager->setFollowRotation(value);
	}

	void cameraSetRotationMultiplierCondition(float v) {
		core->anchorManager->setRotationMultiplierCondition(v);
	}

	bool cameraAlignWithBody(const std::string& name, double duration){
		return core->anchorManager->alignCameraToBody(name,duration);
	}

    CoreLink(Core* _core);
    ~CoreLink();

private: 
    Core *core = nullptr;
};

#endif