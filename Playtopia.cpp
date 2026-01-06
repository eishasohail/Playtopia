#include <iostream>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <string>
#include <thread>
#include <chrono>
#include <conio.h>
#include <windows.h>
#include <time.h>

#define SCREEN_WIDTH 90
#define SCREEN_HEIGHT 26
#define WIN_WIDTH 70

using namespace std;
HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
COORD CursorPosition;

const int MAP_WIDTH = 20;   // Width of the game map
const int MAP_HEIGHT = 5;   // Height of the game map
const char EMPTY = '-';     // Character for empty space
const char COIN = '$';      // Character for coins
const char PLAYER = 'P';    // Character for the player
const char OBSTACLE = 'X';  // Character for obstacles

void gotoxy(int x, int y) {
    CursorPosition.X = x;
    CursorPosition.Y = y;
    SetConsoleCursorPosition(console, CursorPosition);
}

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void showLoading() {
    cout << "Loading";
    for (int i = 0; i < 10; ++i) {
        cout << ".";
        cout.flush();
        this_thread::sleep_for(chrono::milliseconds(300));
    }
    cout << endl;
}

void clearScreen() {
#ifdef _WIN32
    system("CLS");
#else
    system("clear");
#endif
}

void setcursor(bool visible, DWORD size) {
    if (size == 0)
        size = 20;

    CONSOLE_CURSOR_INFO lpCursor;
    lpCursor.bVisible = visible;
    lpCursor.dwSize = size;
    SetConsoleCursorInfo(console, &lpCursor);
}

class Game {
protected:
    int score;

public:
    Game() : score(0) {}

    virtual void play() = 0;  // Pure virtual function to play the game
    virtual void instructions() = 0;  // Pure virtual function for game instructions
};

class Highway : public Game {
private:
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD CursorPosition;

    int obstacleX[2], obstacleY[2];
    bool obstacleActive[2];
    char obstacleType[2];

    char car[4][4] = {' ', '*', '*', ' ',
                      '*', '*', '*', '*',
                      ' ', '*', '*', ' ',
                      '*', '*', '*', '*'};
    int carPos;

    void gotoxy(int x, int y) {
        CursorPosition.X = x;
        CursorPosition.Y = y;
        SetConsoleCursorPosition(console, CursorPosition);
    }

    void setcursor(bool visible, DWORD size) {
        if (size == 0)
            size = 20;

        CONSOLE_CURSOR_INFO lpCursor;
        lpCursor.bVisible = visible;
        lpCursor.dwSize = size;
        SetConsoleCursorInfo(console, &lpCursor);
    }

    void drawBorder() {
        for (int i = 0; i < SCREEN_HEIGHT; i++) {
            for (int j = 0; j < 17; j++) {
                gotoxy(0 + j, i); cout << "#";
                gotoxy(WIN_WIDTH - j, i); cout << "#";
            }
        }
        for (int i = 0; i < SCREEN_HEIGHT; i++) {
            gotoxy(SCREEN_WIDTH, i); cout << "#";
        }
    }

    void genObstacles() {
        obstacleX[0] = 17 + rand() % 33;
        obstacleY[0] = 1;
        obstacleActive[0] = false;  // Start with no coin
        obstacleType[0] = '$';  // Coin

        obstacleX[1] = 17 + rand() % 33;
        obstacleY[1] = 1;
        obstacleActive[1] = true;   // Start with a rock
        obstacleType[1] = 'X';  // Rock
    }

    void drawObstacle(int index) {
        if (obstacleActive[index]) {
            gotoxy(obstacleX[index], obstacleY[index]); cout << (obstacleType[index] == '$' ? "$$" : "XX");
            gotoxy(obstacleX[index], obstacleY[index] + 1); cout << (obstacleType[index] == '$' ? "$$" : "XX");
            gotoxy(obstacleX[index], obstacleY[index] + 2); cout << (obstacleType[index] == '$' ? "$$" : "XX");
            gotoxy(obstacleX[index], obstacleY[index] + 3); cout << (obstacleType[index] == '$' ? "$$" : "XX");
        }
    }

    void eraseObstacle(int index) {
        if (obstacleActive[index]) {
            gotoxy(obstacleX[index], obstacleY[index]); cout << "    ";
            gotoxy(obstacleX[index], obstacleY[index] + 1); cout << "    ";
            gotoxy(obstacleX[index], obstacleY[index] + 2); cout << "    ";
            gotoxy(obstacleX[index], obstacleY[index] + 3); cout << "    ";
        }
    }

    void resetObstacle(int index) {
        eraseObstacle(index);
        obstacleY[index] = 1;
        obstacleX[index] = 17 + rand() % 33;
        obstacleActive[index] = true;
    }

