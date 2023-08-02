#include <stdio.h>
#include <stdlib.h>

#include "Map.h"





Map * createMap(bool compare(void* a, void* b)){
    Map * map = (Map *)malloc(sizeof(Map));

    
    KeyValuePair * pairs = (KeyValuePair*)malloc(sizeof(KeyValuePair));
    map->count = 0;
    map->mapSize = 1;
    map->pairs = pairs;
    map->compareFunction = compare;
    return map;
}


void insert(Map * map, void * key, void * value){




    //If key exists, replace value
    for (int i = 0; i < map->count; i++){
        if (map->compareFunction(map->pairs[i].key, key)){
            map->pairs[i].value = value;
            return;
        }
    }



    if (map->count >= map->mapSize) {
        // If the map is full, resize it
        map->mapSize *= 2;
        map->pairs = realloc(map->pairs, map->mapSize * sizeof(KeyValuePair));
    }



    map->pairs[map->count].key = key;
    map->pairs[map->count].value = value;
    map->count++;






}




void* get(Map * map, void * key){
    for (int i = 0; i < map->count; i++) {
        if (map->compareFunction(map->pairs[i].key, key)) {
            return map->pairs[i].value;
        }
    }
    return NULL;
}





void modifyData(Map * map, void * modificationValue, void * modifyFunc(void * data, void * modification)){

    for(int i = 0; i < map->count; i++){


        KeyValuePair  pair = map->pairs[i];
        void * data = pair.value;

        void * newData = modifyFunc(data, modificationValue);
        map->pairs[i].value = newData;

    }
}


void printMap(Map * map, void printKey(void * a), void printValue(void * a)){
    printf("Map contents:\n");
    for (int i = 0; i < map->count; i++) {
        KeyValuePair pair = map->pairs[i];

        printf("Key: ");
        printKey(pair.key);
        printf(" Value: ");
        printValue(pair.value);
        printf("\n");
    }
}