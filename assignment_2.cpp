#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <algorithm>

// Typedefs
typedef std::vector<std::vector<int>> Connect4Board; // 1 = player 1, 2 = player 2, 0 = empty
typedef std::vector<std::pair<int, int>> Connect4Moves; // (player, column)

enum Player { PLAYER1 = 1, PLAYER2 = 2 };
enum Mode { HUMANvsAI = 1, AIvsAI = 2, HUMANvsHUMAN = 3 };

// File to output results
const char* FILENAME;

// Game variables
bool gameOver = false;
int player = PLAYER1;
char column = 'A';
int mode = HUMANvsAI;
int WIDTH = 7;
int HEIGHT = 6;

void initializeBoard(Connect4Board &board);
void printBoard(Connect4Board &board, Connect4Moves &winning);
void outputMoves(Connect4Moves &moves, const char* filename);
int randomNum(int min, int max);
int scoreMove(Connect4Board &board, int player);
int evalWindow(std::vector<int> window, int player); // Non-terminal Heuristic
bool checkWin(Connect4Board &board, Connect4Moves &winning, int player);
bool placePiece(Connect4Board &board, Connect4Moves &moves, int player, char column);
bool isTerminalNode(Connect4Board &board);
bool isFull(Connect4Board &board);

std::pair<int, char> minimax(Connect4Board &board, int player, int depth, int alpha, int beta, bool isMaximizing);

// Main function
int main(int argc, char **argv) {
    Connect4Board board;
    Connect4Moves moves;
    Connect4Moves winningMoves;

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [mode]" << std::endl;
        std::cout << "\nModes:" << std::endl;
        std::cout << "\t1: Human vs AI" << std::endl;
        std::cout << "\t2: AI vs AI" << std::endl;
        std::cout << "\t3: Human vs Human" << std::endl;

        return 0;
    }

    mode = atoi(argv[1]);

    srand(time(0));
    initializeBoard(board);

    // Set mode
    switch (mode) {
        case HUMANvsAI:
            std::cout << "Mode: Human vs AI" << std::endl;
            FILENAME = "human_vs_ai_results.txt";  

            break;
        case AIvsAI:
            std::cout << "Mode: AI vs AI" << std::endl;
            FILENAME = "ai_vs_ai_results.txt";

            break;
        case HUMANvsHUMAN:
            std::cout << "Mode: Human vs Human" << std::endl;
            FILENAME = "human_vs_human_results.txt";

            break;
        default:
            std::cout << "Default Mode: Human vs AI" << std::endl;
            FILENAME = "results.txt";

            break;
    }

    printBoard(board, winningMoves);

    // Game loop
    while (!gameOver) {
        switch (mode) {
            case HUMANvsAI: // Player 1 is human, Player 2 is AI
                if (player == PLAYER1) {
                    std::cout << "\nPlayer " << player << ", enter column: ";
                    std::cin >> column;
                
                    if (!placePiece(board, moves, player, column)) {
                        std::cout << "Invalid column, try again." << std::endl;
                        continue;
                    }

                } else {
                    std::cout << "\nPlayer " << player << " is thinking..." << std::endl;

                    sleep(0.5); // Wait for 1/2 second

                    column = minimax(board, player, 4, -100, 100, true).second;

                    if (!placePiece(board, moves, player, column)) {
                        continue;
                    }
                }

                break;
            case AIvsAI:
                std::cout << "\nPlayer " << player << " is thinking..." << std::endl;

                sleep(0.5); // Wait for 1/2 second

                column = minimax(board, player, 4, -100, 100, true).second;

                if (!placePiece(board, moves, player, column)) {
                    continue;
                }

                break;
            case HUMANvsHUMAN:
                std::cout << "\nPlayer " << player << ", enter column: ";
                std::cin >> column;
                
                if (!placePiece(board, moves, player, column)) {
                    std::cout << "Invalid column, try again." << std::endl;
                    continue;
                }

                break;
            default: // Default to Human vs AI (Random)
                if (player == PLAYER1) {
                    std::cout << "\nPlayer " << player << ", enter column: ";
                    std::cin >> column;
                
                    if (!placePiece(board, moves, player, column)) {
                        std::cout << "Invalid column, try again." << std::endl;
                        continue;
                    }

                } else {
                    std::cout << "\nPlayer " << player << " is thinking..." << std::endl;

                    sleep(1); // Wait for 1 second

                    do {
                        column = randomNum('A', 'G');
                    } while (!placePiece(board, moves, player, column));
                }

                break;
        }

        std::cout << "Player " << player << " placed a piece in column " << column << std::endl;

        // Check if player has won
        if (checkWin(board, winningMoves, player)) {
            std::cout << "Player " << player << " wins!" << std::endl;
            gameOver = true;
        }

        // If all moves have been made, it's a draw
        if(moves.size() == WIDTH*HEIGHT) { 
            std::cout << "It's a draw!" << std::endl;
            gameOver = true;
        }

        printBoard(board, winningMoves);

        // Switch player
        player = 3 - player;
    }

    outputMoves(moves, FILENAME);

    return 0;
}

