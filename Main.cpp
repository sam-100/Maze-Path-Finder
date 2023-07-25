#include <iostream>
#include <vector>
#include <queue>
#include <unordered_set>
#include <math.h>
#include <unistd.h>

using namespace std;

// Symbols to be displayed on the screen 
#define SYMBOL_EMPTY '.'
#define SYMBOL_WALL '#'
#define SYMBOL_START 'S'
#define SYMBOL_END 'E'
#define SYMBOL_CURSER '+'
#define SYMBOL_EXPLORED '@'
#define SYMBOL_VISITED '*'

// Buffer clear bits for Game Class 
#define BUFFER_BIT_EXPLORED 1<<0
#define BUFFER_BIT_VISITED 1<<1
#define BUFFER_BIT_COST 1<<2
#define BUFFER_BIT_PARENT 1<<3
#define BUFFER_ALL_BIT BUFFER_BIT_EXPLORED | BUFFER_BIT_VISITED | BUFFER_BIT_COST | BUFFER_BIT_PARENT

// Class (or struct) declaration section -->
struct Position
{
    int row, col;

    Position(int r=-1, int c=-1);
    bool operator == (const Position &pos);
    Position operator - (const Position &second_pos) const;
    friend std::ostream& operator<<(std::ostream& os, const Position& pos);
};
class Node
{
private:
    bool is_walkable;
    bool visited, explored;
    Node *parent;
    Position pos;
    float gCost, hCost;
    
public:
    Node();
    bool isWalkable() const;
    void setPosition(Position pos);

    friend class NodeHandle;
};
class NodeHandle
{
    Node* node;
    static NodeHandle *start, *end;
public:

    NodeHandle(Node *ptr=NULL);
    NodeHandle(Node& node);
    float getGCost() const;
    float getHCost() const;
    float getFCost() const;
    Node* getParent();
    Position getPosition() const;
    void insertWall();
    bool isExplored();
    bool isVisited() const;
    bool isWalkable() const;
    void markAsExplored(bool val = true);
    void markAsVisited(bool val = true);
    void operator = (Node* node_ptr);
    void operator = (Node& node);
    bool operator == (const NodeHandle &second) const;
    bool operator != (const NodeHandle &second) const;
    bool operator < (const NodeHandle &second) const;
    bool pointsTo(const Node &node) const;
    void removeWall();
    void setGCost(float cost);
    void setHCost(float cost);
    void setNodeHandle(Node &node);
    void setParent(NodeHandle);
    void setPosition(Position pos);
    void toggle();
    friend class Game;

};

NodeHandle* NodeHandle::start=NULL;
NodeHandle* NodeHandle::end=NULL;

class Result
{
    string algorithm;
    int search_cost, path_cost;
    string status;

public:
    Result();
    void reset();
    void incSearchCost();
    void incPathCost();
    void display();
    void setSuccess();
    void setFailure();
    void setAlgorithm(string algo);

};

class Game
{
private:
    Node** board;
    int size;
    bool should_close;
    NodeHandle curser, start, end;
    enum GameEnum {EDIT, PATH_FINDING, MENU, SETTINGS} gameMode;
    enum CurserMode {SELECT, INSERT_WALL, REMOVE_WALL} curserMode;
    bool diagonalMovesAllowed;
    Result result;


public:
    Game(int size=10);
    void applyCurser();
    void aStarSearch();
    bool breadthFirstSearch();
    void changeCurserMode(CurserMode mode);
    void clean();
    void clearBuffer(int buffer_clear_bit);
    bool depthFirstSearch(NodeHandle &curr);
    bool bestFirstSearch();
    void display();
    void displayEditControls();
    void displayEditUI();
    void displayEditMode();
    void displayGameState();
    void displayPath();
    void enterEditMode();
    void exitGame();
    void findPath();
    static float getChessBoardDistance(const NodeHandle src, const NodeHandle end);
    string getCurserMode();
    static float getEuclidianDistance(const NodeHandle src, const NodeHandle end);
    void getInput();
    static float getManhattanDistance(const NodeHandle src, const NodeHandle end);
    bool greedyBestFirstSearch();
    vector<NodeHandle> getNeighbours(const NodeHandle &curr);
    bool isOutOfBounds(Position curr) const;
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void putEnd();
    void putStart();
    void retracePath();
    bool shouldClose();
    void updateNeighbourCost(NodeHandle curr);
};


