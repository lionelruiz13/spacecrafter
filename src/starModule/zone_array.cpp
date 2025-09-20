/*
 * The big star catalogue extension to Stellarium:
 * Author and Copyright: Johannes Gajdosik, 2006, 2007
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



//#include "spacecrafter.hpp"
#include "coreModule/projector.hpp"
#include "starModule/zone_array.hpp"
#include "starModule/geodesic_grid.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"
#include "tools/object_base.hpp"
#include "tools/s_texture.hpp"
#ifdef __linux__
#include <unistd.h>
#endif

namespace BigStarCatalog {

static const Vec3d north(0,0,1);

void ZoneArray::initTriangle(int index, const Vec3d &c0, const Vec3d &c1, const Vec3d &c2)
{
	// initialize center,axis0,axis1:
	ZoneData &z(zones[index]);
	z.center = c0+c1+c2;
	z.center.normalize();
	z.axis0 = north ^ z.center;
	z.axis0.normalize();
	z.axis1 = z.center ^ z.axis0;
	// initialize star_position_scale:
	double mu0,mu1,f,h;
	mu0 = (c0-z.center)*z.axis0;
	mu1 = (c0-z.center)*z.axis1;
	f = 1.0/sqrt(1.0-mu0*mu0-mu1*mu1);
	h = fabs(mu0)*f;
	if (star_position_scale < h) star_position_scale = h;
	h = fabs(mu1)*f;
	if (star_position_scale < h) star_position_scale = h;
	mu0 = (c1-z.center)*z.axis0;
	mu1 = (c1-z.center)*z.axis1;
	f = 1.0/sqrt(1.0-mu0*mu0-mu1*mu1);
	h = fabs(mu0)*f;
	if (star_position_scale < h) star_position_scale = h;
	h = fabs(mu1)*f;
	if (star_position_scale < h) star_position_scale = h;
	mu0 = (c2-z.center)*z.axis0;
	mu1 = (c2-z.center)*z.axis1;
	f = 1.0/sqrt(1.0-mu0*mu0-mu1*mu1);
	h = fabs(mu0)*f;
	if (star_position_scale < h) star_position_scale = h;
	h = fabs(mu1)*f;
	if (star_position_scale < h) star_position_scale = h;
}



static inline int ReadInt(FILE *f,unsigned int &x)
{
	const int rval = (4 == fread(&x,1,4,f)) ? 0 : -1;
	return rval;
}


#define FILE_MAGIC 0x835f040a
#define FILE_MAGIC_OTHER_ENDIAN 0x0a045f83
#define FILE_MAGIC_NATIVE 0x835f040b
#define MAX_MAJOR_FILE_VERSION 0

// The #warning preprocessor is not implemented on MSVC
// #if (!defined(__GNUC__))
// #warning Star catalogue loading has only been tested with gcc
// #endif

ZoneArray *ZoneArray::create(const HipStarMgr &hip_star_mgr, const std::string& extended_file_name)
{
	std::string fname(extended_file_name);
	bool use_mmap = false;
	if (fname.find("mmap:") != std::string::npos) {
		fname = fname.substr(5);
		use_mmap = true;
	}
	try {
		fname = AppSettings::Instance()->getDataRoot() + "stars/" + fname;
	} catch (std::exception &e) {
		std::cout << "ZoneArray::create(" << extended_file_name << "): warning while loading \"" << fname << "\": " << e.what();
		return 0;
	}
	FILE *f = fopen(fname.c_str(),"rb");
	if (f == 0) {
		fprintf(stderr,"ZoneArray::create(%s): fopen failed\n", extended_file_name.c_str());
		return 0;
	}
	//printf("Loading %s: ",extended_file_name.c_str());
	unsigned int magic,major,minor,type,level,mag_min,mag_range,mag_steps;
	if (ReadInt(f,magic) < 0 ||
	        ReadInt(f,type) < 0 ||
	        ReadInt(f,major) < 0 ||
	        ReadInt(f,minor) < 0 ||
	        ReadInt(f,level) < 0 ||
	        ReadInt(f,mag_min) < 0 ||
	        ReadInt(f,mag_range) < 0 ||
	        ReadInt(f,mag_steps) < 0) {
		printf("bad file\n");
		return 0;
	}
	const bool byte_swap = (magic == FILE_MAGIC_OTHER_ENDIAN);
	if (byte_swap) {
		// ok, FILE_MAGIC_OTHER_ENDIAN, must swap
		if (use_mmap) {
			printf("you must convert catalogue "
			       #if (!defined(__GNUC__))
			       "to native format "
			       #endif
			       "before mmap loading\n");
			return 0;
		}
		printf("byteswap ");
		type = SDL_Swap32(type);
		major = SDL_Swap32(major);
		minor = SDL_Swap32(minor);
		level = SDL_Swap32(level);
		mag_min = SDL_Swap32(mag_min);
		mag_range = SDL_Swap32(mag_range);
		mag_steps = SDL_Swap32(mag_steps);
	} else if (magic == FILE_MAGIC) {
		// ok, FILE_MAGIC
		#if (!defined(__GNUC__))
		if (use_mmap) {
			// mmap only with gcc:
			printf("you must convert catalogue to native format before mmap loading\n");
			return 0;
		}
		#endif
	} else if (magic == FILE_MAGIC_NATIVE) {
		// ok, will work for any architecture and any compiler
	} else {
		printf("no star catalogue file\n");
		return 0;
	}
	ZoneArray *rval = 0;

	std::ostringstream oss;
	oss << extended_file_name << ":" << level << ":" << type << ":" << major << ":" << minor;
	cLog::get()->write( oss.str() , LOG_TYPE::L_INFO);

	switch (type) {
		case 0:
			if (major > MAX_MAJOR_FILE_VERSION) {
				printf("unsupported version, ");
			} else {
				// When this assertion fails you must redefine Star1 for your compiler.
				// Because your compiler does not pack the data, which is crucial for this application.
				assert(sizeof(Star1) == 28);
				rval = new ZoneArray1(f,byte_swap,use_mmap,hip_star_mgr,level, mag_min,mag_range,mag_steps);
				if (rval == 0) {
					printf("no memory, ");
				}
			}
			break;
		case 1:
			if (major > MAX_MAJOR_FILE_VERSION) {
				printf("unsupported version, ");
			} else {
				// When this assertion fails you must redefine Star2 for your compiler.
				// Because your compiler does not pack the data, which is crucial for this application.
				assert(sizeof(Star2) == 10);
				rval = new SpecialZoneArray<Star2>(f,byte_swap,use_mmap,hip_star_mgr, level, mag_min,mag_range,mag_steps);
				if (rval == 0) {
					printf("no memory, ");
				}
			}
			break;
		case 2:
			if (major > MAX_MAJOR_FILE_VERSION) {
				printf("unsupported version, ");
			} else {
				// When this assertion fails you must redefine Star3 for your compiler.
				// Because your compiler does not pack the data, which is crucial for this application.
				assert(sizeof(Star3) == 6);
				rval = new SpecialZoneArray<Star3>(f,byte_swap,use_mmap,hip_star_mgr, level, mag_min,mag_range,mag_steps);
				if (rval == 0) {
					printf("no memory, ");
				}
			}
			break;
		default:
			printf("bad file type, ");
			break;
	}
	if (rval && rval->isInitialized()) {
		std::ostringstream oss;
		oss <<  "stars:  " << rval->getNrOfStars();
		cLog::get()->write( oss.str() , LOG_TYPE::L_INFO);
	} else {
		printf("initialization failed\n");
		if (rval) {
			delete rval;
			rval = 0;
		}
	}
	fclose(f);
	return rval;
}



ZoneArray::ZoneArray(const HipStarMgr &hip_star_mgr,int level, int mag_min,int mag_range,int mag_steps)
	:level(level), mag_min(mag_min),mag_range(mag_range),mag_steps(mag_steps), star_position_scale(0.0), hip_star_mgr(hip_star_mgr), zones(0)
{
	nr_of_zones = GeodesicGrid::nrOfZones(level);
	nr_of_stars = 0;
}

bool ZoneArray::readStarFile(FILE *f,void *data,size_t size)
{
	while (size > 0) {
		const size_t to_read = /*(part_size < size) ? part_size :*/ size;
		const size_t read_rc = fread(data,1,to_read,f);
		if (read_rc != to_read) return false;
		size -= read_rc;
		data = ((char*)data) + read_rc;
	}
	return true;
}

