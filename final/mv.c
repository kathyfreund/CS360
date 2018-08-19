
void mv_file(char *path)
{
                int i = 0, j, ino = 0;
                int fdDest = 0;
                int fdSrc = 0; 
                char buf[1024];
                char src[1024];
                char dest[1024];

                MINODE *mip; 
                INODE* ip; 

                printf("Moving file...\n");

                
                if (!strcmp(path, ""))
                {
                                printf("No source specified.\n");
                                return;
                }
                strcpy(src, path);
                if (!strcmp(third, ""))
                {
                                printf("No destination specified.\n");
                                return;
                }
                strcpy(dest, third);

                ino = getino(running->cwd, path);
                if(ino == 0)
                {
                                printf("Source does not exist.\n");
                                return;
                }
                strcpy(third, dest);
                my_link(src);

                my_unlink(src);

                printf("File has been moved successfully!\n");
                return;
}
