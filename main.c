#include "2dcc.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "not enough arguments\n");
    return 1;
  }

  user_input = argv[1];
  global_vars = new_vec();
  token = tokenize(user_input);
  program();

  printf(".intel_syntax noprefix\n");
  for (int i = 0; code[i] != NULL; i++) {
      gen(code[i]);
      if (code[i+1])
        printf("  pop rax\n");
  }

  return 0;
}