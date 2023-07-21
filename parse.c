#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ncc.h"

static bool consume(char *op);
static Token *consume_ident();
static void expect(char *op);
static int expect_number();
static bool is_at_eof();

static Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
static Node *new_node_lvar(int offset);
static Node *new_node_num(int val);

// Abstruct Syntax Tree
// -------------------------------------------------------------------------
// program  = statement*
// statement= expr ";"
// expr		= assign
// assign   = equality ( "=" assign )?
// equality	= relational ( "==" relational | "!=" relational )*
// relational	= add ( ">=" add | "<=" add | ">" add | "<" add )*
// add		= mul ( "+" mul | "-" mul )*
// mul		= unary ( "*" unary | "/" unary )*
// unary	= ( "+" | "-" )? primary
// primary	= num | ident | "(" expr ")"
// -------------------------------------------------------------------------
static Node *statement();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

Node *code[100];

// Syntax Rules
// program  = statement*
void program() {
    int i = 0;
    while (!is_at_eof()) code[i++] = statement();
    code[i] = NULL;
}

// statement= expr ";"
static Node *statement() {
    Node *node = expr();
    expect(";");
    return node;
}

// expr		= assign
static Node *expr() { return assign(); }

// assign   = equality ( "=" assign )?
static Node *assign() {
    Node *node = equality();
    if (consume("=")) node = new_node(ND_ASSIGN, node, assign());
    return node;
}

// equality	= relational ( "==" relational | "!=" relational )*
static Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NEQ, node, relational());
        else
            return node;
    }
}

// relational	= add ( ">=" add | "<=" add | ">" add | "<" add )*
static Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume(">="))
            node = new_node(ND_GE, node, add());
        else if (consume("<="))
            node = new_node(ND_LE, node, add());
        else if (consume(">"))
            node = new_node(ND_GT, node, add());
        else if (consume("<"))
            node = new_node(ND_LT, node, add());
        else
            return node;
    }
}

// add		= mul ( "+" mul | "-" mul )*
static Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul		= unary ( "*" unary | "/" unary )*
static Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary	= ( "+" | "-" )? primary
static Node *unary() {
    if (consume("+")) return primary();
    if (consume("-")) return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

// primary	= num | ident | "(" expr ")"
static Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok) {
        return new_node_lvar(tok->str[0] - 'a' + 1);
    }

    return new_node_num(expect_number());
}

static Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_node_lvar(int offset_stacks) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset_bytes = offset_stacks * 8;
    return node;
}

static Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

static bool is_at_eof() {
    if (token->kind == TK_EOF) return true;
    return false;
}

// Consumes the current token if it matches `op`.
static bool consume(char *op) {
    if (token->kind != TK_RESERVED || token->len != strlen(op) ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

static Token *consume_ident() {
    if (token->kind != TK_IDENT || token->len != 1) {
        return false;
    }
    Token *tok = token;
    token = token->next;
    return tok;
}

// Ensure that the current token is `op`.
static void expect(char *op) {
    if (token->kind != TK_RESERVED || token->len != strlen(op) ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "expected '%s'", op);
    token = token->next;
}

// Ensure that the current token is TK_NUM.
static int expect_number() {
    if (token->kind != TK_NUM) error_at(token->str, "expected a number");
    int val = token->val;
    token = token->next;
    return val;
}
