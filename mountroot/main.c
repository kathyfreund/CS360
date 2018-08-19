#include <stdio.h>
#include <stdlib.h>

#include "type.h"
#include "util.c"  
#include "mkdir.c" 
#include "rmdir.c" 
#include "create.c" 
#include "mount_root.c" 
#include "ls_cd_pwd.c"
#include "util.c"

int main()
{
                char *commands[23] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat"};
                char device_name[64], input[128] = "";
                char command[64], pathname[64] = "";

                int (*functions[23]) (char path[]);
                int i;

                functions[0] = make_dir; //works
                functions[1] = remove_dir; //works
                functions[2] = ls; //works
                functions[3] = cd; //works

                functions[4] = my_pwd; //works
                functions[5] = creat_file; //works


                printf("Enter device name: "); //prompt user for device
                scanf("%s", device_name); //scan in device name

                //mount device
                init();
                mount_root(device_name);

                getchar();
                while(1) //run program forever until quit
                { 
                                fgets(input, 128, stdin);

                                //remove \r at end of input
                                input[strlen(input) - 1] = 0;
                                if(input[0] == 0)
                                                continue;

                                //split up the input into the variables
                                sscanf(input, "%s %s %s", command, pathname, third);

                                for(i = 0; i < 23; i++)
                                {
                                                //commands is list of commands, command is the inputted command
                                                if(!strcmp(commands[i], command))
                                                {
                                                                (*functions[i])(pathname); //pathname is just file name
                                                                break; //break once command is found and executed
                                                }
                                }

                                if(i == 23) //if no command has been found, then it doesn't exist
                                                printf("Error: Invalid command!\n");

                                //reset variables
                                strcpy(pathname, "");
                                strcpy(command, "");
                                strcpy(input, "");
                                strcpy(third, "");
                }
                return 0;
}
