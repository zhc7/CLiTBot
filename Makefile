.out/main.exe:main.cpp interface.cpp interface.h CLiTBot.h
	g++ main.cpp interface.cpp -o .out/main.exe -std=c++17

.PHONY:test
.ONESHELL:
test:
	g++ main.cpp interface.cpp -o .out/main.exe -std=c++17 -g
