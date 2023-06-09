#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#define MAXIMUM_ALLOC 4096

struct tokens {
  size_t TokenSize;
  char** tokens;
  size_t ChunkSize;
  char** chunks;
};


void vector_replace(char*** pointer, int index, void* elem) {;
  (*pointer)[index] = elem;
}

static void* vector_insert_last(char*** pointer, size_t* size, void* elem) {
  *pointer = (char**)realloc(*pointer, sizeof(char*) * (*size + 1));
  (*pointer)[*size] = elem;
  *size += 1;
  return elem;
}

static void* clone(char* source, size_t n) {
  source[n] = '\0';
  char* cloned = (char*)malloc(n + 1);
  strncpy(cloned, source, n + 1);
  return cloned;
}

struct tokens* MakeTokens(const char* line) {
  if (line == NULL) {
    return NULL;
  }
  static char token[MAXIMUM_ALLOC];
  size_t n = 0, maximum_num = MAXIMUM_ALLOC;
  struct tokens* tokens;
  size_t llen = strlen(line);
  tokens = (struct tokens*)malloc(sizeof(struct tokens));
  tokens->TokenSize = 0;
  tokens->tokens = NULL;
  tokens->ChunkSize = 0;
  tokens->chunks = NULL;
  const int case1 = 0, case2 = 1, case3 = 2;
  int mode = case1;

  for (int i = 0; i < llen; i++) {
    char c = line[i];
    if (mode == case1) {
      if (c == '\'') {
        mode = case2;
      } else if (c == '"') {
        mode = case3;
      } else if (c == '\\') {
        if (i + 1 < llen) {
          token[n++] = line[++i];
        }
      } else if (isspace(c)) {
        if (n > 0) {
          void* cloned = clone(token, n);
          vector_insert_last(&tokens->tokens, &tokens->TokenSize, cloned);
          n = 0;
        }
      } else {
        token[n++] = c;
      }
    } else if (mode == case2) {
      if (c == '\'') {
        mode = case1;
      } else if (c == '\\') {
        if (i + 1 < llen) {
          token[n++] = line[++i];
        }
      } else {
        token[n++] = c;
      }
    } else if (mode == case3) {
      if (c == '"') {
        mode = case1;
      } else if (c == '\\') {
        if (i + 1 < llen) {
          token[n++] = line[++i];
        }
      } else {
        token[n++] = c;
      }
    }
    if (n + 1 >= maximum_num)
      abort();
  }

  if (n > 0) {
    void* cloned = clone(token, n);
    vector_insert_last(&tokens->tokens, &tokens->TokenSize, cloned);
    n = 0;
  }
  return tokens;
}

size_t GetNumberOfTokens(struct tokens* tokens) {
  if (tokens == NULL) {
    return 0;
  } else {
    return tokens->TokenSize;
  }
}

char* GetToken(struct tokens* tokens, size_t n) {
  if (tokens == NULL || n >= tokens->TokenSize) {
    return NULL;
  } else {
    return tokens->tokens[n];
  }
}

void TokenFree(struct tokens* tokens) {
  if (tokens == NULL) {
    return;
  }
  for (int i = 0; i < tokens->TokenSize; i++) {
    free(tokens->tokens[i]);
  }
  for (int i = 0; i < tokens->ChunkSize; i++) {
    free(tokens->chunks[i]);
  }
  if (tokens->tokens) {
    free(tokens->tokens);
  }
  free(tokens);
}
