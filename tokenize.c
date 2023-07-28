#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ncc.h"

static Token *new_token(TokenKind kind, Token *cur, char *str, int len);
static bool is_startswith(char *p, char *q);

static bool is_alphabet(char c);
static bool is_number(char c);
static bool is_underscore(char c);
static bool is_alnumunderscore(char c);

// Tokenize `p` and returns new tokens.
Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // Skip whitespace characters.
        if (isspace(*p)) {
            p++;
            continue;
        }

        // Punctuator (Multi letter)
        if (is_startswith(p, "==") || is_startswith(p, "!=") ||
            is_startswith(p, "<=") || is_startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        // Punctuator (Single letter)
        if (strchr("+-*/()<>;=", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnumunderscore(p[6])) {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnumunderscore(p[2])) {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        // Variables
        // Allow a-zA-Z_ in first char
        // Allow a-zA-Z_- in succeeding char
        if (is_alphabet(*p) || is_underscore(*p)) {
            int len = 1;
            while (is_alphabet(p[len]) || is_number(p[len]) ||
                   is_underscore(p[len]))
                len++;
            cur = new_token(TK_IDENT, cur, p, len);
            p += len;
            continue;
        }

        // Integer literal
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "tokenizer: Invalid Token");
    }

    // EOF
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

// Reports an error location and exit.
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");  // print whitespace pos times
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Reports an error and exit.
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Create a new token and add it as the next token of `cur`.
static Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

static bool is_startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

static bool is_alphabet(char c) {
    return ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z');
}

static bool is_number(char c) { return ('0' <= c && c <= '9'); }

static bool is_underscore(char c) { return (c == '_'); }

static bool is_alnumunderscore(char c) {
    return (is_alphabet(c) || is_number(c) || is_underscore(c));
}
