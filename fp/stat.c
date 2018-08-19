//prints stats
//uses the pointer members
void my_stat(char *path)
{
		//printf("BREAKPOINT 1\n");
                int ino;
                MINODE *mip;
                INODE *ip;
                char *cp;
                char buf[1024];
                char name[64];
                char *my_atime;
                char *my_mtime;
                char *my_ctime;
                DIR *dp;

                ino = getino(running->cwd, path);
                mip = iget(dev, ino);
		//printf("Ino = %d\n", ino);
		if(ino == 0)
		{
			return;
		}

                strcpy(name, basename(path));

                ip = &mip->INODE;
		printf("*********** %s Info ************\n", name);	
                //all of these are found under the inode block pointer (B5)
                //printf("Name: %s\n", name);
                printf("Size: %d\nBlocks: %d\n", ip->i_size, ip->i_blocks);
                if(S_ISDIR(ip->i_mode))
                                printf("Type: Directory\n");
                else
                                printf("Type: File\n");
                printf("Inodes: %d\nLinks:%d\n", ino, ip->i_links_count);

                my_atime = ctime( (time_t*)&ip->i_atime);
                my_mtime = ctime( (time_t*)&ip->i_mtime);
                my_ctime = ctime( (time_t*)&ip->i_ctime);

                printf("Access: %26s", my_atime);
                printf("Modify: %26s", my_mtime);
                printf("Change: %26s", my_ctime);
		printf("********************************\n");
		


                mip->dirty = 1;
                iput(mip);

                return;
}
