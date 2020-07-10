CC=g++
CPPFLAGS= -Wall -g -Wextra -std=c++17
DEPS =

LIBS= -lm


OBJ = main.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

main : $(OBJ)
	$(CC) -o $@ $^ $(CPPFLAGS) $(LIBS)

clean:
	rm *.o main
