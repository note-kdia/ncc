#ifndef NCC_H
#define NCC_H

typedef enum {
    TK_RESERVED,  // Keywords or punctuator
    TK_IDENT,     // Identifier
    TK_NUM,       // Integer literals
    TK_EOF,       // End-of-File markers
} TokenKind;

typedef struct Token Token;

// Token type
struct Token {
    TokenKind kind;  // Token kind
    Token *next;     // Next token
    int val;         // If kind is TK_NUM, its value
    char *str;       // Token string
    int len;         // string len
};

typedef enum {
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_EQ,      // ==
    ND_NEQ,     // !=
    ND_GE,      // >=
    ND_LE,      // <=
    ND_GT,      // >
    ND_LT,      // <
    ND_ASSIGN,  // assign
    ND_NUM,     // integer
    ND_LVAR,    // local vaiables
} NodeKind;

typedef struct Node Node;

// Node type of Abstract Syntax
struct Node {
    NodeKind kind;     // Node kind
    Node *lhs;         // left hand side
    Node *rhs;         // right hand side
    int val;           // If kind is ND_NUM, its value
    int offset_bytes;  // If kind is ND_LVAR, its offset from rbp.
};

// Global variables
extern char *user_input;
extern Token *token;

extern Node *code[100];

//-----------
// Functions
//-----------

// tokenize.c
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
Token *tokenize();

// parse.c
void program();

// codegen.c
void gen(Node *node);

#endif
