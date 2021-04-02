all:
	g++ -g3 maze.h pathfinder.cpp -o main
clean:
	rm -f main