#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include<time.h>


int main()
{
    if(mkfifo("FIFO_CLIENT",0777)==-1)
    {
        if(errno!=EEXIST)
        {
            perror("Eroare la creare fifo\n");
            exit(1);
        }

    }
    if(mkfifo("FIFO_SERVER",0777)==-1)
    {
        if(errno!=EEXIST)
        {
            perror("Eroare la creare fifo\n");
            exit(1);
        }

    }
    int open_client = 1;
    while(open_client){
        char buff[1024];
        printf("\n");
        printf(">");
        fgets(buff, sizeof(buff), stdin);
        printf("\n");
        buff[strlen(buff)-1] = '\0';
        if(strstr(buff, "quit") == buff)
        open_client = 0;
        int fd=open("FIFO_CLIENT",O_WRONLY);
        if(fd==-1)
        {
            perror("Eroare la deschiderea FIFO\n");
            exit(2);
        }
        if(write(fd,buff,sizeof(buff))==-1)
        {
            perror("Eroare la scriere in FIFO\n");
            exit(3);
        }
        
        int f=open("FIFO_SERVER",O_RDONLY);
        if(f==-1)
        {
            perror("Eroare la deschiderea FIFO\n");
            exit(2);
        }
        char reply[1024];
        if(read(f,reply,sizeof(reply))==-1)
        {
            perror("Eroare la scriere in FIFO\n");
            exit(3);
        }
        while(reply[strlen(reply)-1] == '\n')
        reply[strlen(reply)-1] = '\0';
        //sterge '\n' care nu sunt necesare

        printf("%s\n",reply);
    }
}