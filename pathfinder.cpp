#include "maze.h"
#include <curses.h>
#include <thread>
#include <mutex>

int rows;
int cols;
std::pair<int,int> end;
std::pair<int,int> start;



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

void test_print(char ** m, char * sentence){
    printw(sentence);
    printw("\n");
    printw("Press 'd' to fast forward, and 'a' to slow down.\n");
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    for (size_t i = 0; i<rows; ++i){
        for (size_t j = 0; j<cols; ++j){
            if (m[i][j] == 'X')
                attron(COLOR_PAIR(3));
            else if (m[i][j] == 'O')
                attron(COLOR_PAIR(1));
            else if (m[i][j] == 'V')
                attron(COLOR_PAIR(4));
            else
                attron(COLOR_PAIR(2));
            addch(m[i][j]);
        }
        printw("\n");
    }
    refresh();
}


std::mutex speed_lock;
int curr_speed = 500000;



//naieve bfs--could be optimized--but its unnecessary.
void BFSTestQueue(Maze &m){
    char ** maze = (char**)malloc(m.rows*sizeof(char*));
    for (int i = 0; i<m.rows; ++i)
        maze[i] = (char*)malloc(m.cols*sizeof(char*)), strncpy(maze[i], m.maze[i], m.cols);

    auto inMaze = [&] (std::pair<int,int> & current){
        std::vector<std::pair<int,int>> wasd;
        if (current.first-1>=0 && maze[current.first-1][current.second]!='#'){
            wasd.push_back({current.first-1,current.second});
        }
        if (current.second-1>=0 && maze[current.first][current.second-1]!='#'){
            wasd.push_back({current.first,current.second-1});
        }
        if (current.first+1<rows&& maze[current.first+1][current.second]!='#'){
            wasd.push_back({current.first+1,current.second});
        }
        if (current.second+1<cols&& maze[current.first][current.second+1]!='#'){
            wasd.push_back({current.first,current.second+1});
        }
        return wasd;
    };

    std::set<std::pair<int,int>> visited;
    rows = m.rows;
    cols = m.cols;
    std::unordered_map<std::pair<int,int>, std::pair<int,int>, locHash> route;
    std::queue<std::pair<int,int>> toVisit;
    start = m.start;
    end   = m.end;

    toVisit.push(start);
    char ch;
    while(ch = getch() && !toVisit.empty()){

        if(ch!=ERR && ch == 'd')
            curr_speed-=100000;
        else if (ch!=ERR && ch == 'a')
            curr_speed+=100000;

        auto location = toVisit.front();
        toVisit.pop();
        visited.insert(location);

        if (location!=start)
            maze[location.first][location.second] = 'O';

        if (location == end)
            break;
        
        auto possible_moves = inMaze(location);

        for (auto move : possible_moves){
            if (visited.count(move)==0){
                route[move] = location;
                toVisit.push(move);
                maze[move.first][move.second] = 'V';
            }
        }

        //critical section
        speed_lock.lock();
        usleep(curr_speed);
        speed_lock.unlock();

        erase();
        test_print(maze, "Breadth-First-Search with a Queue (sped up 10X comparative to A*)");
    }
    auto location = route[end];
    for (int i = 0; i<m.rows; ++i)
        strncpy(maze[i], m.maze[i], m.cols);
    while (location!=start)
        maze[location.first][location.second] = 'X', location = route[location];
    erase();
    test_print(maze, "Breadth-First-Search with a Queue");
    usleep(3000000);

}

