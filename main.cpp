#include <iostream>
#include <vector>
#include "Zobrist.h"
#include "TTable.h"
#include <chrono>

using namespace std;
#define u64 unsigned long long

//Board representation
//0  1  2  3  4  5  6
//8  9 10 11 12 13 14
//16 17 18 19 20 21 22
//24 25 26 27 28 29 30
//32 33 34 35 36 37 38
//40 41 42 43 44 45 46

u64 files[] = {1103823438081, 2207646876162, 4415293752324, 8830587504648, 17661175009296, 35322350018592,
               70644700037184};
//based on move on square 27
u64 winPos[] = {
        134480385, 68853957120, 35253226045440, 18049651735265280, //Diagonals from top right to bottom left
        135274560, 17315143680, 2216338391040, 283691314053120, //Diagonals from top left to bottom right
        134744072, 34494482432, 8830587502592, 2260630400663552, //Top down
        251658240, 503316480, 1006632960, 2013265920 //Left to right
};
u64 fileBoundaries[] = {8102099357864587376, 7088771780531216480, 4918001437788618816, 0, 72340172838076673,
                        217020518514230019, 5118067228293924615};
u64 rankBoundaries[] = {281470681743360, 280375465082880, 0, 0, 255, 65535};
u64 primeSquares = 30906584662016;

int pvLength[42];
int pvTable[42][42];
int previousPV[42][42];
int totalDepth;
int moveCount = 0;

u64 startTime;
u64 thinkTime;

int countBits(u64 bits) {
    int counter = 0;
    while (bits != 0) {
        counter++;
        bits &= bits - 1;
    }
    return counter;
}

int getLS1B(u64 bits) {
    if (bits != 0) {
        return countBits((bits & -bits) - 1);
    }
    return -1;
}

void copyArray(int source[42][42], int dest[42][42]) {
    for (int i = 0; i < 42; i++) {
        for (int j = 0; j < 42; j++) {
            dest[i][j] = source[i][j];
            source[i][j] = 0;
        }
    }
}

u64 currentMS() {
    auto now = chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
}

vector<int> getMoves(u64 board) {
    vector<int> moves;
    int highestBit;
    for (int i = 0; i < 7; i++) {
        highestBit = getLS1B(board & files[i]);
        if (highestBit >= 8) { //top is not filled
            moves.push_back(highestBit - 8);
        } else if (highestBit == -1) {
            moves.push_back(40 + i);
        }
    }
    return moves;
}

vector<int> reorderMoves(vector<int> moves, int pvMove) {
    vector<int> orderedMoves;
    for (int move : moves) {
        if (move == pvMove) {
            orderedMoves.insert(orderedMoves.begin(), move);
        } else {
            orderedMoves.push_back(move);
        }
    }
    return orderedMoves;
}

int eval(u64 fBoard, u64 eBoard, int pastMove) {
    u64 winLine;
    int file = pastMove % 8;
    int rank = pastMove / 8;
    eBoard &= ~(fileBoundaries[file] | rankBoundaries[rank]); //remove unreachable areas

    int positionalScore = 0;
    positionalScore += countBits(fBoard & primeSquares);
    positionalScore -= countBits(eBoard & primeSquares);

    for (u64 pos: winPos) {
        if (pastMove > 27) {
            winLine = pos << (pastMove - 27);
        } else if (pastMove < 27) {
            winLine = pos >> (27 - pastMove);
        } else {
            winLine = pos;
        }

        if ((winLine & eBoard) == winLine && countBits(winLine) >= 4) {
            return -999 + positionalScore;
        }
    }
    return positionalScore;
}

int negaMax(u64 fBoard, u64 eBoard, u64 key, int player, int pastMove, int depth, int alpha, int beta) {
    int pvIndex = totalDepth - depth;
    pvLength[pvIndex] = pvIndex;

    int score = readEntry(key, alpha, beta, depth);
    if (score != noValue) {
        return score;
    }

    score = eval(fBoard, eBoard, pastMove);
    if (score < -6 || depth == 0) {
        return score - depth;
    }

    int hashFlag = flagAlpha;
    vector<int> moves = reorderMoves(getMoves(fBoard | eBoard), previousPV[pvIndex][pvIndex]);
    for (int move: moves) {
        if (currentMS() - startTime > thinkTime) {
            break;
        }
        score = -negaMax(eBoard, fBoard | (1ULL << move), key ^ sideKey ^ pieceKeys[player][move], (player + 1) % 2,
                         move, depth - 1, -beta, -alpha);
        if (currentMS() - startTime > thinkTime) {
            break;
        }
        if (score >= beta) {
            writeEntry(key, depth, flagBeta, score);
            return beta;
        }
        if (score > alpha) {
            hashFlag = flagExact;
            alpha = score;

            pvTable[pvIndex][pvIndex] = move;
            for (int j = pvIndex+1; j < pvLength[pvIndex+1]; j++) {
                pvTable[pvIndex][j] = pvTable[pvIndex+1][j];
            }
            pvLength[pvIndex] = pvLength[pvIndex+1];
        }
    }
    writeEntry(key, depth, hashFlag, alpha);
    return alpha;
}