void ZoneArray1::updateHipIndex(HipIndexStruct hip_index[]) const
{
	for (const SpecialZoneData<Star1> *z=getZones()+(nr_of_zones-1); z>=getZones(); z--) {
		for (Star1 *s = z->getStars()+z->size-1; s>=z->getStars(); s--) {
			const int hip = s->getHip();
			if (hip < 0 || NR_OF_HIP < hip) {
				std::cerr << "ERROR: ZoneArray1::updateHipIndex: invalid HP number: " << hip << std::endl;
				exit(1);
			}
			if (hip != 0) {
				hip_index[hip].a = this;
				hip_index[hip].z = z;
				hip_index[hip].s = s;
			}
		}
	}
}

void ZoneArray1::draw(int index,bool is_inside, const float *rcmag_table, Projector *prj, Navigator *nav, int max_mag_star_name, float names_brightness, std::vector<starDBtoDraw> &starNameToDraw, std::map<std::string, bool> &selected_stars,  bool atmosphere, bool isolateSelected) const
{
	auto *const z = getZones() + index;
	Vec3d xy;
	const auto *const end = z->getStars() + z->size;
	const double d2000 = 2451545.0;
	const double movement_factor = (M_PI/180)*(0.0001/3600)
	                               * ((HipStarMgr::getCurrentJDay()-d2000)/365.25)
	                               / star_position_scale;
	for (const auto *s = z->getStars(); s < end; s++) {
		if (s->getHidden())
			continue;
		double alt, az;
		Vec3d starJ2000 = s->getJ2000Pos(z,movement_factor);
		Vec3d local_pos = nav->earthEquToLocal(nav->j2000ToEarthEqu(starJ2000));
		int mag = s->getMag();
		// Correct star position accounting for atmospheric refraction
		if (atmosphere) {
		    Utility::rectToSphe(&az,&alt,local_pos);
		    //float press_temp_corr = (1013.f)/1010.f * 283.f/(273.f+10.f) / 60.f; //temperature and pressure correction based on Stellarium's code
		    const float rad2deg = 180.0f/M_PI;
		    const float deg2rad = M_PI/180.0f;
		    float ha = rad2deg*alt;
			if (ha>-5.0)
				ha += 1.02f/tan((ha+10.3f/(ha+5.11f))*deg2rad)/60.0;
		    	//ha += press_temp_corr * (1.f / tan((ha+7.31f/(ha+4.4f))*deg2rad) + 0.0013515f); //Bennett formula
		    alt = deg2rad*ha;
		    Utility::spheToRect(az, alt, local_pos);
		}
		if (is_inside
		        ? prj->projectLocal(local_pos,xy)
		        : prj->projectLocalCheck(local_pos,xy)) {
			if (hip_star_mgr.drawStar(prj,xy,rcmag_table[2*mag], rcmag_table[2*mag|1], HipStarMgr::color_table[s->getBVIndex()], s->getVariableStarIndex())) {
				break;
			}
			if (!isolateSelected) {
				if (mag < max_mag_star_name) {
					const std::string starname = s->getNameI18n();
					if (!starname.empty()) {
						Vec4f Color(HipStarMgr::color_table[s->getBVIndex()][0]*0.75,
								HipStarMgr::color_table[s->getBVIndex()][1]*0.75,
								HipStarMgr::color_table[s->getBVIndex()][2]*0.75,
								names_brightness);
						// prj->printGravity180(starFont,xy[0],xy[1], starname, Color, true, 4, 4);//, false);
						starNameToDraw.push_back(std::make_tuple(xy[0],xy[1], starname, Color));
					}
				}
			} else {
				const std::string starname = s->getNameI18n();
				if (selected_stars.find(starname) != selected_stars.end()) {
					if (!starname.empty()) {
						Vec4f Color(HipStarMgr::color_table[s->getBVIndex()][0]*0.75,
								HipStarMgr::color_table[s->getBVIndex()][1]*0.75,
								HipStarMgr::color_table[s->getBVIndex()][2]*0.75,
								names_brightness);
						// prj->printGravity180(starFont,xy[0],xy[1], starname, Color, true, 4, 4);//, false);
						starNameToDraw.push_back(std::make_tuple(xy[0],xy[1], starname, Color));
					}
				}
			}
		}
	}
}

