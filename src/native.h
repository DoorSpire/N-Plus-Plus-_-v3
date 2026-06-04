#ifndef npp_native_h
#define npp_native_h

#include "value.h"

#define MAX_ARRAYS 1000

typedef struct {
    Value* contents;
    int capacity;
    char* name;
} Array;

void init(const char** args, int argsCount);
void defineNatives();
void addArray(Array newArray);
void clsArray();
Array* getArrayByName(const char* name);

#endif
