/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2021 Jérémy Calvo
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

#ifndef _SSYSTEM_ITERATOR_
#define _SSYSTEM_ITERATOR_

#include <map>
#include "solarsystem.hpp"

class SolarSystem;

class SSystemIterator {
public:
    typedef typename std::map<std::string, struct SolarSystem::BodyContainer *>::iterator iter_type;

    SSystemIterator(SolarSystem* p_data);

    void operator ++ (int)
    {
        this->next();
    }

    bool operator== (SSystemIterator right);
    bool operator!= (SSystemIterator right);

    void begin();
    void last();

    void next();

    bool end();

    iter_type current();

    private:
    SolarSystem *pointer;
    iter_type m_it_;
};

#endif