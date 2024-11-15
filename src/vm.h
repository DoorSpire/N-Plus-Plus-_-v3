#ifndef npp_vm_h
#define npp_vm_h

#include "object.h"
#include "table.h"
#include "value.h"

// Just another header file...
// But this one is very important
// wink wink

#define FRAMES_MAX 88
#define JIT_THRESHOLD 1000000
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
    ObjClosure* closure;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    Value stack[STACK_MAX];
    Value* stackTop;
    Table globals;
    Table strings;
    ObjString* initString;
    ObjUpvalue* openUpvalues;
    size_t bytesAllocated;
    size_t nextGC;
    Obj* objects;
    int grayCount;
    int grayCapacity;
    Obj** grayStack;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void init(const char** args, int argsCountt);
void runtimeError(const char* format, ...);
void defineNative(const char* name, NativeFn function);
void initVM();
void freeVM();
bool call_(ObjClosure* closure, int argCount);
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

#endif