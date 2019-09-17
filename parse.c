#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "2dcc.h"

Type *get_actual_type(Node *node) {
    if (!node) {
        return NULL;
    }
    Type *type = node->type;
    if (!type) {
        return NULL;
    }
    while (type->type == PTR) {
        type = type->pointer_to;
    }
    return type;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    node->ret_type.type = INT;
    node->ret_type.size = 8; // TODO: fix
    return node;
}

LVar *find_lvar(Token *tk) {
    for (int i = 0; i < cur_func->lvar_vec->len; i++) {
        LVar *lvar = vec_get(cur_func->lvar_vec, i);
        if (tk->len == lvar->len && memcmp(tk->str, lvar->name, tk->len) == 0)
            return lvar;
    }
    return NULL;
}

Arg *find_arg(Token *tk){
    for (int i = 0; i < cur_func->arg_vec->len; i++) {
        Arg *arg = vec_get(cur_func->arg_vec, i);

        if (tk->len == arg->len && memcmp(tk->str, arg->name, tk->len) == 0)
            return arg;
    }
    return false;
}

Function* find_func(Token *tk, Arg *args) {
    for (int i = 0; i < func_vec->len; i++) {
        Function *func = vec_get(func_vec, i);
        if (tk->len == func->len && memcmp(tk->str, func->name, func->len) == 0)
            return func;
    }
    return NULL;
}

int strtype_to_int(char *strtype, int len) {
    if (strncmp(strtype, "int", 3) == 0) {
        return INT;
    } else if (strncmp(strtype, "char", 4) == 0) {
        return CHAR;
    } else if (strncmp(strtype, "void", 3) == 0) {
        return VOID;
    }
}

int type_to_sizeof(int type) {
    switch (type) {
    case VOID:
        return 0;
    case INT:
        return 8;
    case CHAR:
        return 1;
    case PTR:
        return 8;
    }
    error("unknown data type %d\n", type);
}

Type* get_type(Token *type_token, int ptr_cnt) {
    Type *type_tmp = calloc(1, sizeof(Type));
    type_tmp->type = strtype_to_int(type_token->str, type_token->len);
    type_tmp->size = type_to_sizeof(type_tmp->type);
    while (ptr_cnt) {
        Type *ptr_type = calloc(1, sizeof(Type));
        ptr_type->pointer_to = type_tmp;
        ptr_type->type = PTR;
        ptr_type->size = type_to_sizeof(ptr_type->type);
        type_tmp = ptr_type;
        ptr_cnt--;
    }
    return type_tmp;
}

bool consume(char *op) {
    if ((token->kind != TK_RESERVED &&
        token->kind != TK_RETURN &&
        token->kind != TK_IF &&
        token->kind != TK_ELSE &&
        token->kind != TK_WHILE &&
        token->kind != TK_FOR) ||
        strlen(op) != token->len ||
        strncmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

 Token *consume_ident() {
    if (token->kind == TK_IDENT) {
        Token *tmp = token;
        token = token->next;
        return tmp;
    }
    return NULL;
}

Token *consume_type() {
    if (token->kind == TK_INT ||
    token->kind == TK_CHAR ||
    token->kind == TK_VOID) {
        Token *tmp = token;
        token = token->next;
        return tmp;
    }
    return NULL;
}

void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op)
        error("'%c' is expected", op);
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM)
        error("not a number");
    int val = token->val;
    token = token->next;
    return val;
}

Node *expr();
Node *stmt();

Node *parse_func_call(Token *tok) {
    Function *func = find_func(tok, NULL); // TODO: arg type check
    if (!func) {
        error("function [%.*s] is not defined\n", tok->len, tok->str);
    }
    Node *node = new_node(ND_CALL, NULL, NULL);
    node->call_arg_vec = new_vec();
    while (!consume(")")) {
        if (node->call_arg_vec->len) {
            expect(',');
        }
        vec_push(node->call_arg_vec, expr());
    }
    node->name = tok->str;
    node->name_len = tok->len;
    node->ret_type = func->ret_type;
    return node;
}

Node *parse_func_decl(Token *type, Token *tok) {
    // define func
    Arg *args = calloc(1, sizeof(Arg)); // dummy pointer
    Arg *arg_prv = args;
    Vector *arg_vec = new_vec();

    int arg_offset = 0;
    while (!consume(")")) {
        if (arg_vec->len) {
            expect(',');
        }
        Token *arg_type = consume_type();
        int ptr_cnt = 0;
        while (consume("*")) {
            ptr_cnt++;
        }
        Token *arg_ident = consume_ident();

        if (!arg_type || !arg_ident) {
            error("syntax error in function argument field\n");
        }
        Arg *arg = calloc(1, sizeof(Arg));
        arg->name = arg_ident->str;
        arg->len = arg_ident->len;
        arg->type = get_type(arg_type, ptr_cnt);
        arg->offset = arg_offset;
        arg_offset += type_to_sizeof(arg->type->type);
        vec_push(arg_vec, arg);
    }

    Node *node = new_node(ND_FUNC, NULL, NULL);
    node->arg_vec = arg_vec;
    node->name = tok->str;
    node->name_len = tok->len;

    if (!type) {
        error("type for function [%.*s] is not specified\n", tok->len, tok->str);
    }

    if (!func_vec) {
        func_vec = new_vec();
    }
    Function *func = calloc(1, sizeof(Function));
    func->ret_type.type = strtype_to_int(type->str, type->len);
    func->ret_type.size = type_to_sizeof(func->ret_type.type); // TODO: remove
    func->arg_vec = arg_vec;
    func->name = tok->str;
    func->len = tok->len;
    vec_push(func_vec, func);

    Function *tmp_func = cur_func;
    cur_func = func;
    node->body = stmt();
    cur_func = tmp_func;

    node->has_return = func->has_return;
    node->lvar_vec = func->lvar_vec;
    node->total_lval_size = func->variable_offset;
    node->ret_type = func->ret_type;
    // set args on node
    // WARN: is this C spec?
    return node;
}

