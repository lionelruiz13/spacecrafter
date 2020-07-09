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
* Les conditions IF sont évaluées simplement par un boolean qui indique si l'instruction doit etre exécutée ou non.
* Le logiciel recopie toutes les instructions en mémoire avant de les exécuter une à une.
* L'obectif de cette classe est d'indiquer dans une hiérarchie de if imbriqués si l'instruction qui suit doit etre exécutée ou non 
* Elle se réfère au vector m_ifSwapCommand qui contient les différents niveaux de scripts 
* 
* m_ifSwapCommand[i] = true indique que le if n°i est dans la partie ou il réfute les instructions qui suivent, on ne doit pas les executer
* (et donc dans ce cas tous les m_ifSwapCommand[i+1] et suivant sont inutiles)
*
* m_ifSwapCommand[i] = false indique que le if n°i est dans la partie ou il accepte les instructions qui suivent, on doit les executer
*/

class IfSwap : public NoCopy {
public:
    IfSwap();
    ~IfSwap();
    //! décale pour supprimer un ancien if
    void pop();
    //! décale pour construire un nouveau if dans l'état v
    void push(bool v);
    //! remet à zéro toutes les conditions sur les if
    void reset();
    //! permutte la valeur du dernier if 
    void revert();
    //! renvoie la valeur indiquant l'execution de la commande qui suit
    bool get() const ;
private:
    //! fonction qui indique si l'on doit executer les commandes qui sont définies dans un script
    void defineCommandSwap();
    std::vector<bool> m_ifSwapCommand;
    bool commandSwap = false;
};

#endif