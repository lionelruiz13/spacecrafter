/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2026 Association Sirius
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
 */

#include <algorithm>
#include <cctype>
#include <iterator>
#include <SDL2/SDL_scancode.h>
#include "uiModule/key_binding.hpp"
#include "uiModule/joypad_controller.hpp"
#include "uiModule/ui.hpp"
#include "eventModule/event_recorder.hpp"
#include "eventModule/EventScript.hpp"

static const std::pair<KeyBinding::Base, uint8_t> ANY_KEY{{KeyBinding::Source::KEY, SDL_SCANCODE_UNKNOWN}, NONE};

// Name each base key by its SDL_SCANCODE_ suffix and each modifier by its MODIFIER identifier
#define KEY(x) {#x, SDL_SCANCODE_##x}
static const struct {
	const char *name;
	SDL_Scancode code;
} SCANCODES[] = {
	KEY(A), KEY(B), KEY(C), KEY(D), KEY(E), KEY(F), KEY(G), KEY(H), KEY(I), KEY(J), KEY(K), KEY(L), KEY(M),
	KEY(N), KEY(O), KEY(P), KEY(Q), KEY(R), KEY(S), KEY(T), KEY(U), KEY(V), KEY(W), KEY(X), KEY(Y), KEY(Z),
	KEY(1), KEY(2), KEY(3), KEY(4), KEY(5), KEY(6), KEY(7), KEY(8), KEY(9), KEY(0), KEY(RETURN), KEY(ESCAPE),
	KEY(BACKSPACE), KEY(TAB), KEY(SPACE), KEY(MINUS), KEY(EQUALS), KEY(LEFTBRACKET), KEY(RIGHTBRACKET),
	KEY(BACKSLASH), KEY(NONUSHASH), KEY(SEMICOLON), KEY(APOSTROPHE), KEY(GRAVE), KEY(COMMA), KEY(PERIOD),
	KEY(SLASH), KEY(CAPSLOCK), KEY(F1), KEY(F2), KEY(F3), KEY(F4), KEY(F5), KEY(F6), KEY(F7), KEY(F8), KEY(F9),
	KEY(F10), KEY(F11), KEY(F12), KEY(PRINTSCREEN), KEY(SCROLLLOCK), KEY(PAUSE), KEY(INSERT), KEY(HOME),
	KEY(PAGEUP), KEY(DELETE), KEY(END), KEY(PAGEDOWN), KEY(RIGHT), KEY(LEFT), KEY(DOWN), KEY(UP),
	KEY(NUMLOCKCLEAR), KEY(KP_DIVIDE), KEY(KP_MULTIPLY), KEY(KP_MINUS), KEY(KP_PLUS), KEY(KP_ENTER), KEY(KP_1),
	KEY(KP_2), KEY(KP_3), KEY(KP_4), KEY(KP_5), KEY(KP_6), KEY(KP_7), KEY(KP_8), KEY(KP_9), KEY(KP_0),
	KEY(KP_PERIOD), KEY(NONUSBACKSLASH), KEY(APPLICATION), KEY(POWER), KEY(KP_EQUALS), KEY(F13), KEY(F14),
	KEY(F15), KEY(F16), KEY(F17), KEY(F18), KEY(F19), KEY(F20), KEY(F21), KEY(F22), KEY(F23), KEY(F24),
	KEY(EXECUTE), KEY(HELP), KEY(MENU), KEY(SELECT), KEY(STOP), KEY(AGAIN), KEY(UNDO), KEY(CUT), KEY(COPY),
	KEY(PASTE), KEY(FIND), KEY(MUTE), KEY(VOLUMEUP), KEY(VOLUMEDOWN), KEY(KP_COMMA), KEY(KP_EQUALSAS400),
	KEY(INTERNATIONAL1), KEY(INTERNATIONAL2), KEY(INTERNATIONAL3), KEY(INTERNATIONAL4), KEY(INTERNATIONAL5),
	KEY(INTERNATIONAL6), KEY(INTERNATIONAL7), KEY(INTERNATIONAL8), KEY(INTERNATIONAL9), KEY(LANG1), KEY(LANG2),
	KEY(LANG3), KEY(LANG4), KEY(LANG5), KEY(LANG6), KEY(LANG7), KEY(LANG8), KEY(LANG9), KEY(ALTERASE),
	KEY(SYSREQ), KEY(CANCEL), KEY(CLEAR), KEY(PRIOR), KEY(RETURN2), KEY(SEPARATOR), KEY(OUT), KEY(OPER),
	KEY(CLEARAGAIN), KEY(CRSEL), KEY(EXSEL), KEY(KP_00), KEY(KP_000), KEY(THOUSANDSSEPARATOR),
	KEY(DECIMALSEPARATOR), KEY(CURRENCYUNIT), KEY(CURRENCYSUBUNIT), KEY(KP_LEFTPAREN), KEY(KP_RIGHTPAREN),
	KEY(KP_LEFTBRACE), KEY(KP_RIGHTBRACE), KEY(KP_TAB), KEY(KP_BACKSPACE), KEY(KP_A), KEY(KP_B), KEY(KP_C),
	KEY(KP_D), KEY(KP_E), KEY(KP_F), KEY(KP_XOR), KEY(KP_POWER), KEY(KP_PERCENT), KEY(KP_LESS),
	KEY(KP_GREATER), KEY(KP_AMPERSAND), KEY(KP_DBLAMPERSAND), KEY(KP_VERTICALBAR), KEY(KP_DBLVERTICALBAR),
	KEY(KP_COLON), KEY(KP_HASH), KEY(KP_SPACE), KEY(KP_AT), KEY(KP_EXCLAM), KEY(KP_MEMSTORE),
	KEY(KP_MEMRECALL), KEY(KP_MEMCLEAR), KEY(KP_MEMADD), KEY(KP_MEMSUBTRACT), KEY(KP_MEMMULTIPLY),
	KEY(KP_MEMDIVIDE), KEY(KP_PLUSMINUS), KEY(KP_CLEAR), KEY(KP_CLEARENTRY), KEY(KP_BINARY), KEY(KP_OCTAL),
	KEY(KP_DECIMAL), KEY(KP_HEXADECIMAL), KEY(LCTRL), KEY(LSHIFT), KEY(LALT), KEY(LGUI), KEY(RCTRL),
	KEY(RSHIFT), KEY(RALT), KEY(RGUI), KEY(MODE), KEY(AUDIONEXT), KEY(AUDIOPREV), KEY(AUDIOSTOP),
	KEY(AUDIOPLAY), KEY(AUDIOMUTE), KEY(MEDIASELECT), KEY(WWW), KEY(MAIL), KEY(CALCULATOR), KEY(COMPUTER),
	KEY(AC_SEARCH), KEY(AC_HOME), KEY(AC_BACK), KEY(AC_FORWARD), KEY(AC_STOP), KEY(AC_REFRESH),
	KEY(AC_BOOKMARKS), KEY(BRIGHTNESSDOWN), KEY(BRIGHTNESSUP), KEY(DISPLAYSWITCH), KEY(KBDILLUMTOGGLE),
	KEY(KBDILLUMDOWN), KEY(KBDILLUMUP), KEY(EJECT), KEY(SLEEP), KEY(APP1), KEY(APP2), KEY(AUDIOREWIND),
	KEY(AUDIOFASTFORWARD), KEY(SOFTLEFT), KEY(SOFTRIGHT), KEY(CALL), KEY(ENDCALL)
};
#undef KEY

