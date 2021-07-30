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

#include "ssystem_iterator.hpp"

SSystemIterator::SSystemIterator(SolarSystem* p_data) : pointer(p_data) {
    m_it_ = pointer->systemBodies.begin();
}

void SSystemIterator::begin() {
    m_it_ = pointer->systemBodies.begin();
}

void SSystemIterator::last() {
    m_it_ = pointer->systemBodies.end();
}

void SSystemIterator::next() {
    m_it_++;
}

bool SSystemIterator::end() {
    return (m_it_ == pointer->systemBodies.end());
}

std::map< std::string, std::shared_ptr<SolarSystem::BodyContainer>>::iterator SSystemIterator::current() {
    return m_it_;
}