    void drawCar() {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                gotoxy(j + carPos, i + 22); cout << car[i][j];
            }
        }
    }

    void eraseCar() {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                gotoxy(j + carPos, i + 22); cout << " ";
            }
        }
    }

    int collision() {
        // Check collision with rocks (obstacle 1)
        if (obstacleActive[1]) {
            if (obstacleY[1] + 4 >= 23) {
                if (obstacleX[1] + 4 - carPos >= 0 && obstacleX[1] + 4 - carPos < 9) {
                    return 1;  // Game over if car hits a rock
                }
            }
        }

        // Check collision with coins (obstacle 0)
        if (obstacleActive[0]) {
            if (obstacleY[0] + 4 >= 23) {
                if (obstacleX[0] + 4 - carPos >= 0 && obstacleX[0] + 4 - carPos < 9) {
                    score++;  // Increase score if car hits a coin
                    gotoxy(WIN_WIDTH + 7, 5); cout << "Score: " << score << endl;
                    resetObstacle(0);
                }
            }
        }

        return 0;
    }

    void gameover() {
        system("cls");
        cout << endl;
        cout << "\t\t--------------------------" << endl;
        cout << "\t\t-------- Game Over -------" << endl;
        cout << "\t\t--------------------------" << endl << endl;
        cout << "\t\tPress any key to go back to menu.";
        getch();
    }

    void updateScore() {
        gotoxy(WIN_WIDTH + 7, 5); cout << "Score: " << score << endl;
    }

    void instructions() override {
        system("cls");
        cout << "Instructions" << endl;
        cout << "----------------" << endl;
        cout << "Avoid rocks by moving left or right." << endl;
        cout << "Collect coins to increase your score." << endl;
        cout << "\nPress 'a' to move left" << endl;
        cout << "Press 'd' to move right" << endl;
        cout << "Press 'escape' to exit" << endl;
        cout << "\nPress any key to go back to menu";
        getch();
    }

public:
    Highway() : carPos(WIN_WIDTH / 2) {
        setcursor(0, 0);
        srand((unsigned)time(0));
    }

    void play() override {
        carPos = -1 + WIN_WIDTH / 2;
        score = 0;
        obstacleActive[0] = false;  // No coins at start
        obstacleActive[1] = true;   // Start with a rock
        genObstacles();

        system("cls");
        drawBorder();
        updateScore();
        drawObstacle(0);
        drawObstacle(1);

        while (1) {
            if (kbhit()) {
                char ch = getch();
                if (ch == 'a' || ch == 'A') {
                    if (carPos > 18)
                        carPos -= 4;
                }
                if (ch == 'd' || ch == 'D') {
                    if (carPos < 50)
                        carPos += 4;
                }
                if (ch == 27) {
                    break;
                }
            }

            drawCar();
            drawObstacle(0);
            drawObstacle(1);

            if (collision() == 1) {
                gameover();
                return;
            }

            Sleep(50);
            eraseCar();
            eraseObstacle(0);
            eraseObstacle(1);

            if (obstacleY[0] == SCREEN_HEIGHT - 4) {
                resetObstacle(0);
            } else {
                obstacleY[0] += 1;
            }

            if (obstacleY[1] == SCREEN_HEIGHT - 4) {
                resetObstacle(1);
            } else {
                obstacleY[1] += 1;
            }
        }
    }
};
class Mario : public Game {
private:
    int score;
    bool gameOver;
    int playerX, playerY;
    int coinsCollected;
    char map[MAP_HEIGHT][MAP_WIDTH];
    string playerName;
    int difficulty;

    void initializeMap() {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            for (int x = 0; x < MAP_WIDTH; ++x) {
                map[y][x] = EMPTY;
            }
        }

        srand(static_cast<unsigned>(time(nullptr)));
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            for (int x = 0; x < MAP_WIDTH; ++x) {
                if (rand() % (6 - difficulty) == 0 && map[y][x] != COIN) {
                    map[y][x] = COIN;
                }
            }
        }

        int numObstacles = rand() % (MAP_WIDTH * MAP_HEIGHT / 10);
        for (int i = 0; i < numObstacles; ++i) {
            int x = rand() % MAP_WIDTH;
            int y = rand() % MAP_HEIGHT;
            if (map[y][x] == EMPTY) {
                map[y][x] = OBSTACLE;
            }
        }
    }

    void displayMap() const {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            for (int x = 0; x < MAP_WIDTH; ++x) {
                if (x == playerX && y == playerY) {
                    cout << PLAYER;
                } else {
                    cout << map[y][x];
                }
            }
            cout << endl;
        }
        cout << "Score: " << score << " | Coins Collected: " << coinsCollected << " | Player: " << playerName << endl;
    }

    void movePlayer(int moveX, int moveY) {
        int newPlayerX = playerX + moveX;
        int newPlayerY = playerY + moveY;

        if (newPlayerX < 0 || newPlayerX >= MAP_WIDTH || newPlayerY < 0 || newPlayerY >= MAP_HEIGHT) {
            cout << "You cannot move outside the map!" << endl;
            return;
        }

        if (map[newPlayerY][newPlayerX] == COIN) {
            cout << "You collected a coin!" << endl;
            map[newPlayerY][newPlayerX] = EMPTY;
            coinsCollected++;
            score += 10;
        } else if (map[newPlayerY][newPlayerX] == OBSTACLE) {
            cout << "You hit an obstacle and lost points!" << endl;
            score -= 5;
            if (score < 0) score = 0;
        }

        playerX = newPlayerX;
        playerY = newPlayerY;
    }

    bool isGameOver() const {
        return gameOver;
    }

    string getPlayerName() const {
        return playerName;
    }

    void endGame() {
        gameOver = true;
        cout << "Game Over! Final Score: " << score << endl;
    }

    void clearScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void clearInputBuffer() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