/**
 * @brief Sets up the board of size 6x7
 * 
 * @param board 
 */
void initializeBoard(Connect4Board &board) {
    board.resize(HEIGHT);

    for (int i = 0; i < HEIGHT; i++) {
        board[i].resize(WIDTH);
    }

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            board[i][j] = 0;
        }
    }
}

/**
 * @brief Prints the board
 * 
 * @param board 
 */
void printBoard(Connect4Board &board, Connect4Moves &winning) {
    // Colored Header
    std::cout << "\n\033[0;37;44m|\033[0m\033[0;37;47m A \033[0;37;44m|\033[0;37;47m B \033[0;37;44m|\033[0;37;47m C \033[0;37;44m|\033[0;37;47m D \033[0;37;44m|\033[0;37;47m E \033[0;37;44m|\033[0;37;47m F \033[0;37;44m|\033[0;37;47m G \033[0m\033[0;37;44m|\033[0m" << std::endl;

    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            int piece = board[i][j];

            if (j == 0) { std::cout <<"\033[0;37;44m|\033[0m"; }

            // Check if piece is in winning moves, color them accordingly
            if (std::find(winning.begin(), winning.end(), std::make_pair(i, j)) != winning.end()) {
                std::cout << "\033[0;37;42m " << piece << " \033[0m";
            } else {
                // Color pieces Red for player 1, Yellow for player 2
                if (piece == 1) {
                    std::cout << "\033[0;37;41m " << piece << " \033[0m";
                } else if (piece == 2) {
                    std::cout << "\033[0;37;43m " << piece << " \033[0m";
                } else {
                    std::cout << " " << piece << " ";
                }
            }
            
            std::cout << "\033[0;37;44m|\033[0m"; // End of column
        }
        std::cout << std::endl;
    }

    std::cout << "\033[0;37;44m=============================\033[0m" << std::endl;
}