// Main program logic -->
int main(int argc, char** argv)
{

    // Create a game object and initialize it 
    Game game(30);

    // Rendering Loop 
    while(!game.shouldClose())
    {
        // clear the screen 
        system("clear");

        // Display game and menu
        game.display();
        
        // Get input and execute them 
        game.getInput();                // blocking call
    }


    // Clean and Exit the game 
    game.clean();


    return 0;
}

// Helper class/struct
struct NodeHandleHashFunction
{
    int operator() (const NodeHandle &nodeHandle) const
    {
        return (nodeHandle.getPosition().row)^(nodeHandle.getPosition().col);
    }
};

struct NodeHandleComparatorFCost
{
    bool operator() (const NodeHandle &lhs, const NodeHandle &rhs) const 
    {
        if(lhs.getFCost() > rhs.getFCost())
            return true;
        else if((lhs.getFCost() == rhs.getFCost()) && (lhs.getHCost() > rhs.getHCost()) )
            return true;
        else 
            return false;
    }
};

struct NodeHandleComparatorGCost
{
    bool operator() (const NodeHandle &lhs, const NodeHandle &rhs) const 
    {
        return lhs.getGCost() > rhs.getGCost();
    }
};

struct NodeHandleComparatorHCost
{
    bool operator() (const NodeHandle &lhs, const NodeHandle &rhs) const 
    {
        return lhs.getHCost() > rhs.getHCost();
    }
};


// Game Method definations --> 
Game::Game(int size)
{
    this->size=size;
    should_close = false;

    diagonalMovesAllowed = true;

    // Create the board
    board = new Node*[size];
    for(int i=0; i<size; i++)
    {
        board[i] = new Node[size];
        for(int j=0; j<size; j++)
        {
            board[i][j].setPosition(Position(i, j));            
        }
    }


    // Initialize the board 
    start.setNodeHandle(board[size/5][size/5]);
    end.setNodeHandle(board[size/3][size/2]);
    curser.setNodeHandle(board[size/2][size/2]);

    NodeHandle::start = &start;
    NodeHandle::end = &end;

    // set cost of start as zero
    start.setGCost(0);    

}
void Game::applyCurser()
{
    if(curserMode == CurserMode::INSERT_WALL)
        curser.insertWall();
    else if(curserMode == CurserMode::REMOVE_WALL)
        curser.removeWall();
}
void Game::aStarSearch()
{
    priority_queue<NodeHandle, vector<NodeHandle>,  NodeHandleComparatorFCost> openList;
    unordered_set<NodeHandle, NodeHandleHashFunction> open;
    openList.push(start);

    while(!openList.empty())
    {

        NodeHandle curr = openList.top();
        openList.pop();
        open.erase(curr);
        curr.markAsExplored();

        result.incSearchCost();

        // Display the progress and add a delay 
        system("clear");
        cout<<"Finding a path ... "<<endl;
        displayGameState();
        usleep(50000);


        // if current is the target node 
            // return 
        if(curr == end)
        {
            retracePath();
            result.setSuccess();
            return;
        }
        
        // for each neighbour of the current node 
            // if neighbour is not traversable OR neighbour is in Closed 
            //     skip to the next neighbour 
            
            // if new path to neighbour is shorter OR neighbour is not in Open
            //     set f_cost of neighbour 
            //     set parent of neighbour 
            //     if neighbour is not in Open  
            //         add neighbour to Open  
        vector<NodeHandle> neighbourList = getNeighbours(curr);
        for(int i=0; i<neighbourList.size(); i++)
        {
            NodeHandle &neighbour = neighbourList[i];

            if(!neighbour.isWalkable() || neighbour.isExplored())
                continue;
            
            float new_cost_to_neighbour;
            new_cost_to_neighbour = curr.getGCost() + getChessBoardDistance(curr, neighbour);
            if(new_cost_to_neighbour < neighbour.getGCost() || open.find(neighbour) == open.end())
            {
                neighbour.setGCost(new_cost_to_neighbour);
                neighbour.setParent(curr);
                // no need to set f cost as it is calculated dynamically 
                if(open.find(neighbour) == open.end())
                {
                    openList.push(neighbour);
                    open.insert(neighbour);
                }
            }
        }

    }

    result.setFailure();
}

