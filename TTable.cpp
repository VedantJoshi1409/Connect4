#include <cstdlib>
#include <cstring>

#define u64 unsigned long long
int flagExact = 0;
int flagAlpha = 1;
int flagBeta = 2;
int noValue = 123456789;
int initialCapacity = 64000000;


struct TTEntry {
    u64 key;
    int depth;
    int flag;
    int value;
};

struct TTable {
    TTEntry **entries;
    int size;
    int count;
};

TTEntry *createEntry(u64 key, int depth, int flag, int value) {
    TTEntry *entry = (TTEntry *) malloc(sizeof(TTEntry));
    entry->key = key;
    entry->depth = depth;
    entry->flag = flag;
    entry->value = value;
    return entry;
}

TTable *createTable() {
    TTable *table = (TTable *) malloc(sizeof(TTable));
    table->size = initialCapacity;
    table->count = 0;
    table->entries = (TTEntry **) calloc(initialCapacity, sizeof(TTEntry *));
    return table;
}

TTable *tt = createTable();

void writeEntry(u64 key, int depth, int flag, int value) {
    TTEntry *entry = tt->entries[key % initialCapacity];
    if (entry == nullptr) {
        tt->entries[key % initialCapacity] = createEntry(key, depth, flag, value);
    } else {
        if (depth > entry->depth) {
            entry->key = key;
            entry->depth = depth;
            entry->flag = flag;
            entry->value = value;
        }
    }
}

int readEntry(u64 key, int alpha, int beta, int depth) {
    TTEntry *entry = tt->entries[key%initialCapacity];
    if (entry != nullptr && entry->depth >= depth && entry->key == key) {
        if (entry->flag == flagExact) {
            return entry->value;
        } else if (entry->flag == flagAlpha && entry->value <= alpha) {
            return alpha;
        } else if (entry->flag == flagBeta && entry->value >= beta) {
            return beta;
        }
    }
    return noValue;
}