void outputMoves(Connect4Moves &moves, const char* filename) {
    std::ofstream file;

    file.open(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    /* Print Moves */
    file << "Moves:" << std::endl;

    for (int i = 0; i < moves.size(); i++) {
        file << "<" << moves[i].first << ", " << moves[i].second << ">";

        if (i < moves.size() - 1) { file << ", "; }
    }

    file << std::endl;
}

/* @brief Generate a random number between min and max
 * 
 * @param min 
 * @param max 
 * @return int 
 */
int randomNum(int min, int max) {
    return min + (rand() % (max - min + 1));
}

/**
 * @brief Score a move based on the number of pieces in a window
 * 
 * @param board 
 * @param player 
 * @return int 
 */
int scoreMove(Connect4Board &board, int player) {
    int score = 0;
    
    // Score center column
    std::vector<int> centerArray;

    for (int i = 0; i < 6; i++) {
        centerArray.push_back(board[i][3]);
    }

    int centerCount = std::count(centerArray.begin(), centerArray.end(), player);
    score += centerCount * 3;

    // Score horizontal
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < (WIDTH - 3); j++) {
            std::vector<int> window;

            for (int k = 0; k < 4; k++) {
                window.push_back(board[i][j+k]);
            }

            score += evalWindow(window, player);
        }
    }

    // Score vertical
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < (HEIGHT - 3); j++) {
            std::vector<int> window;

            for (int k = 0; k < 4; k++) {
                window.push_back(board[j+k][i]);
            }

            score += evalWindow(window, player);
        }
    }

    // Score diagonal
    for (int i = 0; i < (HEIGHT - 3); i++) {
        for (int j = 0; j < (WIDTH - 3); j++) {
            std::vector<int> window;

            for (int k = 0; k < 4; k++) {
                window.push_back(board[i+k][j+k]);
            }
            
            score += evalWindow(window, player);
        }
    }

    for (int i = 0; i < (HEIGHT - 3); i++) {
        for (int j = 3; j < WIDTH; j++) {
            std::vector<int> window;

            for (int k = 0; k < 4; k++) {
                window.push_back(board[i+k][j-k]);
            }

            score += evalWindow(window, player);
        }
    }

    return score;
}

/**
 * @brief Return the score of any given window on the board (Non-terminal heuristic)
 * 
 * @param board 
 * @param player 
 * @return int 
 */
int evalWindow(std::vector<int> window, int player) {
    int score = 0;
    int opponent = 3 - player;

    // Prioritize winning moves
    if (std::count(window.begin(), window.end(), player) == 4) {
        score += 100;
    } else if (std::count(window.begin(), window.end(), player) == 3 && std::count(window.begin(), window.end(), 0) == 1) { // 3 in a row
        score += 5;
    } else if (std::count(window.begin(), window.end(), player) == 2 && std::count(window.begin(), window.end(), 0) == 2) { // 2 in a row
        score += 2;
    }

    // Prioritize blocking opponent
    if (std::count(window.begin(), window.end(), opponent) == 3 && std::count(window.begin(), window.end(), 0) == 1) { // 3 in a row
        score -= 4;
    }

    return score;
}

/**
 * @brief Check if a player has won
 * 
 * @param board 
 * @param player 
 * @return true 
 * @return false 
 */
bool checkWin(Connect4Board &board, Connect4Moves &winning, int player) {
    // Check horizontal
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 4; j++) {
            if (board[i][j] == player && board[i][j+1] == player && board[i][j+2] == player && board[i][j+3] == player) {
                winning.push_back(std::make_pair(i, j));
                winning.push_back(std::make_pair(i, j+1));
                winning.push_back(std::make_pair(i, j+2));
                winning.push_back(std::make_pair(i, j+3));
                return true;
            }
        }
    }

    // Check vertical
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 7; j++) {
            if (board[i][j] == player && board[i+1][j] == player && board[i+2][j] == player && board[i+3][j] == player) {
                winning.push_back(std::make_pair(i, j));
                winning.push_back(std::make_pair(i+1, j));
                winning.push_back(std::make_pair(i+2, j));
                winning.push_back(std::make_pair(i+3, j));
                return true;
            }
        }
    }

    // Check diagonal
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            if (board[i][j] == player && board[i+1][j+1] == player && board[i+2][j+2] == player && board[i+3][j+3] == player) {
                winning.push_back(std::make_pair(i, j));
                winning.push_back(std::make_pair(i+1, j+1));
                winning.push_back(std::make_pair(i+2, j+2));
                winning.push_back(std::make_pair(i+3, j+3));
                return true;
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 3; j < 7; j++) {
            if (board[i][j] == player && board[i+1][j-1] == player && board[i+2][j-2] == player && board[i+3][j-3] == player) {
                winning.push_back(std::make_pair(i, j));
                winning.push_back(std::make_pair(i+1, j-1));
                winning.push_back(std::make_pair(i+2, j-2));
                winning.push_back(std::make_pair(i+3, j-3));
                return true;
            }
        }
    }

    return false;
}

