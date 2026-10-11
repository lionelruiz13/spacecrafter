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

#ifndef _KEY_BINDING_HPP_
#define _KEY_BINDING_HPP_

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <SDL2/SDL_scancode.h>

// Hold the scripts bound on the press and the release of keys, looked up before the native handlers
class KeyBinding {
public:
	enum class Scope : char {GLOBAL, SCRIPT, ONCE};

	// Register a script, empty for the default, on one event of a key; false with the reason for an unknown key
	bool bind(const std::string &key, bool release, const std::string &script, Scope scope, std::string &reason);
	// Run the live binding of this event and return true; false means the default
	bool resolve(SDL_Scancode key, uint8_t plane, bool pressed);
	// Drop every script-scoped binding, the execution ended
	void endScript();

private:
	using Key = std::pair<SDL_Scancode, uint8_t>;
	struct Entry {
		std::string script;
		Scope scope;
	};
	struct Held {
		uint8_t plane;
		bool taken;
	};
	static bool parse(const std::string &token, Key &key);
	static std::string name(const Key &key);

	std::map<Key, std::vector<Entry>> chains[2];	// press, release; the key "*" is SDL_SCANCODE_UNKNOWN
	std::map<SDL_Scancode, Held> held;
};

#endif // _KEY_BINDING_HPP_
