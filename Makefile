CFLAGS = -g -lm -pedantic

all: ./bin ./bin/controller.exe ./bin/test.exe ./bin/TestFeedback.exe
	rm *.o
./bin: 
	mkdir bin

./bin/controller.exe: Controller.o Sim.o ML.o -lm
	gcc -o ./bin/controller.exe -fopenmp Controller.o Sim.o ML.o $(CFLAGS)

./bin/test.exe: test.o Sim.o ML.o FeedbackNN.o
	gcc -o ./bin/test.exe test.o Sim.o ML.o FeedbackNN.o $(CFLAGS)

./bin/TestFeedback.exe: TestFeedback.o Sim.o FeedbackNN.o
	gcc -fopenmp -o ./bin/TestFeedback.exe TestFeedback.o Sim.o FeedbackNN.o $(CFLAGS)

TestFeedback.o: ./src/TestFeedback.c ./src/Sim.h ./src/FeedbackNN.h ./src/defs.h
	gcc -c -fopenmp ./src/TestFeedback.c $(CFLAGS)

test.o: ./src/test.c ./src/Sim.h ./src/defs.h
	gcc -c ./src/test.c $(CFLAGS)

Sim.o: ./src/Sim.c ./src/Sim.h ./src/defs.h
	gcc -c ./src/Sim.c $(CFLAGS)

ML.o: ./src/ML.c ./src/ML.h ./src/defs.h
	gcc -c ./src/ML.c $(CFLAGS)

Controller.o : ./src/Controller.c ./src/Sim.h ./src/ML.h ./src/defs.h
	gcc -c -fopenmp ./src/Controller.c $(CFLAGS)

FeedbackNN.o: ./src/FeedbackNN.c ./src/FeedbackNN.h ./src/MathUtil.c ./src/defs.h
	gcc -c ./src/FeedbackNN.c $(CFLAGS)