/**
 * @brief Places a piece on the board
 * 
 * @param board 
 * @param player 
 * @param column 
 * @return `true` if successful, `false` otherwise
 */
bool placePiece(Connect4Board &board, Connect4Moves &moves, int player, char column) {
    int c = column - 'A'; // Convert column to integer

    // Check if column is valid
    if (c < 0 || c > 6) {
        return false;
    }

    // Starts from the bottom and goes up, when successful, return true
    for (int i = 5; i >= 0; i--) {
        if (board[i][c] == 0) {
            board[i][c] = player;

            moves.push_back(std::make_pair(player, c));

            return true;
        }
    }

    // Column is full
    return false;
}

/**
 * @brief Check if the board is full
 * 
 * @param board 
 * @return true 
 * @return false 
 */
bool isFull(Connect4Board &board) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            if (board[i][j] == 0) {
                return false;
            }
        }
    }

    return true;
}

/**
 * @brief Check if the node is a terminal node
 * 
 * @param board 
 * @return true 
 * @return false 
 */
bool isTerminalNode(Connect4Board &board) {
    Connect4Moves moves;

    return checkWin(board, moves, PLAYER1) || checkWin(board, moves, PLAYER2) || isFull(board);
}

/**
 * @brief Minimax algorithm to determine the best move
 * 
 * @param board
 * @param moves
 * @param player
 * @param depth
 * @param alpha
 * @param beta
 * @param isMaximizing
 * 
 * @return `std::pair<int, char>` (score, column)
 */
std::pair<int, char> minimax(Connect4Board &board, int player, int depth, int alpha, int beta, bool isMaximizing) {
    Connect4Moves moves; // Isn't used, but needed for checkWin
    int opponent = 3 - player;

    if (depth == 0 || isTerminalNode(board)) {
        if (isTerminalNode(board)) {
            // Check if AI wins
            if (checkWin(board, moves, player)) {
                return std::make_pair(-999, NULL);
            
            // Check if opponent wins
            } else if (checkWin(board, moves, opponent)) {
                return std::make_pair(999, NULL);

            // Draw/ No valid moves
            } else {
                return std::make_pair(0, NULL);
            }
        
        // Return score of move
        } else {
            return std::make_pair(scoreMove(board, player), NULL);
        }
    }

    if (isMaximizing) { // Maximize player
        int bestScore = -999;
        char column = randomNum('A', 'G'); // Randomly Initialize

        for (char c = 'A'; c < WIDTH + 'A'; c++) {
            Connect4Board newBoard = board;

            if (placePiece(newBoard, moves, player, c)) {
                int new_score = minimax(newBoard, player, depth - 1, alpha, beta, false).first;

                if (new_score > bestScore) { 
                    bestScore = new_score;
                    column = c;
                }

                alpha = std::max(alpha, bestScore);

                if (alpha >= beta) {
                    break;
                }
            }
        }

        return std::make_pair(bestScore, column);
    } else { // Minimize opponent
        int bestScore = 999;
        char column = randomNum('A', 'G'); // Randomly Initialize

        for (char c = 'A'; c < WIDTH + 'A'; c++) {
            Connect4Board newBoard = board;

            if (placePiece(newBoard, moves, opponent, c)) {
                int new_score = minimax(newBoard, opponent, depth - 1, alpha, beta, true).first;

                if (new_score < bestScore) {
                    bestScore = new_score;
                    column = c;
                }

                beta = std::min(beta, bestScore);

                if (alpha >= beta) {
                    break;
                }
            }
        }

        return std::make_pair(bestScore, column);
    }
}