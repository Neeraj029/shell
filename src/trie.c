#include "trie.h"
struct dirent *dir;
TrieNode *root;  // global


TrieNode *createNode()
{
    TrieNode *node = (TrieNode *)calloc(1, sizeof(TrieNode));
    for (int i = 0; i < NUM; i++)
        node->children[i] = NULL;
    node->terminal = false;
    return node;
}

int char_index(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= '0' && c <= '9') return 26 + (c - '0');
    return -1;
}

void insert(TrieNode *root, const char *word) {
    TrieNode *curr = root;
    while (*word) {
        int idx = char_index(*word);
        
        if (idx == -1) { word++; continue; } // skip invalid chars
        if (curr->children[idx] == NULL)
            curr->children[idx] = calloc(1, sizeof(TrieNode));
        curr = curr->children[idx];
        word++;
    }
    curr->terminal = true;
}

char last_match[256];
int n = 5;
int match_cnt = 0;
char **words ;  // array of n char* pointers

void dfs(TrieNode *node, char *prefix, int depth, char word[])
{
    if (node->terminal)
    {
        prefix[depth] = '\0';
        char buf[100];
        strcpy(buf, word);
        strcat(buf, prefix);
        if((match_cnt + 1) > n){
            n = n + 5;
            words = realloc(words, n * sizeof(char*));
        }
        
        words[match_cnt] = strdup(buf);
        
        strncpy(last_match, buf, sizeof(last_match) - 1);
        last_match[sizeof(last_match) - 1] = '\0';
        match_cnt++;
    }

    for (int i = 0; i < NUM; i++)
    {
        if (node->children[i] != NULL)
        {
            if (i < 26)
                prefix[depth] = (char)('a' + i);
            else
                prefix[depth] = (char)(i - 26 + '0');
            dfs(node->children[i], prefix, depth + 1, word);
        }
    }
}

int searchWord(TrieNode *root, char *word)
{
    words = malloc(n * sizeof(char *));
    if (!root) return 0;
    TrieNode *temp = root;
    for (int i = 0; i < strlen(word); i++)
    {
        int idx = char_index(word[i]);
        if (idx == -1) continue;  
        if (temp->children[idx] == NULL)
        {
            return 0;  
        }
        temp = temp->children[idx];
    }
    char buf[200];
    match_cnt = 0;  
    dfs(temp, buf, 0, word);

    return match_cnt;
}



void buildTrie()
{   root = createNode();
    char *septr = ":";
    char *path_var = getenv("PATH");
    char *path_cp = strdup(path_var);
    char *tkn = strtok(path_cp, septr);
    DIR *d;
    int file_cnt = 0;
    while (tkn)
    {
        if (strncmp(tkn, "/mnt/", 5) != 0)
        {
            d = opendir(tkn); // Opens the current directory
            if (d)
            {
                while ((dir = readdir(d)) != NULL)
                {
                    insert(root, dir->d_name);
                    file_cnt++;
                }
                closedir(d);
            }

        }
        tkn = strtok(NULL, septr);
    }
    free(path_cp);
}
