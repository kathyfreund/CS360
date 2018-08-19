
void my_symlink(char *path)
{
                int ino, i;
                int link_ino;
                int parent_ino;
                char temp[64], parent[64], child[64];
                char old_name[64];

                MINODE *mip;
                MINODE *parent_mip;
                MINODE *link_mip;

                INODE *ip;
                INODE *parent_ip;
                INODE *link_ip;

                strcpy(temp, path);
                strcpy(old_name, basename(temp));

                //old file
                ino = getino(running->cwd, path);
                mip = iget(dev, ino);

                if(strlen(path) >= 60) 
                {
                                printf("ERROR: Name is too long!\n");
                                return;
                }

                if(!mip) //path exists
                {
                                printf("ERROR: %s does not exist!\n", path);
                                return;
                }

                strcpy(temp, third);
                strcpy(parent, dirname(temp));
                strcpy(child, basename(third));
		

                printf("Parent is %s,  Child is %s\n", parent, child);

                parent_ino = getino(running->cwd, parent);
                parent_mip = iget(dev, parent_ino);

                if(!parent_mip)
                {
                                printf("[!] ERROR - Cannot get parent MIP.\n");
                                return;
                }

                if(!S_ISDIR(parent_mip->INODE.i_mode)) //parent is dir
                {
                                printf("[!] ERROR - Parent not a directory.\n");
                                return;
                }

                if(getino(running->cwd, child) > 0) //child doesn't already exist
                {
                                printf("[!] ERROR: %s already exists.\n", child);
                                return;
                }

                link_ino = my_creat(parent_mip, child); 
                link_mip = iget(dev, link_ino);
                link_ip = &link_mip->INODE;

               
                link_ip->i_mode = 0120666;
                link_ip->i_size = strlen(old_name);

                link_mip->dirty = 1;
                iput(link_mip);
                iput(mip);
}
