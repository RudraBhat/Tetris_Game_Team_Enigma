#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <cstring>
using namespace std;

// ANSI Color Codes
#define PINK    "\033[38;5;213m"  
#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define ORANGE  "\033[38;5;208m"
#define PURPLE  "\033[38;5;93m"
#define BOLD    "\033[1m"
#define BG_BLACK "\033[40m"
#define BG_RED   "\033[41m"
#define BG_GREEN "\033[42m"
#define BG_YELLOW "\033[43m"
#define BG_BLUE  "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN   "\033[46m"
#define BG_WHITE  "\033[47m"
#define BG_GRAY "\033[48;5;240m"

const int consoleWidth = 80;
const int consoleHeight = 30;
const int fieldWidth = 12;
const int fieldHeight = 20;
const int playWidth = fieldWidth - 2;

const string TETROMINO_COLORS[7] = { CYAN, PINK, ORANGE, YELLOW, RED, PURPLE, GREEN };
const string TETROMINO_NAMES[7] = { "I", "J", "L", "O", "S", "T", "Z" };

class TetrisGame {
public:
    TetrisGame() : currentPiece(rand() % 7), 
        currentRotation(0), currentX(playWidth / 2 - 1), currentY(0), speed(30), 
        speedCounter(0), forcePieceDown(false), rotationHold(true), pieceCounter(0), 
        score(0), isGameOver(false), isPaused(false), level(1), highScore(0), 
        nextPiece(rand() % 7), previousField(nullptr), linesCleared(0), totalLinesCleared(0) {
        srand(time(0));
        initializeField();
        initializePieces();
        initializeScreen();
        loadHighScore();
    }

    ~TetrisGame() {
        delete[] field;
        delete[] screen;
        delete[] previousField;
        saveHighScore();
    }

