// This is a big file since I cant submit multiple files.

// DONT REMOVE THE MAGIC MACROS
// EVERYTHING EXPLODES IF YOU REMOVE THEM
#define _GNU_SOURCE

#include "common.h"


// includes
// stdlib
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
// system
#include <unistd.h>
#include <regex.h>
#include <sys/wait.h>


#include <sys/random.h>

#include "list.h"
#include "tokenizer.h"


int init_rand()
{
    unsigned int seed;
    if (getrandom((void *)&seed, sizeof(unsigned int), 0) == -1)
        return 1;
    srand(seed);
    return 0;
}

int init() {
    if (init_rand())
        return 1;
        // dont initialize quotes if they are disabled
    return 0;
}

void process_tokens(list_t *_Nonnull tokens) {
    token_t* tok;
    for(list_node_t* cur = tokens->head; cur != NULL; cur = cur -> next) {
        tok = cur->elem;
        switch(tok->type) {
            case WORD:
                // do glob expansion 
                break;
            case OPERATOR:
                break;
        }
    }
}

typedef struct command {
    char *_Nullable args[100];
    int argc;
} command_t;

command_t *new_command() {
    command_t *cmd = malloc(sizeof(command_t));
    cmd->argc = 0;
    cmd->args[0] = NULL;
    return cmd;
}


// discriminated union
typedef struct command_tree {
    operator_t op;
    union {
        struct {
            struct command_tree *_Nonnull right;
            struct command_tree *_Nonnull left;
        } pipe;
        struct {
            struct command_tree *_Nonnull right;
            struct command_tree *_Nonnull left;
        } loop;
        struct {
            command_t *_Nonnull command;
        } simple;
    };
} command_tree_t;

// returns null pointer on error
command_tree_t *_Nullable build_command_tree(list_t *_Nonnull tokens) {
    // we do a little operator precidence parsing
    list_t *op_stack = new_list();
    list_t *command_stack = new_list();
    
    command_tree_t *tmp_tree = NULL;

    command_t *cmd = new_command();
    operator_t *o1, *o2;
    //operator_t pipe = OP_PIPE, loop = OP_LOOP, simple = OP_SIMPLE;
    //printf("pipe %d, loop: %d, simple %d\n", pipe, loop, simple);

    token_t* tok;
    
    for(list_node_t* cur = tokens->head; cur != NULL; cur = cur -> next) { 
        tok = cur->elem;
        switch(tok->type) {
            case WORD:
                if(cmd == NULL)
                    cmd = new_command();
                // push it into the command
                cmd->args[cmd->argc++] = tok->data.word;
                tok->data.word = NULL;  // null the address so the strings arent freed when the list is dropped
                break;
            case OPERATOR:
            default:
                if(cmd != NULL) {
                    cmd->args[cmd->argc] = NULL; // null terminate
                    tmp_tree = malloc(sizeof(command_tree_t));
                    tmp_tree->op=OP_SIMPLE;
                    tmp_tree->simple.command = cmd;
                    push_front(command_stack, tmp_tree);
                    cmd = NULL;
                    tmp_tree = NULL;
                }

                o1 = &tok->data.operator;
                /*if(tok->type==PIPE)
                    o1 = &pipe;
                if(tok->type==LOOP)
                    o1 = &loop;*/

                while(peek_front(op_stack, (void**)&o2) != 0 && *o1 <= *o2) {
                    operator_t oop = *(operator_t*)pop_front(op_stack);
                    tmp_tree = malloc(sizeof(command_tree_t));
                    tmp_tree->op = oop;
                    switch(tmp_tree->op) {
                        case OP_PIPE:
                            tmp_tree->pipe.right=(command_tree_t*)pop_front(command_stack);
                            tmp_tree->pipe.left=(command_tree_t*)pop_front(command_stack);
                            break;
                        case OP_SIMPLE:
                            break;
                    } 
                    //printf("pushed: %p, %p\n", tmp_tree->loop.left, tmp_tree->loop.right);
                    push_front(command_stack, tmp_tree);
                    tmp_tree = NULL;
                    // combine the top 2 elems and push 
                }
                push_front(op_stack, o1);
        }
    }

    if(cmd != NULL) {
        cmd->args[cmd->argc] = NULL; // null terminate
        tmp_tree = malloc(sizeof(command_tree_t));
        tmp_tree->op=OP_SIMPLE;
        tmp_tree->simple.command = cmd;
        push_front(command_stack, tmp_tree);
        cmd = NULL;
        tmp_tree = NULL;
    }

    while(peek_front(op_stack, (void**)&o2) != 0 ) {
        tmp_tree = malloc(sizeof(command_tree_t));
        tmp_tree->op = *(operator_t*)pop_front(op_stack);
        switch(tmp_tree->op) {
            case OP_PIPE:
                tmp_tree->pipe.right=pop_front(command_stack);
                tmp_tree->pipe.left=pop_front(command_stack);
                break;
            default:
                break;
        }
        push_front(command_stack, tmp_tree);
        tmp_tree = NULL;
        // combine the top 2 elems and push 
    }
    // combine the top 2 elems and push 

    /*drop_list(op_stack, NULL);
    drop_list(command_stack, NULL);
    drop_list(tokens, drop_token);*/
    return  pop_front(command_stack);
}


