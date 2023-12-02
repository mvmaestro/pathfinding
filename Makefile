
output: main.o 
	g++ -std=c++11  main.o -o output -lsdl2 -lsdl2_image

main.o: main.cpp
	g++ -std=c++11  -c main.cpp

run:
	./output
