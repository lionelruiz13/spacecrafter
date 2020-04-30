CC=g++
CPPFLAGS= -Wall -g -Wextra 
DEPS = 

LIBS= -lm


OBJ = main.o app.o core.o ui.o AppCommandInterface.o coreLink.o CoreMode.o CoreMode1.o CoreMode2.o CoreMode3.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

main : $(OBJ)
	$(CC) -o $@ $^ $(CPPFLAGS) $(LIBS)

clean:
	rm *.o main