#include <stdio.h>
#include "2dcc.h"

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR && node->kind != ND_LARRAY &&
        node->kind != ND_LVARDECL && node->kind != ND_LARRAY_INIT &&
        node->kind != ND_DEREF) {
        error("not a left variable [%s] %d", node->name, node->kind);
    }

    switch (node->kind) {
        case ND_DEREF: case ND_LARRAY: case ND_LARRAY_INIT:
            if (node->kind == ND_DEREF) {
                gen(node->lhs);
                return;
            } else if (node->kind == ND_LARRAY) {
                int offst = 1;
                for (int i = node->array_sizes->len - 1; i >= 0; i--) {
                    gen(vec_get(node->array_idx_exprs, i));
                    // add if size is over 2
                    if (node->array_sizes->len - i - 1 >= 1) {
                        printf("  pop rdi\n");
                        printf("  imul rdi, %d\n", offst);
                        printf("  pop rax\n");
                        printf("  add rax, rdi\n");
                        printf("  push rax\n");
                    }
                    offst *= (int)vec_get(node->array_sizes, i);
                }
            }
            printf("  pop rdi # %d %.*s\n", node->kind, node->name_len, node->name);
            printf("  imul rdi, %d\n", get_actual_type(node->type)->size);
            printf("  add rdi, %d\n", node->offset);
            break;
        default:
            printf("  mov rdi, %d\n", node->offset);
    }
    printf("  mov rax, rbp\n");
    printf("  sub rax, rdi\n");
    printf("  push rax\n");
}

void gen_arg(Node *node) {
    // qward for stack machine
    printf("  mov rax, [rbp+%d]\n", 16 + node->offset);
    printf("  push rax\n");
}

void gen_rvalue_epilogue(Type *type) {
    printf("  pop rax\n");
    int ty = type->type;
    if (ty == CHAR) {
        printf("  movsx rax, BYTE PTR [rax]\n"); // movzx
    } else if (ty == INT) {
        printf("  mov eax, DWORD PTR [rax]\n");
    } else {
        printf("  mov rax, [rax]\n");
    }
    printf("  push rax\n");
}

