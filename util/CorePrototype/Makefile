CC=g++
CPPFLAGS= -Wall -g -Wextra 
DEPS = 

LIBS= -lm


OBJ = main.o app.o core.o ui.o AppCommandInterface.o coreLink.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

main : $(OBJ)
	$(CC) -o $@ $^ $(CPPFLAGS) $(LIBS)

clean:
	rm *.o main