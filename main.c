#include <stdio.h>
#include <stdlib.h>

#include "ncc.h"

// Global variables
char *user_input;  // Input program
Token *token;      // Input program
LVar *locals;      // local variables

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Give proper number of arguments\n");
        return 1;
    }

    // Initialize locals<LVar *>
    locals = calloc(1, sizeof(LVar));

    user_input = argv[1];
    token = tokenize();
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // prologue
    // Allocate stacks of 26(alphabets) variables
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    // Generate codes
    for (int i = 0; code[i]; i++) {
        gen(code[i]);

        // be sure to pop
        printf("    pop rax\n");
    }

    // Returns the result of last statement
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}
