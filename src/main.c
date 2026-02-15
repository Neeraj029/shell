
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
  // Load config files, if any.
  rl_bind_key('\t', auto_complete);

  if (read_history(".lsh_history") == 0)
  {

    lsh_loop();
  }
  else
  {
    printf("History file not found");
  }

  append_history(0, ".lsh_history");

  return EXIT_SUCCESS;
}