template<class Star> SpecialZoneArray<Star>::~SpecialZoneArray(void)
{
	if (stars) {
		#ifdef __linux__
		if (mmap_start != MAP_FAILED) {
			munmap(mmap_start,((char*)stars-(char*)mmap_start) +sizeof(Star)*nr_of_stars);
		} else {
			delete[] stars;
		}
		#else
		if (mmap_start != NULL) {
			CloseHandle(mapping_handle);
		} else {
			delete[] stars;
		}
		#endif /* LINUX */
		stars = 0;
	}
	if (zones) {
		delete[] getZones();
		zones = NULL;
	}
	nr_of_zones = 0;
	nr_of_stars = 0;
}

template<class Star>
void SpecialZoneArray<Star>::draw(int index,bool is_inside, const float *rcmag_table, Projector *prj, Navigator *nav, int max_mag_star_name, float names_brightness, std::vector<starDBtoDraw> &starNameToDraw, std::map<std::string, bool> &selected_stars,  bool atmosphere, bool isolateSelected) const
{
	SpecialZoneData<Star> *const z = getZones() + index;
	Vec3d xy;
	const Star *const end = z->getStars() + z->size;
	const double d2000 = 2451545.0;
	const double movement_factor = (M_PI/180)*(0.0001/3600)
	                               * ((HipStarMgr::getCurrentJDay()-d2000)/365.25)
	                               / star_position_scale;
	for (const Star *s=z->getStars(); s<end; s++) {
		double alt, az;
		Vec3d starJ2000 = s->getJ2000Pos(z,movement_factor);
		Vec3d local_pos = nav->earthEquToLocal(nav->j2000ToEarthEqu(starJ2000));
		int mag = s->getMag();
		// Correct star position accounting for atmospheric refraction
		if (atmosphere) {
		    Utility::rectToSphe(&az,&alt,local_pos);
		    //float press_temp_corr = (1013.f)/1010.f * 283.f/(273.f+10.f) / 60.f; //temperature and pressure correction based on Stellarium's code
		    const float rad2deg = 180.0f/M_PI;
		    const float deg2rad = M_PI/180.0f;
		    float ha = rad2deg*alt;
			if (ha>-5.0)
				ha += 1.02f/tan((ha+10.3f/(ha+5.11f))*deg2rad)/60.0;
		    //r = press_temp_corr * (1.f / tan((ha+7.31f/(ha+4.4f))*deg2rad) + 0.0013515f); //Bennett formula
		    alt = deg2rad*ha;
		    Utility::spheToRect(az, alt, local_pos);
		}
		if (is_inside
		        ? prj->projectLocal(local_pos,xy)
		        : prj->projectLocalCheck(local_pos,xy)) {
			if (hip_star_mgr.drawStar(prj,xy,rcmag_table[2*mag], rcmag_table[2*mag|1], HipStarMgr::color_table[s->getBVIndex()])) {
				break;
			}
			if (!isolateSelected) {
				if (mag < max_mag_star_name) {
					const std::string starname = s->getNameI18n();
					if (!starname.empty()) {
						Vec4f Color(HipStarMgr::color_table[s->getBVIndex()][0]*0.75,
								HipStarMgr::color_table[s->getBVIndex()][1]*0.75,
								HipStarMgr::color_table[s->getBVIndex()][2]*0.75,
								names_brightness);
						// prj->printGravity180(starFont,xy[0],xy[1], starname, Color, true, 4, 4);//, false);
						starNameToDraw.push_back(std::make_tuple(xy[0],xy[1], starname, Color));
					}
				}
			} else {
				const std::string starname = s->getNameI18n();
				if (selected_stars.find(starname) != selected_stars.end()) {
					if (!starname.empty()) {
						Vec4f Color(HipStarMgr::color_table[s->getBVIndex()][0]*0.75,
								HipStarMgr::color_table[s->getBVIndex()][1]*0.75,
								HipStarMgr::color_table[s->getBVIndex()][2]*0.75,
								names_brightness);
						// prj->printGravity180(starFont,xy[0],xy[1], starname, Color, true, 4, 4);//, false);
						starNameToDraw.push_back(std::make_tuple(xy[0],xy[1], starname, Color));
					}
				}
			}
		}
	}
}


