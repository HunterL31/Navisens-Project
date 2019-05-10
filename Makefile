

all:
	g++ -c main.cpp -std=c++11
	g++ main.o -o Main -std=c++11

clean:
	rm main.o Main