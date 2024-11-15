#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <windows.h>

#include "native.h"
#include "common.h"
#include "compiler.h"
#include "object.h"
#include "memory.h"
#include "vm.h"

#define MAX_ARRAYS 1000

const char** globalArgs;
int globalArgsCount;

Array globalArrays[MAX_ARRAYS];
int globalArrayCount = 0;

// Transfer the args so they can be used
void init(const char** args, int argsCountt) {
    globalArgs = args;
    globalArgsCount = argsCountt;
}

static Value clockNative(int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d.", argCount);
    }

    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value waitNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number.");
    }

    Sleep(AS_NUMBER(args[0]) * 1000);
    return NULL_VAL;
}

void formatDateTime(char* format, char* output) {
    time_t currentTime;
    struct tm* localTime;

    time(&currentTime);
    localTime = localtime(&currentTime);

    int day = localTime->tm_mday;
    int month = localTime->tm_mon + 1;
    int year = localTime->tm_year + 1900;
    int hour = localTime->tm_hour;
    int minute = localTime->tm_min;
    int second = localTime->tm_sec;

    char temp[200];
    int i, j = 0;

    for (i = 0; format[i] != '\0'; i++) {
        if (format[i] == 'D') {
            j += sprintf(&temp[j], "%02d", day);
        } else if (format[i] == 'M') {
            j += sprintf(&temp[j], "%02d", month);
        } else if (format[i] == 'Y') {
            j += sprintf(&temp[j], "%d", year);
        } else if (format[i] == 'H') {
            j += sprintf(&temp[j], "%02d", hour);
        } else if (format[i] == 'm') {
            j += sprintf(&temp[j], "%02d", minute);
        } else if (format[i] == 'S') {
            j += sprintf(&temp[j], "%02d", second);
        } else {
            temp[j++] = format[i];
        }
    }

    temp[j] = '\0';

    strcpy(output, temp);
}

static Value timeNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string.");
    }

    const char* format = AS_CSTRING(args[0]);
    char output[200];
    formatDateTime((char*)format, output);
    size_t length = strlen(output);

    return OBJ_VAL(copyString(output, length));
}

static Value argcNative(int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d.", argCount);
    }

    return NUMBER_VAL(globalArgsCount);
}

static Value argvNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number.");
    }

    int index = (int)AS_NUMBER(args[0]);
    if (index < 0 || index >= globalArgsCount) {
        runtimeError("Index out of bounds. There are %d arguments.", globalArgsCount);
    }

    const char* arg = globalArgs[index];
    if (arg == NULL) {
        runtimeError("Argument at index %d is NULL.", index);
    }

    return OBJ_VAL(copyString(arg, (int)strlen(arg)));
}

static Value stringizeNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (IS_STRING(args[0])) {
        return args[0];
    } else if (IS_NUMBER(args[0])) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%g", AS_NUMBER(args[0]));
        return OBJ_VAL(copyString(buffer, (int)strlen(buffer)));
    } else {
        runtimeError("Unsupported type for stringize.");
    }
}

static Value integizeNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (IS_STRING(args[0])) {
        char* end;
        const char* str = AS_CSTRING(args[0]);
        double number = strtod(str, &end);
        
        if (end != str && *end == '\0') {
            return NUMBER_VAL(number);
        } else {
            runtimeError("String could not be converted to a number.");
        }
    } else if (IS_NUMBER(args[0])) {
        return args[0];
    } else {
        runtimeError("Unsupported type for integize.");
    }
}

static Value isNumNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    return BOOL_VAL(IS_NUMBER(args[0]));
}

static Value isBoolNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    return BOOL_VAL(IS_BOOL(args[0]));
}

static Value isObjNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    return BOOL_VAL(IS_OBJ(args[0]));
}

static Value isStrNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    return BOOL_VAL(IS_STRING(args[0]));
}

static Value isInstanceNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    return BOOL_VAL(IS_INSTANCE(args[0]));
}

static Value isNullNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    return BOOL_VAL(IS_NULL(args[0]));
}

static Value isNativeNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    return BOOL_VAL(IS_NATIVE(args[0]));
}

static Value isBoundMethodNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    return BOOL_VAL(IS_BOUND_METHOD(args[0]));
}


