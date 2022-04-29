#include <cstdlib>
#include <sdl.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <deque>
#include <algorithm>
#include <iomanip>
using namespace std;

int init = SDL_Init(SDL_INIT_EVERYTHING);
SDL_Window * win = SDL_CreateWindow("bfs_with_sdl_lab-jesse", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 812, 812, 0);
SDL_Renderer * ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

struct Mouse {
    // a single pixel rectangle for collision detection
    SDL_Rect rect{0, 0, 1, 1};

    void update() {
        //update the position of the rectangle
        SDL_GetMouseState(&rect.x, &rect.y);
    }
};

/* Each square is a node connected to other nodes adjacent to it. Each connected 
node has a pointer pushed back on the cn vector. 
 */

struct Node {
    vector<Node*> cn;
    Node*parent = nullptr;
    bool isWall = false;
    bool visited = false;
    bool isSelected = false;
    SDL_Rect rect{0, 0, 25, 25};
    static SDL_Color selectedColor;
    static SDL_Color normalColor;
    static SDL_Color pathColor;
    static SDL_Color wallColor;
    static SDL_Color discoveredColor;
    static SDL_Color targetColor;
    SDL_Color * currentColor = &normalColor;

    void draw() {
        SDL_SetRenderDrawColor(ren, currentColor->r, currentColor->g, currentColor->b, 255);
        SDL_RenderFillRect(ren, &rect);
    }
};
//Node color definitions
SDL_Color Node::selectedColor{125, 222, 0, 255};
SDL_Color Node::normalColor{200, 98, 0, 255};
SDL_Color Node::discoveredColor{100, 200, 230, 255};
SDL_Color Node::pathColor{65, 163, 0, 255};
SDL_Color Node::wallColor{99, 32, 0, 255};
SDL_Color Node::targetColor{200, 0, 0, 255};

struct Grid {
    static const int SIZE = 30;
    Node* maze[SIZE][SIZE];
    Node* hovered = nullptr;
    vector<Node *> bfsPath;
    Node *start = nullptr;
    Node *target = nullptr;
    Node *current = nullptr;
    deque<Node *> queue;
    //A map showing, which nodes are pathways and walls. traversed and not.

    string positionsForWalls =
            "x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x "
            "x o o o x o x o x o x o o o o o x o o o o o o o o o o o x x "
            "x x x o o o x x x o x x x o o o x o o x o o x x x o o x o x "
            "x o x o x o o o x o x o x o x o o o x o o o x o o o o x o x "
            "x o o o x x x o o o x o o o x x x o o o x o x o o o o x o x "
            "x o x x x o x x x o o o x x x o x x x o o o o x o o o x o x "
            "x o o x o o x o x o x o o o x o x o o o o o o o x o o o o x "
            "x o o x o o x o x x x o o o o o o o o o o x o o x o o o o x "
            "x x o x o o o o o o x x o x x o o o o o o o o o x x x o o x "
            "x o o o o x x x x o o x o o o o x o x o o o o x o o o o o x "
            "x x o o x x o o x x x x x o x o x o x o o o x o o o o o o x "
            "x o o o x o o o o o o x o o x o x o o o o o x o o o o o o x "
            "x x o x x o x x x o x x x o o o x x x o o o x o o o o o o x "
            "x o o o x o o o x o x o x o x o o o x o o o x o o o o o o x "
            "x o o o x o x o o o x o x o x x x o o o o o x o o o x o o x "
            "x o x x x o x x x o x o x o x o x o x x x x x o o o x o o x "
            "x o o o x o x o o o o o o o x o x o o o o o o o o o x o o x "
            "x o o o x o o o o x x o o o x o o o x o o o o o o o x o o x "
            "x o o o x o o o o x x x o o x o x x o o o o o o o x o o o x "
            "x o o o x o o o o o o x o o x o x o o o x o o o x o o o o x "
            "x o o o o o x x o x o x o o x o o x o o x o o x o o o o o x "
            "x o x x x o o o o x o x o o x o o x o o o o x o o o o o o x "
            "x o o o x o o x o x o x o o x o x x o o o o o x x x x x o x "
            "x o o o x o o x o x o o o o x o x o o o x o o x o x o o o x "
            "x o o o o o o x o x x x o o x o x o o o x o o x o x o o o x "
            "x o o x o o o x o o o o o x x o x o o o x x x x x x o o o x "
            "x o o x o o o x o x o x o x o o x x o o o o o x o o o o o x "
            "x o o o o o x o o x o x o x o o o o o o o o o o o o x o o x "
            "x o o x o o o o o x o o o x o o x x o o o o o o o o o o o x "
            "x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x ";

