.out/main.exe:main.cpp interface.cpp interface.h CLiTBot.h
	g++ main.cpp interface.cpp -o .out/main.exe

.PHONY:test
.ONESHELL:
test:
	g++ main.cpp interface.cpp -o .out/main.exe -g