Node *primary() {
    Node *node;
    if (consume("(")) {
        node = expr();
        expect(')');
        return node;
    }

    Token *type = consume_type();
    int ptr_cnt = 0;
    while (consume("*")) {
        ptr_cnt++;
    }
    Token *tok = consume_ident();
    if (tok) {
        // function
        if (consume("(")) {
            if (!type) { // func call
                return parse_func_call(tok);
            }
            return parse_func_decl(type, tok);
        }

        node = calloc(1, sizeof(Node));
        node->name = tok->str;
        node->name_len = tok->len;
        if (type && strncmp(token->str, "=", token->len) != 0) {
            node->kind = ND_LVARDECL;
        } else {
            node->kind = ND_LVAR;
        }
        // TODO: bellow will be deprecated
        if (cur_func) {
            if (!cur_func->lvar_vec) {
                cur_func->lvar_vec = new_vec();
            }
            LVar *lvar_in_vec = find_lvar(tok);
            if (lvar_in_vec) {
                node->offset = lvar_in_vec->offset;
                node->type = lvar_in_vec->type;
            } else {
                Arg *arg = find_arg(tok);

                if (arg) {
                    node->kind = ND_ARG;
                    node->offset = arg->offset;
                    node->type = arg->type;
                    return node;
                } else if (!type) {
                    error("type for variable [%.*s] is not specified\n", tok->len, tok->str);
                }
                lvar_in_vec = calloc(1, sizeof(LVar));
                lvar_in_vec->type = get_type(type, ptr_cnt);
                lvar_in_vec->name = tok->str;
                lvar_in_vec->len = tok->len;
                lvar_in_vec->offset = cur_func->variable_offset + lvar_in_vec->type->size;
                node->offset = lvar_in_vec->offset;
                node->type = lvar_in_vec->type;
                cur_func->variable_offset += lvar_in_vec->type->size;
                vec_push(cur_func->lvar_vec, lvar_in_vec);
            }
        }
        return node;
    }

    return new_node_num(expect_number());
}

Node *unary() {
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    if (consume("*"))
        return new_node(ND_DEREF, unary(), NULL);
    if (consume("&"))
        return new_node(ND_ADDR, unary(), NULL);
    return primary();
}

Node *mul() {
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

Node *add() {
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

Node *relational() {
    Node *node = add();
    for (;;) {
        if (consume("<="))
            node = new_node(ND_GTEQ, node, add());
        else if (consume("<"))
            node = new_node(ND_GT, node, add());
        else if (consume(">="))
            node = new_node(ND_GTEQ, add(), node);
        else if (consume(">"))
            node = new_node(ND_GT, add(), node);
        else
            return node;
    }
}

Node *equality() {
    Node *node = relational();
    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQU, node, relational());
        else if (consume("!="))
            node = new_node(ND_NEQ, node, relational());
        else
            return node;
    }
}


Node *assign() {
    Node *node = equality();
    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *expr() {
    return assign();
}

Node *stmt() {
    Node *node;

    if (consume("if")) {
        node = new_node(ND_IF, NULL, NULL);
        expect('(');
        node->cond = expr();
        expect(')');
        node->then = stmt();
        if (consume("else")) {
            node->els = stmt();
        }
        return node;
    }

    if (consume("while")) {
        node = new_node(ND_WHILE, NULL, NULL);
        expect('(');
        node->cond = expr();
        expect(')');
        node->body = stmt();
        return node;
    }

    if (consume("for")) {
        node = new_node(ND_FOR, NULL, NULL);
        expect('(');
        node->init = expr();
        expect(';');
        node->cond = expr();
        expect(';');
        node->inc = expr();
        expect(')');
        node->body = stmt();
        return node;
    }

    if (consume("{")) {
        node = new_node(ND_BLOCK, NULL, NULL);
        int i = 0;
        while (!consume("}")) {
            node->block[i++] = stmt();
        }
        return node;
    }

    if (consume("return")) {
        node = new_node(ND_RETURN, NULL, NULL);
        node->type = &cur_func->ret_type;
        node->ret_type = cur_func->ret_type;
        if (cur_func->ret_type.type != VOID) {
            node->lhs = expr();
        }
        cur_func->has_return = true;
    } else {
        node = expr();
        if (node->kind == ND_FUNC && node->body) {
            return node;
        }
    }

    expect(';');
    return node;
}

void program() {
    int i = 0;
    while (!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}