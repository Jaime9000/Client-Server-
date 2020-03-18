#include <stdio.h> //printf
#include <string.h> //strlen
#include <sys/socket.h> //socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netdb.h>
int main(int argc, char *argv[])
{
    int sock;                     //define vars 
    struct sockaddr_in server;
    char message[1024], server_reply[1024];
    int read_FD; 
    struct hostent *he; //pointer to hostent
    int maxfd, ret; ///maxfd is the largest fd, ret is the retur value for the #of fds...for use with select
    fd_set read_fds; //set of read descriptors
    
    ///getopt() -boios:
    extern char *optarg;
    extern int optind;
    int c, err = 0; 
    int hflag=0, pflag=0; 
    char *hname = "default_hname";
    int prtnumbr; 
    static char usage[] = "usage: %s -h hname -p prtmumbr \n";
    
    while ((c = getopt(argc, argv, "h:p:")) != -1)
          switch(c) {
          case 'h':
                  hname = optarg;
                  break;
          case 'p':
                  prtnumbr = optarg[3];
                  break;
          case '?':
                  err = 1;
                  break;
          
          }
      
          


    //Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)       
    {
        printf("Could not create socket");
    }
    write(STDOUT_FILENO, "Socket created \n", 18);    //write to std out
   
    // server.sin_addr.s_addr = inet_addr("127.0.0.1");   //gethostbyname():  //specify server
    //resolove hostname//
    const char hostname[] = "senna";
    if (( he = gethostbyname(hostname) ) == NULL){
        exit(1); /*Error*/
     }
   
   
   
   ///////////////////////////////////////////////////////////////////
   //copy network adress to sockaddr_in structure */
    memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);
    server.sin_family = AF_INET;
    server.sin_port = htons( 4567 );                  //specify port number
    


    //connect to server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        
        perror("connection failed, Error");
        return -1;
    } 
    
    printf("Connection successful\n");
  
   //Continue communication with server:
    FD_ZERO(&read_fds);
    if(STDIN_FILENO > read_FD)
        maxfd = STDIN_FILENO;
    else maxfd = read_FD;       
    
    while(1){
      
      printf("Please type a message : "); fflush(stdout);  //read input from client keyboard             
      FD_SET(sock, &read_fds);
      FD_SET(read_FD, &read_fds); //add fds you need backinto fdset
      FD_SET(STDIN_FILENO, &read_fds);
      ret = select(maxfd+1, &read_fds, NULL, NULL, NULL); //returns the # of fds that are ready
                                                        //FD_ISSET //returns  non-zero if fd is a member of fdset, zero otherwise
       if(FD_ISSET(STDIN_FILENO, &read_fds) != 0){
          ret  =  read(STDIN_FILENO,  message, sizeof(message) );
          if(ret  == -1 ){
             perror("Error, -1");
                return -1;
          }
        
          else if( ret == 0 ){
            printf("EOF...");
            break;
          }
          //send data
          if( write(sock, message, ret) < 0)                       
          {
            printf("Send failure");
            return -1;
            }
       } 
        
        //Get Reply from server:
       if(FD_ISSET(sock, &read_fds) != 0 ){  
          ret = read(sock, server_reply, sizeof(server_reply) );
          if( ret  == -1){ 
            printf("read failure");
            break;
          }
        } 
          else if (ret == 0) {
            printf("EOF...");
            break;
          }
          if( write(STDOUT_FILENO, server_reply, ret) < 0) {
            printf("Send failure");
            return -1;
       
          }
     }   


       
      
    close(sock);
    return 0;
}   

