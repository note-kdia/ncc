#include <stdio.h>

#include "ncc.h"

// Global variables
char *user_input;  // Input program
Token *token;      // Input program

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Give proper number of arguments\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    Node *node = parse();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // Traverse the AST to emit assembly.
    gen(node);

    // A result must be at the top of the stack, so pop it
    // to RAX to make it a program exit code.
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}