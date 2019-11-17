#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "2dcc.h"

Type *get_actual_type(Type *type) {
    if (!type) {
        return NULL;
    }

    while (type->type == PTR || type->type == ARRAY) {
        type = type->pointer_to;
    }
    return type;
}

int type_to_sizeof(int type) {
    switch (type) {
    case VOID:
        return 0;
    case INT:
        return 4;
    case CHAR:
        return 1;
    case PTR:
        return 8;
    case ARRAY:
        return 8;
    }
    error("unknown data type %d\n", type);
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    if (kind == ND_DEREF) {
        node->type = lhs->type->pointer_to;
    } else{
        // TODO: type check
        if (lhs && rhs) {
            if (lhs->type->type == PTR || rhs->type->type == PTR) {
                if (lhs->type->type == PTR) {
                    node->type = lhs->type;
                } else {
                    node->type = rhs->type;
                }
            } else if (lhs->type->type == rhs->type->type) {
                node->type = lhs->type;
            } else {
                //error
            }
        }
    }

    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->name = token->str;
    node->name_len = token->len;
    node->val = val;
    node->type = calloc(1, sizeof(Type));
    node->type->type = INT;
    node->type->size = 8;
    node->ret_type.type = INT;
    node->ret_type.size = 8; // TODO: fix
    return node;
}

Node *new_node_string(char* str, int len) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_STRING;
    node->name = token->str; // TODO this is bug
    node->name_len = token->len; // TODO this is bug
    node->str_val = str;
    node->str_val_len = len;
    node->type = calloc(1, sizeof(Type));
    node->type->type = ARRAY;
    node->type->size = type_to_sizeof(ARRAY);
    node->ret_type.type = ARRAY;
    node->ret_type.size = type_to_sizeof(ARRAY); // TODO: fix
    return node;
}

LVar *find_lvar(Token *tk) {
    if (!cur_func)
        return NULL;
    for (int i = 0; i < cur_func->lvar_vec->len; i++) {
        LVar *lvar = vec_get(cur_func->lvar_vec, i);
        if (tk->len == lvar->len && memcmp(tk->str, lvar->name, tk->len) == 0)
            return lvar;
    }
    return NULL;
}

LVar *find_gvar(Token *tk) {
    for (int i = 0; i < global_vars->len; i++) {
        LVar *gvar = vec_get(global_vars, i);
        if (tk->len == gvar->len && memcmp(tk->str, gvar->name, tk->len) == 0)
            return gvar;
    }
    return NULL;
}

Arg *find_arg(Token *tk){
    if (!cur_func)
        return NULL;
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
    } else if (strncmp(strtype, "void", 4) == 0) {
        return VOID;
    }
    error("unknown data type %.*s\n", len, strtype);
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
        token->kind != TK_FOR &&
        token->kind != TK_SIZEOF) ||
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
        error("[%c] is expected, got [%.*s]\n", op, token->len, token->str);
    token = token->next;
}

