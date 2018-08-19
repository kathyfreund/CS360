//read: path is source, third is dest
void mv_file(char *path)
{
                int i = 0, j, ino = 0;
                int fdDest = 0; //fd destination
                int fdSrc = 0; //fd source
                char buf[1024];
                char src[1024]; //source
                char dest[1024]; //destination

                MINODE *mip; //minode pointer
                INODE* ip; //inode pointer

                printf("Moving file!\n");

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

                //verify that src exists and get its ino
                //returns out if the src does not exist
                ino = getino(running->cwd, path);
                if(ino == 0)
                {
                                printf("Source does not exist!\n");
                                return;
                }
                //call our link function which will link src to dest
                //ensure third is set as destination
                strcpy(third, dest);
                my_link(src);

                //call unlink on source to unlink it to its old parent
                //this removes source from its parents directory
                //this also reduces its link count by one
                my_unlink(src);

                printf("File has been moved successfully!\n");
                return;
}
