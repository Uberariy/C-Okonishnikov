#include <stdio.h>
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
    struct list *next;
};

struct list *curlistadd (struct list *L, char *w, int s, int n)
{
    if (L==NULL)
    {
        L = (struct list *)malloc(sizeof(struct list));
        L -> word = (char *)malloc(s+1);

        strcpy(L -> word, w);
        L -> number = n;
        L -> size = s;
        L -> next = NULL;
    }
    else L -> next = curlistadd (L -> next, w, s, n);
    return L;
}

void listTOsargv (struct list *L, char **sargv, int i)
{
    if (i == L -> number)
    {
        strcpy ((*sargv), L -> word);  //printf("1:%s   2:%s   ", (*sargv), L -> word);
    }
    else listTOsargv (L -> next, sargv, i);
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

int main(int argc, char *argv[])
{
    struct list *curlist;          // List to determine maxsize of a word in a single cmd
    char *curpath, *curpathtmp, *curstr, *curpathelem;
    char c;

    int sargc, curstrsize, curstrmax, curpathsize;
    int quotesflag, begincmd, endflag;                  // Flags for quotes in a command line; for start of cmd;
    int i, j;                           // Counters
    char **sargv;                       // Argv of a cmd

    pid_t pid;

    curpathsize = 2;
    curpath = (char *)malloc(2);
    //curpathtmp = (char *)malloc(2);
    *curpath = '\0';
    //*curpathtmp = '\0';

    while(1)
    {
        curstr = (char *)malloc(2);

        printf("shell:~");
        printf("%s", curpath);
        printf("$ ");

        curlist=NULL;
        endflag=0; 
        quotesflag=0;
        begincmd=1;
        curstrmax=0;
        sargc=0;

        while ((c = getchar()) != '\n')     // Read the line
        {
            if (begincmd)          // If EOF is met in the beginning without a '\n'
                if (c == EOF)
                {
                    printf("exit\n");
                    endflag=1;
                    break;
                }

            curstrsize = 0;
            while (((c != ' ') && (c != '\t') && (c != '\n') && (c != EOF)) || (quotesflag))
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
            }

            if (curstrsize>0)
            {
                curstr[curstrsize] = '\0';                          //printf("sargc = %d\n", sargc);
                curlist = curlistadd(curlist,curstr,curstrsize,sargc);   // Create Dynamic memmory for curlist
                if (curstrmax < curstrsize) curstrmax = curstrsize;
                curstrsize = 0;
                sargc++;
                free(curstr);
                curstr = (char *)malloc(2);
            }
            if (c == '\n') break;
        }
        if (curstr != NULL) free(curstr);
        if (endflag) break;

        sargv = (char **)malloc((sargc+1)*(sizeof(char *)));         // Create Dynamic memory for argv
        for (i = 0; i<=sargc; i++)
            sargv[i] = (char *)malloc(curstrmax+2);
                                                    //printf("here sargc = %d  curstrmax = %d\n", sargc, curstrmax);
        for (i = 0; i<sargc; i++)
            listTOsargv (curlist, &(sargv[i]), i);
        sargv[sargc][0] = '\0';
                                                    //printf("here sargc = %d  curstrmax = %d\n", sargc, curstrmax);
        //for (i = 0; i<sargc; i++)
        //printf("%s\t", sargv[i]);
                                                    //printf("here sargc = %d  curstrmax = %d\n", sargc, curstrmax);
        if (strcmp(sargv[0], "exit") == 0)
        {
            endflag=1;
        }
        else if (strcmp(sargv[0], "cd") == 0)
        {   
            if ((sargc==1) || (strcmp(sargv[1],"~") == 0))
            {
                chdir("/");
                *curpath = '\0';
                curpathsize = 2;
            }
            else if (chdir(sargv[1]) == 0)
            { 
                curpathsize += curstrmax+2;
                curpath = (char *)realloc(curpath, curpathsize);
               // if (sargv[1][0] != '/')
             //   {
           //         strcat(curpath, "/");
         //           strcat(curpath, sargv[1]);
       //         }
     //           else strcpy(curpath, sargv[1]);
                for(i=0; ((i<=curstrmax+1) && (sargv[1][i] != '\0')); i++) //Тут остановился
                {
                    strcat(curpath, "/");
                    j = 0;
                    curpathelem = (char *)malloc(curstrmax+2);
                    while(sargv[1][i] != '/')
                    {
                        curpathelem[j] = sargv[1][i];
                        i++; j++;
                    }
                    if (strcmp(curpathelem, "..") == 0)
                    {
                        for (i=0; curpath[i] != '\0' ; i++)
                            if (curpath[i] == '/') j=i;  // Может привести к ошибкt стрката
                        curpath[j] = '\0';
                    }
                    else 
                    {
                        strcat(curpath, "/");
                        strcat(curpath, curpathelem);
                    }
                    free(curpathelem);
                }                
            }
            else 
            {
                fprintf(stderr, "cd: %s: Not such file of directory\n", sargv[1]);
            }
        }
        else 
        {   
            if (fork() == 0)
            {
                execv(sargv[0], sargv);
                fprintf(stderr, "%s: command not found\n", sargv[0]);
                exit('R');
            }
            else
            {
                pid = wait(NULL);
            }
        }

        for (i = 0; i<=sargc; i++)               // Clear Dynamic memory for argv
            free(sargv[i]);
        free(sargv);

        freelist(curlist);                      // Clear Dynamic memory for curlist

        if (endflag) break;
    }

    free(curpath);
    return(0);
}