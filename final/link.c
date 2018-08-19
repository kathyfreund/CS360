
void my_link(char *path)
{               
                int ino;
                int p_ino;
		char old[64], new[64], temp[64];
                char link_parent[64], link_child[64];

                MINODE *mip;
                MINODE *p_mip; 
                INODE *ip; 
                INODE *p_ip; 

                if(!strcmp(path, ""))
                {
                                printf("ERROR: No old file!\n");
                                return;
                }

                if(!strcmp(third, ""))
                {
                                printf("ERROR: No new file!\n");
                                return;
                }

                strcpy(old, path);
                strcpy(new, third);

                ino = getino(running->cwd, old);
                mip = iget(dev, ino);

                //file exists
                if(!mip)
                {
                                printf("ERROR: %s does not exist!\n", old);
                                return;
                }
                //is a file
                if(S_ISDIR(mip->INODE.i_mode))
                {
                                printf("ERROR: Can't link a directory!\n");
                                return;
                }

                if(!strcmp(new, "/"))
                {
                                strcpy(link_parent, "/");
                }
                else
                {
                                strcpy(temp, new);
                                strcpy(link_parent, dirname(temp));
                }

                strcpy(temp, new);
                strcpy(link_child, basename(temp));

                //get new parent
                p_ino = getino(running->cwd, link_parent);
                p_mip = iget(dev, p_ino);

                //parent exist
                if(!p_mip)
                {
                                printf("[!] ERROR - No parent.\n");
                                return;
                }

                //parent is dir
                if(!S_ISDIR(p_mip->INODE.i_mode))
                {
                                printf("[!] ERROR - Not a directory.\n");
                                return;
                }

                //child already exist?
                if(getino(running->cwd, new))
                {
                                printf("[!] ERROR - %s already exists.\n", new);
                                return;
                }

                printf("Entering name for %s\n", link_child);
                enter_name(p_mip, ino, link_child);

                ip = &mip->INODE;

		//update
                ip->i_links_count++;
                mip->dirty = 1;
                p_ip = &p_mip->INODE;
                p_ip->i_atime = time(0L);
                p_mip->dirty = 1;

                iput(p_mip);
                iput(mip);
                return;
}
