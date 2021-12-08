#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stringlib.h"

/*
 * Returns a pointer to a new string that is a concatenated versione of the firsto two with the char in the middle.
 * Like this "[str1][chr][str2]"
 */
char* strcatchr(const char* str1, const char* str2, const char chr) {
    int n = strlen(str1) + strlen(str2) + 1;
    char* new_string = (char*) malloc (n+1);
    memset(new_string, '\0', n+1);
    strcpy(new_string, str1);
    new_string[strlen(new_string)] = chr;
    strcat(new_string, str2);
    return new_string;
}

/*
 * Returns an array of pointers to every part the string is split around chr. The array is
 * terminated by a NULL pointer
 */
char** strsplitchr(char* src, const char chr) {
    int count = 0;
    int n;
    char** dst;

    char* chrp = src;
    while(chrp) {
        count++;
        chrp = strchr(++chrp, chr);
    }

    dst = malloc((count+1)*(sizeof(char*)));
    dst[count] = NULL;

    char* startp = src;
    for (int i = 0; i < count; i++) {
        chrp = strchr(startp, chr);
        if (chrp)
            n = chrp - startp;
        else
            n = strlen(startp);

        dst[i] = (char*) malloc(n+1);
        strncpy(dst[i], startp, n);
        dst[i][n] = '\0';
        startp = ++chrp;
    }

    return dst;
}
