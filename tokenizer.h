#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <stdbool.h>
#include "common.h"
#include "list.h"
/*
typedef enum {
    PENDING,
    READ_TOKEN, 
    FINISHED,
} tokenizer_status_t;
*/

typedef enum {
    OP_PIPE = 1,
    OP_SIMPLE
} operator_t ;

typedef enum token_type {
    WORD,
    OPERATOR
} token_type_t;

typedef struct token {
    token_type_t type;
    union data {
        char *word;
        operator_t operator;
    } data;
} token_t;

typedef struct tokenizer {
    list_t *tokens;
    char *buf;
    size_t buflen;
    size_t bufused;
    bool escaped;
    char quot_char;
} tokenizer_t;

void drop_token(token_t *_Nullable tok);

tokenizer_t *_Nullable new_tokenizer();

list_t *_Nonnull drop_tokenizer(tokenizer_t *_Nonnull tokenizer);

token_t *_Nullable insert_token_node(tokenizer_t *_Nonnull tokenizer);

void delimit_token(tokenizer_t *_Nonnull tokenizer);

void push_char(tokenizer_t *_Nonnull tokenizer, char ch);

// -1 means invalid escape character
int char_to_escape(char ch);

void parse_tokens(tokenizer_t *_Nonnull tokenizer, FILE *_Nonnull strm);

void print_tokens(list_t *_Nonnull lst);

#endif