bool Game::breadthFirstSearch()
{
    // Declare and Initialize data-structures
    queue<NodeHandle> que;
    unordered_set<NodeHandle, NodeHandleHashFunction> open;
    que.push(start);
    open.insert(start);
    start.markAsExplored();

    while(!que.empty())
    {
        // get the first node from the list
        NodeHandle curr = que.front();

        // increment the search cost
        result.incSearchCost();

        
        // Display the progress and add a delay 
        system("clear");
        cout<<"Finding a path ... "<<endl;
        displayGameState();
        usleep(50000);
            
        
        // check if its the end node 
        if(curr == end) 
        {
            retracePath();
            result.setSuccess();
            return true;
        }
        
        // update its neighbours
        updateNeighbourCost(curr);

        // push its neighbours 
        vector<NodeHandle> neighbours = getNeighbours(curr);
        for(int i=0; i<neighbours.size(); i++)
        {
            
            if(!neighbours[i].isWalkable() || open.find(neighbours[i]) != open.end() || neighbours[i].isExplored()) 
                continue;
            
            que.push(neighbours[i]);
            open.insert(neighbours[i]);
        }

        // pop the current node
        que.pop();
        open.erase(curr);

        // Mark as explored
        curr.markAsExplored();

        
    }    

    result.setFailure();
    return false;

}
void Game::changeCurserMode(CurserMode mode)
{
    curserMode = mode;
    applyCurser();
}
void Game::clean() 
{

}
void Game::clearBuffer(int buffer_clear_bit)
{
    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
            NodeHandle curr = board[i][j];
            if(buffer_clear_bit & BUFFER_BIT_EXPLORED)
                curr.markAsExplored(false);
            if(buffer_clear_bit & BUFFER_BIT_VISITED)
                curr.markAsVisited(false);
            if(buffer_clear_bit & BUFFER_BIT_COST)
                curr.setGCost(100000);
            if(buffer_clear_bit & BUFFER_BIT_PARENT)
                curr.setParent(NULL);
        }
    }
    start.setGCost(0);
}
bool Game::depthFirstSearch(NodeHandle &curr)
{
    // Display the progress and add a delay 
    system("clear");
    curr.markAsExplored();
    cout<<"Finding a path ... "<<endl;
    result.incSearchCost();
    displayGameState();
    usleep(50000);


    // Implement the algorithm here
    if(curr == end)
    {
        retracePath();
        result.setSuccess();
        return true;
    }


    // update the neighbours 
    updateNeighbourCost(curr);

    // Recursive call on all directions
    vector<NodeHandle> neighbours = getNeighbours(curr);
    for(int i=0; i<neighbours.size(); i++)
    {
        if(!neighbours[i].isWalkable() || neighbours[i].isExplored())
            continue;
            
        if(depthFirstSearch(neighbours[i])) {
            return true;
        }

    }
    
    // If not found, return false
    result.setFailure();
    return false;
}
bool Game::bestFirstSearch()
{
    // Declare and Initialize data-structures
    priority_queue<NodeHandle, vector<NodeHandle>, NodeHandleComparatorGCost> que;
    unordered_set<NodeHandle, NodeHandleHashFunction> open;
    que.push(start);
    open.insert(start);
    start.markAsExplored();

    while(!que.empty())
    {
        // pop the first node 
        NodeHandle curr = que.top();
        que.pop();
        open.erase(curr);
        curr.markAsExplored();
        
        // Display the progress and add a delay 
        system("clear");
        cout<<"Finding a path ... "<<endl;
        displayGameState();
        usleep(50000);
        result.incSearchCost();
            
        
        // check if its the end node 
        if(curr == end) 
        {
            retracePath();
            result.setSuccess();
            return true;
        }
        
        // update its neighbours
        updateNeighbourCost(curr);

        // push its neighbours 
        vector<NodeHandle> neighbours = getNeighbours(curr);
        for(int i=0; i<neighbours.size(); i++)
        {            
            if(!neighbours[i].isWalkable() || open.find(neighbours[i]) != open.end() || neighbours[i].isExplored())
                continue;
            
            que.push(neighbours[i]);
            open.insert(neighbours[i]);
        }
        
    }    

    result.setFailure();
    return false;
}

