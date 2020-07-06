FILES = main.cpp PImage.cpp P5Image.cpp P8Image.cpp BitCluster.cpp DataCompress.cpp
BINARY = convert.out

INCLUDES =
FLAGS =
DEBUG_FLAGS = -g3 -Wall -Wextra

all:
	g++ -o $(BINARY) $(FLAGS) $(FILES) $(INCLUDES)

clean:
	rm -f vgcore*

fclean:	clean
	rm -f $(BINARY)

re:	fclean	all

debug:
	echo -en "\ec"
	g++ -o $(BINARY) $(FLAGS) $(FILES) $(INCLUDES)

c:
	echo -en "\ec"