int negaMax(u64 fBoard, u64 eBoard, u64 key, int player, int depth) {
    pvLength[0] = 0;
    int alpha = -999999;
    int beta = 999999;
    int score = readEntry(key, alpha, beta, depth);
    if (score != noValue) {
        return score;
    }
    int bestMove;
    int hashFlag = flagAlpha;
    vector<int> moves = reorderMoves(getMoves(fBoard | eBoard), previousPV[0][0]);
    for (int move: moves) {
        if (currentMS() - startTime > thinkTime) {
            return 0;
        }
        score = -negaMax(eBoard, fBoard | (1ULL << move), key ^ sideKey ^ pieceKeys[player][move], (player + 1) % 2,
                         move, depth - 1, -beta, -alpha);
        if (currentMS() - startTime > thinkTime) {
            return 0;
        }
        if (score > alpha) {
            hashFlag = flagExact;
            alpha = score;
            bestMove = move;

            pvTable[0][0] = move;
            for (int j = 1; j < pvLength[1]; j++) {
                pvTable[0][j] = pvTable[1][j];
            }
            pvLength[0] = pvLength[1];
        }
    }
    writeEntry(key, depth, hashFlag, alpha);
    return bestMove;
}

int engineMove(u64 fBoard, u64 eBoard, u64 key, int player, int time) {
    startTime = currentMS();
    thinkTime = time;
    int temp;
    int bestMove = -1;

    for (int i = 1; i <= 42-moveCount; i++) {
        totalDepth = i;
        temp = negaMax(fBoard, eBoard, key, player, i);
        if (currentMS() - startTime > thinkTime) {
            break;
        } else {
            cout<<"Depth: "<<i<<endl;
            bestMove = temp;
            copyArray(pvTable, previousPV);
        }
    }
    return bestMove;
}

bool gameOver(u64 fBoard, u64 eBoard, int pastMove) {
    if ((fBoard & eBoard) == 140185576636287) { //board filled position
        return true;
    }
    if (eval(fBoard, eBoard, pastMove) < -6) {
        //only smaller than -6 when game over. eval can be at max -6 if all prime squares filled by opponent and game not over
        return true;
    }
    return false;
}

int stringMoveToInt(u64 board, string move) {
    int file = move[0] - 'a';
    u64 bits = board & files[file];
    int square = getLS1B(bits);
    if (square == -1) {
        return 40 + file;
    } else {
        return square - 8;
    }
}

void printBoard(bool player, u64 fBoard, u64 eBoard, int pastMove) {
    char p1;
    char p2;
    if (player) {
        p1 = 'x';
        p2 = 'o';
        cout<<"Player Board: " << eBoard<<" Computer Board: "<<fBoard<<" Past Move: "<<pastMove<<endl;
    } else {
        p1 = 'o';
        p2 = 'x';
        cout<<"Player Board: " << fBoard<<" Computer Board: "<<eBoard<<" Past Move: "<<pastMove<<endl;
    }
    char currentToken;
    int square;
    cout << "---------------" << endl;
    for (int i = 0; i < 6; i++) {
        cout << '|';
        for (int j = 0; j < 7; j++) {
            square = i * 8 + j;
            currentToken = ' ';
            if ((fBoard & 1ULL << square) != 0) {
                currentToken = p1;
            } else if ((eBoard & 1ULL << square) != 0) {
                currentToken = p2;
            }
            if (pastMove == square) {
                currentToken = toupper(currentToken);
            }
            cout << currentToken << '|';
        }
        cout << endl << "---------------" << endl;
    }
    cout << " a b c d e f g" << endl << endl;
}

int main() {
    generateKeys();
    u64 fBoard = 0ULL;
    u64 eBoard = 0ULL;
    u64 temp;
    u64 key = 0;
    int pastMove = -1;
    int player = 0;
    string stringMove;

    while (!gameOver(fBoard, eBoard, pastMove)) {
        printBoard(player % 2, fBoard, eBoard, pastMove);
        if (player % 2 == 0) {
            cout << "Enter move" << endl;
            cin >> stringMove;
            pastMove = stringMoveToInt(fBoard | eBoard, stringMove);
        } else {
            pastMove = engineMove(fBoard, eBoard, key, player % 2, 1000);
        }

        fBoard |= 1ULL << pastMove;
        key = key ^ sideKey ^ pieceKeys[player % 2][pastMove];
        temp = fBoard;
        fBoard = eBoard;
        eBoard = temp;
        player++;
        moveCount++;
    }
    printBoard(player % 2, fBoard, eBoard, pastMove);
    cout << "Game Over";
    return 0;
}