#include <stdio.h>
#include "debug.h"
#include "object.h"
#include "value.h"

void disassembleChunk(Chunk* chunk, const char* name) {
    printf("\033[0;33m");
    printf("== %s ==\n", name);
    printf("\033[0m");
    
    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
    printf("\033[0;36m");
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s", name);
    printf("\033[0;31m");
    printf(" %4d '", constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    printf("\033[0m");
    return offset + 2;
}

static int invokeInstruction(const char* name, Chunk* chunk, int offset) {
    printf("\033[0;36m");
    uint8_t constant = chunk->code[offset + 1];
    uint8_t argCount = chunk->code[offset + 2];
    printf("%-16s", name);
    printf("\033[0;31m");
    printf(" (%d args) %4d '", argCount, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    printf("\033[0m");
    return offset + 3;
}

static int simpleInstruction(const char* name, int offset) {
    printf("\033[0;36m");
    printf("%s\n", name);
    printf("\033[0m");
    return offset + 1;
}

static int variableInstruction(const char* name, Chunk* chunk, int offset) {
    printf("\033[0;36m");
    uint8_t varSlotOrConstant = chunk->code[offset + 1];
    uint8_t isSet = chunk->code[offset + 2];
    
    printf("%-16s", name);
    printf("\033[0;31m");
    printf(" %4d ", varSlotOrConstant);
    printf(isSet ? "SET: '" : "GET: '");
    
    if (strcmp(name, "OP_GLOBAL") == 0) {
        printValue(chunk->constants.values[varSlotOrConstant]);
    } else {
        printf("%d", varSlotOrConstant);
    }
    printf("'\n");
    printf("\033[0m");
    return offset + 3;
}

static int jumpInstruction(const char* name, int sign, Chunk* chunk, int offset) {
    printf("\033[0;36m");
    int16_t jump = (int16_t)(chunk->code[offset + 1] << 8);
    jump |= chunk->code[offset + 2];
    printf("%-16s", name);
    printf("\033[0;31m");
    printf(" %4d -> %d\n", offset, offset + 3 + sign * jump);
    printf("\033[0m");
    return offset + 3;
}

static int byteInstruction(const char* name, Chunk* chunk, int offset) {
    printf("\033[0;36m");
    uint8_t slot = chunk->code[offset + 1];
    printf("%-16s", name);
    printf("\033[0;31m");
    printf(" %4d\n", slot);
    printf("\033[0m");
    return offset + 2;
}

int disassembleInstruction(Chunk* chunk, int offset) {
    printf("\033[0;35m");
    printf("%04d ", offset);
    printf("\033[0;36m");
    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", chunk->lines[offset]);
    }
    
    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_BOOL:
            return constantInstruction("OP_BOOL", chunk, offset);
        case OP_POP:
            return simpleInstruction("OP_POP", offset);
        case OP_LOCAL:
            return variableInstruction("OP_LOCAL", chunk, offset);
        case OP_GLOBAL:
            return variableInstruction("OP_GLOBAL", chunk, offset);
        case OP_DEFINE_GLOBAL:
            return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
        case OP_UPVALUE:
            return variableInstruction("OP_UPVALUE", chunk, offset);
        case OP_GET_PROPERTY:
            return constantInstruction("OP_GET_PROPERTY", chunk, offset);
        case OP_SET_PROPERTY:
            return constantInstruction("OP_SET_PROPERTY", chunk, offset);
        case OP_GET_SUPER:
            return constantInstruction("OP_GET_SUPER", chunk, offset);
        case OP_BINARY:
            return constantInstruction("OP_BINARY", chunk, offset);
        case OP_COMPARE:
            return constantInstruction("OP_COMPARE", chunk, offset);
        case OP_UNARY:
            return simpleInstruction("OP_UNARY", offset);
        case OP_JUMP:
            return jumpInstruction("OP_JUMP", 1, chunk, offset);
        case OP_JUMP_IF_FALSE:
            return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
        case OP_CALL:
            return byteInstruction("OP_CALL", chunk, offset);
        case OP_INVOKE:
            return invokeInstruction("OP_INVOKE", chunk, offset);
        case OP_SUPER_INVOKE:
            return invokeInstruction("OP_SUPER_INVOKE", chunk, offset);
        case OP_CLOSURE: {
            offset++;
            uint8_t constant = chunk->code[offset++];
            printf("\033[0;36m");
            printf("%-16s ", "OP_CLOSURE");
            printf("\033[0;31m");
            printf("%4d ", constant);
            printValue(chunk->constants.values[constant]);
            printf("\n");

            ObjFunction* function = AS_FUNCTION(chunk->constants.values[constant]);
            for (int j = 0; j < function->upvalueCount; j++) {
                int isLocal = chunk->code[offset++];
                int index = chunk->code[offset++];
                printf("%04d      |                     %s %d\n", offset - 2, isLocal ? "local" : "upvalue", index);
            }

            printf("\033[0m");
            return offset;
        }
        case OP_CLOSE_UPVALUE:
            return simpleInstruction("OP_CLOSE_UPVALUE", offset);
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        case OP_CLASS:
            return constantInstruction("OP_CLASS", chunk, offset);
        case OP_INHERIT:
            return simpleInstruction("OP_INHERIT", offset);
        case OP_METHOD:
            return constantInstruction("OP_METHOD", chunk, offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}