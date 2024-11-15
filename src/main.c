#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "vm.h"
#include "native.h"

bool debug = false;

static void repl() {
    char line[1024];
    for (;;) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        interpret(line);
    }
}

static void runMain(const char* path) {
    if (!hasSuffix(path, ".npp")) {
        fprintf(stderr, "Error: The file \"%s\" does not have the required \".npp\" extension.\n", path);
        exit(74);
    }

    char* source = readFile(path);
    if (source == NULL) {
        fprintf(stderr, "Error: Unable to read the file \"%s\".\n", path);
        exit(66); // File read error
    }

    InterpretResult result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
    clsArray();
}

int main(int argc, const char* argv[]) {
    initVM();
    const char* suffix = ".npp";

    if (argc == 2 && strcmp(argv[1], "help") == 0) {
        printf("Usage: nppc2 [main_file] // [args...]\n");
        exit(0);
    } else if (argc == 1) {
        repl();
    } else if (argc == 2 && hasSuffix(argv[1], suffix)) {
        runMain(argv[1]);
    } else if (argc >= 3 && hasSuffix(argv[1], suffix)) {
        if (strcmp(argv[2], "//") == 0) {
            int argsCount = argc - 3;
            const char** args = &argv[3];
            init(args, argsCount);
            runMain(argv[1]);
        } else if (strcmp(argv[2], "--debug") == 0) {
            debug = true;
            if (argc > 3 && strcmp(argv[3], "//") == 0) {
                int argsCount = argc - 4;
                const char** args = &argv[4];
                init(args, argsCount);
                runMain(argv[1]);
            } else {
                runMain(argv[1]);
            }
        }
    }

    freeVM();
    return 0;
}