public:
    Mario(string name, int diff) : score(0), gameOver(false), playerX(MAP_WIDTH / 2), playerY(MAP_HEIGHT / 2), coinsCollected(0), playerName(name), difficulty(diff) {
        initializeMap();
    }

    void instructions() override {
        clearScreen();
        cout << "Instructions:" << endl;
        cout << "Use W, A, S, D to move up, left, down, and right respectively." << endl;
        cout << "Collect coins ($) to score points." << endl;
        cout << "Avoid obstacles (X)." << endl;
        cout << "Press Q to quit." << endl;
        cout << "Press any key to start the game..." << endl;
        cin.get();
    }

    void play() override {
        while (!isGameOver()) {
            clearScreen();
            displayMap();

            char input;
            cout << "Enter your move (w/s/a/d) or 'q' to quit: ";
            cin >> input;

            if (input == 'a') {
                movePlayer(-1, 0);
            } else if (input == 'd') {
                movePlayer(1, 0);
            } else if (input == 'w') {
                movePlayer(0, -1);
            } else if (input == 's') {
                movePlayer(0, 1);
            } else if (input == 'q') {
                endGame();
            } else {
                cout << "Invalid input! Use 'w' to move up, 's' to move down, 'a' to move left, and 'd' to move right." << endl;
                clearInputBuffer();
                continue;
            }

            bool coinsExist = false;
            for (int y = 0; y < MAP_HEIGHT; ++y) {
                for (int x = 0; x < MAP_WIDTH; ++x) {
                    if (map[y][x] == COIN) {
                        coinsExist = true;
                        break;
                    }
                }
                if (coinsExist) break;
            }

            if (!coinsExist || score < 0) {
                endGame();
            }
        }
    }
};

class GameMenu {
private:
    Game* games[2];  // Array to hold the games

public:
	string playerName;
    char difficultyChar;
    int difficulty;
    GameMenu() {
        games[0] = new Mario(playerName,difficulty);
        games[1] = new Highway();
    }

    ~GameMenu() {
        for (int i = 0; i < 2; ++i) {
            delete games[i];
        }
    }

    void display() {
        while (true) {
            clearScreen();
             cout<< "Welcome to PlayTopia: The Epic Hunt for treassure!"<<endl;
    cout<< "\nSelect the game you want to play: "<<endl;
    cout<<"\n1.Coin collector Mario"<<endl;
    cout<< "2.Highway Madness"<<endl;
            int choice;
            cin >> choice;
if(choice==1){
	string playerName;
    char difficultyChar;
    int difficulty;
// Welcome message and player info
    cout << "\nWelcome to Coin collector Mario Game!" << endl;
    cout << "Please enter your name: ";
      clearInputBuffer();
    getline(cin, playerName);

    cout << "\nChoose your difficulty level (E for Easy, M for Medium, H for Hard): ";
    cin >> difficultyChar;
    clearInputBuffer();

    // Set difficulty based on input
    switch (difficultyChar) {
        case 'E':
        case 'e':
            difficulty = 1;
            break;
        case 'M':
        case 'm':
            difficulty = 2;
            break;
        case 'H':
        case 'h':
            difficulty = 3;
            break;
        default:
            cout << "Invalid difficulty! Defaulting to Easy." << endl;
            difficulty = 1;
    }
        showLoading();

	games[0]->instructions();
                games[0]->play();
				}
            if (choice==2) {
            
                	do {
        system("cls");
        gotoxy(10, 5); cout << " -------------------------- ";
        gotoxy(10, 6); cout << " |      Highway Madness     | ";
        gotoxy(10, 7); cout << " --------------------------";
        gotoxy(10, 9); cout << "1. Start Game";
        gotoxy(10, 10); cout << "2. Instructions";
        gotoxy(10, 11); cout << "3. Quit";
        gotoxy(10, 13); cout << "Select option: ";
        char op = _getche();

        if (op == '1') 
		 games[1]->play();
        else if (op == '2')
	games[1]->instructions();
        else if (op == '3') exit(0);

    } while (1);
            }

            cout << "Press any key to return to the menu..." << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        }
    }
};

int main() {
    GameMenu menu;
    menu.display();
    return 0;
}
