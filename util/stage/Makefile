FILES = NbrList.cpp \
	main.cpp
INCLUDES = -I include
FLAGS =
BINARY = a.out

all:
	g++ -o $(BINARY) $(FLAGS) $(FILES) $(INCLUDES)

clean:
	rm -f vgcore*

fclean:	clean
	rm -f $(BINARY)

re:	fclean	all

debug:
	echo -en "\ec"
	g++ -g3 -Wall -Wextra -o $(BINARY) $(FLAGS) $(FILES) $(INCLUDES)

c:
	echo -en "\ec"