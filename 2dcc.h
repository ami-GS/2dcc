#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_RETURN,
    TK_EOF,
} TokenKind;

typedef struct LVar LVar;

struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

LVar *locals;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

Token *token; // TODO: will be local
char *user_input;

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_EQU,
    ND_NEQ,
    ND_GT,
    ND_GTEQ,
    ND_ASSIGN,
    ND_LVAR,
    ND_RETURN,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
    int offset; // offset from base pointer rbp
};
Node *code[100];

void gen(Node *node);
Token *tokenize(char *p);
void program();
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool at_eof();