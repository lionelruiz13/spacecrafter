/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 of Association Sirius
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

#include "interfaceModule/script_interface.hpp"
#include "interfaceModule/script_mgr.hpp"

ScriptInterface::ScriptInterface(ScriptMgr* _scriptMgr)
{
    scriptMgr = _scriptMgr;
    SelectedScript = SelectedScriptDirectory = "";
}

ScriptInterface::~ScriptInterface()
{}

bool ScriptInterface::isScriptPlaying() const {
	return scriptMgr->isPlaying();
}

bool ScriptInterface::isScriptRecording() const {
	return scriptMgr->isRecording();
}

bool ScriptInterface::isScriptPaused() const {
    return scriptMgr->isPaused();
}

void ScriptInterface::slowerSpeed() {
    return scriptMgr->slowerSpeed();
}

void ScriptInterface::fasterSpeed() {
    return scriptMgr->fasterSpeed();
}

void ScriptInterface::defaultSpeed() {
    return scriptMgr->defaultSpeed();
}

void ScriptInterface::resumeScript() {
    scriptMgr->resumeScript();
}

void ScriptInterface::pauseScript() {
    scriptMgr->pauseScript();
}

// void ScriptInterface::resetScriptTimer() {
//     scriptMgr->resetTimer();
// }
void ScriptInterface::resetScriptLoop() {
    scriptMgr->resetScriptLoop();
}

void ScriptInterface::cancelScript() {
    scriptMgr->cancelScript();
}

void ScriptInterface::cancelRecordScript() {
    scriptMgr->cancelRecordScript();
}

void ScriptInterface::initScriptIterator() {
    scriptMgr->initIterator();
}

void ScriptInterface::setScriptNbrLoop(int a) {
    scriptMgr->setNbrLoop(a);
}

bool ScriptInterface::playScript(const std::string& _script) {
    return scriptMgr->playScript(_script);
}

bool ScriptInterface::addScriptFirst(const std::string& _script) {
    return scriptMgr->addScriptFirst(_script);
}

void ScriptInterface::recordScript(const std::string &script_filename){
    scriptMgr->recordScript(script_filename);
}

void ScriptInterface::recordCommand(const std::string &commandline){
    scriptMgr->recordCommand(commandline);
}

void ScriptInterface::setScriptLoop(bool _value) {
    scriptMgr->setLoop(_value);
}

void ScriptInterface::setScriptPath(const std::string& _path) {
    scriptMgr->setPath(_path);
}

std::string ScriptInterface::getScriptPath() const {
	return scriptMgr->getScriptPath();
}

std::string ScriptInterface::getScriptList(const std::string &directory) const {
    return scriptMgr->getScriptList(directory);
}