    Grid() {
        stringstream ss(positionsForWalls, ios::in);
        //create the nodes
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                char buffer;
                Node *n = new Node();
                ss >> buffer;
                if (buffer == 'x') {
                    n->isWall = true;
                }
                maze[j][i] = n;
            }
        }
        //connect the current node to the surrounding nodes
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                if ((i - 1) >= 0) { //If the current node is at least on the second row, add that node, for the left node
                    maze[i][j]->cn.push_back(maze[i - 1][j]);
                }
                if ((i + 1) <= SIZE - 1) {//If the current node is at least on the second last row, add that node, for the right node
                    maze[i][j]->cn.push_back(maze[i + 1][j]);
                }
                if ((j - 1) >= 0) {//if the current node is at least on the second highest row, add that node, for the up node
                    maze[i][j]->cn.push_back(maze[i][j - 1]);
                }
                if ((j + 1) <= SIZE - 1) {//if the current node is at least on the second lowest row, add that node, for the down node
                    maze[i][j]->cn.push_back(maze[i][j + 1]);
                }
            }
        }
    }

    ~Grid() {
        //Deallocate the pointers
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                delete maze[i][j];
            }
        }
        cout << "Nodes cleared\n";
    }

    void bfs(Grid * grid) {
        // BFS CODE
        grid->current = grid->start;
        grid->queue.push_back(grid->current);
        while (!grid->queue.empty() && grid->current != grid->target) {
            grid->queue.pop_front();
            for (int x = 0; x < 4; x++) {
                if (grid->current->cn[x]->isWall == false && grid->current->cn[x]->visited == false) {
                    grid->queue.push_back(grid->current->cn[x]);
                    grid->current->cn[x]->parent = grid->current;
                    grid->current->cn[x]->visited = true;
                    if (grid->current->cn[x] != grid->target && grid->current->cn[x] != grid->start) {
                        grid->current->cn[x]->currentColor = &Node::discoveredColor;
                    }
                }
            }
            grid->current = grid->queue.front();
        }
        // RETURN FROM IMPOSSIBLE SEARCH
        if (grid->current != grid->target) {
            grid->reset();
            return;
        }
        // PUSH PARENT SEARCH NODES ONTO PATH
        while (grid->current != grid->start) {
            if (grid->current != grid->target) {
                grid->bfsPath.push_back(grid->current);
            }
            grid->current = grid->current->parent;
        }
        // DISPLAY PATH
        for (int x = 0; x < grid->bfsPath.size(); x++) {
            grid->bfsPath[x]->currentColor = &Node::pathColor;
        }
    }

    void draw() {
        //draw the grid based on the size of a node's rectangle
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                maze[i][j]->rect.x = i * (maze[i][j]->rect.w + 2) + 2;
                maze[i][j]->rect.y = j * (maze[i][j]->rect.w + 2) + 2;
                if (maze[i][j]->isWall) {

                    maze[i][j]->currentColor = &Node::wallColor;
                }
                maze[i][j]->draw();
            }
        }
    }

    void reset() {
        bfsPath.clear();
        queue.clear();
        start = nullptr;
        target = nullptr;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Node* n = maze[i][j];
                n->parent = nullptr;
                if (n->isWall)
                    n->currentColor = &Node::wallColor;

                else n->currentColor = &Node::normalColor;
                n->visited = false;
            }
        }

    }

};

int main(int argc, char** argv) {
    /* TO SELECT THE ROOT NODE: LEFT CLICK. TO SELECT THE TARGET NODE: MIDDLE CLICK.
     * TO RESET THE GRAPH: RIGHT CLICK.
     */

    Grid grid;
    Mouse mouse;
    bool running = true;

    // main loop
    while (running) {
        mouse.update();
        SDL_Event e;
        // event loop
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    SDL_Quit();
                    return 0;
                case SDL_MOUSEBUTTONUP:
                    // mouse click handler		    
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        // RESET OLD GRAPH
                        if (grid.start != nullptr) {
                            grid.reset();
                        }
                        // check collision between last hovered rectangle and mouse point
                        if (SDL_HasIntersection(&grid.hovered->rect, &mouse.rect)) {
                            // make changes to selected node (SELECT ROOT NODE)
                            grid.hovered->isSelected = true;
                            grid.hovered->currentColor = &Node::selectedColor;
                            grid.start = grid.hovered;
                            // DESELECT OVERLAPPING NODE
                            if (grid.start == grid.target) {
                                grid.target = nullptr;
                            }
                        }
                    } else if (e.button.button == SDL_BUTTON_RIGHT) {
                        if (SDL_HasIntersection(&grid.hovered->rect, &mouse.rect)) {
                            //RESET THE GRAPH
                            grid.reset();

                        }
                    } else if (e.button.button == SDL_BUTTON_MIDDLE) {
                        // RESET OLD GRAPH
                        if (grid.target != nullptr) {
                            grid.reset();
                        }
                        // check collision between last hovered rectangle and mouse point
                        if (SDL_HasIntersection(&grid.hovered->rect, &mouse.rect)) {
                            // make changes to selected node (SELECT TARGET NODE)
                            grid.hovered->isSelected = true;
                            grid.hovered->currentColor = &Node::targetColor;
                            grid.target = grid.hovered;
                            // DESELECT OVERLAPPING NODE
                            if (grid.start == grid.target) {
                                grid.start = nullptr;
                            }
                        }

                    }
                    // BEGIN BFS ON SELECTION
                    if (grid.start != nullptr && grid.target != nullptr) {
                        grid.bfs(&grid);
                    }
                    break;
                case SDL_MOUSEMOTION:
                    // checks the grid for mouse to node collisions but only when the mouse is moved. 
                    for (int i = 0; i < grid.SIZE; i++) {
                        for (int j = 0; j < grid.SIZE; j++) {
                            if (SDL_HasIntersection(&grid.maze[i][j]->rect, &mouse.rect)) {
                                grid.hovered = grid.maze[i][j]; //sets the grid's 'hovered' pointer to the hovered node.
                            }
                        }
                    }
                    break;

            }
        }// end event loop

        //set background color
        SDL_SetRenderDrawColor(ren, 100, 100, 100, 255);
        SDL_RenderClear(ren);

        grid.draw();
        SDL_RenderPresent(ren);

    }

    return 0;
}