static Value isClassNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    return BOOL_VAL(IS_CLASS(args[0]));
}

static Value broadcastNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    printValue(args[0]);
    printf("\n");

    return NULL_VAL;
}

static Value broadcastXNNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    printValue(args[0]);

    return NULL_VAL;
}

static Value setColorNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument 1 must be a number.");
    }

    int colorCode = (int)AS_NUMBER(args[0]);
    if (colorCode < 30 || colorCode > 38) {
        runtimeError("Argument 1 must be between or equal to 30 and 38.");
    }

    if (colorCode <= 37) {
        printf("\033[0;%dm", colorCode);
    } else if (colorCode == 38) {
        printf("\033[0m");
    }

    return NULL_VAL;
}

static Value receiveNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    printValue(args[0]);

    char input[1024];
    int i = 0;
    int c = getchar();

    while (c != '\n' && i < 1023) {
        input[i] = c;
        i++;
        c = getchar();
    }

    input[i] = '\0';
    return OBJ_VAL(copyString(input, i));
}

static Value systemNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a number.");
    }

    char* cmd = AS_CSTRING(args[0]);
    system(cmd);

    return NULL_VAL;
}

typedef int (*FunctionPointerInt)(void);
typedef void (*FunctionPointerVoid)(void);
typedef const char* (*FunctionPointerStr)(void);

uintptr_t cGetFunc(const char* dllPath, const char* funcName, int returnsInt, int argCount, ...) {
    HMODULE hModule = LoadLibrary(dllPath);
    if (!hModule) {
        runtimeError("Error loading DLL: %d\n", GetLastError());
        return 0;
    }

    FARPROC procAddress = GetProcAddress(hModule, funcName);
    if (!procAddress) {
        FreeLibrary(hModule);
        runtimeError("Error finding function %s: %d", funcName, GetLastError());
        return 0;
    }

    va_list args;
    va_start(args, argCount);

    int intResult = 0;
    const char* strResult = NULL;

    if (returnsInt) {
        if (argCount == 0) {
            FunctionPointerInt funcInt = (FunctionPointerInt)procAddress;
            intResult = funcInt();
        } else if (argCount == 1) {
            int arg1 = va_arg(args, int);
            intResult = ((int (*)(int))procAddress)(arg1);
        } else if (argCount == 2) {
            int arg1 = va_arg(args, int);
            int arg2 = va_arg(args, int);
            intResult = ((int (*)(int, int))procAddress)(arg1, arg2);
        }

        if (intResult == 0) {
            runtimeError("Function %s did not return a valid result (0).", funcName);
            va_end(args);
            FreeLibrary(hModule);
            return 0;
        }
    } else {
        if (argCount == 0) {
            FunctionPointerStr funcStr = (FunctionPointerStr)procAddress;
            strResult = funcStr();
        } else if (argCount == 1) {
            int arg1 = va_arg(args, int);
            ((void (*)(int))procAddress)(arg1);
        } else if (argCount == 2) {
            int arg1 = va_arg(args, int);
            int arg2 = va_arg(args, int);
            ((void (*)(int, int))procAddress)(arg1, arg2);
        }

        if (!strResult) {
            runtimeError("Function %s did not return a valid string result (NULL).", funcName);
            va_end(args);
            FreeLibrary(hModule);
            return 0;
        }
    }

    va_end(args);

    if (!returnsInt && strResult) {
        char* copiedStr = _strdup(strResult);
        FreeLibrary(hModule);
        return (uintptr_t)copiedStr;
    }

    FreeLibrary(hModule);
    return intResult;
}

