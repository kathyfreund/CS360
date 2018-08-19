//is directory empty? 
int isEmptyDir(MINODE *mip)
{
	char buf[1024];
        INODE *ip = &mip->INODE; 
        char *cp;
        char name[64];
        DIR *dp; 

        if(ip->i_links_count > 2) // has files
        {
        	printf("Dir has files!\n");
                return 1;
        }
        else if(ip->i_links_count == 2)
        {
        	//printf("Dir could stil have files, checking...\n"); //maybe has files?
                if(ip->i_block[1])
                {
                	get_block(dev, ip->i_block[1], buf);
			cp = buf; //pointer
                        dp = (DIR*)buf; //redfine dir pointer

                       	while(cp < buf + 1024)
                        {
                        	strncpy(name, dp->name, dp->name_len); //copy names
                        	name[dp->name_len] = 0; //set names to 0

                                if(strcmp(name, ".") != 0 && strcmp(name, "..") != 0)
                                {
                                	printf("--- Directory not empty.\n");
                                        return 1;
                                }
                        }
                }
	}
        else
	{
		printf("--- Directory empty. \n");
		return 0;
	}
}



void rm_child(MINODE *parent, char *name)
{
	int i;
	INODE *p_ip = &parent->INODE;
	DIR *dp; 
	DIR *prev_dp; 
	DIR *last_dp;
	char buf[1024];
	char *cp;
 	char temp[64];
	char *last_cp;
	int start, end;

	printf("--- Removing %s\n", name);
	printf("--- Parent size is %d\n", p_ip->i_size);

        //find child
	for(i = 0; i < 12; i++)
	{
		if(p_ip->i_block[i] == 0)
		{
			return;
		}
		get_block(dev, p_ip->i_block[i], buf);
		cp = buf;
		dp = (DIR*)buf;

		printf("--- dp at %s.\n", dp->name);

		while(cp < buf + 1024)
		{
			strncpy(temp, dp->name, dp->name_len);
                        temp[dp->name_len] = 0;

			printf("--- dp at %s.\n", temp);

			if(!strcmp(temp, name))
                        {
                        	printf("Child found.\n");
                        	if(cp == buf && cp + dp->rec_len == buf + 1024)
                                {
                                	free(buf);
                                        bdealloc(dev, ip->i_block[i]); //deallocate

                                        p_ip->i_size -= 1024;

                                        while(p_ip->i_block[i + 1] && i + 1 < 12)
                                        {
                                        	i++;
                                                get_block(dev, p_ip->i_block[i], buf);
                                                put_block(dev, p_ip->i_block[i - 1], buf);
                                        }
                                }
			}
			else if(cp + dp->rec_len == buf + 1024)
			{
				printf("--- Removing last entry.\n");
				prev_dp->rec_len += dp->rec_len;
				put_block(dev, p_ip->i_block[i], buf);
                        }
			else
			{
				printf("--- Before dp is %s.\n", dp->name);

				last_dp = (DIR*)buf;
                                last_cp = buf;
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

                                memmove(cp, start, end - start);

                                put_block(dev, p_ip->i_block[i], buf);

			}
			parent->dirty = 1;
                        iput(parent);
                        return;
		}
		prev_dp = dp;
                cp += dp->rec_len;
                dp = (DIR*)cp;
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

                if(!path)
                {
                                printf("[!] ERROR - No directory name.\n");
                                return;
                }

                strcpy(temp, path);
                strcpy(child, basename(temp));

                ino = getino(running->cwd, path);
                printf("%s ino is %d\n", path, ino);
                mip = iget(dev, ino);

                if(!mip)
                {
                                printf("[!] ERROR - mip does not exist.\n");
                                return;
                }

                //check if dir
                if(!S_ISDIR(mip->INODE.i_mode))
                {
                                printf("[!] ERROR - %s not a directory.\n", path);
                                return;
                }

                //check if empty
                if(isEmptyDir(mip))
                {
                                printf("[!] ERROR - Directory not empty!\n");
                                return 0;
                }

                //we can start removing from this point

                ip = &mip->INODE;

                findino(mip, &ino, &parent_ino);
                printf("INO = %d\nParent INO = %d\n", ino, parent_ino);
                p_mip = iget(dev, parent_ino);
                p_ip = &p_mip->INODE;

                //deallocate blocks
                for(i = 0; i < 15 && ip->i_block[i] != 0; i++)
                {
                                bdealloc(dev, ip->i_block[i]);
                }

                idealloc(dev, ino);

                rm_child(p_mip, child);

                p_ip->i_links_count--;
                p_ip->i_atime = time(0L);
                p_ip->i_mtime = time(0L);
                p_mip->dirty = 1;

                iput(p_mip);
                mip->dirty = 1;
                iput(mip);

                return;
}
