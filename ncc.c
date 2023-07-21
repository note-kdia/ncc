#include "ncc.h"

typedef enum
{
    TK_RESERVED, // Keywords or punctuator
    TK_NUM,      // Integer literals
    TK_EOF,      // End-of-File markers
} TokenKind;

typedef struct Token Token;

// Token type
struct Token
{
    TokenKind kind; // Token kind
    Token *next;    // Next token
    int val;        // If kind is TK_NUM, its value
    char *str;      // Token string
    int len;        // string len
};

typedef enum
{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ,  // ==
    ND_NEQ, // !=
    ND_GE,  // >=
    ND_LE,  // <=
    ND_GT,  // >
    ND_LT,  // <
    ND_NUM, // integer
} NodeKind;

typedef struct Node Node;

// Node type of Abstract Syntax
struct Node
{
    NodeKind kind; // Node kind
    Node *lhs;     // left hand side
    Node *rhs;     // right hand side
    int val;       // If kind is ND_NUM, its value
};

// Input program
char *user_input;

// Current token
Token *token;

// Reports an error and exit.
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Reports an error location and exit.
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // print whitespace pos times
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Consumes the current token if it matches `op`.
bool consume(char *op)
{
    if (token->kind != TK_RESERVED ||
        token->len != strlen(op) ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

// Ensure that the current token is `op`.
void expect(char *op)
{
    if (token->kind != TK_RESERVED ||
        token->len != strlen(op) ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "expected '%c'", op);
    token = token->next;
}

// Ensure that the current token is TK_NUM.
int expect_number()
{
    if (token->kind != TK_NUM)
        error_at(token->str, "expected a number");
    int val = token->val;
    token = token->next;
    return val;
}

bool is_at_eof()
{
    return token->kind == TK_EOF;
}

// Create a new token and add it as the next token of `cur`.
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool is_startwith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

// Tokenize `p` and returns new tokens.
Token *tokenize()
{
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        // Skip whitespace characters.
        if (isspace(*p))
        {
            p++;
            continue;
        }

        // Punctuator (Multi letter)
        if (is_startwith(p, "==") || is_startwith(p, "!=") ||
            is_startwith(p, "<=") || is_startwith(p, ">="))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        // Punctuator (Single letter)
        if (strchr("+-*/()<>", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        // Integer literal
        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "expected a number");
    }

    // EOF
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

//
// Parser
//

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// expr		= equality
// equality	= relational ( "==" relational | "!=" relational )*
// relational	= add ( ">=" add | "<=" add | ">" add | "<" add )*
// add		= mul ( "+" mul | "-" mul )*
// mul		= unary ( "*" unary | "/" unary )*
// unary	= ( "+" | "-" )? primary
// primary	= num | "(" expr ")"

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// expr		= equality
Node *expr()
{
    Node *node = equality();
    return node;
}

// equality	= relational ( "==" relational | "!=" relational )*
Node *equality()
{
    Node *node = relational();

    for (;;)
    {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NEQ, node, relational());
        else
            return node;
    }
}

// relational	= add ( ">=" add | "<=" add | ">" add | "<" add )*
Node *relational()
{
    Node *node = add();

    for (;;)
    {
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
Node *add()
{
    Node *node = mul();

    for (;;)
    {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul		= unary ( "*" unary | "/" unary )*
Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary	= ( "+" | "-" )? primary
Node *unary()
{
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

// primary	= num | "(" expr ")"
Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    return new_node_num(expect_number());
}

void gen(Node *node)
{
    if (node->kind == ND_NUM)
    {
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_GE:
        printf("    cmp rdi, rax\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_GT:
        printf("    cmp rdi, rax\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NEQ:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Give proper number of arguments\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    Node *node = expr();

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
