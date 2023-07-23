#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ncc.h"

static Token *new_token(TokenKind kind, Token *cur, char *str, int len);
static bool is_startswith(char *p, char *q);

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

        // Variables
        // Allow a-zA-Z_ in first char
        // Allow a-zA-Z_- in succeeding char
        if ('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z' || *p == '_') {
            int len = 1;
            while ('a' <= p[len] && p[len] <= 'z' ||
                   'A' <= p[len] && p[len] <= 'Z' ||
                   '0' <= p[len] && p[len] <= '9' || p[len] == '_')
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
