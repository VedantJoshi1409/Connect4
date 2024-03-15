#ifndef CONNECT4_TTABLE_H
#define CONNECT4_TTABLE_H

#define u64 unsigned long long
extern int flagExact;
extern int flagAlpha;
extern int flagBeta;
extern int noValue;
extern int initialCapacity;

struct TTEntry {
    u64 key;
    int depth;
    int flag;
    int value;
};

struct TTable {
    struct TTEntry **entries;
    int size;
    int count;
};

struct TTEntry *createEntry(u64 key, int depth, int flag, int value);
struct TTable *createTable();
extern TTable *tt;
void writeEntry(u64 key, int depth, int flag, int value);
int readEntry(u64 key, int alpha, int beta, int depth);


#endif //CONNECT4_TTABLE_H
