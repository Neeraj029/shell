#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#define NUM 36

typedef struct trieNode
{
    struct trieNode *children[NUM];
    bool terminal;
} TrieNode;

TrieNode *createNode();
extern struct dirent *dir;
extern TrieNode *root;
extern char last_match[256];
extern int match_cnt;
extern char **words;
void buildTrie();

int char_index(char c);

void insert(TrieNode *root, const char *word);
void dfs(TrieNode *node, char *prefix, int depth, char word[]);
int searchWord(TrieNode *root, char *word);