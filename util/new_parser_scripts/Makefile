CC=g++
CPPFLAGS= -Wall -g -Wextra 
DEPS = 

LIBS= -lm


OBJ = main.o FileWriter.o FileReader.o TextToHtml.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

main : $(OBJ)
	$(CC) -o $@ $^ $(CPPFLAGS) $(LIBS)

clean:
	rm *.o main