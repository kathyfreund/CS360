
void creat_file(char path[124])
{
                int i, ino;
                MINODE *pmip; //minode pointer
                INODE *pip; //inode pointer

                char buf[1024];
                char temp1[1024], temp2[1024];
                char parent_name[1024], child_name[1024];

                strcpy(temp1, path);
                strcpy(temp2, path);

                strcpy(parent_name, dirname(temp1)); 
                strcpy(child_name, basename(temp2));
		printf("Dir name is %s\nBasename is %s\n", parent_name, child_name); 

                //get parent ino
                ino = getino(running->cwd, parent_name);
                printf("Ino = %d\n", ino);
                pmip = iget(dev, ino);
                pip = &pmip->INODE;

                if(!pmip)
                {
                                printf("ERROR: Parent does not exist!\n");
                                return;
                }

                if(!S_ISDIR(pip->i_mode))
                {
                                printf("ERROR: Parent is not a directory!\n");
                                return;
                }

                if(getino(running->cwd, path) != 0)
                {
                                printf("ERROR: %s already exists!\n", path);
                                return;
                }

                my_creat(pmip, child_name); //create the file

		pip->i_links_count++; //MAY OR MAY NOT TAKE IT OUT
                pip->i_atime = time(0L);
                pmip->dirty = 1; 

                iput(pmip);

                return;
}

int my_creat(MINODE *pmip, char *child_name) //actual creat function, similar to my_mkdir
{
                int i;
                int ino = ialloc(dev);

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

                ip->i_blocks = 0;
                for(i = 0; i < 15; i++)
                {
                                ip->i_block[i] = 0;
                }

                mip->dirty = 1;
                iput(mip);

                enter_name(pmip, ino, child_name); 
                return ino;
}
