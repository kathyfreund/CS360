
void cp_file(char *path)
{
                int i = 0, j;
                int fdDest = 0;
                int fdSrc = 0;
                char buf[1024];
                char src[1024]; 
                char dest[1024];

                MINODE *mip; 
                INODE* ip;

                printf("Copying file!\n");

                if (!strcmp(path, ""))
                {
                                printf("No source specified!\n");
                                return;
                }
                strcpy(src, path);
                if (!strcmp(third, ""))
                {
                                printf("No destination specified!\n");
                                return;
                }
                strcpy(dest, third);

                touch_file(dest);

                strcpy(third, "0");

                fdSrc = open_file(src);

                strcpy(third, "1");

                fdDest = open_file(dest);
                my_pfd("");

                //loop until nothing left to read
                while (i = my_read(fdSrc, buf, BLKSIZE))
                {
                                my_write(fdDest, buf, i);
                }

                close_file(fdSrc);
                close_file(fdDest);
                my_pfd("");

                printf("Done copying!\n");
                return;
}
