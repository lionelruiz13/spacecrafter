#
# construction de la library planetsephems 
#

cmake_minimum_required(VERSION 3.0)


project(planetsephems)

set(SRCS
	vsop87.c
	pluto.c
	l1.c
	tass17.c
	gust86.c
	marssat.c
	elp82b.c
	elliptic_to_rectangular.c
	misc_stellplanet.c
	calc_interpolated_elements.c
	sideral_time.c
	stellplanet.c
    )
    
set(HEADERS
	vsop87.h
	pluto.h
	l1.h
	gust86.h
	marssat.h
	tass17.h  
	elp82b.h  
	elliptic_to_rectangular.h
	misc_stellplanet.h
	calc_interpolated_elements.h
	sideral_time.h
	stellplanet.h 
    )

add_library(planetsephems ${SRCS} ${HEADERS})
