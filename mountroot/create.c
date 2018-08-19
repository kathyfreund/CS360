void creat_file(char path[124])
{
                int i, ino;
                MINODE *pmip; //minode pointer
                INODE *pip; //inode pointer

                char buf[1024];
                char temp1[1024], temp2[1024];
                char parent_name[1024], child_name[1024];

                strcpy(temp1, path); //make copies so original is not destroyed
                strcpy(temp2, path);

                strcpy(parent_name, dirname(temp1)); //get the parent name
                strcpy(child_name, basename(temp2)); //then get the child name

                //get the ino of the parent
                ino = getino(running->cwd, parent_name); //from util.c
                printf("%d\n", ino);
                //set pmip to the parent minode from its ino
                pmip = iget(dev, ino);
                pip = &pmip->INODE;

                if(!pmip) //make sure it exists first
                {
                                printf("ERROR: Parent does not exist!\n");
                                return;
                }

                //check if dir
                if(!S_ISDIR(pip->i_mode))
                {
                                printf("ERROR: Parent is not a directory!\n");
                                return;
                }

                //check if dir already exists
                if(getino(running->cwd, path) != 0)
                {
                                printf("ERROR: %s already exists!\n", path);
                                return;
                }

                my_creat(pmip, child_name); //create the file

                pip->i_atime = time(0L); //size = 0, linkcount = 1, don't increment parent's links count
                pmip->dirty = 1; //set dirty and iput
                iput(pmip);

                return;
}

int my_creat(MINODE *pmip, char *child_name) //this is the actual creat function when called in the program
{
                int i;
                //allocate inode for new file
                int ino = ialloc(dev);

                //create it in memory
                MINODE *mip = iget(dev, ino);
                INODE *ip = &mip->INODE;

                ip->i_mode = 0x81A4; //file type
                ip->i_uid  = running->uid; //owner uid
                ip->i_gid  = running->gid; //group Id
                //set the size to 0 because it is an empty file
                ip->i_size = 0; //size in bytes
                ip->i_links_count = 1; //links to parent directory
                ip->i_atime = time(0L); //set last access to current time
                ip->i_ctime = time(0L);
                ip->i_mtime = time(0L);

                //set to 0 cause we dont need . and .. cause it isnt a dir
                ip->i_blocks = 0;

                //set data blocks to 0
                for(i = 0; i < 15; i++)
                {
                                ip->i_block[i] = 0;
                }

                //set dirty and iput
                mip->dirty = 1;
                iput(mip);

                //enters the files name in the parent directory
                enter_name(pmip, ino, child_name); //enter name defined in mkdir.c

                return ino;
}
