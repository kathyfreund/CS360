#include <stdio.h>
#include <stdlib.h>

#include "type.h" 
#include "util.c" 
#include "alloc_dealloc.c"
#include "pwd.c" 
#include "mkdir.c" 
#include "rmdir.c"
#include "creat_file.c" 
#include "chmod_file.c"
#include "touch_file.c" 
#include "mount_root.c" //includes cd and ls
#include "link.c" 
#include "open.c" 
#include "close.c" 
#include "pfd.c" 
#include "symlink.c" 
#include "unlink.c" //using it at rm also
#include "stat.c"
#include "cp.c"
#include "mv.c" 
#include "lseek.c" 
#include "read.c" 
#include "write.c" 
#include "cat.c"


//Lvl 1 - mkdir, creat, rmdir (with ls,cd,pwd)

//Lvl 2 - open, close, lseek, read, cat, write, cp, mv

//Lvl 3 - mount, unmount [extra credit if getino(), iget(), iput()]

int main()
{
                char *commands[22] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "link", "unlink", "symlink", "stat", "chmod", "touch", "quit", "open", "close", "pfd", "write", "cat", "read", "cp", "mv", "lseek"};
                char device_name[64], input[128] = "";
                char command[64], pathname[64] = "";

                int (*functions[22]) (char path[]);
                int i;

		                

                functions[0] = make_dir; 
                functions[1] = remove_dir; //fixed
                functions[2] = ls; 
                functions[3] = cd; 

                functions[4] = my_pwd; 
                functions[5] = creat_file; 
                functions[6] = my_link; 
                functions[7] = my_unlink; //fixed

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


                init();
                mount_root("diskimage");

                while(1) 
                {
                                printf("input command: ");
                                fgets(input, 128, stdin);

                                input[strlen(input) - 1] = 0;
                                if(input[0] == 0)
                                                continue;

                                sscanf(input, "%s %s %s", command, pathname, third);

                                for(i = 0; i < 23; i++)
                                {
                                                if(!strcmp(commands[i], command))
                                                {
                                                                (*functions[i])(pathname); 
                                                                break; 
                                                }
                                }

                                if(i == 23) 
                                                printf("Error: Invalid command!\n");

                                //reset
                                strcpy(pathname, "");
                                strcpy(command, "");
                                strcpy(input, "");
                                strcpy(third, "");
                }
                return 0;
}
