#include <random>
#include <iostream>

using namespace std;
#define u64 unsigned long long

u64 pieceKeys[2][64];
u64 sideKey;

u64 randomLong() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<u64> dist(numeric_limits<u64>::min(), numeric_limits<u64>::max());
    return dist(gen);
}

void generateKeys() {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 64; j++) {
            pieceKeys[i][j] = randomLong();
        }
    }
    sideKey = randomLong();
}

