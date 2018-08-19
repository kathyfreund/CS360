//unlinks files, aka rm and decrements the refcount between all linked files
void my_unlink(char *path)
{
                int ino, i;
                int parent_ino;

                //pointers
                MINODE *mip;
                MINODE *parent_mip;
                INODE *ip;
                INODE *parent_ip;

                char temp[64];
                char my_dirname[64];
                char my_basename[64];

                //checks
                if(!path) //make sure path was given, path is file name
                {
                                printf("ERROR: No file name given!\n");
                                return;
                }

                //gets the ino and checks to ensure it exists
                ino = getino(running->cwd, path);

                if(ino == 0)
                {
                                printf("ERROR: File does not exist!\n");
                                return;
                }

                //get the minode and check to make sure that it is a file
                mip = iget(dev, ino);

                if(!mip)
                {
                                printf("ERROR: Missing minode!\n");
                                return;
                }

                //make sure its a file
                if(S_ISDIR(mip->INODE.i_mode))
                {
                                printf("ERROR: Can't unlink a directory!\n");
                                return;
                }

                printf("Unlinking...\n");
                ip = &mip->INODE;

                //decrement link count
                ip->i_links_count--;
                printf("Links: %d\n", ip->i_links_count);

                //deallocate its blocks
                for(i = 0; i < 12 && ip->i_block[i] != 0; i++)
                {
                                bdealloc(dev, ip->i_block[i]); //deallocate blocks
                }

                //deallocate its inode
                idealloc(dev, ino); //deallocate inodes

                strcpy(temp, path);
                strcpy(my_dirname, dirname(temp));

                strcpy(temp, path);
                strcpy(my_basename, basename(temp));

                printf("Dir name is %s basename is %s\n", my_dirname, my_basename);

                //gets the parent and removes the file from its parent
                parent_ino = getino(running->cwd, my_dirname);
                parent_mip = iget(dev, parent_ino);
                parent_ip = &parent_mip->INODE;

                //removes the child from the parent
                printf("Removing %s from %s\n", my_basename, my_dirname);
                rm_child(parent_mip, my_basename);

                //update the time, set dirty, and iput
                parent_ip->i_atime = time(0L);
                parent_ip->i_mtime = time(0L);
                parent_mip->dirty = 1;
                iput(parent_mip);
                mip->dirty = 1;
                iput(mip);

                return;
}
