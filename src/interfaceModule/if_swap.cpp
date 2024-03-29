/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2020 Association_Sirius
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

#include <assert.h>
#include "interfaceModule/if_swap.hpp"
#include "tools/log.hpp"

IfSwap::IfSwap()
{
    m_ifSwapCommand.reserve(10);
    this->reset();
}

IfSwap::~IfSwap()
{
    this->reset();
}

void IfSwap::pop()
{
    //assert(!m_ifSwapCommand.empty());
    // end without if
    if (m_ifSwapCommand.empty()){ //nothing to do
        cLog::get()->write("end without if",LOG_TYPE::L_ERROR, LOG_FILE::SCRIPT);
        return;
    }

    m_ifSwapCommand.pop_back();
    if (m_ifSwapCommand.empty())
        commandSwap = false;
    else
        defineCommandSwap();
}


void IfSwap::push(bool v)
{
    m_ifSwapCommand.push_back(v);
    defineCommandSwap();
}


void IfSwap::reset()
{
    m_ifSwapCommand.clear();
    commandSwap = false;
}


void IfSwap::revert()
{
//   assert(!m_ifSwapCommand.empty());
    // else without if
    if (m_ifSwapCommand.empty()){
        cLog::get()->write("else without if",LOG_TYPE::L_ERROR, LOG_FILE::SCRIPT);
        return;
    }
    m_ifSwapCommand[m_ifSwapCommand.size()-1] = ! m_ifSwapCommand[m_ifSwapCommand.size()-1];
    defineCommandSwap();
}


bool IfSwap::get() const 
{
    return commandSwap;
}


void IfSwap::defineCommandSwap()
{
    //assert(!m_ifSwapCommand.empty());
    for(auto it : m_ifSwapCommand ) {
        if (it==true) {
            commandSwap = true;
            return;
        }
    }
    commandSwap = m_ifSwapCommand[m_ifSwapCommand.size()-1];
}