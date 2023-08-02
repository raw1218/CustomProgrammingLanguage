#ifndef MAP_H
#define MAP_H

#include <stdbool.h>

typedef struct {
    void* key;
    void* value;
} KeyValuePair;

typedef struct {
    KeyValuePair* pairs;
    int count;
    int mapSize;

    bool (*compareFunction)(void* a, void* b);
} Map;

Map* createMap(bool compare(void* a, void* b));
void insert(Map* map, void* key, void* value);
void* get(Map* map, void* key);

void printMap(Map * map, void printKey(void * a),  void printItem(void * a));

void modifyData(Map * map, void * modificationValue, void * modifyFunc(void * data, void * modification));

#endif /* MAP_H */
