#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char *argv[])
{
    pid_t pid, p;
    int i, j, fd1, fd2, stat, numb=0;
    char c;
    int fd[2];

    if (argc<2)
    {   fprintf(stderr, "error");   return(1);  }

    fd1 = open(argv[1], O_RDONLY, 0777);
    fd2 = open(argv[1], O_RDWR, 0777);
    int fsize=0;
    int fsize2=0;
    while (read(fd1, &c, 1))
    {
        fsize++;
        write(fd2, &c, 1);
        if (c == '\n') 
        {
            fsize2 = fsize;
        }

        if ((c >= '0') && (c <= '9'))
            numb++;

        write(fd2, &c, 1);
        if (numb==2)
        {
            while ((read(fd1, &c, 1)) && (c != '\n'))
            {
                write(fd2, &c, 1);
            }
            lseek(fd2, fsize2, SEEK_SET);
        }
        write(fd2, &c, 1);
    }
    ftruncate(fd2, fsize);

    return(0);
}