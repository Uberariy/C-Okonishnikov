#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

struct tree
{
    char* word;
    int number;
    struct tree *left;
    struct tree *right;
};

char *inword(char *s, int size)
{
    char *p;
    p = (char *)malloc(size+1);
    if (p != NULL)
        strcpy(p, s);
    return p;
}

struct tree *addnode (struct tree *T, char *w, int size, int *maxN)
{
    int cond;
    if (T==NULL)    
    {
        T = (struct tree *)malloc(sizeof(struct tree));
        T -> word = inword(w,size);
        T -> number = 1;
        T -> left = T -> right = NULL;
        (*maxN)=1;
    }
    else if ((cond = strcmp(w, T -> word)) == 0)
    {
        T -> number++;
    }
    else if (cond < 0)
        T -> left = addnode(T -> left, w, size, maxN);
    else
        T -> right = addnode(T -> right, w, size, maxN);
    if ((*maxN) < (T -> number)) (*maxN) = T -> number;
    return T;
}

void printree (struct tree *T, int i, double freq)
{
    if (T != NULL)
    {
        printree (T -> left, i, freq);
        if (i == T -> number)
            printf("%s  %d  %6f\n", T -> word, i, freq);
        printree (T -> right, i, freq);    
    }
}

void printbynumber (struct tree *T, int *maxN, int wholeN)
{
    int i;
    for (i=(*maxN); i>=1; i--)
        printree(T,i,(double)i/wholeN);
}

int main()
{
    struct tree *T=NULL;
    char* w=NULL;
    char c;
    int size = 0, wholeN = 0;
    int maxN = 0;
    
    while ((c=getchar()) != EOF)
    {
        w = malloc(1);
        while ((c != '\n') && (c != EOF) && (c != ' ')) // && (c != '.') && (c != ',') && (c != '-') && (c != ':'))
        {
            w[size] = c;
            size++;
            w = realloc(w, size+1);               
            c = getchar();
        }
        if (size>0) 
        {
            T = addnode(T,w,size,&maxN); 
            size=0;    free(w);
            wholeN++;
        }
    }
    printf("\n%s\n","----- start of the print:");
    printbynumber(T,&maxN,wholeN);
    printf("\n%s\n","----- end of the print:");

    return(0);
}