#include "memory.h"

void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }
    
    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void shrinkValueArray(ValueArray* array) {
    if (array->capacity > array->count) {
        array->capacity = array->count;
        array->values = realloc(array->values, sizeof(Value) * array->capacity);
    }
}

void printValue(Value value) {
    if (IS_BOOL(value)) {
        printf(AS_BOOL(value) ? "[TRUE]" : "[FALSE]");
    } else if (IS_NULL(value)) {
        printf("[NULL]");
    } else if (IS_NUMBER(value)) {
        printf("%g", AS_NUMBER(value));
    } else if (IS_OBJ(value)) {
        printObject(value);
    }
}

bool valuesEqual(Value a, Value b) {
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return AS_NUMBER(a) == AS_NUMBER(b);
    }

    return a == b;
}