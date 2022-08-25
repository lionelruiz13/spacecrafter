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

/* Define swap for multiple if command to app command interface */

#ifndef _IF_SWAP_HPP_
#define _IF_SWAP_HPP_

#include <vector>
#include "tools/no_copy.hpp"

/**
* \file if_swap.hpp
* \brief Define swap for multiple if command to app_command_interface
* \author Olivier NIVOIX
* \version 1
*
* \class IfSwap
*
* \brief Define swap for multiple if command to app command interface
*
* IF conditions are simply evaluated by a boolean that indicates whether the instruction should be executed or not.
* The software copies all the instructions in memory before executing them one by one
* The purpose of this class is to indicate in a hierarchy of nested if statements whether the following statement should be executed or not 
* It refers to the vector m_ifSwapCommand which contains the different levels of scripts 
* 
* m_ifSwapCommand[i] = true indicates that the if n°i is in the part where it refutes the following instructions, they should not be executed
* (and so in this case all m_ifSwapCommand[i+1] and following are useless)
*
* m_ifSwapCommand[i] = false indicates that the if n°i is in the part where it accepts the following instructions, they must be executed
*/

class IfSwap : public NoCopy {
public:
    IfSwap();
    ~IfSwap();
    //! shift to delete an old if
    void pop();
    //! shift to build a new if in state v
    void push(bool v);
    //! resets all conditions on if
    void reset();
    //! swap the value of the last if 
    void revert();
    //! returns the value indicating the execution of the next command
    bool get() const ;
private:
    //! function that indicates whether to execute the commands that are defined in a script
    void defineCommandSwap();
    std::vector<bool> m_ifSwapCommand;
    bool commandSwap = false;
};

#endif