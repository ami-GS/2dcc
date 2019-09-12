#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "2dcc.h"

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
    return node;
}

LVar* find_lvar(Token *tk) {
    for (LVar *lvar = locals; lvar; lvar = lvar->next) {
        if (tk->len == lvar->len && memcmp(tk->str, lvar->name, tk->len) == 0)
            return lvar;
    }
    return NULL;
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

Node *primary() {
    Node *node;
    if (consume("(")) {
        node = expr();
        expect(')');
        return node;
    }

    Token *tok = consume_ident();
    if (tok) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            if (!locals) {
                locals = calloc(1, sizeof(LVar));
                locals->offset = 0;
            }
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
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
        node = new_node(ND_RETURN, expr(), NULL);
    } else {
        node = expr();
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