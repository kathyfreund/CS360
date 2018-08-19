int open_file(char path[124])
{
                int i, ino, fd = 0, perm = 0, mask = 0, mode = -1, offset = 0;
                MINODE *mip;
                INODE* ip; 
                OFT* of = NULL;

                char buf[1024];

                printf("Opening file...\n");
                if (!strcmp(third, ""))
                {
                                printf("No open mode specified!\n");
                                return;
                }
                if (!strcmp(third, "0")) //read
		{
                                mode = 0;
		}                
		else if (!strcmp(third, "1")) //write
		{
                                mode = 1;
		}
                else if (!strcmp(third, "2")) //RW
		{
                                mode = 2;
		}
                else if (!strcmp(third, "3")) //append
		{
                                mode = 3;
		}
                else
                {
                                printf("Invalid mode!\n");
                                return;
                }



                if (path[0] == '/')
		{
                                ino = getino(root, path);
		}
                else
		{
                                ino = getino(running->cwd, path);
		}

                //check that file exists
                printf("%d\n", ino);
                if (ino == 0)
                {
                                printf("ERROR: No such file!\n");
                                return;
                }

                //iget on the ino and ensure it is a file
                mip = iget(dev, ino);
                ip = &mip->INODE;

                if (!S_ISREG(ip->i_mode))
                {
                                printf("ERROR: Not a file!\n");
                                iput(mip);
                                return;
                }

                iput(mip);
		
		if(running->uid != mip->INODE.i_uid && running->gid != mip->INODE.i_gid)
		{
			printf("[!] ERROR - Permission Denied.\n");
			iput(mip);
			return 0;
		}       

                //check for open slot
                for (fd = 0; fd < NFD; fd++)
                {
                                if (running->fd[fd] == NULL) //found spot
				{
                                                break;
				}

                                if (fd == NFD -1)
                                {
                                                printf("Out of processes!\n"); //no spots left
                                                iput(mip);
                                                return;
                                }
                }

                //loop OpenFileTable
                for (i = 0; i < NOFT; i++)
                {
                                of = &OpenFileTable[i];

                                //multiple reads is OK
                                if(mode != 0 && of->refCount > 0 && of->inodeptr == mip && of->mode != 0)
                                {
                                                printf("[!] ERROR - File is being used.\n");
                                                iput(mip);
                                                return;
                                }

                                //found -> set variables
                                if (of->refCount == 0)
                                {
                                                of->mode = mode;
                                                of->offset = offset;
                                                of->inodeptr = mip;
                                                of->refCount++;

                                                running->fd[fd] = of;

                                                break;
                                }

                                //no more room
                                if (i == NOFT - 1)
                                {
                                                printf("No more room!\n");
                                                iput(mip);
                                                return;
                                }
                }

                //set the time and dirty
                if(mode == 0)
		{
       			ip->i_atime = time(0L);
    		}
		else
		{ 
       			ip->i_atime = time(0L);
       			ip->i_mtime = time(0L);
    		}
		mip->dirty = 1;
                return fd;
}
