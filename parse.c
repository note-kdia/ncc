#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ncc.h"

static bool consume(char *op);
static void expect(char *op);
static int expect_number();

static Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
static Node *new_node_num(int val);

// Abstruct Syntax Tree
// -------------------------------------------------------------------------
// expr		= equality
// equality	= relational ( "==" relational | "!=" relational )*
// relational	= add ( ">=" add | "<=" add | ">" add | "<" add )*
// add		= mul ( "+" mul | "-" mul )*
// mul		= unary ( "*" unary | "/" unary )*
// unary	= ( "+" | "-" )? primary
// primary	= num | "(" expr ")"
// -------------------------------------------------------------------------
static Node *expr();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

// Parse tokens
Node *parse() { return expr(); }

// Consumes the current token if it matches `op`.
static bool consume(char *op) {
    if (token->kind != TK_RESERVED || token->len != strlen(op) ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

// Ensure that the current token is `op`.
static void expect(char *op) {
    if (token->kind != TK_RESERVED || token->len != strlen(op) ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "expected '%c'", op);
    token = token->next;
}

// Ensure that the current token is TK_NUM.
static int expect_number() {
    if (token->kind != TK_NUM) error_at(token->str, "expected a number");
    int val = token->val;
    token = token->next;
    return val;
}

//
// Parser
//
static Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// Syntax Rules
// expr		= equality
static Node *expr() {
    Node *node = equality();
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

// primary	= num | "(" expr ")"
static Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    return new_node_num(expect_number());
}
