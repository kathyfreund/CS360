// This is the echo SERVER server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>

#define  MAX 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

int  mysock, client_sock;              // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables


// Server initialization code:

int server_init(char *name)
{
   printf("==================== server init ======================\n");   
   // get DOT name and IP address of this host

   printf("1 : get and show server host info\n");
   hp = gethostbyname(name);
   if (hp == 0){
      printf("unknown host\n");
      exit(1);
   }
   printf("    hostname=%s  IP=%s\n",
               hp->h_name,  inet_ntoa(*(long *)hp->h_addr));
  
   //  create a TCP socket by socket() syscall
   printf("2 : create a socket\n");
   mysock = socket(AF_INET, SOCK_STREAM, 0);
   if (mysock < 0){
      printf("socket call failed\n");
      exit(2);
   }

   printf("3 : fill server_addr with host IP and PORT# info\n");
   // initialize the server_addr structure
   server_addr.sin_family = AF_INET;                  // for TCP/IP
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address  
   server_addr.sin_port = 0;   // let kernel assign port

   printf("4 : bind socket to host info\n");
   // bind syscall: bind the socket to server_addr info
   r = bind(mysock,(struct sockaddr *)&server_addr, sizeof(server_addr));
   if (r < 0){
       printf("bind failed\n");
       exit(3);
   }

   printf("5 : find out Kernel assigned PORT# and show it\n");
   // find out socket port number (assigned by kernel)
   length = sizeof(name_addr);
   r = getsockname(mysock, (struct sockaddr *)&name_addr, &length);
   if (r < 0){
      printf("get socketname error\n");
      exit(4);
   }

   // show port number
   serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
   printf("    Port=%d\n", serverPort);

   // listen at port with a max. queue of 5 (waiting clients) 
   printf("5 : server is listening ....\n");
   listen(mysock, 5);
   printf("===================== init done =======================\n");
}


main(int argc, char *argv[])
{
   char *hostname;
   char line[MAX];

   if (argc < 2)
      hostname = "localhost";
   else
      hostname = argv[1];
 
   server_init(hostname); 

   // Try to accept a client request
   while(1){
     printf("server: accepting new connection ....\n"); 

     // Try to accept a client connection as descriptor newsock
     length = sizeof(client_addr);
     client_sock = accept(mysock, (struct sockaddr *)&client_addr, &length);
     if (client_sock < 0){
        printf("server: accept error\n");
        exit(1);
     }
     printf("server: accepted a client connection from\n");
     printf("-----------------------------------------------\n");
     printf("        IP=%s  port=%d\n", inet_ntoa(client_addr.sin_addr.s_addr),
                                        ntohs(client_addr.sin_port));
     printf("-----------------------------------------------\n");

     while(1){
       n = read(client_sock, line, MAX);
       if (n==0){
           printf("server: client died, server loops\n");
           close(client_sock);
           break;
        }
      printf("line = <%s>\n", line);
      char *commands[MAX];
      int ncommands = 0;
      char *token = strtok(line, " ");
      while (token)
      {
        commands[ncommands++] = token;
        token = strtok(NULL, " ");
      }

    //for (int i = 0; i < ncommands; i++)
    //{
    //  printf("commands[%d] = %s\n", i, commands[i]);
    //}
      // show the line string
    if(!strcmp(commands[0],"get"))
    {
      char cwd[256];
      getcwd(cwd, 128);
      strcat(cwd,"/");
      char filename[128];
      strcpy(filename,commands[1]);
      strcat(cwd,commands[1]);
      
      struct stat fileStat;
      if(stat(cwd,&fileStat) < 0){
         write(client_sock, "error with stat", MAX);
      }    
      else if(S_ISREG(fileStat.st_mode))
       {
        char size[128];
        sprintf(size, "%d",fileStat.st_size);
        write(client_sock, size, MAX);
        n = read(client_sock, line, MAX);
        int fd = open(filename,O_RDONLY);
        perror("open");
         printf("file id: %d \n",fd);
        char buf[MAX];
        while(n=read(fd, buf, MAX))
        {
          write(client_sock, buf, n);
        }
        close(fd);
       }
       else
       {
        write(client_sock, "BAD", MAX);
       }
    }
    else if(!strcmp(commands[0],"put"))
    {
      printf("Writing line to client!\n");
      char temp[MAX], ans[MAX];
      n = write(client_sock, temp, MAX);
      n = read(client_sock, ans, MAX);
      printf("Recieved size from client!\n");
      int size = atoi(ans);
      int status = mkdir("/home/Desktop/Server", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      int ret = chdir("/home/Desktop/Server");
      printf("Created and changed to server directory for testing purposes!\n");
      int fd = open("test.txt", O_WRONLY | O_CREAT, 0644);
      printf("Created test.txt!\n");
      int count = 0;
      char buf[MAX];
      write(client_sock, "Tells client to run!", MAX);
      while(count < size)
      {
        n=read(client_sock, buf, MAX);
        write(fd, buf, n);
        count += n;
      }
      close(fd);
      printf("Writing complete, all done!\n");
    }

    else if (!strcmp(commands[0],"quit"))
    {
      printf("Exiting!\n");
      exit(0);
    }

      //printf("server: read  n=%d bytes; line=[%s]\n", n, line);
      //strcat(line, " ECHO");


      // send the echo line to client 
      //n = write(client_sock, line, MAX);

    }
  }
}




