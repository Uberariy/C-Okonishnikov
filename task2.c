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
    int i, j, fd1, stat, c;
    int fd[2];

    if (argc<2)
    {   fprintf(stderr, "error");   return(1);  }

    fd1 = open(argv[1], O_RDWR, 0777);
    int fsize=0;
    while (read(fd1, &c, 1))
    {
        fsize++;
        write(fd1, &c, 1);
    }

    return(0);
}