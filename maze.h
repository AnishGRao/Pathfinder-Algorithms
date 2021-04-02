#include <bits/stdc++.h>
#include <curses.h>
#include <cstdlib>
#include <unistd.h>
#include <time.h>
// a '#' is a wall, and a ' ' is a free space. 

class Maze{
    public:
        size_t rows;
        size_t cols;
        std::pair<int,int> start;
        std::pair<int,int> end;
        char ** maze;
        
        //default intitialization
        //100 x 100.
        Maze(int _rows = 100, int _cols = 100) : 
            rows(_rows), cols(_cols)
        {

            srand(time(NULL));
            maze = (char**)malloc(rows*sizeof(char*));
            for (size_t i = 0; i<cols; ++i)
                maze[i] = (char*)malloc(cols*sizeof(char*));


            //make a start and end point that are distinct.
            start = (std::pair<int,int>){rand()%rows, rand()%cols};
            usleep(rand()%(rows+cols));
            end = (std::pair<int,int>){rand()%rows, rand()%cols};
            while (end == start)
                end = (std::pair<int,int>){rand()%rows, rand()%cols};
            generateMaze();
        }
        
    
        std::vector<std::pair<int,int>> inMaze (std::pair<int,int> & current){
            std::vector<std::pair<int,int>> wasd;
            if (current.first-1>=0){
                wasd.push_back({current.first-1,current.second});
            }
            if (current.second-1>=0){
                wasd.push_back({current.first,current.second-1});
            }
            if (current.first+1<rows){
                wasd.push_back({current.first+1,current.second});
            }
            if (current.second+1<cols){
                wasd.push_back({current.first,current.second+1});
            }
            return wasd;
        }

        //neglect sqrt due to its slowness, and that it is asinine here.
        double euclidean_distance(std::pair<int,int> & current){
            return (current.first-end.first)*(current.first-end.first) + (current.second-end.second)*(current.second-end.second);
        }
        struct locHash {
            public:
            template <typename T, typename U>
            std::size_t operator()(const std::pair<T, U> &x) const{
                return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
            }
        };
        //wrapper for priority queue that makes it less ugly
        template<typename T, typename p>
        struct PQ{
            std::priority_queue<std::pair<p,T>, std::vector<std::pair<p,T>>, 
                std::greater<std::pair<p,T>> > pq; 
            
            [[nodiscard]] inline bool empty(){
                return pq.empty();
            }

            inline void push(T val, p priority){
                pq.emplace(priority, val);
            }

            [[nodiscard]] T pop(){
                T ret = pq.top().second;
                pq.pop();
                return ret;
            }
        };
    private:
        void weightedAStarGeneration(){
            //seed with pseudorandom weights: [0,255).
            for (size_t i = 0; i<rows; ++i)
                for (size_t j = 0; j<cols; ++j)
                    maze[i][j] = (rand()%100);
            
            //unweighted begin and end.
            maze[start.first][start.second] = maze[end.first][end.second] = 0;

            //simple priority queue for the frontier of elements.
            //double is the priority, and the pair is the location.
            PQ<std::pair<int,int>, double> frontier;

            std::unordered_map<std::pair<int,int>,double, locHash> costs; 
            
            frontier.push(start, 0.0);
            costs[start] = 0.0;
            generatedPath[start] = start;
            double current_cost;
            double current_priority;

            while (!frontier.empty()){
                auto location = frontier.pop();

                if (location == end)
                    return;
                
                auto possible_moves = inMaze(location);

                for (auto move : possible_moves){
                    //the current cost at location, plus the random cost to move.
                    current_cost = costs[location] + maze[move.first][move.second];
                    
                    if (costs.find(move)==costs.end() || current_cost < costs[move]){
                        costs[move] = current_cost;
                        current_priority = current_cost + euclidean_distance(move);
                        frontier.push(move, current_priority);
                        generatedPath[move] = location;
                    }

                }
            }
        }

        void generateMaze(){
            

            //path is of type: std::unordered_map<std::pair<int,int>, std::pair<int,int>>
            weightedAStarGeneration();
            auto location = generatedPath[end];
            while (location!=start)
                maze[location.first][location.second] = 'X', location = generatedPath[location];
            
            for (size_t i = 0; i<rows; ++i)
                for (size_t j = 0; j<cols; ++j)
                    if (maze[i][j]!='X')
                        maze[i][j] = rand()%2 ? ' ' : '#';
                    else
                        maze[i][j] = ' ';
                

            maze[start.first][start.second] = 'S';
            maze[end.first][end.second] = 'E';

            //test_print();
            //std::cout<< "\n\nMaze with >=1 solutions generated." << std::endl;
        }
        void generateMazeDebug(){
            

            //path is of type: std::unordered_map<std::pair<int,int>, std::pair<int,int>>
            weightedAStarGeneration();
            auto location = generatedPath[end];
            while (location!=start)
                maze[location.first][location.second] = 'X', location = generatedPath[location];
            
            for (size_t i = 0; i<rows; ++i)
                for (size_t j = 0; j<cols; ++j)
                    if (maze[i][j]!='X')
                        maze[i][j] = rand()%2 ? ' ' : '#';
                    //else
                    //    maze[i][j] = ' '
                

            maze[start.first][start.second] = 'S';
            maze[end.first][end.second] = 'E';

            //test_print();
            //std::cout<< "\n\nMaze with >=1 solutions generated." << std::endl;
        }
        std::unordered_map<std::pair<int,int>,std::pair<int,int>, locHash> generatedPath;
        uint seed;

};