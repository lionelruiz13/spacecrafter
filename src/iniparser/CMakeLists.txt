#
# construction de la library iniparser
#

cmake_minimum_required(VERSION 3.0)


project(iniparser)


set(SRCS
	dictionary.c
	iniparser.c
    )
    
set(HEADERS
    dictionary.h
    iniparser.h
    )

add_library(iniparser ${SRCS} ${HEADERS})
