
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "lsh.h"
#include "trie.h"

int main(int argc, char **argv)
{
  buildTrie();
  rl_bind_key('\t', auto_complete);

  char histpath[1024];
  const char *home = getenv("HOME");
  if (home)
    snprintf(histpath, sizeof(histpath), "%s/.lsh_history", home);
  else
    snprintf(histpath, sizeof(histpath), ".lsh_history");

  if(read_history(histpath) == -1){
    FILE *histfile = fopen(histpath, "w");
    if (histfile) {
      fclose(histfile);
    } else {
      fprintf(stderr, "Warning: Could not create history file at %s\n", histpath);
    }
  }

  lsh_loop();

  write_history(histpath);

  return EXIT_SUCCESS;
}
