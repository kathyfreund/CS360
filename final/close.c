
void close_file(int fd)
{
                int i;
                OFT *fp; 

                //Checks if the fd is possible
                if(fd < 0 || fd >= NFD)
                {
                                printf("ERROR: fd out of range\n");
                                return;
                }

                //Checks if the fd is on the OFT
                for(i = 0; i < NOFT; i++)
                {
                                fp = &OpenFileTable[i];

                                if(fp->inodeptr == running->fd[fd]->inodeptr)
				{
                                                break;
				}

                                if(i == NOFT - 1)
                                {
                                                printf("ERROR: File not in OpenFileTable\n");
                                                return;
                                }
                }

                //sets fp, an OFT, to the correct fd from the running proccess
                fp = running->fd[fd];
                running->fd[fd] = NULL;

                //Ensures that the minodes are running as should be
                fp->refCount--;

                if(fp->refCount == 0)
		{
                                iput(fp->inodeptr);
		}

                return;
}

//Calls close file for the given fd
void my_close(char *path)
{
                int fd;

                if(!path)
                {
                                printf("ERROR: no file name given!\n");
                                return;
                }

                fd = atoi(path);
                close_file(fd);
                return;
}
