#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

//new includes lab3:
#include <sys/select.h>
#include <fcntl.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <bits/sigset.h>


#define MAX_CLIENTS 10 

#define WRITE_FD 1
#define READ_FD 0

/**
  * RELAY SERVER: - relays chat messages 
  * @param mrfd - monitor read file descriptor
  * @param mwfd - monitor write file descriptor
  * @param portno - TCP/IP port number
  */

int RS(int read_FD, int write_FD, int PRT_NUM){

    int socket_dskrpt, client_sock, c, read_size;
    struct sockaddr_in server, client;
    char client_message[1024], relay_message[1024];

    memset(&server, 0, sizeof(struct sockaddr_in));
    memset(&client, 0, sizeof(struct sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 4567 );







    //MAKE SOCKET-->:

    socket_dskrpt = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_dskrpt  == -1){
           printf("Unable to create socket... ");
    }
    printf("Socket successfully created");

    //PREPARE SOCKADDR:
    
    int val =1;
    setsockopt(socket_dskrpt, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));


    //NOW WE BIND:
    
    if(bind(socket_dskrpt, (struct sockaddr *)&server, sizeof(server)) < 0){
           perror("Bind has failed...Error..");
    }

    else
        printf("Bind Complete.. ");
    
    //LISTEN
    listen(socket_dskrpt, 20);

    //ACCEPT CONNECTION FROM SIGNAL:
    printf("Waiting to accept incoming connections....\n");
    c = sizeof(struct sockaddr_in);

    //ACCEPT CONNECTION FROM INCOMING CLIENT:
    client_sock = accept(socket_dskrpt, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0){
           perror("Connection Acceptance Failure...");
           return 1; 
    }
    else{
           printf("Connection Acceptance Accepted\n");
    
    
    
           //RECIEVE MESSAGE FROM CLIENT: 
           read_size  = read(client_sock, client_message, sizeof(client_message));
    while(1){      
          // printf("client:  %s\n", client_message);

           //WRITE MESSAGE TO THE CONSOL:
           if(read_size == 0)
              printf("Client has been Discconected...\n");
          
           else if(read_size ==-1)
              perror("read failure");
          
           else
              write(write_FD, client_message, read_size);

           //RECIEVE MESSAGE FROM THE SERVER:
           read_size = read(read_FD, relay_message, sizeof(relay_message));
          // printf("server %s\n", relay_message);

           //write message to client
           if(read_size ==0)
              printf("client has been dissconnected\n");
           else if(read_size == -1)
              perror("read failure");
           else
              write(client_sock, relay_message, read_size);
              read_size = read(client_sock, client_message, sizeof(client_message));
            }
         }

    //CLOSE DEM THANGS...
      close(write_FD);
      close(read_FD);
      close(client_sock);
      close(socket_dskrpt);
    }
   /**
    * nonblock - a function that makes a file descriptir non-blocking
    * @param fd file descriptor 
    */
  void nonblock(int fd) {
    int flags;

    if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
      perror("fcntl (get):");
      exit(1);
    }
    if (fcntl(fd, F_SETFL, flags | FNDELAY) == -1) {
      perror("fcntl (set):");
      exit(1);
    }
  }
   
   
   /**
    * Monitor - provides a local chat window
    * @param srfd - server read file descriptor
    * @param swfd - server write file descriptor
    **/
        
    void monitor(int read_FD, int write_FD){
      char buf[1024];
      int rbytes;
      int maxfd; 
      //vars for select() sets:
      fd_set master; //master fd list
      fd_set read_fds; //temp fd listt for select()/

      
      
      FD_ZERO(&read_fds);
      if(STDIN_FILENO > read_FD)
          maxfd = STDIN_FILENO;
      else if(read_FD > STDIN_FILENO)
          maxfd = read_FD;
          
      while(1) {
          FD_SET(read_FD, &read_fds);
          FD_SET(STDIN_FILENO, &read_fds);
          select(maxfd, read_fds, NULL, NULL, NULL); 
        
          rbytes = read(read_FD, buf, sizeof(buf));
          if(rbytes == 0){
            printf("EOF");
            break;
          }
          else if(rbytes == -1){
            perror("read failure.");
            exit(0);
          }
          else{
          write(STDOUT_FILENO,">: ", 4);
          write(STDOUT_FILENO, buf, rbytes);
           //printf("STDOUT executed");
          rbytes = read(STDIN_FILENO, buf, sizeof(buf));
          if(rbytes == 0){
            printf("EOF");
            break;
          }
          else if(rbytes == -1){
            perror("read failure..");
            exit(0);
          }
          else
          write(write_FD, buf, rbytes);
        } }
        close(write_FD);
        close(read_FD);
      }

///MAIN::

 int main(int argc, char *argv[]){
    int mon_relay[2];
    int relay_mon[2];

    pid_t pid;
    char *buf[1024];

    if (pipe(mon_relay) == -1) {
    perror("pipe 1");
    exit(1);
    }

    if (pipe(relay_mon) ==-1) {
    perror("pipe 2");
    exit(1);
    }

    pid = fork();
    if (pid == -1) {
    perror("fork");
    exit(1);
    }

    else if(pid ==0) {
       // printf("child pid: %d\n", getpid());
        close(relay_mon[WRITE_FD]);
        close(mon_relay[READ_FD]);
        monitor(relay_mon[READ_FD], mon_relay[WRITE_FD]);
        printf("(CTRL + 4 to QUIT) >");
    }

    else {
       // printf("parent pid: %d\n", getpid());
        close(mon_relay[WRITE_FD]);
        close(relay_mon[READ_FD]);
        monitor(mon_relay[READ_FD], relay_mon[WRITE_FD]);
        RS(mon_relay[READ_FD], relay_mon[WRITE_FD], 4567);
        printf("(CTRL + 4 to QUIT) >");
         }
    
 exit(0);
 }






       