    void run() {
        showStartingAnimation();
        setTerminalRawMode(true);
        
        // Initial draw
        clearScreen();
        drawGame();
        
        while (!isGameOver) {
            if (!isPaused) {
                this_thread::sleep_for(chrono::milliseconds(50));
                speedCounter++;
                forcePieceDown = (speedCounter == speed);

                handleInput();
                updateGame();
                drawGame();

                fill(begin(keys), end(keys), false);
            } else {
                drawPauseScreen();
                handleInput();
                // Clear the pause screen completely when unpausing
                if (!isPaused) {
                    clearScreen();
                    drawGame();
                }
            }
        }
        setTerminalRawMode(false);
        drawGameOverScreen();
        
        // Wait for restart or exit
        while (true) {
            if (kbhit()) {
                char keyPressed = getch();
                if (keyPressed == 'r' || keyPressed == 'R') {
                    initialize();
                    run();
                    return;
                } else if (keyPressed == 'x' || keyPressed == 'X') {
                    return;
                }
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }

private:
    wstring tetrominoes[7];
    unsigned char *field;
    wchar_t *screen;
    bool keys[4] = {false, false, false, false};
    int currentPiece;
    int currentRotation;
    int currentX;
    int currentY;
    int speed;
    int speedCounter;
    bool forcePieceDown;
    bool rotationHold;
    int pieceCounter;
    int score;
    vector<int> completedLines;
    bool isGameOver;
    bool isPaused;
    int level;
    int highScore;
    int nextPiece;
    int linesCleared;
    int totalLinesCleared;

    unsigned char *previousField;
    int previousPiece;
    int previousRotation;
    int previousX;
    int previousY;
    int previousScore;

    void clearScreen() {
        cout << "\033[2J\033[H";
    }

    void initializeField() {
        field = new unsigned char[fieldWidth * fieldHeight];
        for (int x = 0; x < fieldWidth; x++) {
            for (int y = 0; y < fieldHeight; y++) {
                field[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1) ? 9 : 0;
            }
        }
    }

    void initializePieces() {
        // I-piece (4x4)
        tetrominoes[0] = L"....XXXX........";
        // J-piece (3x3)
        tetrominoes[1] = L"..X..X.XX";
        // L-piece (3x3)
        tetrominoes[2] = L"X..X..XX.";
        // O-piece (2x2)
        tetrominoes[3] = L"XXXX";
        // S-piece (3x3)
        tetrominoes[4] = L".XXXX..";
        // T-piece (3x3)
        tetrominoes[5] = L".X.XXX.";
        // Z-piece (3x3)
        tetrominoes[6] = L"XX..XX";
    }

    void initializeScreen() {
        screen = new wchar_t[consoleWidth * consoleHeight];
        for (int i = 0; i < consoleWidth * consoleHeight; i++) screen[i] = L' ';
    }

    void loadHighScore() {
        ifstream file("highscore.txt");
        if (file.is_open()) {
            file >> highScore;
            file.close();
        }
    }

    void saveHighScore() {
        if (score > highScore) {
            highScore = score;
            ofstream file("highscore.txt");
            if (file.is_open()) {
                file << highScore;
                file.close();
            }
        }
    }

    void showStartingAnimation() {
        clearScreen();
        cout << BG_BLUE << WHITE << BOLD;
        
        vector<string> tetrisArt = {
            "████████╗███████╗████████╗██████╗ ██╗███████╗",
            "╚══██╔══╝██╔════╝╚══██╔══╝██╔══██╗██║██╔════╝",
            "   ██║   █████╗     ██║   ██████╔╝██║███████╗",
            "   ██║   ██╔══╝     ██║   ██╔══██╗██║╚════██║",
            "   ██║   ███████╗   ██║   ██║  ██║██║███████║",
            "   ╚═╝   ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚══════╝",
            "    M A D E   B Y   T E A M   E N I G M A    "
        };
        
        for (int i = 0; i < tetrisArt.size(); i++) {
            cout << tetrisArt[i] << endl;
            usleep(300000);
        }
        
        cout << RESET << "\n\nStarting game in 3...";
        cout.flush();
        usleep(1000000);
        cout << "2...";
        cout.flush();
        usleep(1000000);
        cout << "1...";
        cout.flush();
        usleep(1000000);
    }

    int rotate(int px, int py, int r, int pieceSize) {
        switch (r % 4) {
            case 0: return py * pieceSize + px;
            case 1: return (pieceSize - 1 - px) * pieceSize + py;
            case 2: return (pieceSize - 1 - py) * pieceSize + (pieceSize - 1 - px);
            case 3: return px * pieceSize + (pieceSize - 1 - py);
        }
        return 0;
    }

    bool doesPieceFit(int pieceIdx, int rot, int posX, int posY) {
        int pieceSize = (pieceIdx == 0) ? 4 : (pieceIdx == 3) ? 2 : 3;
        
        for (int px = 0; px < pieceSize; px++) {
            for (int py = 0; py < pieceSize; py++) {
                int pi = rotate(px, py, rot, pieceSize);
                if (pi >= tetrominoes[pieceIdx].size() || tetrominoes[pieceIdx][pi] == L'.') 
                    continue;
                
                int fx = posX + px + 1;
                int fy = posY + py;
                
                if (fx <= 0 || fx >= fieldWidth - 1) return false;
                if (fy >= fieldHeight - 1) return false;
                if (field[fy * fieldWidth + fx] != 0) return false;
            }
        }
        return true;
    }

    bool kbhit() {
        struct timeval tv = {0L, 0L};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    }

    char getch() {
        char buf = 0;
        if (read(STDIN_FILENO, &buf, 1) < 0) perror("read()");
        return buf;
    }

    void setTerminalRawMode(bool enable) {
        static struct termios oldt, newt;
        if (enable) {
            tcgetattr(STDIN_FILENO, &oldt);
            newt = oldt;
            newt.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        } else {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        }
    }

    void handleInput() {
        if (kbhit()) {
            char keyPressed = getch();
            switch (keyPressed) {
                case 'd': case 'D': keys[0] = true; break;
                case 'a': case 'A': keys[1] = true; break;
                case 's': case 'S': keys[2] = true; break;
                case 'w': case 'W': keys[3] = true; break;
                case 'x': case 'X': isGameOver = true; break;
                case 'r': case 'R': initialize(); break;
                case 'p': case 'P': 
                    isPaused = !isPaused; 
                    if (!isPaused) {
                        clearScreen();
                        drawGame();
                    }
                    break;
                case ' ': dropPiece(); break;
                case 'u': case 'U': undo(); break;
                default: break;
            }
        }
    }

    void dropPiece() {
        while (doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) {
            currentY++;
        }
        forcePieceDown = true;
    }

    void initialize() {
        currentPiece = rand() % 7;
        currentRotation = 0;
        currentX = playWidth / 2 - 1;
        currentY = 0;
        speed = 30;
        speedCounter = 0;
        forcePieceDown = false;
        rotationHold = true;
        pieceCounter = 0;
        score = 0;
        linesCleared = 0;
        totalLinesCleared = 0;
        level = 1;
        completedLines.clear();
        isGameOver = false;
        isPaused = false;
        nextPiece = rand() % 7;
        initializeField();
        delete[] previousField;
        previousField = nullptr;
    }

    void updateGame() {
        if (keys[0] && doesPieceFit(currentPiece, currentRotation, currentX + 1, currentY)) currentX++;
        if (keys[1] && doesPieceFit(currentPiece, currentRotation, currentX - 1, currentY)) currentX--;
        if (keys[2] && doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) currentY++;

        if (keys[3]) {
            if (rotationHold && doesPieceFit(currentPiece, currentRotation + 1, currentX, currentY)) {
                currentRotation++;
            }
            rotationHold = false;
        } else {
            rotationHold = true;
        }

        if (forcePieceDown) {
            speedCounter = 0;
            pieceCounter++;
            if (pieceCounter % 50 == 0 && speed >= 10) speed--;

            if (doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) {
                currentY++;
            } else {
                saveState();
                int pieceSize = (currentPiece == 0) ? 4 : (currentPiece == 3) ? 2 : 3;
                
                for (int px = 0; px < pieceSize; px++) {
                    for (int py = 0; py < pieceSize; py++) {
                        int pi = rotate(px, py, currentRotation, pieceSize);
                        if (pi < tetrominoes[currentPiece].size() && tetrominoes[currentPiece][pi] != L'.') {
                            int fx = currentX + px + 1;
                            int fy = currentY + py;
                            if (fx > 0 && fx < fieldWidth - 1 && fy < fieldHeight - 1) {
                                field[fy * fieldWidth + fx] = currentPiece + 1;
                            }
                        }
                    }
                }

                score += 250;

                completedLines.clear();
                for (int y = 0; y < fieldHeight - 1; y++) {
                    bool lineComplete = true;
                    for (int x = 1; x < fieldWidth - 1; x++) {
                        if (field[y * fieldWidth + x] == 0) {
                            lineComplete = false;
                            break;
                        }
                    }
                    if (lineComplete) {
                        completedLines.push_back(y);
                    }
                }
    
                if (!completedLines.empty()) {
                    linesCleared += completedLines.size();
                    totalLinesCleared += completedLines.size();
                    
                    level = max(1, totalLinesCleared / 2 + 1);
                    speed = max(2, 30 - (level * 2));

                    switch (completedLines.size()) {
                        case 1: score += 1000 * level; break;
                        case 2: score += 2000 * level; break;
                        case 3: score += 3000 * level; break;
                        case 4: score += 5000 * level; break;
                    }
                    
                    for (int i = 0; i < 2; i++) {
                        for (int line : completedLines) {
                            for (int x = 1; x < fieldWidth - 1; x++) {
                                field[line * fieldWidth + x] = 8;
                            }
                        }
                        drawGame();
                        usleep(200000);
                        
                        for (int line : completedLines) {
                            for (int x = 1; x < fieldWidth - 1; x++) {
                                field[line * fieldWidth + x] = currentPiece + 1;
                            }
                        }
                        drawGame();
                        usleep(200000);
                    }

                    for (int line : completedLines) {
                        for (int y = line; y > 0; y--) {
                            for (int x = 1; x < fieldWidth - 1; x++) {
                                field[y * fieldWidth + x] = field[(y - 1) * fieldWidth + x];
                            }
                        }
                        for (int x = 1; x < fieldWidth - 1; x++) {
                            field[x] = 0;
                        }
                    }
                }

                currentPiece = nextPiece;
                nextPiece = rand() % 7;
                currentX = playWidth / 2 - 1;
                currentY = 0;
                currentRotation = 0;

                isGameOver = !doesPieceFit(currentPiece, currentRotation, currentX, currentY);
            }
        }
    }

    void drawField() {
        cout << "\033[3;1H" << BG_GRAY << "  " << RESET;
        for (int x = 1; x < fieldWidth - 1; x++) {
            cout << BG_GRAY << "  " << RESET;
        }
        cout << BG_GRAY << "  " << RESET;

        for (int y = 0; y < fieldHeight - 1; y++) {
            cout << "\033[" << y + 4 << ";1H" << BG_GRAY << "  " << RESET;
            
            for (int x = 1; x < fieldWidth - 1; x++) {
                unsigned char cell = field[y * fieldWidth + x];
                if (cell >= 1 && cell <= 7) {
                    cout << BG_BLACK << TETROMINO_COLORS[cell-1] << "■ " << RESET;
                } else if (cell == 8) {
                    cout << BG_WHITE << BLACK << "■ " << RESET;
                } else {
                    cout << BG_BLACK << "  " << RESET;
                }
            }
            
            cout << BG_GRAY << "  " << RESET;
        }

        cout << "\033[" << fieldHeight + 3 << ";1H" << BG_GRAY << "  " << RESET;
        for (int x = 1; x < fieldWidth - 1; x++) {
            cout << BG_GRAY << "  " << RESET;
        }
        cout << BG_GRAY << "  " << RESET;
    }

    void drawCurrentPiece() {
        int pieceSize = (currentPiece == 0) ? 4 : (currentPiece == 3) ? 2 : 3;
        
        for (int py = 0; py < pieceSize; py++) {
            for (int px = 0; px < pieceSize; px++) {
                int pi = rotate(px, py, currentRotation, pieceSize);
                if (pi < tetrominoes[currentPiece].size() && tetrominoes[currentPiece][pi] != L'.') {
                    int screenY = currentY + py + 4;
                    int screenX = (currentX + px + 1) * 2 + 1;
                    cout << "\033[" << screenY << ";" << screenX << "H";
                    cout << BG_BLACK << TETROMINO_COLORS[currentPiece] << "■" << RESET;
                }
            }
        }
    }

    void clearNextPieceArea() {
        for (int y = 4; y < 8; y++) {
            cout << "\033[" << y << ";25H";
            for (int x = 0; x < 8; x++) {
                cout << BG_BLACK << "  " << RESET;
            }
        }
    }

    void drawSidePanel() {
        // Next piece
        cout << "\033[3;25H" << "  Next piece: ";
        
        // Clear and draw next piece
        clearNextPieceArea();
        int pieceSize = (nextPiece == 0) ? 4 : (nextPiece == 3) ? 2 : 3;
        for (int py = 0; py < pieceSize; py++) {
            for (int px = 0; px < pieceSize; px++) {
                int pi = rotate(px, py, 0, pieceSize);
            
                if (pi < tetrominoes[nextPiece].size() && tetrominoes[nextPiece][pi] != L'.') {
                    cout << "\033[" << (4 + py) << ";" << (27 + px*2) << "H";
                    cout << BG_BLACK << TETROMINO_COLORS[nextPiece] << "■ " << RESET;
                }
            }
        }
        
        // Scoring
        cout << "\033[10;25H" << "     Scoring System:";
        cout << "\033[11;25H" << "      Single line: " << GREEN << "1000 × level" << RESET;
        cout << "\033[12;25H" << "      Double lines: " << YELLOW << "2000 × level" << RESET;
        cout << "\033[13;25H" << "      Triple lines: " << ORANGE << "3000 × level" << RESET;
        cout << "\033[14;25H" << "      Tetris (4): " << RED << "5000 × level" << RESET;
        cout << "\033[15;25H" << "      Piece placed: " << CYAN << "250" << RESET;
        
        // Controls
        cout << "\033[17;25H" << "     Controls:";
        cout << "\033[18;25H" << "      W - Rotate"<<"    A - Left";
        cout << "\033[19;25H" << "      S - Down"<<"      D - Right";
        cout << "\033[20;25H" << "      Space - Drop"<<"  P - Pause";
        cout << "\033[21;25H" << "      R - Restart"<<"   X - Exit";
    }

    void drawGame() {
        // Header
        cout << "\033[1;1H" << BG_BLUE << WHITE << BOLD << " TETRIS " << RESET << "  ";
        cout << BG_GREEN << BLACK << " Level: " << level << " " << RESET << "  ";
        cout << BG_YELLOW << BLACK << " Score: " << score << " " << RESET << "  ";
        cout << BG_MAGENTA << WHITE << " Lines: " << totalLinesCleared << " " << RESET << "  ";
        cout << BG_RED << WHITE << " High: " << highScore << " " << RESET << "\n";
        
        drawField();
        drawCurrentPiece();
        drawSidePanel();
        
        cout.flush();
    }

    void drawPauseScreen() {
        // Clear screen and draw pause message
        cout << "\033[2J\033[H";
        cout << BG_BLUE << WHITE << BOLD << "\n\n\n\n";
        cout << "         ██████╗  █████╗ ██╗   ██╗███████╗███████╗       \n";
        cout << "         ██╔══██╗██╔══██╗██║   ██║██╔════╝██╔════╝       \n";
        cout << "         ██████╔╝███████║██║   ██║███████╗█████╗         \n";
        cout << "         ██╔═══╝ ██╔══██║██║   ██║╚════██║██╔══╝         \n";
        cout << "         ██║     ██║  ██║╚██████╔╝███████║███████╗       \n";
        cout << "         ╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚══════╝       \n";
        cout << RESET << "\n\n";
        cout << BG_YELLOW << BLACK << "           Press P to continue           " << RESET << "\n";
        usleep(1000000);
        cout.flush();
    }

    void drawGameOverScreen() {
        clearScreen();
        cout << BG_RED << WHITE << BOLD << "\n\n\n\n";
        cout << "          ██████╗  █████╗ ███╗   ███╗███████╗     \n";
        cout << "         ██╔════╝ ██╔══██╗████╗ ████║██╔════╝     \n";
        cout << "         ██║  ███╗███████║██╔████╔██║█████╗       \n";
        cout << "         ██║   ██║██╔══██║██║╚██╔╝██║██╔══╝       \n";
        cout << "         ╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗     \n";
        cout << "          ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     \n";
        cout << "          ██████╗ ██╗   ██╗███████╗██████╗        \n";
        cout << "         ██╔═══██╗██║   ██║██╔════╝██╔══██╗       \n";
        cout << "         ██║   ██║██║   ██║█████╗  ██████╔╝       \n";
        cout << "         ██║   ██║╚██╗ ██╔╝██╔══╝  ██╔══██╗       \n";
        cout << "         ╚██████╔╝ ╚████╔╝ ███████╗██║  ██║       \n";
        cout << "          ╚═════╝   ╚═══╝  ╚══════╝╚═╝  ╚═╝       \n";
        cout << RESET << "\n\n";
        cout << BG_GREEN << BLACK << "           Your Score: " << score << "           " << RESET << "\n";
        cout << BG_BLUE << WHITE << "        High Score: " << highScore << "        " << RESET << "\n\n";
        cout << BG_YELLOW << BLACK << "     Press R to restart or X to exit     " << RESET << "\n";
        cout.flush();
    }

    void saveState() {
        delete[] previousField;
        previousField = new unsigned char[fieldWidth * fieldHeight];
        memcpy(previousField, field, fieldWidth * fieldHeight * sizeof(unsigned char));
        previousPiece = currentPiece;
        previousRotation = currentRotation;
        previousX = currentX;
        previousY = currentY;
        previousScore = score;
    }

    void undo() {
        if (previousField) {
            int pieceSize = (previousPiece == 0) ? 4 : (previousPiece == 3) ? 2 : 3;
            
            for (int px = 0; px < pieceSize; px++) {
                for (int py = 0; py < pieceSize; py++) {
                    int pi = rotate(px, py, previousRotation, pieceSize);
                    if (pi < tetrominoes[previousPiece].size() && tetrominoes[previousPiece][pi] != L'.') {
                        field[(previousY + py) * fieldWidth + (previousX + px + 1)] = 0;
                    }
                }
            }

            memcpy(field, previousField, fieldWidth * fieldHeight * sizeof(unsigned char));
            currentPiece = previousPiece;
            currentRotation = previousRotation;
            currentX = previousX;
            currentY = previousY;
            score = previousScore;
            delete[] previousField;
            previousField = nullptr;
        }
    }
};

int main() {
    system("clear");
    TetrisGame game;
    game.run();
    return 0;
}