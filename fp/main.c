#include <stdio.h>
#include <stdlib.h>

#include "type.h" 
#include "util.c" //get_block, put_block, iget, tokenize, getino, iput
#include "alloc_dealloc.c" //allocation and deallocation of memory for inodes and blocks
#include "pwd.c" //my_pwd function
#include "mkdir.c" //my_mkdir function, includes helper functions
#include "rmdir.c" //my_rmdir function, includes helper functions
#include "creat_file.c" //my_creat function for creation of files
//#include "rm_file.c"
#include "chmod_file.c" //change mode function
#include "touch_file.c" //simple file creation
#include "mount_root.c" //mounts the disk, contains ls and cd
#include "link.c" //link files together
#include "open.c" //open file in preperation of read/write
#include "close.c" //close file after reading/writing
#include "pfd.c" //print file descripter of open files
#include "symlink.c" //links to another file sort of like a shortcut
#include "unlink.c" //removes links and deletes files
#include "stat.c" //print stats about file or dir
#include "cp.c" //copy file from location to location
#include "mv.c" //move file from location to location
#include "lseek.c" //changes the file offset for the open fd
#include "read.c" //reads the contents of a file
#include "write.c" //writes to files
#include "cat.c" //adds contents to file


  //Lvl 1 - mkdir, creat, rmdir (with ls,cd,pwd)

  //Lvl 2 - open, close, lseek, read, cat, write, cp, mv

  //Lvl 3 - mount, unmount [extra credit if getino(), iget(), iput()]




int main()
{
		//commands order must match functions order
                char *commands[22] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "link", "unlink", "symlink", "stat", "chmod", "touch", "quit", "open", "close", "pfd", "write", "cat", "read", "cp", "mv", "lseek"};
                char input[128] = "", command[64] = "", pathname[64] = "";

                int (*functions[22]) (char path[]); //store
                int i;
		
		//Part 1
                functions[0] = make_dir; 
                functions[1] = remove_dir;
                functions[2] = ls; 
                functions[3] = cd; 
                functions[4] = my_pwd; 
                functions[5] = creat_file; 
		//functions[6] = rm_file;
		//Part 2
                functions[6] = my_link; 
                functions[7] = my_unlink;
                functions[8] = my_symlink; 
                functions[9] = my_stat; 
                functions[10] = chmod_file; 
                functions[11] = touch_file; 
                functions[12] = quit; 
                functions[13] = open_file; 
                functions[14] = my_close; 
                functions[15] = my_pfd; 
                functions[16] = do_write; 
                functions[17] = my_cat; 
                functions[18] = read_file; 
                functions[19] = cp_file; 
                functions[20] = mv_file; 
                functions[21] = my_lseek; 

                //mount_root.c
                init();
                mount_root("mydisk");

                while(1) 
                {
				printf("input command : [mkdir|rmdir|ls|cd|pwd|creat|link|unlink|symlink|stat|chmod|touch|quit|open|close|pfd|write|cat|read|cp|mv|lseek] \n");
                                fgets(input, 128, stdin);

                                input[strlen(input) - 1] = 0; //remove /r
                                if(input[0] == 0)
				{
                                                continue; //nothing
				}

                                sscanf(input, "%s %s %s", command, pathname, third);

                                for(i = 0; i < 23; i++)
                                {
                                                if(strcmp(commands[i], command) == 0)
                                                {
								//should all take in one parameter
								printf("%d\n", i);
                                                                (*functions[i])(pathname);
                                                                break;
                                                }
                                }

                                if(i == 23)
				{ 
                                                printf("[!] Error - Unknown argument.\n");
				}		
                                strcpy(pathname, "");
                                strcpy(command, "");
                                strcpy(input, "");
                                strcpy(third, "");
                }
                return 0;
}


