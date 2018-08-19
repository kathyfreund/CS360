// The echo client client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdarg.h>

#define MAX 256

// Define variables
struct hostent *hp;              
struct sockaddr_in  server_addr; 
const char EOS[] = "EOS";
int server_sock, r, n;
int SERVER_IP, SERVER_PORT; 


// clinet initialization code

int client_init(char *argv[])
{
  printf("======= clinet init ==========\n");

  printf("1 : get server info\n");
  hp = gethostbyname(argv[1]);
  if (hp==0){
     printf("unknown host %s\n", argv[1]);
     exit(1);
  }

  SERVER_IP   = *(long *)hp->h_addr;
  SERVER_PORT = atoi(argv[2]);

  printf("2 : create a TCP socket\n");
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock<0){
     printf("socket call failed\n");
     exit(2);
  }

  printf("3 : fill server_addr with server's IP and PORT#\n");
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = SERVER_IP;
  server_addr.sin_port = htons(SERVER_PORT);

  // Connect to server
  printf("4 : connecting to server ....\n");
  r = connect(server_sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
  if (r < 0){
     printf("connect failed\n");
     exit(1);
  }

  printf("5 : connected OK to \007\n"); 
  printf("---------------------------------------------------------\n");
  printf("hostname=%s  IP=%s  PORT=%d\n", 
          hp->h_name, inet_ntoa(SERVER_IP), SERVER_PORT);
  printf("---------------------------------------------------------\n");

  printf("========= init done ==========\n");
}

main(int argc, char *argv[ ])
{
  int n;
  char line[MAX], ans[MAX];

  if (argc < 3){
     printf("Usage : client ServerName SeverPort\n");
     exit(1);
  }

  client_init(argv);

  // sock <---> server
  printf("********  processing loop  *********\n");
  while (1){
    printf("input a line : ");
    bzero(line, MAX);                // zero out line[ ]
    fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

    line[strlen(line)-1] = 0;        // kill \n at end
    char temp[MAX];
    strcpy(temp,line);
    if (line[0]==0)                  // exit if NULL line
       exit(0);

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

    if(!strcmp(commands[0],"get"))
    {
      printf("Writing line to server!\n");
      n = write(server_sock, temp, MAX);
      n = read(server_sock, ans, MAX);
      printf("Recieved size from server!\n");
      int size = atoi(ans);
      int status = mkdir("/home/saif/client", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      int ret = chdir("/home/saif/client");
      printf("Created and changed to client directory for testing purposes!\n");
      int fd = open("test.txt", O_WRONLY | O_CREAT, 0644);
      printf("Created test.txt!\n");
      int count = 0;
      char buf[MAX];
      write(server_sock, "Tells server to run!", MAX);
      while(count < size)
      {
        n=read(server_sock, buf, MAX);
        write(fd, buf, n);
        count += n;
      }
      close(fd);
      printf("Writing complete, all done!\n");
    }
    if(!strcmp(commands[0],"put"))
    {
      n = write(server_sock, temp, MAX);
      n = read(server_sock, ans, MAX);
      char cwd[256];
      getcwd(cwd, 128);
      strcat(cwd,"/");
      char filename[128];
      strcpy(filename,commands[1]);
      strcat(cwd,commands[1]);
      
      struct stat fileStat;
      if(stat(cwd,&fileStat) < 0){
         write(server_sock, "error with stat", MAX);
      }    
      else if(S_ISREG(fileStat.st_mode))
       {
        char size[128];
        sprintf(size, "%d",fileStat.st_size);
        write(server_sock, size, MAX);
        n = read(server_sock, line, MAX);
         // this is where i start writing the data

        int fd = open(filename,O_RDONLY);
        perror("open");
         printf("file id: %d \n",fd);
        char buf[MAX];
        while(n=read(fd, buf, MAX))
        {
          write(server_sock, buf, n);
        }
        close(fd);
       }
       else
       {
        write(server_sock, "BAD", MAX);
       }
    }

    else if (!strcmp(commands[0],"quit"))
    {
      n = write(server_sock, temp, MAX);
      printf("Exiting!\n");
      exit(0);
    }
    //n = write(server_sock, line, MAX);
    //printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

    // Read a line from sock and show it
    //n = read(server_sock, ans, MAX);
    //printf("client: read  n=%d bytes; output = \n%s \n",n, ans);
  }
}


