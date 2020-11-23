#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

struct list
{
    char* word;
    int size;
    int number;
    int istoken;        // 1 if token, 0 if not
    struct list *next;
};

struct list *curlistadd (struct list *L, char *w, int s, int n, int t)
{
    if (L==NULL)
    {
        L = (struct list *)malloc(sizeof(struct list));
        L -> word = (char *)malloc(s+1);

        strcpy(L -> word, w);
        L -> number = n;
        L -> size = s;
        L -> istoken = t;                           //printf("|%s|%d\t", L -> word, L -> istoken);
        L -> next = NULL;
    }
    else L -> next = curlistadd (L -> next, w, s, n, t);
    return L;
}

void listTOsargv (struct list *L, char ****sargv, int i, int j)
{
    if (L != NULL)
    {                                               //printf("\ti:%d, j:%d, %s", i, j, L -> word);
        if (L -> istoken)
        {                                           //printf("\n");
            (*sargv)[i][j] = NULL;
            j=0;    i++;
        }
        else
        {                                           //printf("   Writing word\n");
            strcpy ((*sargv)[i][j], L -> word);
            j++;
        }        
        listTOsargv (L -> next, sargv, i, j);
    }
    else
    {                                               //printf("\ti:%d, j:%d\n", i, j);
        (*sargv)[i][j] = NULL;
        j=0;    i++;                                //printf("\ti:%d, j:%d Ending\n", i, j);
        (*sargv)[i][j] = NULL;
    }
}

void listTOtoken (struct list *L, char ***sargv, int j)
{
    if (L != NULL)
    {
        if (L -> istoken)
        {
            strcpy ((*sargv)[j++], L -> word);
        }
        listTOtoken (L -> next, sargv, j);
    }
    else (*sargv)[j] = NULL;
}

void freelist (struct list *L)
{
    if (L!=NULL)
    {
        freelist (L -> next);
        free (L -> word);
        free (L);
    }
}

int ifistoken (char c)
{
    if ((c == '>') || (c == '<') || (c == '|')) return(1);
    else return(0);
}