void Game::display()
{
    // Create a buffer 
    char **buffer = new char*[size];
    for(int i=0; i<size; i++)
    {
        buffer[i] = new char[size];
    }

    // Set the buffer
    cout<<"\t***Game Board***\t"<<endl;
    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
            buffer[i][j] = board[i][j].isWalkable() ? SYMBOL_EMPTY : SYMBOL_WALL;
        }
    }
    buffer[start.getPosition().row][start.getPosition().col] = SYMBOL_START;
    buffer[end.getPosition().row][end.getPosition().col] = SYMBOL_END;

    // Print the buffer 
    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
            cout<<buffer[i][j]<<" ";
        }
        cout<<endl;
    }

    cout<<endl;
}
void Game::displayEditControls()
{
    cout<<">> Controls: "<<endl;
    cout<<"1. Move controls:\t\tw - Up;\t\ts - Down;\ta - Left;\td - Right."<<endl;
    cout<<"2. Change Curser Mode: \tx - Insert Wall;\tz - Remove Wall;\tc - Select Cell."<<endl;
    cout<<"3. Replace Start and End: \tq - Put start node;\te - Put end node."<<endl;
    cout<<endl;
}
void Game::displayEditUI()
{
    cout << ">> Legend:\t" << endl;
    cout << ". : Empty Cell" << endl;
    cout << "# : Wall" << endl;
    cout << "@ : Explored Cell" << endl;
    cout << "$ : Path to traverse" << endl;
    cout << "S : Start Node" << endl;
    cout << "E : End Node" << endl;
    cout << "+ : Curser Position." << endl;
    cout<<endl;
}