static Value cGetFuncNative(int argCount, Value* args) {
    if (argCount < 4) {
        runtimeError("Expected at least 4 arguments, but got %d.", argCount);
    }

    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        runtimeError("First two arguments must be strings.");
    }

    if (!IS_BOOL(args[2])) {
        runtimeError("Argument 3 must be boolean.");
    }

    const char* dllPath = AS_CSTRING(args[0]);
    const char* funcName = AS_CSTRING(args[1]);
    int returnsInt = AS_BOOL(args[2]);

    uintptr_t result = 0;
    const char* strResult = NULL;

    if (argCount == 3) {
        result = cGetFunc(dllPath, funcName, returnsInt, 0);
    } else if (argCount == 4) {
        int arg1 = AS_NUMBER(args[3]);
        result = cGetFunc(dllPath, funcName, returnsInt, 1, arg1);
    } else if (argCount == 5) {
        int arg1 = AS_NUMBER(args[3]);
        int arg2 = AS_NUMBER(args[4]);
        result = cGetFunc(dllPath, funcName, returnsInt, 2, arg1, arg2);
    }

    if (returnsInt) {
        if (result == 0) {
            runtimeError("The function %s did not return a valid integer result.", funcName);
        }
        return NUMBER_VAL((int)result);
    } else {
        strResult = (const char*)result;
        if (!strResult) {
            runtimeError("The function %s did not return a valid string result.", funcName);
        }
        return OBJ_VAL(copyString(strResult, strlen(strResult)));
    }
}

static Value quitNative(int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d.", argCount);
    }

    exit(0);
}

static Value sinNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number.");
    }

    return NUMBER_VAL(sin(AS_NUMBER(args[0])));
}

static Value cosNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number.");
    }

    return NUMBER_VAL(cos(AS_NUMBER(args[0])));
}

static Value tanNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number.");
    }

    return NUMBER_VAL(tan(AS_NUMBER(args[0])));
}

static Value asinNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number.");
    }

    return NUMBER_VAL(asin(AS_NUMBER(args[0])));
}

static Value acosNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number.");
    }

    return NUMBER_VAL(acos(AS_NUMBER(args[0])));
}

static Value atanNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number.");
    }

    return NUMBER_VAL(atan(AS_NUMBER(args[0])));
}

static Value absNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number.");
    }

    return NUMBER_VAL(fabs(AS_NUMBER(args[0])));
}

static Value hypotNative(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Arguments must be a number.");
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError("Arguments must be a number.");
    }

    return NUMBER_VAL(hypot(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

static Value sqrtNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number.");
    }

    return NUMBER_VAL(sqrt(AS_NUMBER(args[0])));
}

static Value powrNative(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Arguments must be a number.");
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError("Arguments must be a number.");
    }

    return NUMBER_VAL(pow(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

static Value mdlsNative(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Arguments must be a number.");
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError("Arguments must be a number.");
    }

    int a = AS_NUMBER(args[0]);
    int b = AS_NUMBER(args[1]);
    return BOOL_VAL(a % b == 0);
}

static Value randNative(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Arguments must be a number.");
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError("Arguments must be a number.");
    }

    srand(time(0)); // Random seed
    int a = AS_NUMBER(args[0]);
    int b = AS_NUMBER(args[1]);
    return NUMBER_VAL((rand() % (b - a + 1)) + a);
}

static Value collectGarbageNative(int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d.", argCount);
    }

    collectGarbage();
    
    return NULL_VAL;
}

static Value runtimeErrorNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 arguments but got %d.", argCount);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument 1 must be a string.");
    }

    runtimeError(AS_CSTRING(args[0]));
    
    return NULL_VAL;
}

static Value getNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string.");
    }

    const char* source = readFile(AS_CSTRING(args[0]));
    ObjFunction* function = compile(source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));
    ObjClosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    call_(closure, 0);
    
    return NULL_VAL;
}

static Value strLenNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string.");
    }

    int length = strlen(AS_CSTRING(args[0]));
    return NUMBER_VAL(length);
}

static Value strIndexNative(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument 1 must be a string.");
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError("Argument 2 must be a number.");
    }

    const char* cstr = AS_CSTRING(args[0]);
    int length = strlen(cstr);
    int index = (int)AS_NUMBER(args[1]);

    if (index >= 0 && index < length) {
        char result[2] = {cstr[index], '\0'};
        return OBJ_VAL(copyString(result, 1));
    }

    return NULL_VAL;
}

void addArray(Array newArray) {
    if (globalArrayCount >= MAX_ARRAYS) {
        runtimeError("Exceeded the maximum number of arrays allowed.");
    }
    globalArrays[globalArrayCount++] = newArray;
}

Array* getArrayByName(const char* name) {
    for (int i = 0; i < globalArrayCount; i++) {
        if (strcmp(globalArrays[i].name, name) == 0) {
            return &globalArrays[i];
        }
    }
    return NULL;
}

