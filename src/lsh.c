#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "lsh.h"
#include "trie.h"

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
    "cd",
    "help",
    "exit"};

int (*builtin_func[])(char **) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit};

int lsh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

int lsh_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args)
{
    int i;
    printf("Fenric's custom shell\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < lsh_num_builtins(); i++)
    {
        printf("  %s\n", builtin_str[i]);
    }

    return 1;
}

int lsh_exit(char **args)
{
    return 0;
}

int auto_complete(int count, int key)
{
    if (root == NULL)
        return 0;

    char *last = strrchr(rl_line_buffer, ' ');
    char *buf = last ? last + 1 : rl_line_buffer;

    printf("\n");                        // move off the current prompt line
    int matches = searchWord(root, buf); // print suggestions, returns count

    if (matches == 1)
    {
        // only one match — autocomplete it
        rl_replace_line(last_match, 0);
        rl_point = rl_end; // move cursor to end
    }
    else
    {
        for (int i = 0; i < matches; i++)
        {
            printf("%s\n", words[i]);
        }
    }

    for (int i = 0; i < match_cnt; i++)
    {
        free(words[i]); 
    }
    free(words);


    rl_on_new_line(); // tell readline cursor is on a new line
    rl_redisplay();   // redraw prompt
    return 0;
}

int lsh_launch(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        // Child process
        if (execvp(args[0], args) == -1)
        {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        // Error forking
        perror("lsh");
    }
    else
    {
        // Parent process
        do
        {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int lsh_execute(char **args)
{
    int i;

    if (args[0] == NULL)
    {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < lsh_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }

    return lsh_launch(args);
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token, **tokens_backup;

    if (!tokens)
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += LSH_TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                free(tokens_backup);
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void lsh_loop(void)
{
    rl_initialize();

    char *line;
    char **args;
    int status;

    char *buf;
    do
    {
        buf = readline(">> ");
        if (strlen(buf) > 0)
        {
            add_history(buf);
        }
        args = lsh_split_line(buf);

        status = lsh_execute(args);

        free(buf);
        free(args);
    } while (status && buf);
}
