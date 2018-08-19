//takes in fd and position by converting the string inputs into ints
//use it for adding contents in the middle of a file for example
int my_lseek(char *path)
{
                int fd; //fd is file descriptor
                int position;
                OFT *oft;

                //check for the fd
                if(!path)
                {
                                printf("ERROR: Missing file name!\n");
                                return;
                }
                //check for the wanted position
                if(!third)
                {
                                printf("ERROR: Missing position!\n");
                                return;
                }

                //convert fd and postion to integers
                fd = atoi(path);
                position = atoi(third);

                //ensure the fd exists and is not busy
                if(running->fd[fd] && running->fd[fd]->refCount > 0)
                {
                                oft = running->fd[fd];

                                //Ensure we are not putting the position beyond the size of the fd
                                if(position <= oft->inodeptr->INODE.i_size)
                                {
                                                //Change the offset variable of the fd to the given position
                                                oft->offset = position;
                                                running->fd[fd] = oft;
                                }
                                else
                                {
                                                //will get this if fd is set to poition that is greater than number of
                                                //bits in the file
                                                printf("ERROR: Position is out of bounds!\n");
                                                return -1;
                                }
                                return position;
                }
                else
                {
                                printf("Unknown ERROR occurred!\n");
                                return -1;
                }
}