void removeArrayFromGlobalList(const char* name) {
    for (int i = 0; i < globalArrayCount; i++) {
        if (strcmp(globalArrays[i].name, name) == 0) {
            for (int j = 0; j < globalArrays[i].capacity; j++) {
                free(globalArrays[i].contents[j]);
            }
            free(globalArrays[i].contents);
            free(globalArrays[i].name);

            for (int j = i; j < globalArrayCount - 1; j++) {
                globalArrays[j] = globalArrays[j + 1];
            }

            globalArrayCount--;
            return;
        }
    }
}

static Value arrayNative(int argCount, Value* args) {
    if (argCount < 1) {
        runtimeError("Expected at least 1 argument but got %d.", argCount);
    }

    for (int i = 0; i < argCount; i++) {
        if (!IS_STRING(args[i])) {
            runtimeError("Arguments must be strings");
        }
    }

    Array newArray;
    newArray.capacity = argCount - 1;
    newArray.contents = malloc(newArray.capacity * sizeof(char*));
    newArray.name = strdup(AS_CSTRING(args[0]));

    for (int i = 1; i < argCount; i++) {
        newArray.contents[i - 1] = strdup(AS_CSTRING(args[i]));
    }

    addArray(newArray);

    return NULL_VAL;
}

void clsArray() {
    memset(globalArrays, 0, sizeof(globalArrays));
}

static Value getArrayNative(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument 1 must be a string.");
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError("Argument 2 must be a number.");
    }

    Array* array = getArrayByName(AS_CSTRING(args[0]));
    if (array == NULL) {
        runtimeError("Array with name '%s' not found.", AS_CSTRING(args[0]));
    }

    int index = (int)AS_NUMBER(args[1]);

    if (index < 0 || index >= array->capacity) {
        runtimeError("Index %d out of bounds for array '%s' (size: %d).", index, array->name, array->capacity);
    }

    return OBJ_VAL(copyString(array->contents[index], (int)strlen(array->contents[index])));
}

static Value lenArrayNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 arguments but got %d.", argCount);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string.");
    }

    Array* array = getArrayByName(AS_CSTRING(args[0]));
    if (array == NULL) {
        runtimeError("Array with name '%s' not found.", AS_CSTRING(args[0]));
    }

    return NUMBER_VAL(array->capacity);
}

static Value addArrayNative(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 1 arguments but got %d.", argCount);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Arguments must be a number.");
    }

    if (!IS_STRING(args[1])) {
        runtimeError("Arguments must be a number.");
    }

    Array* array = getArrayByName(AS_CSTRING(args[0]));
    if (array == NULL) {
        runtimeError("Array with name '%s' not found.", AS_CSTRING(args[0]));
    }

    int newCapacity = array->capacity + 1;  // Increment capacity by 1
    array->contents = realloc(array->contents, newCapacity * sizeof(char*));

    if (array->contents == NULL) {
        runtimeError("Failed to allocate memory for array expansion.");
    }

    array->contents[array->capacity] = strdup(AS_CSTRING(args[1]));
    array->capacity++;

    return NULL_VAL;
}

static Value rmvArrayNative(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d.", argCount);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument 1 must be a string.");
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError("Argument 2 must be a number.");
    }

    Array* array = getArrayByName(AS_CSTRING(args[0]));
    if (array == NULL) {
        runtimeError("Array with name '%s' not found.", AS_CSTRING(args[0]));
    }

    int index = (int)AS_NUMBER(args[1]);

    if (index < 0 || index >= array->capacity) {
        runtimeError("Index %d out of bounds for array '%s' (size: %d).", index, array->name, array->capacity);
    }

    free(array->contents[index]);

    for (int i = index; i < array->capacity - 1; i++) {
        array->contents[i] = array->contents[i + 1];
    }

    int newCapacity = array->capacity - 1;
    array->contents = realloc(array->contents, newCapacity * sizeof(char*));
    
    if (array->contents == NULL && newCapacity > 0) {
        runtimeError("Failed to allocate memory while shrinking the array.");
    }

    array->capacity = newCapacity;

    return NULL_VAL;
}