void Game::displayEditMode()
{
    // Create a buffer 
    char **buffer = new char*[size];
    for(int i=0; i<size; i++)
    {
        buffer[i] = new char[size];
    }

    // Set the buffer
    cout<<"\t***Game Board***\t"<<endl;
    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
            buffer[i][j] = board[i][j].isWalkable() ? SYMBOL_EMPTY : SYMBOL_WALL;
        }
    }
    buffer[start.getPosition().row][start.getPosition().col] = SYMBOL_START;
    buffer[end.getPosition().row][end.getPosition().col] = SYMBOL_END;
    buffer[curser.getPosition().row][curser.getPosition().col] = SYMBOL_CURSER;    // just added the curser 
    
    // Print the buffer 
    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
            cout<<buffer[i][j]<<" ";
        }
        cout<<endl;
    }
    cout<<"Curser mode: "<<getCurserMode()<<endl;

    cout<<endl;
}
void Game::displayGameState()
{
    // Create a buffer 
    char **buffer = new char*[size];
    for(int i=0; i<size; i++)
    {
        buffer[i] = new char[size];
    }

    // Set the buffer
    cout<<"\t***Game Board***\t"<<endl;
    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
            NodeHandle curr = board[i][j];
            if(curr.isExplored())
                buffer[i][j] = SYMBOL_EXPLORED;
            else
                buffer[i][j] = board[i][j].isWalkable() ? SYMBOL_EMPTY : SYMBOL_WALL;
        }
    }

    buffer[start.getPosition().row][start.getPosition().col] = SYMBOL_START;
    buffer[end.getPosition().row][end.getPosition().col] = SYMBOL_END;

    // Print the buffer 
    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
            cout<<buffer[i][j]<<" ";
        }
        cout<<endl;
    }

    cout<<endl;
}
void Game::displayPath()
{
    // Create a buffer 
    char **buffer = new char*[size];
    for(int i=0; i<size; i++)
    {
        buffer[i] = new char[size];
    }

    // Set the buffer
    cout<<"\t***Game Board***\t"<<endl;
    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
            NodeHandle curr=board[i][j];
            if(curr.isVisited())
                buffer[i][j] = SYMBOL_VISITED;
            else
                buffer[i][j] = board[i][j].isWalkable() ? SYMBOL_EMPTY : SYMBOL_WALL;
        }
    }

    buffer[start.getPosition().row][start.getPosition().col] = SYMBOL_START;
    buffer[end.getPosition().row][end.getPosition().col] = SYMBOL_END;

    // Print the buffer 
    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
            cout<<buffer[i][j]<<" ";
        }
        cout<<endl;
    }

    cout<<endl;

}
void Game::enterEditMode()
{
    gameMode = GameEnum::EDIT;
    curserMode = CurserMode::SELECT;

    // Edit loop 
    while(gameMode == GameEnum::EDIT)
    {
        // Clear the screen first 
        system("clear");

        // dipslay board in edit mode 
        cout<<"\t***Edit Mode***\t"<<endl;
        displayEditMode();

        // display edit instructions 
        displayEditControls();
        displayEditUI();
        // get input and execute edit command 
        cout<<"Your Response: ";

        system("stty raw");
        char key = getchar();
        system("stty cooked");

        switch(key)
        {
            // Execute the responses 
            case 'w':
                moveUp();
                break;
            case 'a':
                moveLeft();
                break;
            case 's':
                moveDown();
                break;
            case 'd':
                moveRight();
                break;
            case 'q':
                putStart();
                break;
            case 'e':
                putEnd();
                break;
            case 'x':
                changeCurserMode(CurserMode::INSERT_WALL);
                break;
            case 'z':
                changeCurserMode(CurserMode::REMOVE_WALL);
                break;
            case 'c':
                changeCurserMode(CurserMode::SELECT);
                break;
            case '0':
                gameMode = GameEnum::PATH_FINDING;
                break;
        }


    }

}
void Game::exitGame()
{
    should_close = true;
}
void Game::findPath()
{
    while(gameMode == GameEnum::PATH_FINDING)
    {
        // clear screan 
        system("clear");

        // display the game
        displayPath();
        result.display();

        // clear the buffers
        clearBuffer(BUFFER_ALL_BIT);
        result.reset();

        // display algorithms menu
        cout<<"\t***Chose an Algorithm to Solve The Maze***\t"<<endl;
        cout<<"1. Depth First Search"<<endl;
        cout<<"2. Breadth First Search"<<endl;
        cout<<"3. Best First Search algorithm"<<endl;
        cout<<"4. Greedy Best First Search algorithm"<<endl;
        cout<<"5. A Star algorithm"<<endl;
        cout<<"0. Exit"<<endl;
        cout<<"Enter your choice: ";


        // get input and execute that algorithm 
        char choice;
        system("stty raw");
        choice=getchar();
        system("stty cooked");

        switch(choice)
        {
            case '1':
                result.setAlgorithm("Depth First Search");
                depthFirstSearch(start);
                break;
            case '2':
                result.setAlgorithm("Breadth First Search");
                breadthFirstSearch();
                break;
            case '3':
                result.setAlgorithm("Best First Search");
                bestFirstSearch();
                break;
            case '4':
                result.setAlgorithm("Greedy Best First Search");
                greedyBestFirstSearch();
                break;
            case '5':
                result.setAlgorithm("A star");
                aStarSearch();
                break;
            case '0':
                gameMode = GameEnum::MENU;
                break;
                
        }

        
        
    }

}
float Game::getChessBoardDistance(const NodeHandle src, const NodeHandle dst) 
{
    Position distance = src.getPosition() - dst.getPosition();

    int dx = abs(distance.row);
    int dy = abs(distance.col);

    return sqrt(2.0f)*min(dx, dy) + abs(dx-dy);
}
string Game::getCurserMode()
{
    if(curserMode == CurserMode::INSERT_WALL)
        return "Insesrt Wall";
    else if(curserMode == CurserMode::REMOVE_WALL)
        return "Remove Wall";
    else 
        return "Select";
}
float Game::getEuclidianDistance(const NodeHandle src, const NodeHandle dst) 
{
    Position distance = src.getPosition() - dst.getPosition();
    
    int dx = abs(distance.row);
    int dy = abs(distance.col);

    return sqrt(dx*dx+dy*dy);
}

