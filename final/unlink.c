
void my_unlink(char *path)
{
                int ino, i;
                int parent_ino;

                MINODE *mip;
                MINODE *parent_mip;
                INODE *ip;
                INODE *parent_ip;

                char temp1[64], temp2[64];
                char my_dirname[64];
                char my_basename[64];

		strcpy(temp1, path);
                strcpy(temp2, path);
                strcpy(my_dirname, dirname(temp1));
                strcpy(my_basename, basename(temp2));
		
                printf("Dir name is %s\nBasename is %s\n", my_dirname, my_basename);

                //checks
                if(!path) 
                {
                                printf("ERROR: No file name given!\n");
                                return;
                }

                ino = getino(running->cwd, path);

                if(ino == 0)
                {
                                printf("ERROR: File does not exist!\n");
                                return;
                }

                mip = iget(dev, ino);

                if(!mip)
                {
                                printf("ERROR: Missing minode!\n");
                                return;
                }

                //file
                if(S_ISDIR(mip->INODE.i_mode))
                {
                                printf("ERROR: Can't unlink a directory!\n");
                                return;
                }

                printf("Unlinking...\n");
                ip = &mip->INODE;


                
		ip->i_links_count--; 


                printf("Links: %d\n", ip->i_links_count);

                for(i = 0; i < 12 && ip->i_block[i] != 0; i++)
                {
                                bdealloc(dev, ip->i_block[i]); //deallocate blocks
                }


                idealloc(dev, ino);

		/*
                strcpy(temp, path);			//PUT BACK IF IT DOESNT WORK
                strcpy(my_dirname, dirname(temp));
                strcpy(temp, path);
                strcpy(my_basename, basename(temp));
		*/

              

                //removes file from parent
                parent_ino = getino(running->cwd, my_dirname);
                parent_mip = iget(dev, parent_ino);
                parent_ip = &parent_mip->INODE;
	

                //removes the child
                printf("Removing %s from %s\n", my_basename, my_dirname);
                rm_child(parent_mip, my_basename);

                //update
		parent_ip->i_links_count--; //REMOVE IF DOESNT WORK
                parent_ip->i_atime = time(0L);
                parent_ip->i_mtime = time(0L);
                parent_mip->dirty = 1;
                iput(parent_mip);
                mip->dirty = 1;
                iput(mip);

                return;
}
