//helper function - following functions follow same logic as mkdir
void creat_file(char path[124])
{
                int i, ino;
                MINODE *pmip; 
                INODE *pip; 

                char buf[1024];
                char temp1[1024], temp2[1024];
                char parent_name[1024], child_name[1024];

                strcpy(temp1, path);
                strcpy(temp2, path);

                strcpy(parent_name, dirname(temp1)); 
                strcpy(child_name, basename(temp2));
		printf("Creating File -> Parent: %s, New Child: %s\n", parent_name, child_name); 
                
                ino = getino(running->cwd, parent_name); //get parent ino
		printf("Parent ino: %d\n", ino);
                printf("%d\n", ino);

                pmip = iget(dev, ino);
                pip = &pmip->INODE;

                if(!pmip) //does parent exist?
                {
                                printf("[!] ERROR - Parent does not exist.\n");
                                return;
                }

                //dir?
                if(!S_ISDIR(pip->i_mode))
                {
                                printf("[!] ERROR - Parent is not a directory.\n");
                                return;
                }

                //check if it already exists
                if(getino(running->cwd, path) != 0)
                {
                                printf("[!] ERROR - %s already exists!\n", path);
                                return;
                }

                my_creat(pmip, child_name); //actually create file

                pip->i_atime = time(0L); 
                pmip->dirty = 1; 
                iput(pmip);

                return;
}

int my_creat(MINODE *pmip, char *child_name) //actually create
{
                int i;
                int ino = ialloc(dev);

                MINODE *mip = iget(dev, ino);
                INODE *ip = &mip->INODE;

                ip->i_mode = 0x81A4; 
                ip->i_uid  = running->uid;
                ip->i_gid  = running->gid;
                ip->i_size = 0; 
                ip->i_links_count = 1; 
                ip->i_atime = time(0L); 
                ip->i_ctime = time(0L);
                ip->i_mtime = time(0L);

                ip->i_blocks = 0;

                for(i = 0; i < 15; i++)
                {
                                ip->i_block[i] = 0;
                }

                mip->dirty = 1;
                iput(mip);

                enter_name(pmip, ino, child_name); //mkdir.c

                return ino;
}
