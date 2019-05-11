LIBS=-lsfml-graphics -lsfml-window -lsfml-system

all:
	g++ -rdynamic -c main.cpp -std=c++11
	g++ -rdynamic main.o -o Main $(LIBS) -std=c++11
	./Main user1.csv user2.csv
clean:
	rm main.o Main