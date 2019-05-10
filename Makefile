

all:
	g++ -c main.cpp -std=c++11 `pkg-config --cflags --libs gtk+-3.0` -rdynamic
	g++ main.o -o Main -std=c++11 `pkg-config --cflags --libs gtk+-3.0` -rdynamic
clean:
	rm main.o Main