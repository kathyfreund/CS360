int isEmptyDir(MINODE *mip)
{
                char buf[1024];
                INODE *ip = &mip->INODE; //make it an INODE pointer so we can use the inode pointer members
                char *cp;
                char name[64];
                DIR *dp; //dir pointer

                //link count greater than 2, it has files
                if(ip->i_links_count > 2) //found in inode block
                {
                                printf("Dir has files!\n");
                                return 1;
                }
                else if(ip->i_links_count == 2)
                {
                                printf("Dir could stil have files, checking...\n");
                                //link count of 2 could still have files, check data blocks
                                if(ip->i_block[1])
                                {
                                                get_block(dev, ip->i_block[1], buf); //get blocks

                                                cp = buf; //pointer
                                                dp = (DIR*)buf; //redfine dir pointer

                                                while(cp < buf + 1024)
                                                {
                                                                strncpy(name, dp->name, dp->name_len); //copy names
                                                                name[dp->name_len] = 0; //set names to 0

                                                                if(strcmp(name, ".") != 0 && strcmp(name, "..") != 0)
                                                                {
                                                                                //not empty!
                                                                                printf("Dir is NOT empty!\n");
                                                                                return 1;
                                                                }
                                                }
                                }
                }
                else
                {
                                printf("Dir is empty\n");
                                return 0;  //is empty, return 0
                }
}

void rm_child(MINODE *parent, char *name)
{
                int i;
                INODE *p_ip = &parent->INODE;
                DIR *dp; //dir pointer
                DIR *prev_dp; //another dir pointer
                DIR *last_dp; //another dir pointer
                char buf[1024];
                char *cp;
                char temp[64];
                char *last_cp;
                int start, end;

                printf("Removing %s...\n", name);
                printf("Parent size is %d...\n", p_ip->i_size);

                //iterate through blocks
                //this finds the child
                for(i = 0; i < 12; i++)
                {
                                if(p_ip->i_block[i] == 0) //if p_ip's i_block = 0, have found, continue
                                                return;

                                get_block(dev, p_ip->i_block[i], buf);
                                cp = buf;
                                dp = (DIR*)buf;

                                printf("dp at %s...\n", dp->name);

                                while(cp < buf + 1024)
                                {
                                                strncpy(temp, dp->name, dp->name_len);
                                                temp[dp->name_len] = 0;

                                                printf("dp at %s...\n", temp);

                                                if(!strcmp(temp, name))
                                                {
                                                                printf("Child has been found...\n");
                                                                if(cp == buf && cp + dp->rec_len == buf + 1024)
                                                                {
                                                                                //it's the first and only entry, need to delete entire block
                                                                                free(buf);
                                                                                bdealloc(dev, ip->i_block[i]); //deallocate blocks

                                                                                p_ip->i_size -= 1024;

                                                                                //shift blocks left
                                                                                while(p_ip->i_block[i + 1] && i + 1 < 12)
                                                                                {
                                                                                                i++;
                                                                                                get_block(dev, p_ip->i_block[i], buf);
                                                                                                put_block(dev, p_ip->i_block[i - 1], buf);
                                                                                }
                                                                }
                                                                else if(cp + dp->rec_len == buf + 1024)
                                                                {
                                                                                //just have to remove the last entry
                                                                                printf("Removing last entry...\n");
                                                                                prev_dp->rec_len += dp->rec_len;
                                                                                put_block(dev, p_ip->i_block[i], buf);
                                                                }
                                                                else
                                                                {
                                                                                //not last entry, this is where we have problems
                                                                                printf("Before dp is %s...\n", dp->name);

                                                                                last_dp = (DIR*)buf;
                                                                                last_cp = buf;

                                                                                //step into last entry
                                                                                while(last_cp + last_dp->rec_len < buf + BLKSIZE)
                                                                                {
                                                                                                printf("Last dp at %s...\n", last_dp->name);
                                                                                                last_cp += last_dp->rec_len;
                                                                                                last_dp = (DIR*)last_cp;
                                                                                }

                                                                                printf("%s and %s\n", dp->name, last_dp->name);

                                                                                last_dp->rec_len += dp->rec_len;

                                                                                start = cp + dp->rec_len;
                                                                                end = buf + 1024;

                                                                                memmove(cp, start, end - start); //built in function, move memory left

                                                                                put_block(dev, p_ip->i_block[i], buf);

                                                                }

                                                                parent->dirty = 1;
                                                                iput(parent);
                                                                return;
                                                } //end of child found

                                                prev_dp = dp;
                                                cp += dp->rec_len;
                                                dp = (DIR*)cp;
                                }
                }

                return;
}

void remove_dir(char *path)
{
                int i;
                int ino, parent_ino;
                MINODE *mip;
                MINODE *p_mip;
                INODE *ip;
                INODE *p_ip;
                char temp[64], child[64];

                //checks
                if(!path)
                {
                                printf("ERROR: No directory name given!\n");
                                return;
                }

                strcpy(temp, path);
                strcpy(child, basename(temp));

                //get the ino of the child and ensure it exists
                ino = getino(running->cwd, path);
                printf("%s ino is %d\n", path, ino);
                mip = iget(dev, ino);

                if(!mip)
                {
                                printf("ERROR: MIP does not exist!\n");
                                return;
                }

                //check if dir
                if(!S_ISDIR(mip->INODE.i_mode))
                {
                                printf("ERROR: %s is not a directory!\n", path);
                                return;
                }

                //check if empty
                if(isEmptyDir(mip))
                {
                                printf("ERROR: Directory is not empty!\n");
                                return 0;
                }

                printf("Starting to remove...\n");

                ip = &mip->INODE;

                //get parent ino
                findino(mip, &ino, &parent_ino);
                printf("INO is %d\nParent INO is%d\n", ino, parent_ino);
                p_mip = iget(dev, parent_ino);
                p_ip = &p_mip->INODE;

                //go through blocks deallocating them
                //deallocate blocks
                for(i = 0; i < 15 && ip->i_block[i] != 0; i++)
                {
                                bdealloc(dev, ip->i_block[i]); //deallocate blocks
                }

                //deallocate inode
                idealloc(dev, ino);

                //remove entry from parent dir
                rm_child(p_mip, child);

                //update parent
                p_ip->i_links_count--;
                p_ip->i_atime = time(0L);
                p_ip->i_mtime = time(0L);
                p_mip->dirty = 1;

                //write parent changes to disk
                iput(p_mip);
                //write changes to deleted directory to disk
                mip->dirty = 1;
                iput(mip);

                return;
}
