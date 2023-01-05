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
#include <sys/socket.h>
#include <unistd.h>
#include <utmp.h>
#define LOGIN 10
#define GET_LOGGED_USERS 20
#define GET_PROC_INFO 30
#define LOGOUT 40
#define QUIT 50
#define FILENAME "config.txt"
#define NO_CMD 999 // no proper command was found
int main()
{
    int logged = 0; // 0 = not logged in, 1 = logged in
    int open_server = 1; // 1 = server open, 0 = server close
    while(open_server)
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
        char buff[1024];
        printf("Waiting for client to write.\n");
        int fd=open("FIFO_CLIENT", O_RDONLY);
        if(fd==-1)
        {
            perror("Eroare la deschidere FIFO\n");
            exit(2);
        }

            if(read(fd, buff,sizeof(buff))==-1)
            {
                perror("Eroare la citire din FIFO\n");
                exit(3);
            }
            printf("Received: %s\n",buff);
        
        char reply[1024];

        strcpy(reply, buff);

        int sockp[2], child; 

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
      { 
        perror("Err... socketpair"); 
        exit(1); 
      }

    if ((child = fork()) == -1) perror("Err...fork"); 
    else 
      if (child)   //parinte 
        {  
            close(sockp[0]);
            if (write(sockp[1], reply, sizeof(reply)) < 0) perror("[parinte]Err...write"); 
            if (read(sockp[1], reply, sizeof(reply)) < 0) perror("[parinte]Err...read"); 

            if(strstr(reply, "Successfully logged in as ") == reply)
            logged = 1;

            if(strstr(reply, "Successfully logged out.") == reply)
            logged = 0;

            if(strstr(reply, "quit") == reply)
            open_server = 0;

            if(strcmp(reply, buff) == 0 && strstr(reply,"get-proc-info : ") == reply)

            strcpy(reply,"Couldn't find process with such PID.");

            close(sockp[1]);
        } 
        else     //copil
          { 
            char msg[1024];
            char answer[1024];
            answer[0] = '\0';
            close(sockp[1]); 
            if (read(sockp[0], msg, sizeof(msg)) < 0) perror("[copil]Err..read"); 
            //this is where we actually do some computing lmfao
            int cmd_id = NO_CMD;
					if(strstr(msg,"login : ") == msg) {
						cmd_id = LOGIN;
					}
					if(strstr(msg,"get-logged-users") == msg) {
						cmd_id = GET_LOGGED_USERS;
					}
					if(strstr(msg,"get-proc-info : ") == msg) {
						cmd_id = GET_PROC_INFO;
					}
					if(strstr(msg,"logout") == msg) {
						cmd_id = LOGOUT;
					}
					if(strstr(msg,"quit") == msg) {
						cmd_id = QUIT;
					}
                    if(cmd_id == NO_CMD)
                    {
                        strcat(answer, "Invalid command.");
                        strcpy(msg,answer);
                    }
                    else if(cmd_id == LOGIN) 
                    {
                        int i = 8; //LUNGIMEA "LOGIN : "
						int counter = 0;
                        char trial_user[1024];
                        trial_user[0] = '\0';
                        int found = 0;
                        if(logged == 1)
                        found = -1;
						while(msg[i]) {
							trial_user[counter] = msg[i];
							counter = counter + 1;
							i = i + 1;
						}
                        char *line_buf = NULL;
                        size_t line_buf_size = 0;
                        int line_count = 0;
                        ssize_t line_size;
                        FILE *fp = fopen(FILENAME, "r");
                        if (!fp)
                        {
                            fprintf(stderr, "Error opening file '%s'\n", FILENAME);
                            return EXIT_FAILURE;
                        }

                        /* Get the first line of the file. */
                        line_size = getline(&line_buf, &line_buf_size, fp);
                        if(line_buf[strlen(line_buf)-1] = '\n')
                            line_buf[strlen(line_buf)-1] = '\0';
                        
                        /* Show the line details */
                        if(strcmp(line_buf, trial_user) == 0 && found == 0)
                        found = 1;
                        /* Loop through until we are done with the file. */
                        while (line_size >= 0 && found == 0)
                        {
                            /* Increment our line count */
                            line_count = line_count + 1;

                            /* Get the next line */
                            line_size = getline(&line_buf, &line_buf_size, fp);
                            if(line_buf[strlen(line_buf)-1] = '\n')
                            line_buf[strlen(line_buf)-1] = '\0';
                            /* Show the line details */
                            if(strcmp(line_buf, trial_user) == 0)
                            found = 1;
                        }

                        /* Close the file now that we are done with it */
                        fclose(fp);

                        if(found == 1) { //daca se logheaza cu success
                            strcat(answer, "Successfully logged in as ");
                            strcat(answer, line_buf);
                            strcat(answer, ".");
                        }
                        if(found == 0)
                            strcat(answer, "Failed to log in.");

                        if(found == -1) 
                            strcat(answer, "Already logged in.");

                        strcpy(msg,answer);
                    }
                    else if(cmd_id == GET_LOGGED_USERS)
                    {
                        if (logged == 0)
                        strcat(answer, "You can't use this command unless you're logged in.");
                        else
                        {
                            struct utmp *p;
                            char time[1024];
                            time[0]='\0';
                            while(p = getutent())
                            {
                                strcat(answer,"Username: ");
                                strcat(answer, p->ut_user);
                                strcat(answer,"\n");
                                strcat(answer,"Hostname for remote login: ");
                                strcat(answer, p->ut_host);
                                strcat(answer,"\n");
                                strcat(answer,"Time entry was made (in seconds): ");
                                sprintf(time, "%d", p->ut_tv.tv_sec);
                                strcat(answer, time);
                                strcat(answer,"\n");
                                strcat(answer,"\n");
                            }
                        }
                        strcpy(msg,answer);
                    }
                    else if(cmd_id == GET_PROC_INFO)
                    {
                        
                        if(logged == 0)
                        strcat(answer, "You can't use this command unless you're logged in.");
                        else
                        {
                            char *line_buf = NULL;
                            size_t line_buf_size = 0;
                            int line_count = 0;
                            ssize_t line_size;
                            char path[256];
                            path[0] = '\0';
                            int i = 16; //LUNGIMEA "get-proc-info : ""
                            int counter = 0;
                            char pid[256];
                            pid[0] = '\0';
                            while(msg[i]) {
                                pid[counter] = msg[i];
                                counter = counter + 1;
                                i = i + 1;
                            }
                            pid[counter] = '\0';
                            strcat(path,"/proc/");
                            strcat(path,pid);
                            strcat(path,"/status");
                            FILE *fp = fopen(path, "r");
                            if (!fp)
                            {
                                fprintf(stderr, "Error opening file '%s'\n", path);//probabil trimit error message-ul pe answer => client
                                return EXIT_FAILURE;
                            }
                            else
                            {
                                line_size = getline(&line_buf, &line_buf_size, fp);
                                if(strstr(line_buf,"Name:") == line_buf)
                                strcat(answer,line_buf);
                            
                            /* Show the line details */
                                

                                while (line_size >= 0)
                                {
                                    /* Increment our line count */
                                    line_count = line_count + 1;

                                    /* Get the next line */
                                    line_size = getline(&line_buf, &line_buf_size, fp);
                                    if(strstr(line_buf,"State:") == line_buf)
                                    strcat(answer,line_buf);
                                    if(strstr(line_buf,"PPid:") == line_buf)
                                    strcat(answer,line_buf);
                                    if(strstr(line_buf,"Uid:") == line_buf)
                                    strcat(answer,line_buf);
                                    if(strstr(line_buf,"VmSize:") == line_buf)
                                    strcat(answer,line_buf);

                                }
                            }
                        }
                        strcpy(msg,answer);
                    }   
                    else if(cmd_id == LOGOUT)
                    {
                        if(logged == 1)
                        strcat(answer, "Successfully logged out.");
                        else strcat(answer, "You weren't logged in to begin with.");
                        strcpy(msg,answer);
                    }
                    //comment this next if you dont want to close the whole server when closing the client
                    else if(cmd_id == QUIT)
                    {
                        strcat(answer, "quit");
                        strcpy(msg, answer);
                    }

            //this is where we actually do some computing lmfao
            if (write(sockp[0], msg, sizeof(msg)) < 0) perror("[copil]Err...write"); 
            close(sockp[0]);
            return 0;
           }        

        printf("Sending to client.\n");
        int f=open("FIFO_SERVER",O_WRONLY);
        if(f==-1)
        {
            perror("Eroare la deschiderea FIFO\n");
            exit(2);
        }
        printf("Waiting for client to read.\n");
        if(write(f,reply,sizeof(reply))==-1)
        {
            perror("Eroare la scriere in FIFO\n");
            exit(3);
        }
        close(f);
        close(fd);
    }
}