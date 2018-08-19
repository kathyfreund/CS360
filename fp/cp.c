//read: path is source, third is dest
void cp_file(char *path)
{
		printf("BREAKPOINT 2\n");
                int i = 0, j;
                int fdDest = 0;
                int fdSrc = 0;
                char buf[1024];
                char src[1024]; //source
                char dest[1024]; //destination

                MINODE *mip; //minode pointer
                INODE* ip; //inode pointer

                printf("Copying file!\n");

                //check to make sure there is a sorce and dest defined
                //strcpy to src and dest so we dont lose them
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

                //call the touch function to see if the destination exists already
                //if the dest doesnt exist, touch calls creat to make it for us
                touch_file(dest);

                //open the src for read
                //make sure open mode is read
                strcpy(third, "0");

                //open with 0 for read
                fdSrc = open_file(src);

                //open the dest for write
                //make sure open mode is write
                strcpy(third, "1");

                //open with 0 for read
                fdDest = open_file(dest);
                my_pfd("");

                //this loop reads from the src until there is nothing left to read and writes it into the dest
                while (i = my_read(fdSrc, buf, BLKSIZE))
                {
                                my_write(fdDest, buf, i);
                }

                //close the open files
                close_file(fdSrc);
                close_file(fdDest);
                my_pfd("");

                printf("Done copying!\n");
                return;
}