void Game::getInput()
{
    // display the main menu 
    cout<<"1. Edit Board"<<endl;
    cout<<"2. Find Path"<<endl;
    cout<<"0. Exit"<<endl;

    // ask for choice 
    char choice;
    cout<<"Enter your choice here: ";
    system("stty raw");
    choice=getchar();
    system("stty cooked");

    switch(choice)
    {
        case '1':
            enterEditMode();
            break;
        case '2':
            findPath();
            break;
        case '0':
            exitGame();
            break;
    }

}
float Game::getManhattanDistance(const NodeHandle src, const NodeHandle dst) 
{
    Position distance = src.getPosition() - dst.getPosition();
    return abs(distance.row) + abs(distance.col);
}
bool Game::greedyBestFirstSearch()
{
    // Declare and Initialize data-structures
    priority_queue<NodeHandle, vector<NodeHandle>, NodeHandleComparatorHCost> openList;
    unordered_set<NodeHandle, NodeHandleHashFunction> open;
    openList.push(start);
    open.insert(start);

    while(!openList.empty())
    {
        // pop the first node 
        NodeHandle curr = openList.top();
        openList.pop();
        open.erase(curr);
        curr.markAsExplored();
        result.incSearchCost();
        
        // Display the progress and add a delay 
        system("clear");
        cout<<"Finding a path ... "<<endl;
        displayGameState();
        usleep(50000);
                    
        // check if its the end node 
        if(curr == end) 
        {
            retracePath();
            result.setSuccess();
            return true;
        }
        
        // update its neighbours
        updateNeighbourCost(curr);

        // push its neighbours 
        vector<NodeHandle> neighbours = getNeighbours(curr);
        for(int i=0; i<neighbours.size(); i++)
        {            
            if(!neighbours[i].isWalkable() || neighbours[i].isExplored() || open.find(neighbours[i]) != open.end())
                continue;
            
            openList.push(neighbours[i]);
            open.insert(neighbours[i]);
        }
        
    }    

    result.setFailure();
    return false;
}
vector<NodeHandle> Game::getNeighbours(const NodeHandle &curr)
{
    vector<NodeHandle> neighbourList;
    int dx, dy;
    for(int dx=-1; dx<=1; dx++)
    {
        for(int dy=-1; dy<=1; dy++)
        {
            if(dx == 0 && dy == 0)
                continue;
            
            int x=curr.getPosition().row+dx;
            int y=curr.getPosition().col+dy;
            
            if(isOutOfBounds(Position(x,y)))
                continue;
            
            if(board[x][y].isWalkable())
                neighbourList.push_back(board[x][y]);

        }
    }
    return neighbourList;
}

bool Game::isOutOfBounds(Position curr) const
{
    if(curr.row < 0 || curr.col < 0 || curr.row >= size || curr.col >= size)
        return true;
    return false;
}
void Game::moveUp()
{
    int x, y;
    x = curser.getPosition().row-1;
    y = curser.getPosition().col;

    if(isOutOfBounds(Position(x,y)))
        return;
    
    curser = board[x][y];
    applyCurser();
}
void Game::moveDown()
{
    int x, y;
    x = curser.getPosition().row+1;
    y = curser.getPosition().col;

    if(isOutOfBounds(Position(x,y)))
        return;
    curser = board[x][y];
    applyCurser();
}
void Game::moveLeft()
{
    int x, y;
    x = curser.getPosition().row;
    y = curser.getPosition().col-1;

    if(isOutOfBounds(Position(x,y)))
        return;
    curser = board[x][y];
    applyCurser();
}
void Game::moveRight()
{
    int x, y;
    x = curser.getPosition().row;
    y = curser.getPosition().col+1;

    if(isOutOfBounds(Position(x,y)))
        return;
    curser = board[x][y];
    applyCurser();
}
void Game::putEnd()
{
    if(curser.isWalkable() && curser != start)
        end = curser;
}
void Game::putStart()
{
    if(curser.isWalkable() && curser != end)
        start = curser;
}
void Game::retracePath()
{
    // clear the explored buffer 
    if(end.getParent() == NULL) {
        cout<<"Parent of End node is NULL!"<<endl;
        return;
    }
    
    NodeHandle curr = end.getParent();
    while(curr.getParent() != NULL && curr != start) 
    {
        // display the progress 
        system("clear");
        displayPath();
        
        // add a delay 
        usleep(300000);

        // move to next node
        curr.markAsVisited();
        curr = curr.getParent();
        result.incPathCost();
    }

}
bool Game::shouldClose()
{
    return should_close;
}

void Game::updateNeighbourCost(NodeHandle curr)
{
    vector<NodeHandle> neighbours = getNeighbours(curr);
    for(int i=0; i<neighbours.size(); i++)
    {
        
        if(!neighbours[i].isWalkable())
            continue;
        
        float new_neighbour_cost = curr.getGCost() + getChessBoardDistance(curr, neighbours[i]);
        if(new_neighbour_cost < neighbours[i].getGCost())
        {
            neighbours[i].setGCost(new_neighbour_cost);
            neighbours[i].setHCost(getChessBoardDistance(curr, neighbours[i]));
            neighbours[i].setParent(curr);
        }
    }
}



