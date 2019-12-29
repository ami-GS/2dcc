#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "2dcc.h"

char* read_file(char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("failed to open file\n");
        exit(-1);
    }
    if (fseek(fp, 0, SEEK_END) == -1)
        error("%s: fseek: %s", filename, strerror(-1));
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1)
        error("%s: fseek: %s", filename, strerror(-1));

    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fp);
    if (size == 0 || buf[size - 1] != '\n')
        buf[size++] = '\n';
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool at_eof() {
    return token->kind == TK_EOF;
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}


Vector *new_vec() {
    Vector *v = malloc(sizeof(Vector));
    v->data = malloc(sizeof(void *) * 16);
    v->capacity = 16;
    v->len = 0;
    return v;
}

void vec_push(Vector *vec, void *data) {
    if (vec->len == vec->capacity) {
        vec->capacity += 16;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len++] = data;
}

void *vec_get(Vector *vec, int i) {
    assert(i < vec->len);
    return vec->data[i];
}