//naieve dfs--could be optimized--but its unnecessary.
void DFSTestStack(Maze &m){
    char ** maze = (char**)malloc(m.rows*sizeof(char*));
    for (int i = 0; i<m.rows; ++i)
        maze[i] = (char*)malloc(m.cols*sizeof(char*)), strncpy(maze[i], m.maze[i], m.cols);

    auto inMaze = [&] (std::pair<int,int> & current){
        std::vector<std::pair<int,int>> wasd;
        if (current.first-1>=0 && maze[current.first-1][current.second]!='#'){
            wasd.push_back({current.first-1,current.second});
        }
        if (current.second-1>=0 && maze[current.first][current.second-1]!='#'){
            wasd.push_back({current.first,current.second-1});
        }
        if (current.first+1<rows&& maze[current.first+1][current.second]!='#'){
            wasd.push_back({current.first+1,current.second});
        }
        if (current.second+1<cols&& maze[current.first][current.second+1]!='#'){
            wasd.push_back({current.first,current.second+1});
        }
        return wasd;
    };

    std::set<std::pair<int,int>> visited;
    rows = m.rows;
    cols = m.cols;
    std::unordered_map<std::pair<int,int>, std::pair<int,int>, locHash> route;
    std::stack<std::pair<int,int>> toVisit;
    start = m.start;
    end   = m.end;

    toVisit.push(start);
    char ch;
    while(ch=getch()&&!toVisit.empty()){

        if(ch!=ERR && ch == 'd')
            curr_speed-=100000;
        else if (ch!=ERR && ch == 'a')
            curr_speed+=100000;

        auto location = toVisit.top();
        toVisit.pop();
        visited.insert(location);

        if (location!=start)
            maze[location.first][location.second] = 'O';

        if (location == end)
            break;
        
        auto possible_moves = inMaze(location);

        for (auto move : possible_moves){
            if (visited.count(move)==0){
                route[move] = location;
                toVisit.push(move);
                maze[move.first][move.second] = 'V';
            }
        }

        //critical section
        speed_lock.lock();
        usleep(curr_speed);
        speed_lock.unlock();
        erase();
        test_print(maze, "Depth-First-Search with a Stack");
    }
    auto location = route[end];
    for (int i = 0; i<m.rows; ++i)
        strncpy(maze[i], m.maze[i], m.cols);
    while (location!=start)
        maze[location.first][location.second] = 'X', location = route[location];
    erase();
    test_print(maze, "Depth-First-Search with a Stack");
    usleep(3000000);

}

void AStarTest(Maze &m){
    //simple priority queue for the frontier of elements.
    //double is the priority, and the pair is the location.

    char ** maze = (char**)malloc(m.rows*sizeof(char*));
    for (int i = 0; i<m.rows; ++i)
        maze[i] = (char*)malloc(m.cols*sizeof(char*)), strncpy(maze[i], m.maze[i], m.cols);

    auto inMaze = [&] (std::pair<int,int> & current){
        std::vector<std::pair<int,int>> wasd;
        if (current.first-1>=0 && maze[current.first-1][current.second]!='#'){
            wasd.push_back({current.first-1,current.second});
        }
        if (current.second-1>=0 && maze[current.first][current.second-1]!='#'){
            wasd.push_back({current.first,current.second-1});
        }
        if (current.first+1<rows&& maze[current.first+1][current.second]!='#'){
            wasd.push_back({current.first+1,current.second});
        }
        if (current.second+1<cols&& maze[current.first][current.second+1]!='#'){
            wasd.push_back({current.first,current.second+1});
        }
        return wasd;
    };
    rows = m.rows;
    cols = m.cols;
    
    PQ<std::pair<int,int>, double> frontier;

    std::unordered_map<std::pair<int,int>,double, locHash> costs; 
    std::unordered_map<std::pair<int,int>,std::pair<int,int>, locHash> generatedPath; 

    start = m.start;
    end   = m.end;

    frontier.push(start, 0.0);
    costs[start] = 0.0;
    generatedPath[start] = start;

    double current_cost;
    double current_priority;
    char ch;
    while (ch = getch()){

        if(ch!=ERR && ch == 'd')
            curr_speed-=100000;
        else if (ch!=ERR && ch == 'a')
            curr_speed+=100000;


        auto location = frontier.pop();
        
        if (location!=start)
            maze[location.first][location.second] = 'O';

        if (location == end)
            break;
        
        auto possible_moves = inMaze(location);

        for (auto move : possible_moves){
            //the current cost at location, plus the random cost to move.
            current_cost = costs[location];
            
            if (costs.find(move)==costs.end() || current_cost < costs[move]){
                costs[move] = current_cost;
                current_priority = current_cost + euclidean_distance(move);
                frontier.push(move, current_priority);
                generatedPath[move] = location;
                maze[move.first][move.second] = 'V';
            }
        }

        //critical section
        speed_lock.lock();
        usleep(curr_speed);
        speed_lock.unlock();

        erase();
        test_print(maze, "Unweighted A* implementation");
    }

    usleep(2000000);

    //final_print

    auto location = generatedPath[end];
    for (int i = 0; i<m.rows; ++i)
        strncpy(maze[i], m.maze[i], m.cols);
    while (location!=start)
        maze[location.first][location.second] = 'X', location = generatedPath[location];
    erase();
    test_print(maze, "Unweighted A* implementation");
    usleep(3000000);    
}


int main(int argc, char**argv){
    Maze M = Maze(40,43);
    WINDOW * win;
    win = initscr();
    nodelay(win,TRUE);
    refresh();
    start_color();
    cbreak();
    noecho();
    curs_set(0);
    AStarTest(M);    
    erase();
    refresh();
    DFSTestStack(M);    
    erase();
    refresh();
    BFSTestQueue(M);
    erase();
    refresh();
    endwin();
    return 0;
}