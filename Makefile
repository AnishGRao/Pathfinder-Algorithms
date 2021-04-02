all:
	g++ -Ofast -w maze.h pathfinder.cpp -lncurses -ldl -pthread -o main
clean:
	rm -f main