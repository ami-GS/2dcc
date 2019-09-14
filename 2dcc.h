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
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
    // type
    TK_VOID,
    TK_INT,
    TK_CHAR,
    TK_EOF,
} TokenKind;

typedef struct LVar LVar;
typedef struct Type Type;

enum {
    VOID,
    INT,
    CHAR,
};

struct Type {
    int type;
    int size; // in byte

    Type *pointer_to;
};

struct LVar {
  LVar *next;
  Type type;
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
    ND_IF,
    ND_ELSE,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,
    ND_ADDR,
    ND_DEREF,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
    int offset; // offset from base pointer rbp

    Node *cond;

    // if else
    Node *then;
    Node *els;

    // for while
    Node *body;
    Node *init;
    Node *inc;

    // { stmt }
    // TODO: dynamic
    Node* block[128];

};
Node *code[100];

void gen(Node *node);
Token *tokenize(char *p);
void program();
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool at_eof();