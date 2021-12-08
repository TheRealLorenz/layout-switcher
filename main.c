#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "stringlib.h"

int DEBUG = 0;

#define MAX_LAYOUT_LENGTH 12 // Assuming the longest one is `nec_vndr/jp`
#define MAX_VARIANT_LENGTH 27 // Assuming the longest one is `tifinagh-extended-phonetic`

typedef struct Layout {
    char* layout;
    char* variant;
    struct Layout* next;
} Layout;

/*
 * Compares the content of the pointers to Layout struct
 */
int layoutCompare(const Layout* l1, const Layout* l2) {
    if (l1->variant && !l2->variant) return 1;
    if (!l1->variant && l2->variant) return -1;
    if (l1->variant && l2->variant) {
        if (strcmp(l1->variant, l2->variant) > 0) return 1;
        if (strcmp(l1->variant, l2->variant) < 0) return -1;
    }

    if (strcmp(l1->layout, l2->layout) > 0) return 1;
    if (strcmp(l1->layout, l2->layout) < 0 ) return -1;
    return 0;
}

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

Layout* parseLayouts(const char* path) {
    FILE *f = fopen(path, "r");
    Layout* head = NULL;
    char line[MAX_LAYOUT_LENGTH+MAX_VARIANT_LENGTH];

    if (f == NULL) {
        fprintf(stderr, "An error occurred while parsing `%s`! ERROR: %s\n", path, strerror(errno));
        exit(1);
    }

    Layout* temp;
    while(fgets(line, sizeof(line), f)) {
        line[strlen(line) - 1] = '\0';
        char** split = strsplitchr(line, '-');

        if (!head) {
            head = (Layout*)malloc(sizeof(Layout));
            head->layout = split[0];
            head->variant = split[1];
            temp = head;
        } else {
            temp->next = (Layout*)malloc(sizeof(Layout));
            temp = temp->next;
            temp->layout = split[0];
            temp->variant = split[1];
        }
    }

    return head;
}

void setNewLayout(Layout* layout) {
    pid_t pid = fork();

        if (pid == -1) {
        fprintf(stderr, "An error occurred while trying to fork! ERROR: %s\n", strerror(errno));
        exit(1);
    }

    if (pid == 0) {
        // figlio
        if (!layout->variant) {
            execlp("setxkbmap", "setxkbmap", layout->layout, (char *)0);
            exit(0);
        }
        execlp("setxkbmap", "setxkbmap", layout->layout, "-variant", layout->variant, (char *)0);
        exit(0);
    }

    // padre
    wait(NULL);
}

void deallocateLayout(Layout* l) {
    free(l->layout);
    free(l->variant);
    free(l);
}

int main() {
    char* stringa = getCurrentLayout();
    Layout* currentLayout = (Layout*)malloc(sizeof(Layout));
    currentLayout->layout = parseValue(stringa, "layout");
    currentLayout->variant = parseValue(stringa, "variant");
    free(stringa);

    if (DEBUG) printf("[DEBUG] Current config: %s-%s\n", currentLayout->layout, currentLayout->variant);

    const char* path = "~/.config/layouts.conf";
    Layout* layouts = parseLayouts(expandPath(path));
    Layout* temp;

    if (DEBUG) {
        printf("[DEBUG] Indexed layouts from `%s`:\n", path);
        temp = layouts;
        while(temp != NULL) {
            printf("    [*] %s-%s\n", temp->layout, temp->variant);
            temp = temp->next;
        }
    }

    temp = layouts;
    while (temp != NULL) {
        if (layoutCompare(currentLayout, temp) == 0) break;
        temp = temp->next;
    }
    deallocateLayout(currentLayout);

    Layout* next_layout = temp->next;
    if (!next_layout) next_layout = layouts;
    setNewLayout(next_layout);

    if (DEBUG) printf("[DEBUG] Set new layout `%s-%s`\n", next_layout->layout, next_layout->variant);

    if (!next_layout->variant)
        printf("Set `%s` layout\n", next_layout->layout);
    else
        printf("Set `%s-%s` layout\n", next_layout->layout, next_layout->variant);

    return 0;
}
