#ifndef CONNECT4_ZOBRIST_H
#define CONNECT4_ZOBRIST_H

void generateKeys();
extern unsigned long long pieceKeys[2][64];
extern unsigned long long sideKey;

#endif //CONNECT4_ZOBRIST_H