static Value cngArrayNative(int argCount, Value* args) {
    if (argCount != 3) {
        runtimeError("Expected 3 arguments but got %d.", argCount);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument 1 must be a string.");
    }

    if (!IS_STRING(args[2])) {
        runtimeError("Argument 3 must be a string.");
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError("Argument 2 must be a number.");
    }

    Array* array = getArrayByName(AS_CSTRING(args[0]));
    if (array == NULL) {
        runtimeError("Array with name '%s' not found.", AS_CSTRING(args[0]));
    }

    int index = (int)AS_NUMBER(args[1]);

    if (index < 0 || index >= array->capacity) {
        runtimeError("Index %d out of bounds for array '%s' (size: %d).", index, array->name, array->capacity);
    }

    array->contents[index] = AS_CSTRING(args[2]);
    array->contents = realloc(array->contents, array->capacity * sizeof(char*));
    array->capacity = array->capacity;

    return NULL_VAL;
}

static Value delArrayNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string.");
    }

    const char* arrayName = AS_CSTRING(args[0]);
    
    Array* array = getArrayByName(arrayName);
    if (array == NULL) {
        runtimeError("Array with name '%s' not found.", arrayName);
    }

    for (int i = 0; i < array->capacity; i++) {
        free(array->contents[i]);
    }

    free(array->contents);
    free(array->name);
    removeArrayFromGlobalList(arrayName);

    return NULL_VAL;
}

static Value bctArrayNative(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d.", argCount);
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string.");
    }

    const char* arrayName = AS_CSTRING(args[0]);
    
    Array* array = getArrayByName(arrayName);
    if (array == NULL) {
        runtimeError("Array with name '%s' not found.", arrayName);
    }

    printf("[");
    for (int i = 0; i < array->capacity; i++) {
        if (i == array->capacity - 1) {
            printf("%s", array->contents[i]);
        } else {
            printf("%s, ", array->contents[i]);
        }
    }
    printf("]\n");
}

// * Defines all the native functions
void defineNatives() {
    // Time section
    defineNative("clock", clockNative);
    defineNative("wait", waitNative);
    defineNative("time", timeNative);

    // Args and value section
    defineNative("argc", argcNative);
    defineNative("argv", argvNative);
    defineNative("stringize", stringizeNative);
    defineNative("integize", integizeNative);

    // Value checking section
    defineNative("isNum", isNumNative);
    defineNative("isBool", isBoolNative);
    defineNative("isObj", isObjNative);
    defineNative("isStr", isStrNative);
    defineNative("isNull", isNullNative);
    defineNative("isInst", isInstanceNative);
    defineNative("isNative", isNativeNative);
    defineNative("isClass", isClassNative);
    defineNative("isBoundMethod", isBoundMethodNative);

    // I/O section
    defineNative("broadcast", broadcastNative);
    defineNative("broadcastXN", broadcastXNNative);
    defineNative("setColor", setColorNative);
    defineNative("receive", receiveNative);
    defineNative("system", systemNative);
    defineNative("cGetFunc", cGetFuncNative);
    defineNative("quit", quitNative);

    // Triginometry section
    defineNative("sin", sinNative);
    defineNative("cos", cosNative);
    defineNative("tan", tanNative);
    defineNative("abs", absNative);
    defineNative("asin", asinNative);
    defineNative("acos", acosNative);
    defineNative("atan", atanNative);
    defineNative("hypot", hypotNative);

    // Math section
    defineNative("sqrt", sqrtNative);
    defineNative("powr", powrNative);
    defineNative("mdls", mdlsNative);
    defineNative("rand", randNative);

    // Language development kit section
    defineNative("collectGarbage", collectGarbageNative);
    defineNative("runtimeError", runtimeErrorNative);
    defineNative("get", getNative);
    defineNative("strLen", strLenNative);
    defineNative("strIndex", strIndexNative);

    // Array section
    defineNative("array", arrayNative);
    defineNative("getArray", getArrayNative);
    defineNative("lenArray", lenArrayNative);
    defineNative("addArray", addArrayNative);
    defineNative("rmvArray", rmvArrayNative);
    defineNative("cngArray", cngArrayNative);
    defineNative("delArray", delArrayNative);
    defineNative("bctArray", bctArrayNative);
}