// Node Method definations --> 
Node::Node()
{
    is_walkable = true;
    visited=false;
    explored = false;
    parent = NULL;
    gCost = 100000;
    hCost = 100000;
}
bool Node::isWalkable() const
{
    return is_walkable;
}
void Node::setPosition(Position pos)
{
    this->pos=pos;
}



// NodeHandle Method definations --> 
NodeHandle::NodeHandle(Node *ptr)
{
    node = ptr;
}
NodeHandle::NodeHandle(Node &node)
{
    this->node = &node;
}
float NodeHandle::getGCost() const
{
    return node->gCost;
}
float NodeHandle::getHCost() const
{
    return Game::getChessBoardDistance(*this, *end);
}
float NodeHandle::getFCost() const
{
    return getGCost()+getHCost();
}

Node* NodeHandle::getParent()
{
    return node->parent;
}
Position NodeHandle::getPosition() const
{
    return node->pos;
}
void NodeHandle::insertWall()
{
    if(*start != *this && *end != *this)
        node->is_walkable = false;
}
bool NodeHandle::isExplored()
{
    return node->explored;
}
bool NodeHandle::isVisited() const
{
    return node->visited;
}
bool NodeHandle::isWalkable() const
{
    return node->is_walkable;
}
void NodeHandle::markAsExplored(bool val)
{
    node->explored = val;
}
void NodeHandle::markAsVisited(bool val)
{
    node->visited = val;
}
void NodeHandle::operator=(Node* node_ptr)
{
    node=node_ptr;
}
void NodeHandle::operator=(Node &other)
{
    node=&other;
}
bool NodeHandle::operator==(const NodeHandle &second) const
{
    if(node == second.node)
        return true;
    return false;
}
bool NodeHandle::operator!=(const NodeHandle &second) const
{
    if(node != second.node)
        return true;
    return false;
}
bool NodeHandle::operator<(const NodeHandle &second) const
{
    if(getPosition().row < second.getPosition().row)
        return true;
    return false;
}
bool NodeHandle::pointsTo(const Node &node) const
{
    if(this->node == &node)
        return true;
    return false;
}
void NodeHandle::removeWall()
{
    node->is_walkable = true;
}
void NodeHandle::setGCost(float cost)
{
    node->gCost=cost;
}
void NodeHandle::setHCost(float cost)
{
    node->hCost = cost;
}
void NodeHandle::setNodeHandle(Node &node)
{
    this->node = &node;
}
void NodeHandle::setParent(NodeHandle new_parent)
{
    node->parent =  new_parent.node;
}
void NodeHandle::setPosition(Position pos)
{
    node->setPosition(pos);
}
void NodeHandle::toggle()
{
    node->is_walkable = !node->is_walkable;
}

// Position Method definations -->
Position::Position(int r, int c)
{
    this->row=r;
    this->col=c;
}
bool Position::operator == (const Position &pos)
{
    if(row == pos.row && col == pos.col)
        return true;
    return false;
}
Position Position::operator - (const Position &second_pos) const
{
    Position distance;
    distance.row = second_pos.row-row;
    distance.col = second_pos.col-col;
    return distance;
}

std::ostream& operator<<(std::ostream& os, const Position& pos)
{
    os << "(" << pos.row << ", " << pos.col << ")";
    return os;
}

Result::Result()
{
    algorithm = "None";
    search_cost = path_cost = 0;
    status = "None";
}
void Result::reset()
{
    algorithm = "None";
    search_cost = path_cost = 0;
    status = "None";
}
void Result::incSearchCost()
{
    search_cost++;
}
void Result::incPathCost()
{
    path_cost++;
}
void Result::display()
{
    cout<<"\t===Result==="<<endl;
    cout<<"Algorithm: "<<algorithm<<endl;
    cout<<"Status: "<<status<<endl;
    cout<<"Search nodes = "<<search_cost<<endl;
    cout<<"Path nodes = "<<path_cost<<endl;
}
void Result::setSuccess()
{
    search_cost--;
    path_cost++;
    status = "Path Found Successfully";
}
void Result::setFailure()
{
    status = "Path Not Found!";
}
void Result::setAlgorithm(string algo)
{
    algorithm = algo;
}


