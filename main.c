#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "stringlib.h"

#define MAX_LAYOUT_LENGTH 12 // Assuming the longest one is `nec_vndr/jp`
#define MAX_VARIANT_LENGTH 27 // Assuming the longest one is `tifinagh-extended-phonetic`

/*
 * Returns a char pointer to a string from the needle to '\n' or '\0'.
 *
 * Returns `NULL` if the needle cannot be found in the haystack
 */
char* parseValue(const char* haystack, const char* needle) {
    char* beginning_of_word = strstr(haystack, needle);
    if (beginning_of_word == NULL) return NULL;

    char* new_line = strchr(beginning_of_word, '\n');
    char* last_space = new_line;
    while(*last_space != ' ') last_space--;
    
    int n = new_line - last_space - 1;
    
    char* new_string = (char*) malloc((n+1)*sizeof(char));
    if (new_string == NULL) {
        fprintf(stderr, "An error occurred while using malloc! ERROR: %s\n", strerror(errno));
        exit(1);
    }

    strncpy(new_string, last_space+1, n);
    new_string[n] = '\0';

    return new_string;
}

char* getCurrentLayout() {
    char stringa[1024];
    int link[2];
    pid_t pid;

    if (pipe(link) == -1) {
        fprintf(stderr, "An error occurred while creating the pipe! ERROR: %s\n", strerror(errno));
        exit(1);
    }

    pid = fork();

    if (pid == -1) {
        fprintf(stderr, "An error occurred while trying to fork! ERROR: %s\n", strerror(errno));
        exit(1);
    }

    if (pid == 0) {
        // figlio
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        execlp("setxkbmap", "setxkbmap", "-query", (char *)0);
        exit(0);
    }

    // padre
    close(link[1]);
    int nbytes = read(link[0], stringa, 1024);
    wait(NULL);

    char* new_string = (char*) malloc(nbytes+1);
    strncpy(new_string, stringa, nbytes);
    new_string[nbytes] = '\0';

    return new_string;
}

int countFileLines(const char* path) {
    FILE *f = fopen(path, "r");
    char ch;
    int linesCount = 0;
    
    if (f == NULL) {
        fprintf(stderr, "An error occurred while parsing `%s`! ERROR: %s\n", path, strerror(errno));
        exit(1);
    }

    while((ch=fgetc(f))!=EOF) {
      if(ch=='\n')
         linesCount++;
    }
    
    fclose(f);
    return linesCount;
}

/*
 * Expands the given path using the HOME env variable
 *
 * Returna NULL if the given path doesn't start with '~'
 */
char* expandPath(const char* path) {
    const char* home = getenv("HOME");
    if (path[0] != '~') { return NULL; }
    path++;
    int n = strlen(path) + strlen(home);
    char* expandedPath = (char*) malloc(n+1);
    strncpy(expandedPath, home, strlen(home)+1);
    strncat(expandedPath, path, strlen(path)+1);
    return expandedPath;
}

void parseLayouts(char layouts[][MAX_LAYOUT_LENGTH+MAX_VARIANT_LENGTH], const char* path) {
    FILE *f = fopen(path, "r");
    char line[MAX_LAYOUT_LENGTH+MAX_VARIANT_LENGTH];
    int i = 0;

    if (f == NULL) {
        fprintf(stderr, "An error occurred while parsing `%s`! ERROR: %s\n", path, strerror(errno));
        exit(1);
    }

    while(fgets(line, sizeof(line), f)) {
        line[strlen(line) - 1] = '\0';
        strcpy(layouts[i], line);
        i++;
    }
}

void setNewLayout(char** layout) {
    pid_t pid = fork();

        if (pid == -1) {
        fprintf(stderr, "An error occurred while trying to fork! ERROR: %s\n", strerror(errno));
        exit(1);
    }

    if (pid == 0) {
        // figlio
        printf("[DEBUG] Layout to be set: %s\n", layout[0]);
        printf("[DEBUG] Variant to be set: %s\n", layout[1]);

        if (!layout[1]) {
            execlp("setxkbmap", "setxkbmap", layout[0], (char *)0);
            exit(0);
        }
        execlp("setxkbmap", "setxkbmap", layout[0], "-variant", layout[1], (char *)0);
        exit(0);
    }

    // padre
    wait(NULL);
}

int main() {
    char* stringa = getCurrentLayout();
    char* layout = parseValue(stringa, "layout");
    char* variant = parseValue(stringa, "variant");
    char* current_layout;
    free(stringa);

    if (variant) {
        current_layout = strcatchr(layout, variant, '-');
        free(layout);
        free(variant);
    } else {
        current_layout = layout;
    }

    printf("[DEBUG] Current config: %s\n", current_layout);

    const char* path = "~/.config/layouts.conf";
    int nLayouts = countFileLines(expandPath(path));
    char layouts[nLayouts][MAX_LAYOUT_LENGTH+MAX_VARIANT_LENGTH];
    parseLayouts(layouts, expandPath(path));

    printf("[DEBUG] Indexed %d layouts from `%s`\n", nLayouts, path);
    for (int i = 0; i < nLayouts; i++) {
        printf("    [*] %s\n", layouts[i]);
    }

    int index;
    for (int i = 0; i < nLayouts; i++)
        if (strcmp(current_layout, layouts[i]) == 0) {
            index = i;
            break;
        }
    free(current_layout);

    printf("[DEBUG] Current index: %d\n", index);
    if (++index == nLayouts) index = 0;
    printf("[DEBUG] Next index: %d\n", index);

    char** new_layout = strsplitchr(layouts[index], '-');
    setNewLayout(new_layout);

    printf("[DEBUG] Set new layout `%s`\n", layouts[index]);

    return 0;
}
