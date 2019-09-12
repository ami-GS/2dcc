#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "2dcc.h"


Token* new_token(TokenKind kind, Token *cur, char* str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    char *name_start = NULL;
    while (*p) {
        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
        } else if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
        } else if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
        } else if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            cur = new_token(TK_FOR, cur, p, 3);
            p += 3;
        } else if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
        }

        if (('a' <= *p && *p <= 'z') || ('A' <= *p && *p <= 'Z')) {
            if (!name_start) {
                name_start = p;
            }
            p++;
            continue;
        } else if (name_start) {
            cur = new_token(TK_IDENT, cur, name_start, p-name_start);
            name_start = NULL;
        }

        if (isspace(*p)) {
            p++;
            continue;
        }

        if (strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0 ||
        strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' ||
         *p == '/' || *p == '(' || *p == ')' ||
         *p == '<' || *p == '>' || *p == ';' ||
         *p == '=' || *p == '{' || *p == '}' ||
         *p == '&') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char* pTmp = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - pTmp;
            continue;
        }

        error("failed to tokenize");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}