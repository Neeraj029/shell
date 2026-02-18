#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "lsh.h"
#include "trie.h"
#define RESET "\033[0m"
#define GREEN "\033[34m"
#define PATH_MAX 70
#include <unistd.h>
#include <pwd.h>

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

    // Use HOME if arg is "~", otherwise use the argument provided
    char *path = (args[1] == NULL || strcmp(args[1], "~") == 0) ? getenv("HOME") : args[1];
    if (chdir(path) != 0)
    {
        perror("lsh");
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

    if (!buf[0]) // 1st case (nothing)
    {
        return 0;
    }
    else if (!last && buf[0]) // 2nd case (command autocomplete)
    {
        printf("\n");
        int matches = searchWord(root, buf);
        if (matches == 1)
        {
            // only one match — autocomplete it
            rl_replace_line(last_match, 0);
            rl_point = rl_end; // move cursor to end
        }
        int name_per_row = 4;
        for (int i = 0; i < matches; i++)
        {
            if (i > 0 && i % name_per_row == 0)
                printf("\n");
            printf("%s\t", words[i]);
        }
        printf("\n");
        for (int i = 0; i < match_cnt; i++)
        {
            free(words[i]);
        }
        free(words);
    }
    else
    { // 3rd case (file autocomplete)
        return 0;
    }

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
char cwd[PATH_MAX];
char prompt[PATH_MAX + 20];
void lsh_loop(void)
{

    rl_initialize();

    char **args;
    int status;
    struct passwd *pw = getpwuid(getuid());
    char *name = pw->pw_name;
    char *buf;
    do
    {
        getcwd(cwd, sizeof(cwd));
        snprintf(prompt, sizeof(prompt),
                 "\001" GREEN "\002%s\001" RESET "\002:\001" GREEN "\002%s\001" RESET "\002$ ",
                 name, cwd);
        buf = readline(prompt);
        if (strncmp(buf, "cd", 3) == 0)
        {
            chdir(buf + 3);
        }
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
