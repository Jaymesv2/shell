
#include <stdbool.h>
#include <stdlib.h>
#include "tokenizer.h"
#include "list.h"

void drop_token(token_t *_Nullable tok) {
    if(tok == NULL)
        return;
    if(tok->type == WORD && tok->data.word != NULL)
        free(tok->data.word);
    free(tok);
}

tokenizer_t *_Nullable new_tokenizer() {
    const size_t base_buf_len = 200;
    tokenizer_t *r = malloc(sizeof(tokenizer_t));
    if(r == NULL)
        return NULL;
    r->tokens = new_list();
    r->buf = malloc(base_buf_len);
    r->buflen = base_buf_len;
    r->bufused = 0;
    r->escaped = false;
    r->quot_char = '\0';
    return r;
}

list_t *_Nonnull drop_tokenizer(tokenizer_t *_Nonnull tokenizer) {
    free(tokenizer->buf);
    list_t *lst = tokenizer->tokens;
    free(tokenizer);
    return lst;
}

token_t *_Nullable insert_token_node(tokenizer_t *_Nonnull tokenizer) {
    token_t *token = malloc(sizeof(token_t));
    if(token == NULL)
        return NULL;
    // insert the node
    push_back(tokenizer->tokens, (void*)token);
    return token;
}

void delimit_token(tokenizer_t *_Nonnull tokenizer) {
    if(tokenizer->bufused == 0)
        return;
    char *tmp;
    tmp = malloc(tokenizer->bufused+1);
    if(tmp == NULL) // a good error handling mechanism
        exit(1);
        // ex
    // delimit the word if it is there
    memcpy(tmp, tokenizer->buf, tokenizer->bufused+1);
    // null terminate the character
    tmp[tokenizer->bufused] = '\0';

    token_t *token = insert_token_node(tokenizer);
    // node is implicityly null bc calloc
    token->data.word = tmp;

    token->type = WORD;
    tokenizer->bufused = 0;
    return;
}


void push_char(tokenizer_t *_Nonnull tokenizer, char ch) {
    if(tokenizer->bufused >= tokenizer->buflen) {
        //TODO: resize the buffer
        return; 
    }

    //if(tokenizer->bufused != 0 && tokenizer->buf[tokenizer->bufused])
    // check if the last character was a backslash
    tokenizer->buf[tokenizer->bufused++] = ch;
}



// -1 means invalid escape character
int char_to_escape(char ch) {
    switch(ch) {
        case 'a': return '\a';
        case 'b': return '\b';
        case 'e': return '\e';
        case 'f': return '\f';
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        case 'v': return '\v';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"': return '"';
        case '?': return '?';
        default:
            return -1;
    }
}




void parse_tokens(tokenizer_t *_Nonnull tokenizer, FILE *_Nonnull strm) {
    token_t *tmp_token;
    int ch;
    int is_finished = 0;


    while((ch = fgetc(strm)) && !is_finished) {
        switch(ch) {
            case EOF:
                delimit_token(tokenizer);
                // I LOVE GOTO
                // structured programming, more like CRINGE!!!
                goto loop_end;

#ifdef LOOP_OPERATOR
            case '=':
                if(tokenizer->quot_char != '\0') {
                    push_char(tokenizer, (char)ch);                    
                    break;
                }
                delimit_token(tokenizer);
                tmp_token = insert_token_node(tokenizer);
                tmp_token->type = OPERATOR;
                tmp_token->data.operator = OP_LOOP;
                break;
#endif
            case '|':
                if(tokenizer->quot_char != '\0') {
                    push_char(tokenizer, (char)ch);                    
                    break;
                }
                delimit_token(tokenizer);
                tmp_token = insert_token_node(tokenizer);
                tmp_token->type = OPERATOR;
                tmp_token->data.operator = OP_PIPE;
                break;

            //theses cases all fallthrough to ' '
            case '\n':
                is_finished = 1;
                // fall through
            case '\r':
            case '\t':
            case '\v':
            case '\f':
            case ' ':
                // the input is not quoted
                if(tokenizer->quot_char != '\0')
                    push_char(tokenizer, (char)ch);
                else
                    delimit_token(tokenizer);
                break;

#ifdef QUOTING
            case '\'': 
                // fall through
            case '"':
                // start quoting
                if(tokenizer->quot_char == '\0') {
                    tokenizer->quot_char = (char)ch;
                }  else if(tokenizer->quot_char == ch) {
                    tokenizer->quot_char = '\0';
                } else {
                    push_char(tokenizer, (char)ch);
                }
                    
                break;
#endif
#ifdef ESCAPE_CHARS
            case '\\':
                tokenizer->escaped = true;
                break;
#endif
            default: 
#ifdef ESCAPE_CHARS
                if(tokenizer->escaped==true) {
                    ch = char_to_escape((char)ch);
                    if(ch == -1)
                        break;
                    tokenizer->escaped = false;
                }   
#endif
                // push a character
                push_char(tokenizer, (char)ch);
                break;
        }
    }
    loop_end:
    return;
}

void print_tokens(list_t *_Nonnull lst) {
    printf("printing tokens: \n");
    for(list_node_t *cur = lst->head; cur != NULL; cur = cur->next) {
        token_t *tok = cur->elem;
        if(tok->type == OPERATOR) {
            if(tok->data.operator == OP_PIPE) {
                printf("LOOP");
            } 

        }
        else if(tok->type == WORD) {
            printf("WORD: \"%s\"", tok->data.word);
        } 

        if(cur->next != NULL)
            printf(", ");
    }
    printf("\n");
}
