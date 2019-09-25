#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

Vector *new_vec();
void vec_push(Vector *vec, void *data);
void *vec_get(Vector *vec, int i);


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
    PTR,
};

struct Type {
    int type;
    int size; // in byte

    Type *pointer_to;
};

struct LVar {
  LVar *next;
  Type *type;
  char *name;
  int len;

  int array_size;
  int offset;
};

//LVar *locals; // TODO: to be globals?

typedef struct Function Function;
typedef struct Arg Arg;

struct Arg {
    Type *type;
    char *name;
    int len;

    int offset;
};

struct Function {
    char *name;
    int len;
    Type ret_type;
    Vector *arg_vec;
    Vector *lvar_vec;

    // TODO: will be removed
    int variable_offset;
    // TODO: should be multiple return information?
    bool has_return;
};

Vector *func_vec;
Function *cur_func;

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
    ND_LVARDECL,
    ND_LARRAY,
    ND_LARRAY_INIT,
    ND_ARG,
    ND_RETURN,
    ND_IF,
    ND_ELSE,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,
    ND_FUNC,
    ND_CALL,
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

    // func
    char* name;
    int name_len;
    Vector *arg_vec;
    Vector *lvar_vec;
    int lvars_num;
    int total_lval_size;

    // func call
    Vector *call_arg_vec;
    Type ret_type;
    bool has_return;

    Type *type; // value or expr type to validate

    Vector *array_init; // vector<Node*>
    int array_size;

};
Node *code[100];

void gen(Node *node);
Token *tokenize(char *p);
void program();
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool at_eof();