void print_command_tree(command_tree_t* tree, int height) {
    //printf("tree: %p, height: %d\n", tree, height);
    for(int i = 0; i < height; i++)
        printf("|-----");
    switch(tree->op) {
        case OP_SIMPLE:
            printf("Simple command: ");
            for(int i = 0; tree->simple.command->args[i] != NULL; i++)
                printf("\"%s\" ", tree->simple.command->args[i]);
            printf("\n");
            break;
        case OP_PIPE:
            printf("Pipe: \n");
            print_command_tree(tree->pipe.left, height+1);
            print_command_tree(tree->pipe.right, height+1);
            break;
    }
}


// this should probably take a list of redirections
// returns the process id

int _run_command_tree(command_tree_t *_Nonnull tree, int in_fd[2], int out_fd[2]) {
    int cpid, cpid2, pipes[2][2];
    command_t *cmd;
    switch(tree->op) {
        case OP_PIPE:
            if(pipe(pipes[0]) == -1) {
                elog("failed to make a pipe");
                return 1;
            }

            cpid  = _run_command_tree(tree->pipe.left, in_fd, pipes[0]);
            cpid2 = _run_command_tree(tree->pipe.right, pipes[0], out_fd);

            return cpid2;

        case OP_SIMPLE:
            //eprintf("executing simple command\n");

            if ((cpid = fork()) == -1) {
                elog("FAILED TO FORK CHILD");
                return 1;
            }
            if (cpid == 0) {
                cmd = tree->simple.command;
                close(in_fd[PWRITE]);
                close(out_fd[PREAD]);
                // child
                //eprintf("message from child\n");
                dup2(in_fd[PREAD], STDIN_FILENO);
                dup2(out_fd[PWRITE], STDOUT_FILENO);
                execvp(cmd->args[0], cmd->args);
                elog("after exec for %s", cmd->args[0]);
                // exit after
                exit(1);
            } 
            if(in_fd[PREAD] != STDIN_FILENO)
                close(in_fd[PREAD]);
            if(out_fd[PWRITE] != STDOUT_FILENO)
                close(out_fd[PWRITE]);
            return cpid;
    }
    return 0;
}

void run_command_tree(command_tree_t *_Nonnull tree) {
    int in_fd[2], out_fd[2];
    in_fd[PREAD] = STDIN_FILENO;
    in_fd[PWRITE] = -1;
    out_fd[PREAD] = -1;
    out_fd[PWRITE] = STDOUT_FILENO;
    int cpid = _run_command_tree(tree, in_fd, out_fd);
    waitpid(cpid, NULL, 0);
}


int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    if (init())
        return 1;

    int c;
    extern char *optarg;
    extern int optind, optopt;
    char* inp = NULL;
    bool should_continue = 1;

    if((c = getopt(argc, argv, "c:")) != -1) {
        inp = optarg;
        printf("got \"%s\"\n", optarg);
    }

    char line[ARG_MAX];
    do {
        FILE* f;
        if(inp == NULL) {
            printf("# ");
            fflush(stdout);

            fgets(line, ARG_MAX, stdin);
        
            if(strlen(line) == 1 && *line == '\n')
                continue;
            f = fmemopen(line, strlen(line), "r");
        } else {
            f = fmemopen(inp, strlen(inp), "r");
            should_continue = 0;
        }
        tokenizer_t *tokenizer = new_tokenizer();
        parse_tokens(tokenizer, f);
        list_t *tokens = drop_tokenizer(tokenizer);
        //print_tokens(tokens);
        process_tokens(tokens);
        //print_tokens(tokens);
        command_tree_t *tree = build_command_tree(tokens);
        //print_command_tree(tree, 0);
        run_command_tree(tree);
        drop_list(tokens, (void(*)(void*))drop_token);
        fclose(f);
    } while(should_continue);

    //printf("testing tokenizer\n");
    /*char str[] = "test string\"po ggers\"letsgooo | epic = 32 cool\\nstr *.bash";
    FILE* f = fmemopen(str, strlen(str), "r");*/

    //fclose(f);

}


