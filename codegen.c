#include <stdio.h>
#include "2dcc.h"

void gen_lval(Node *node) {
    if (node->kind == ND_DEREF) {
        // e.g. *a = &b;
        gen(node->lhs);
        return;
    }
    if (node->kind != ND_LVAR) {
        error("not a left variable [%s] %d", node->name, node->kind);
    }
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

void gen_arg(Node *node) {
    printf("  mov rax, [rbp+%d]\n", 16 + node->offset);
    printf("  push rax\n");
}

void gen_call_prologue() {
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

void gen(Node *node) {
    if (!node) {
        return;
    }

    static int if_cnt = 0;
    switch (node->kind) {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LVARDECL:
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case ND_ARG:
            gen_arg(node);
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);
            printf("  pop rdi\n"); // result of gen()
            printf("  pop rax\n"); // addr
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
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
            printf(".LbeginXXX:\n");
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .LendXXX\n");
            gen(node->body);
            printf("  jmp .LbeginXXX\n");
            printf(".LendXXX:\n");
            return;
        case ND_FOR:
            gen(node->init);
            printf(".LbeginXXX:\n");
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .LendXXX\n");
            gen(node->body);
            gen(node->inc);
            printf("  jmp .LbeginXXX\n");
            printf(".LendXXX:\n");
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
                gen_call_prologue();
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
        case ND_ADDR:
            gen_lval(node->lhs);
            return;
        case ND_DEREF:
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case ND_RETURN:
            gen(node->lhs);
            gen_call_prologue();
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