#define MOD(x) {#x, x}
static const struct {
	const char *name;
	uint8_t plane;
} MODIFIERS[] = {MOD(CTRL), MOD(KWIN), MOD(SUPER), MOD(SHIFT), MOD(ALT)};
#undef MOD

bool KeyBinding::bind(const std::string &token, bool release, const std::string &script, Scope scope, std::string &reason)
{
	Key key;
	if (!parse(token, key)) {
		reason = "unknown key '" + token + "': write [CTRL+][SHIFT+][KWIN+][SUPER+][ALT+] then an SDL scancode name "
		         "without SDL_SCANCODE_, such as SHIFT+KP_8; a joystick button or hat direction as BUTTON3 or HAT0_UP, "
		         "no prefix and no _ALT (a binding serves both sets of the joystick's ini); or * for every key";
		return false;
	}
	auto &events = chains[release];
	if (scope != Scope::GLOBAL) {
		events[key].push_back({script, scope});
		return true;
	}
	if (key == ANY_KEY)
		events.clear();
	else
		events.erase(key);
	if (!script.empty())
		events[key] = {{script, scope}};
	return true;
}

bool KeyBinding::resolve(Base base, uint8_t plane, bool pressed)
{
	auto press = held.find(base);
	if (pressed && press != held.end() && base.first == Source::KEY)
		return press->second.taken;
	if (!pressed) {
		if (press == held.end())
			return false;
		plane = press->second.plane;
		held.erase(press);
	}
	auto &events = chains[!pressed];
	auto chain = events.find({base, plane});
	if (chain == events.end())
		chain = events.find(ANY_KEY);
	bool taken = false;
	if (chain != events.end()) {
		const Entry top = chain->second.back();
		if (top.scope == Scope::ONCE) {
			chain->second.pop_back();
			if (chain->second.empty())
				events.erase(chain);
		}
		if (!top.script.empty()) {
			EventRecorder::getInstance()->queue(new ScriptEvent(top.script, name({base, plane})));
			taken = true;
		}
	}
	if (pressed)
		held[base] = {plane, taken};
	return taken;
}

void KeyBinding::endScript()
{
	for (auto &events : chains) {
		for (auto chain = events.begin(); chain != events.end();) {
			auto &entries = chain->second;
			entries.erase(std::remove_if(entries.begin(), entries.end(),
			                             [](const Entry &entry) { return entry.scope == Scope::SCRIPT; }), entries.end());
			chain = entries.empty() ? events.erase(chain) : std::next(chain);
		}
	}
}

bool KeyBinding::parse(const std::string &token, Key &key)
{
	if (token == "*") {
		key = ANY_KEY;
		return true;
	}
	std::string rest = token;
	std::transform(rest.begin(), rest.end(), rest.begin(), ::toupper);
	if (JoypadController::parse(rest, key.first)) {
		key.second = NONE;
		return true;
	}
	uint8_t plane = NONE;
	for (std::size_t plus = rest.find('+'); plus != std::string::npos; plus = rest.find('+')) {
		const std::string prefix = rest.substr(0, plus);
		rest.erase(0, plus + 1);
		auto modifier = std::find_if(std::begin(MODIFIERS), std::end(MODIFIERS),
		                             [&](const auto &m) { return prefix == m.name; });
		if (modifier == std::end(MODIFIERS))
			return false;
		plane |= modifier->plane;
	}
	for (const auto &scancode : SCANCODES) {
		if (rest == scancode.name) {
			key = {{Source::KEY, scancode.code}, plane};
			return true;
		}
	}
	return false;
}

std::string KeyBinding::name(const Key &key)
{
	if (key.first.first != Source::KEY)
		return JoypadController::name(key.first);
	std::string token;
	for (const auto &m : MODIFIERS) {
		if (key.second & m.plane)
			token += std::string(m.name) + "+";
	}
	for (const auto &scancode : SCANCODES) {
		if (scancode.code == key.first.second)
			return token + scancode.name;
	}
	return std::string();
}
