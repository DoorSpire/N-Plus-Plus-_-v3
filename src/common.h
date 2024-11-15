#ifndef npp_common_h
#define npp_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define UINT8_COUNT (UINT8_MAX + 1)

extern bool debug;

static inline bool hasSuffix(const char *str, const char *suffix) {
    size_t fileLen = strlen(str);
    size_t suffixLen = strlen(suffix);

    if (fileLen < suffixLen) {
        return false;
    }

    return strcmp(str + fileLen - suffixLen, suffix) == 0;
}

static inline char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

#endif