int main(int argc, char *argv[])
{
    struct list *curlist;          // List to determine maxsize of a word in a single cmd
    char *curpath, *curpathtmp, *curstr, *curpathelem;
    char c, workroot='~';

    int sargc, stokenc, curstrsize, strmax, curpathsize;
    int quotesflag, begincmd, endflag, readprev, tokenflag;                  // Flags for quotes in a command line; for start of cmd;
    int tmpflag=1;                        // Minor flags
    int i, j, k;                           // Counters
    char **stokenv; 
    char ***sargv;                      // Argv of a cmd

    pid_t pid;

    curpathsize = 2;
    curpath = (char *)malloc(2);
    *curpath = '\0';

    while(1)
    {
        curstr = (char *)malloc(2);

        printf("HAL9000:");
        if (workroot != '1') putchar(workroot);
        printf("%s", curpath);
        printf("$ ");

        readprev=1;
        tokenflag=0;
        curlist=NULL;
        endflag=0; 
        quotesflag=0;
        begincmd=1;

        strmax=0;
        sargc=0;
        stokenc=0;

        c = getchar();
        while (c != '\n')     // Read the line
        {
            if (begincmd)          // If EOF is met in the beginning without a '\n'
                if (c == EOF)
                {
                    printf("exit\n");
                    endflag=1;
                    break;
                }
            curstrsize = 0;
            
            if (ifistoken(c))
            {
                while (ifistoken(c))   // Token input
                {
                    curstr[curstrsize] = c;
                    curstrsize++;
                    curstr = realloc(curstr, curstrsize+1);
                    c = getchar();
                    readprev=0;
                }
                tokenflag=1;
                stokenc++;
            }
            else
            {
                while (((!ifistoken(c) && (c != ' ') && (c != '\t') && (c != EOF)) || (quotesflag)) && (c != '\n'))
                {
                    begincmd = 0;
                    if (c=='"')
                    {
                        quotesflag = (quotesflag + 1) % 2;
                    }
                    else
                    {                
                        curstr[curstrsize] = c;
                        curstrsize++;
                        curstr = realloc(curstr, curstrsize+1);
                    }
                    c = getchar();
                    readprev=0;
                }
                tokenflag=0;
            }

            if (curstrsize>0)
            {
                curstr[curstrsize] = '\0';                          //printf("sargc = %d\n", sargc);
                curlist = curlistadd(curlist,curstr,curstrsize,sargc,tokenflag);   // Create Dynamic memmory for curlist
                if (strmax < curstrsize) strmax = curstrsize;
                curstrsize = 0;
                sargc++;
                free(curstr);
                curstr = (char *)malloc(2);
            }

            if (c == '\n') break;
            if (readprev) c = getchar();
            else readprev=1;
        }
        if (curstr != NULL) free(curstr);
        if (endflag) break;

                                                                    printf("%d, %d, %d\n", stokenc, sargc, strmax);
        stokenv = (char **)malloc((stokenc+1)*(sizeof(char *))); 
        for (i = 0; i<stokenc; i++)
            stokenv[i] = (char *)malloc(strmax+2);
        listTOtoken (curlist, &(stokenv), 0);
                                                                    //for(i = 0; i<=stokenc; i++)    printf("%s\t", stokenv[i]);
                                                    printf("\n");
                                                                    //printf("%d, %d, %d", stokenc, sargc, strmax);
        sargv = (char ***)malloc((stokenc+2)*(sizeof(char **)));        // Create Dynamic memory for argv
        for (i = 0; i<stokenc+2; i++)
        {                                                           //printf("%d, %d, %d", stokenc, sargc, strmax);
            sargv[i] = (char **)malloc((sargc+2)*(sizeof(char *)));         // ЗАМЕНИТЬ НА МАКСИМАЛЬНОЕ КОЛИЧЕСТВО СЛОВ В СТРОКЕ
            for (j = 0; j<sargc+2; j++)
                sargv[i][j] = (char *)malloc(strmax+2);
        }                                                           //printf("Memmory made\n");
        listTOsargv (curlist, &(sargv), 0, 0);
                                                    for(i = 0; i<stokenc+1; i++)
                                                    { 
                                                        printf("\nСтрока %d\t", i);
                                                        for(j=0; j<sargc+2; j++)
                                                            printf("%s\t", sargv[i][j]);
                                                    }printf("\n");


        
        if (sargc == 0) ;
        else if (strcmp(sargv[0][0], "exit") == 0)
        {
            endflag=1;
        }
        else if (strcmp(sargv[0][0], "cd") == 0)
        {   
            if ((sargc==1) || (strcmp(sargv[0][1],"~") == 0))
            {
                chdir(getenv("HOME"));
                workroot = '~';
                *curpath = '\0';
                curpathsize = 2;
            }
            else if (chdir(sargv[0][1]) == 0)
            { 
                curpathsize += strmax+2;                             //printf("curstrmax: %d\n", curstrmax);
                curpath = (char *)realloc(curpath, curpathsize);        //printf("curpathsize: %d\n", curpathsize);
                if (sargv[0][1][0] == '/') 
                {
                    free(curpath);
                    curpath = (char *)malloc(curpathsize); 
                    curpath[0] = '\0';
                    tmpflag = 0;
                    if (sargv[0][1][1] == '\0') workroot = '/';
                    else workroot = '1';
                }
                for(i=0; ((i<strmax) && (sargv[0][1][i] != '\0')); i++) 
                {
                    j = 0;
                    curpathelem = (char *)malloc(strmax+2);
                    while((sargv[0][1][i] != '/') && (sargv[0][1][i] != '\0'))
                    {
                        curpathelem[j] = sargv[0][1][i];
                        i++; j++;                                       //printf("curpathelem: %s\n", curpathelem);
                    }
                    curpathelem[j] = '\0';                              //printf("curpath: %s\n", curpath);
                    if (strcmp(curpathelem, "..") == 0)
                    {                                                   
                        for (k=0; (curpath[k] != '\0'); k++)
                            if (curpath[k] == '/') j=k;                 //printf("k %d   j %d\n", k, j); // Может привести к ошибкt стрката 
                        for (; j<=k; j++)
                        {
                            curpath[j] = '\0';
                        }
                    }
                    else 
                    {
                        if (tmpflag) strcat(curpath, "/");
                        else tmpflag = 1;
                        strcat(curpath, curpathelem);
                    }
                    free(curpathelem);
                }                
            }
            else 
            {
                fprintf(stderr, "cd: %s: Not such file or directory\n", sargv[0][1]);
            }
        }
        else
        {   
            if (fork() == 0)
            {
                execvp(sargv[0][0], sargv[0]);
                fprintf(stderr, "%s: command not found\n", sargv[0][0]);
                exit('R');
            }
            else
            {
                int stat;
                pid = wait(&stat);
                if (WIFEXITED(stat) == 0)
                    if (WEXITSTATUS(stat) != 'R')
                        fprintf(stderr, "%s: exited with error\n", sargv[0][0]);
            }
        }




        for (i = 0; i<stokenc; i++)
            free(stokenv[i]);
        free(stokenv);
                                                    //printf("here sargc = %d  curstrmax = %d\n", sargc, curstrmax);
        for (i = 0; i<stokenc+2; i++)               // Clear Dynamic memory for argv
        {
            for (j = 0; j<sargc+2; j++)
                free(sargv[i][j]);
            free(sargv[i]);
        }
        free(sargv);

        freelist(curlist);                      // Clear Dynamic memory for curlist

        if (endflag) break;
    }

    free(curpath);
    return(0);
}