int expect_string(char** str) {
    if (token->kind != TK_STRING)
        error("not a string");
    *str = token->str;
    int len = token->len;
    token = token->next;
    return len;
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
Node *unary();

Vector* parse_sizeof() {
    Node* node = new_node_num(0);
    Node* target = unary();
    if (target->type->type == ARRAY) {
        if (target->array_idx_exprs)
            node->val = get_actual_type(target->type)->size;
        else
            node->val = get_actual_type(target->type)->size * target->array_size;
    } else {
        node->val = target->type->size;
    }
    return node;
}

Vector* parse_array_sizes_or_indices(Token *type) {
    Vector *array_sizes_or_indices = (Vector*)NULL;
    while (consume("[")) {
        if (!array_sizes_or_indices)
            array_sizes_or_indices = new_vec();
        if (consume("]")) { // int[] array = {1,2,3};
            vec_push(array_sizes_or_indices, 0);
        } else { // int[3] array; or array[a+1];
            if (type) {
                vec_push(array_sizes_or_indices, expect_number());
            } else {
                vec_push(array_sizes_or_indices, expr());
            }
            expect(']');
        }
    }
    return array_sizes_or_indices;
}

void parse_array_ref(Node *node, LVar *lvar_in_vec, Vector *array_indices) {
    node->array_idx_exprs = array_indices;
    node->kind = ND_LARRAY;
    node->array_size = lvar_in_vec->array_size;
    node->array_sizes = lvar_in_vec->array_sizes;
}

void parse_array_init(Node *node, Vector *array_sizes) {
    node->kind = ND_LARRAY_INIT;
    if (consume("{")) {
        Vector* array_init_nodes = new_vec();
        // type should be changed
        while (!consume("}")) {
                if (array_init_nodes->len != 0) {
                    expect(',');
                }
                // TODO: needs better way
                if (cur_func)
                    vec_push(array_init_nodes, expr());
                else
                    vec_push(array_init_nodes, expect_number()); // const_expr()?
        }
        if (array_init_nodes->len == 0) {
            for (int i = 0; i < node->array_size; i++) {
                Node *zero = calloc(1, sizeof(Node));
                zero->kind = ND_NUM;
                zero->val = 0;
                vec_push(array_init_nodes, zero);
            }
        }
        node->array_init = array_init_nodes;
    } else if (token->kind == TK_STRING) {
        // TODO: care about multi dimention;
        int elm_num = vec_get(array_sizes, 0);
        if (vec_get(array_sizes, 0) < token->len-2)
            error("string length must be bellow declared array size [%d]", elm_num);
        node->str_val_len = expect_string(&(node->str_val));
    }
}

void set_new_lvar(Node *node) {
    LVar *lvar_in_vec = calloc(1, sizeof(LVar));
    lvar_in_vec->type = node->type;
    lvar_in_vec->name = node->name;
    lvar_in_vec->len = node->name_len;
    lvar_in_vec->array_size = node->array_size;
    lvar_in_vec->array_sizes = node->array_sizes;
    lvar_in_vec->offset = cur_func->variable_offset + lvar_in_vec->type->size * node->array_size;
    cur_func->variable_offset += lvar_in_vec->type->size * node->array_size;
    node->offset = lvar_in_vec->offset;
    vec_push(cur_func->lvar_vec, lvar_in_vec);
}

void set_new_gvar(Node *node) {
    LVar *gvar_in_vec = calloc(1, sizeof(LVar));
    gvar_in_vec->type = node->type;
    gvar_in_vec->name = node->name;
    gvar_in_vec->len = node->name_len;
    gvar_in_vec->array_size = node->array_size;
    gvar_in_vec->array_sizes = node->array_sizes;
    // gvar location?
    // gvar_in_vec->offset = cur_func->variable_offset + gvar_in_vec->type->size * node->array_size;
    // cur_func->variable_offset += lvar_in_vec->type->size * node->array_size;
    // node->offset = lvar_in_vec->offset;
    vec_push(global_vars, gvar_in_vec);
}

void parse_array_decl(Node *node, Token *type_tkn, int ptr_cnt, Vector *array_sizes) {
    if (consume("=")) {
        parse_array_init(node, array_sizes);
        //if (array_sizes != 0 && array_sizes->len < num)
        //    error("the number of array initialize element are bigger than specified value");
    }
    for (int i = 0; i < array_sizes->len; i++) {
        node->array_size *= (int)vec_get(array_sizes, i);
    }
    Type *type_tmp = calloc(1, sizeof(Type));
    type_tmp->type = strtype_to_int(type_tkn->str, type_tkn->len);
    type_tmp->size = type_to_sizeof(type_tmp->type);

    for (int i = 0; i < array_sizes->len; i++) {
        Type *ptr_type = calloc(1, sizeof(Type));
        ptr_type->pointer_to = type_tmp;
        ptr_type->type = ARRAY;
        ptr_type->size = type_to_sizeof(ptr_type->type);
        type_tmp = ptr_type;
    }
    while (ptr_cnt) {
        Type *ptr_type = calloc(1, sizeof(Type));
        ptr_type->pointer_to = type_tmp;
        ptr_type->type = PTR;
        ptr_type->size = type_to_sizeof(ptr_type->type);
        type_tmp = ptr_type;
        ptr_cnt--;
    }

    node->type = type_tmp;
    node->array_sizes = array_sizes;
}

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
    node->type = &func->ret_type;
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
    func->lvar_vec = new_vec();
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

void parse_arg_ref(Node *node, Arg *arg, Vector* array_indices) {
    node->kind = ND_ARG;
    node->array_idx_exprs = array_indices;
    node->offset = arg->offset;
    node->type = arg->type;
}

void parse_lval_ref(Node *node, LVar *lvar_in_vec) {
    node->offset = lvar_in_vec->offset;
    node->type = lvar_in_vec->type;
}

void parse_lvar_ref(Node *node, LVar *lvar_in_vec, Vector *array_sizes_or_indices) {
    if (array_sizes_or_indices)
        parse_array_ref(node, lvar_in_vec, array_sizes_or_indices);
    parse_lval_ref(node, lvar_in_vec);
}

void parse_lvar_decl(Node *node, Token *type, int ptr_cnt, Vector *array_sizes_or_indices) {
    node->kind = ND_LVARDECL;
    node->array_size = 1;
    if (array_sizes_or_indices) {
        parse_array_decl(node, type, ptr_cnt, array_sizes_or_indices);
    } else {
        node->type = get_type(type, ptr_cnt);
    }
    set_new_lvar(node);
}

void parse_gvar_ref(Node *node, LVar *gvar, Vector *array_sizes_or_indices) {
    if (array_sizes_or_indices) {
        // TODO: doesn't work
        parse_array_ref(node, gvar, array_sizes_or_indices);
    }
    node->type = gvar->type;
    node->kind = ND_GVAR;
}

void parse_gvar_decl(Node *node, Token *type, int ptr_cnt, Vector *array_sizes_or_indices) {
    node->array_size = 1;
    if (array_sizes_or_indices) {
        parse_array_decl(node, type, ptr_cnt, array_sizes_or_indices);
    } else {
        node->type = get_type(type, ptr_cnt);
    }
    node->kind = ND_GVARDECL;
    // TODO: currently allowing simple value
    if (consume("=")) {
        node->has_global_init = true;
        node->val = expect_number();
    }
    set_new_gvar(node);
}

Node *parse_var(Token *tok, Token *type, int ptr_cnt) {
    LVar *lvar_in_vec = find_lvar(tok);
    Arg *arg = find_arg(tok);
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->name = tok->str;
    node->name_len = tok->len;

    Vector *array_sizes_or_indices = parse_array_sizes_or_indices(type);
    if (lvar_in_vec) {
        parse_lvar_ref(node, lvar_in_vec, array_sizes_or_indices);
    } else if (arg) {
        parse_arg_ref(node, arg, array_sizes_or_indices);
    } else {
        LVar *gvar = find_gvar(tok);
        if (gvar) {
            parse_gvar_ref(node, gvar, array_sizes_or_indices);
        } else {
            if (!type)
                error("type for variable [%.*s] is not specified\n", tok->len, tok->str);
            if (cur_func) {
                parse_lvar_decl(node, type, ptr_cnt, array_sizes_or_indices);
            } else {
                parse_gvar_decl(node, type, ptr_cnt, array_sizes_or_indices);
            }
        }
    }
    return node;
}

Node *parse_identifier() {
    Node *node;
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
        return parse_var(tok, type, ptr_cnt);
    }
}

Node* parse_immediate() {
    Node* node;
    if (token->kind == TK_STRING) {
        char* str;
        int len = expect_string(&str);
        node = new_node_string(str, len);
    } else if (token->kind == TK_NUM) {
        node = new_node_num(expect_number());
    }
    return node;
}

Node *primary() {
    Node *node;
    if (consume("(")) {
        node = expr();
        expect(')');
        return node;
    }
    node = parse_identifier();
    if (node)
        return node;
    return parse_immediate();
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
    if (consume("sizeof"))
        return parse_sizeof();
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