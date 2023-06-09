#pragma once

struct tokens;
struct tokens* MakeTokens(const char* line);
size_t GetNumberOfTokens(struct tokens* tokens);
char* GetToken(struct tokens* tokens, size_t n);
void vector_replace(char*** pointer, int index, void* elem);
void TokenFree(struct tokens* tokens);