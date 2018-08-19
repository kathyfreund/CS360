
int my_lseek(char *path)
{
                int fd; 
                int position;
                OFT *oft;

                if(!path)
                {
                                printf("[!] ERROR - File name not given.\n");
                                return;
                }
                if(!third)
                {
                                printf("[!] ERROR - Position not given.\n");
                                return;
                }

                fd = atoi(path);
                position = atoi(third);

                if(running->fd[fd] && running->fd[fd]->refCount > 0)
                {
                                oft = running->fd[fd];

                                
                                if(position <= oft->inodeptr->INODE.i_size)
                                {
                                                
                                                oft->offset = position;
                                                running->fd[fd] = oft;
                                }
                                else
                                {
                                                printf("[!] ERROR - Position out of bounds.\n");
                                                return -1;
                                }
                                return position;
                }
                else
                {
                                printf("[!] ERROR - Unknown error.\n");
                                return -1;
                }
}
