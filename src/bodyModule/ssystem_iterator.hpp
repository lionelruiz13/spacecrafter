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
#include "protosystem.hpp"

class ProtoSystem;

/**
 * \file solarsystem_iterator.hpp
 * \brief Handle solar system bodies iterator
 * \author Jérémy Calvo
 * \version 1
 *
 * \class SolarSystemIterator
 *
 * \brief Allows to iterate through solar system ssystemBodies map
 *
*/

class SSystemIterator {
public:
    typedef typename std::map<std::string, struct std::shared_ptr<ProtoSystem::BodyContainer>>::iterator iter_type;

    SSystemIterator(ProtoSystem* p_data);

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
    ProtoSystem *pointer;
    iter_type m_it_;
};

class SSystemIteratorVector {
public:
    typedef typename std::vector<std::shared_ptr<ProtoSystem::BodyContainer>>::iterator iter_type;

    SSystemIteratorVector(ProtoSystem* p_data);

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

    inline std::shared_ptr<ProtoSystem::BodyContainer> &current() {
        return *m_it_;
    }

private:
    ProtoSystem *pointer;
    iter_type m_it_;
};

#endif