/*


enum token_type {
    AND_IF = '&'+'&'*1000,
    OR_IF = '|'+'|'*1000,
    DSEMI = ';'+';'*1000,
    DLESS = '<' + '<'*1000,
    DGREAT = '>' + '>'*1000,
    LESSAND = '<' + '&'*1000,
    GREATAND = '>' + '&'*1000,
    LESSGREAT = '<' + '>'*1000,
    CLOBBER = '>' + '|'*1000,
    DLESSDASH = 0,
    If,
    Then,
    Elif,
    Fi,
    Do, 
    Done,
    Case,
    Esac,
    While,
    Until,
    For,
    Lbrace,
    Rbrace,
    Bang,
    In,
    Word,
    Newline,
};


// OPERATORS: && || ;;
// << >> <& >& <> <<- 
// >|


// CONTROL OPERATORS:  &  &&  (  )  ;  ;;  newline  |   ||
// REDIRECTION OPERATORS: <  >  >|  <<  >>  <&  >&  <<-  <>


const int
    AND_IF = '&'+'&'*1000;
    OR_IF = '|'+'|'*1000;
    DSEMI = ';'+';'*1000,
    DLESS = '<' + '<'*1000,
    DGREAT = '>' + '>'*1000,
    LESSAND = '<' + '&'*1000,
    GREATAND = '>' + '&'*1000,
    LESSGREAT = '<' + '>'*1000,
    CLOBBER = '>' + '|'*1000;


OPERATORS: 
    && || ;;
    << >> <& >& <> <<- 
    >|
CONTROL OPERATORS:  
    &  &&  (  )  ;  ;;  newline  |   ||
REDIRECTION OPERATORS: 
    <  >  >|  <<  >>  <&  >&  <<-  <>
RESERVED WORDS:
    !       {       }   case
    do      done    elif    else
    esac    fi      for     if
    in      then    until   while
POTENTIALLY:
    [[      ]]      function    select


list_t* tokenize(FILE* input) {
    tokenizer_state_t *tokenizer = calloc(1, sizeof(tokenizer_state_t));
    tokenizer->buflen = 200;
    tokenizer->bufpos = 0;
    tokenizer->buf = malloc(tokenizer->buflen);

    tokenizer->tokens = calloc(1, sizeof(list_t));
    tokenizer->tokens_tail = tokenizer->tokens->next;

    //list_t *current_token;

    int ch;
    while((ch = fgetc(input))) {
        // If the end of input is recognized, the current token (if any) shall be delimited.
        if(ch == EOF) {
            // delmit the current token
            break;            
        }

        // the input is unquoted
        if(tokenizer->quote_char == '\0') {
            
            int can_form_op = can_form_operator(tokenizer->buf[tokenizer->bufpos], ch);
            // If the previous character was used as part of an operator and the current character is not quoted and can be used with the previous characters to form an operator, it shall be used as part of that (operator) token.
            if(tokenizer->quote_char == '\0' && can_form_op != 0) {
                
            }
            // If the previous character was used as part of an operator and the current character cannot be used with the previous characters to form an operator, the operator containing the previous character shall be delimited.
            if(tokenizer->quote_char == '\0') {
                
            }


            
            // If the current character is <backslash>, single-quote, or double-quote and it is not quoted, it shall affect quoting for subsequent characters up to the end of the quoted text. 
            // The rules for quoting are as described in Quoting. 
            // During token recognition no substitutions shall be actually performed, and the result token shall contain exactly the characters that appear in the input (except for <newline> joining), unmodified, including any embedded or enclosing quotes or substitution operators, between the <quotation-mark> and the end of the quoted text. The token shall not be delimited by the end of the quoted field.
            if(tokenizer->quote_char == '\0' && (ch == '\\' || ch == '\'' || ch == '"')) {

            }

            //If the current character is an unquoted '$' or '`', the shell shall identify the start of any candidates for parameter expansion (Parameter Expansion), command substitution (Command Substitution), or arithmetic expansion (Arithmetic Expansion) from their introductory unquoted character sequences: '$' or "${", "$(" or '`', and "$((", respectively. The shell shall read sufficient input to determine the end of the unit to be expanded (as explained in the cited sections). 
            //While processing the characters, if instances of expansions or quoting are found nested within the substitution, the shell shall recursively process them in the manner specified for the construct that is found. 
            //The characters found from the beginning of the substitution to its end, allowing for any recursion necessary to recognize embedded constructs, shall be included unmodified in the result token, including any embedded or enclosing substitution operators or quotes. The token shall not be delimited by the end of the substitution.
            if(tokenizer->quote_char == '\0' ) {

            }



            // If the current character is not quoted and can be used as the first character of a new operator, the current token (if any) shall be delimited. The current character shall be used as the beginning of the next (operator) token.
            if(tokenizer->quote_char == '\0') {

            }

            // If the current character is an unquoted <blank>, any token containing the previous character is delimited and the current character shall be discarded.
            if(tokenizer->quote_char == '\0') {

            }
        }

        // If the previous character was part of a word, the current character shall be appended to that word.
        if(false) {

        }

        // If the current character is a '#', it and all subsequent characters up to, but excluding, the next <newline> shall be discarded as a comment. The <newline> that ends the line is not considered part of the comment.
        if(false) {

        }

        // The current character is used as the start of a new word.
        
    }



    return NULL;

}


*/