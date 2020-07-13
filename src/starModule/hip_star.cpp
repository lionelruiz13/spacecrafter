/*
 * The big star catalogue extension to Stellarium:
 * Author and Copyright: Johannes Gajdosik, 2006, 2007
 *
 * Thanks go to Nigel Kerr for ideas and testing of BE/LE star repacking
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 */

#include "starModule/hip_star.hpp"
#include "starModule/hip_star_mgr.hpp"
#include "tools/utility.hpp"


namespace BigStarCatalog {


std::string Star1::getNameI18n(void) const
{
	if (getHip()) {
		const std::string commonNameI18 = HipStarMgr::getCommonName(getHip());
		if (!commonNameI18.empty()) return commonNameI18;
		if (HipStarMgr::getFlagSciNames()) {
			const std::string sciName = HipStarMgr::getSciName(getHip());
			if (!sciName.empty()) return sciName;
			return "HP " + std::to_string(getHip());
		}
	}
	return "";
}


void Star1::print(void)
{
	std::cout << "hip: " << getHip()
	          << ", component_ids: " << getComponentIds()
	          << ", x0: " << getX0()
	          << ", x1: " << getX1()
	          << ", b_v: " << ((unsigned int)getBV())
	          << ", mag: " << ((unsigned int)getMag())
	          << ", sp_int: " << getSpInt()
	          << ", dx0: " << getDx0()
	          << ", dx1: " << getDx1()
	          << ", plx: " << getPlx()
	          << std::endl;
}


void Star2::print(void)
{
	std::cout << "x0: " << getX0()
	          << ", x1: " << getX1()
	          << ", dx0: " << getDx0()
	          << ", dx1: " << getDx1()
	          << ", b_v: " << getBV()
	          << ", mag: " << getMag()
	          << std::endl;
}


void Star3::print(void)
{
	std::cout << "x0: " << getX0()
	          << ", x1: " << getX1()
	          << ", b_v: " << getBV()
	          << ", mag: " << getMag()
	          << std::endl;
}

} // namespace BigStarCatalog