template<class Star>
void SpecialZoneArray<Star>::searchAround(int index,const Vec3d &v, double cos_lim_fov, std::vector<ObjectBaseP > &result)
{
	const double d2000 = 2451545.0;
	const double movement_factor = (M_PI/180)*(0.0001/3600)
	                               * ((HipStarMgr::getCurrentJDay()-d2000)/365.25)
	                               / star_position_scale;
	const SpecialZoneData<Star> *const z = getZones()+index;
	for (int i=0; i<z->size; i++) {
		if (z->getStars()[i].getJ2000Pos(z,movement_factor)*v >= cos_lim_fov) {
			result.push_back(z->getStars()[i].createStelObject(this,z));
		}
	}
}

template<class Star>
SpecialZoneArray<Star>::SpecialZoneArray(FILE *f,bool byte_swap,bool use_mmap, const HipStarMgr &hip_star_mgr, int level, int mag_min,int mag_range, int mag_steps)
	:ZoneArray(hip_star_mgr,level, mag_min,mag_range,mag_steps), stars(0),
	 #ifdef __linux__
	 mmap_start(MAP_FAILED)
	 #else
	 mmap_start(NULL), mapping_handle(NULL)
	 #endif /* LINUX */
{
	if (nr_of_zones > 0) {
		zones = new SpecialZoneData<Star>[nr_of_zones];
		if (zones == 0) {
			std::cerr << "ERROR: SpecialZoneArray(" << level << ")::SpecialZoneArray: no memory (1)" << std::endl;
			exit(1);
		}
		{
			unsigned int *zone_size = new unsigned int[nr_of_zones];
			if (zone_size == 0) {
				std::cerr << "ERROR: SpecialZoneArray(" << level << ")::SpecialZoneArray: no memory (2)" << std::endl;
				exit(1);
			}
			if (nr_of_zones != fread(zone_size,sizeof(unsigned int),nr_of_zones,f)) {
				delete[] getZones();
				zones = 0;
				nr_of_zones = 0;
			} else {
				const unsigned int *tmp = zone_size;
				for (unsigned int z=0; z<nr_of_zones; z++,tmp++) {
					const unsigned int tmp_spu_int32 = byte_swap?SDL_Swap32(*tmp):*tmp;
					nr_of_stars += tmp_spu_int32;
					getZones()[z].size = tmp_spu_int32;
				}
			}
			// delete zone_size before allocating stars in order to avoid memory fragmentation:
			delete[] zone_size;
		}

		if (nr_of_stars == 0) {
			// no stars ?
			if (zones) delete[] getZones();
			zones = 0;
			nr_of_zones = 0;
		} else {
			if (use_mmap) {
				const int64_t start_in_file = ftell(f);
				#ifdef __linux__
				const int64_t page_size = sysconf(_SC_PAGE_SIZE);
				#else
				SYSTEM_INFO system_info;
				GetSystemInfo(&system_info);
				const int64_t page_size = system_info.dwAllocationGranularity;
				#endif /* LINUX */
				const int64_t mmap_offset = start_in_file % page_size;
				#ifdef __linux__
				mmap_start = mmap(0,mmap_offset+sizeof(Star)*nr_of_stars,PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno(f),start_in_file-mmap_offset);
				if (mmap_start == MAP_FAILED) {
					std::cerr << "ERROR: SpecialZoneArray(" << level << ")::SpecialZoneArray:  mmap(" << fileno(f) << ',' << start_in_file << ','
							  << (sizeof(Star)*nr_of_stars) << ") failed: " << strerror(errno) << std::endl;
					stars = 0;
					nr_of_stars = 0;
					delete[] getZones();
					zones = 0;
					nr_of_zones = 0;
				} else {
					stars = (Star*)(((char*)mmap_start)+mmap_offset);
					Star *s = stars;
					for (unsigned int z=0; z<nr_of_zones; z++) {
						getZones()[z].stars = s;
						s += getZones()[z].size;
					}
				}
				#else
				HANDLE file_handle = (void*)_get_osfhandle(_fileno(f));
				if (file_handle == INVALID_HANDLE_VALUE) {
					std::cerr << "ERROR: SpecialZoneArray(" << level << ")::SpecialZoneArray: _get_osfhandle(_fileno(f)) failed" << std::endl;
				} else {
					mapping_handle = CreateFileMapping(file_handle,NULL,PAGE_WRITECOPY, 0,0,NULL);
					if (mapping_handle == NULL) {
						// yes, NULL indicates failure, not INVALID_HANDLE_VALUE
						std::cerr << "ERROR: SpecialZoneArray(" << level << ")::SpecialZoneArray: CreateFileMapping failed: " << GetLastError() << std::endl;
					} else {
						mmap_start = MapViewOfFile(mapping_handle,
						                           FILE_MAP_COPY,
						                           0,
						                           start_in_file-mmap_offset,
						                           mmap_offset+sizeof(Star)*nr_of_stars);
						if (mmap_start == NULL) {
							std::cerr << "ERROR: SpecialZoneArray(" << level
							     << ")::SpecialZoneArray: "
							     "MapViewOfFile failed: " << GetLastError()
							     << ", page_size: " << page_size << std::endl;
							stars = 0;
							nr_of_stars = 0;
							delete[] getZones();
							zones = 0;
							nr_of_zones = 0;
						} else {
							stars = (Star*)(((char*)mmap_start)+mmap_offset);
							Star *s = stars;
							for (unsigned int z=0; z<nr_of_zones; z++) {
								getZones()[z].stars = s;
								s += getZones()[z].size;
							}
						}
					}
				}
				#endif /* LINUX */
			} else {
				stars = new Star[nr_of_stars];
				if (stars == 0) {
					std::cerr << "ERROR: SpecialZoneArray(" << level << ")::SpecialZoneArray: no memory (3)" << std::endl;
					exit(1);
				}
				if (!readStarFile(f,stars,sizeof(Star)*nr_of_stars)) {
					delete[] stars;
					stars = 0;
					nr_of_stars = 0;
					delete[] getZones();
					zones = 0;
					nr_of_zones = 0;
				} else {
					Star *s = stars;
					for (unsigned int z=0; z<nr_of_zones; z++) {
						getZones()[z].stars = s;
						s += getZones()[z].size;
					}
				}
			}
		}
	}
}

} // namespace BigStarCatalog
