#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "2dcc.h"


Token* new_token(TokenKind kind, Token *cur, char* str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = calloc(len, sizeof(char));
    memcpy(tok->str, str, sizeof(char) * len);
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    char *name_start = NULL;
    while (*p) {
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
         *p == '=') {
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