void gen_call_epilogue() {
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

void gen_assign(Node *lhs, Node *rhs) {
    gen_lval(lhs);
    gen(rhs);
    printf("  pop rdi\n"); // result of gen()
    printf("  pop rax\n"); // addr

    // TODO: refactoring
    if (lhs->type->type == PTR && rhs->kind == ND_ADDR) {
        printf("  mov [rax], rdi\n");
    } else if (lhs->type->type == CHAR || (get_actual_type(lhs->type)->type == CHAR)) {
        printf("  mov [rax], dil\n");
    } else if (lhs->type->type == INT || (get_actual_type(lhs->type)->type == INT)) {
        printf("  mov [rax], edi\n");
    } else {
        printf("  mov [rax], rdi\n");
    }
    printf("  push rdi\n");
}

void gen_assign_char(Node *lhs, char* c) {
    gen_lval(lhs);
    printf("  push %d\n", c);
    printf("  pop rdi\n"); // result of gen()
    printf("  pop rax\n"); // addr
    printf("  mov [rax], dil\n");
    printf("  push rdi\n");
}

void gen(Node *node) {
    if (!node) {
        return;
    }

    static int if_cnt = 0;
    static int local_char_cnt = 0;
    static int loop_cnt = 0;
    switch (node->kind) {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_STRING:
            printf(".LC%d:\n", local_char_cnt++);
            printf("  .string %.*s\n", node->str_val_len, node->str_val);
            return;
        case ND_GVARDECL:
            printf("%.*s:\n", node->name_len, node->name);
            if (node->has_global_init)
                printf("  .long %d\n", node->val);
            else if (node->array_init) {
                for (int i = 0; i < node->array_init->len; i++) {
                    printf("  .long %d\n", vec_get(node->array_init, i));
                }
            } else
                printf("  .zero %d\n", node->type->size * node->array_size);
            return;
        case ND_GVAR:
            if (node->type->type == ARRAY && node->array_idx_exprs) {
                gen(vec_get(node->array_idx_exprs, 0));
                printf("  pop rax\n");
                printf("  mov rax, %.*s[rip+rax]\n", node->name_len, node->name);
            } else {
                printf("  mov rax, %.*s[rip]\n", node->name_len, node->name);
            }
            printf("  push rax\n");
            return;
        case ND_LVARDECL:
            return;
        case ND_LVAR:
            gen_lval(node);
            if (node->type->type != ARRAY) {
                gen_rvalue_epilogue(node->type);
            }
            return;
        case ND_LARRAY_INIT:
            if (node->str_val) {
                for (int i = 1; i < node->str_val_len-1; i++) {
                    printf("  push %d\n", i-1);
                    gen_assign_char(node, node->str_val[i]);
                }
            } else {
                for (int i = 0; i < node->array_size; i++) {
                    printf("  push %d\n", i);
                    gen_assign(node, vec_get(node->array_init, i));
                }
            }
            return;
        case ND_LARRAY:
            // e.g. a[b + c] = 100;
            gen_lval(node); // a
            gen_rvalue_epilogue(node->type);
            return;
        case ND_ARG:
            gen_arg(node);
            if (node->array_idx_exprs) {
                gen(vec_get(node->array_idx_exprs, 0));
                printf("  pop rdi\n");
                printf("  imul rdi, %d\n", get_actual_type(node->type)->size);
                printf("  pop rax\n"); // stack by arg
                printf("  sub rax, rdi\n");
                printf("  push rax\n");
                gen_rvalue_epilogue(node->type);
            }
            return;
        case ND_ADDR:
            gen_lval(node->lhs);
            return;
        case ND_DEREF:
            gen(node->lhs);
            gen_rvalue_epilogue(node->type);
            return;
        case ND_ASSIGN:
            gen_assign(node->lhs, node->rhs);
            return;
        case ND_IF:
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            if (node->els) {
                printf("  je .Lelse_%dXXX\n", if_cnt);
                gen(node->then);
                printf("  je .Lend_%dXXX\n", if_cnt);
                printf(".Lelse_%dXXX:\n", if_cnt);
                gen(node->els);
               /*
                printf("  jne .LifXXX\n");
                gen(node->els);
                printf("  jmp .LendXXX\n");
                printf(".LifXXX:\n");
                gen(node->then);
                */
            } else {
                printf("  je .Lend_%dXXX\n", if_cnt);
                gen(node->then);
            }
            printf(".Lend_%dXXX:\n", if_cnt);
            if_cnt++;
            return;
        case ND_WHILE:
            printf(".Lbegin_%d:\n", loop_cnt);
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend_%d\n", loop_cnt);
            gen(node->body);
            printf("  jmp .Lbegin_%d\n", loop_cnt);
            printf(".Lend_%d:\n", loop_cnt++);
            return;
        case ND_FOR:
            gen(node->init);
            printf(".Lbegin_%d:\n", loop_cnt);
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend_%d\n", loop_cnt);
            gen(node->body);
            gen(node->inc);
            printf("  jmp .Lbegin_%d\n", loop_cnt);
            printf(".Lend_%d:\n", loop_cnt++);
            return;
        case ND_BLOCK:
        {
            int i = 0;
            while (node->block[i]) {
                gen(node->block[i]);
                i++;
            }
        }
            return;
        case ND_FUNC:
            printf(".global %.*s\n", node->name_len, node->name);
            printf("%.*s:\n", node->name_len, node->name);
            printf("  push rbp\n");
            printf("  mov rbp, rsp\n");
            if (node->total_lval_size) {
                printf("  sub rsp, %d\n", node->total_lval_size);
            }
            gen(node->body);
            if (!node->has_return)
                gen_call_epilogue();
            return;
        case ND_CALL:
            for (int i = node->call_arg_vec->len-1; 0 <= i; i--) {
                gen(vec_get(node->call_arg_vec, i));
            }
            printf("  call %.*s\n", node->name_len, node->name);
            if (node->ret_type.type != VOID) {
                printf("  mov rdi, rax\n");
            }
            for (int i = node->call_arg_vec->len-1; 0 <= i; i--) {
                printf("  pop rax\n");
            }
            if (node->ret_type.type != VOID) {
                printf("  push rdi\n");
            }
            return;
        case ND_RETURN:
            gen(node->lhs);
            gen_call_epilogue();
            return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        case ND_EQU:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NEQ:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_GT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_GTEQ:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
    }
    printf("  push rax\n");
}
