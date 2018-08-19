//read: path is fd, third is number of bytes
int read_file(char *path)
{
                //converts the number of bytes we want to read from a string to an int
                int nbytes = atoi(third), actual = 0;
                int fd = 0;
                //set the size of buf to number of bytes we want to read + 1
                //plus one for the null terminator
                char buf[nbytes + 1];

                MINODE *mip; //minode pointer
                INODE* ip; //inode pointer

                strcpy(buf, "");

                printf("Reading file...\n");
                //checks the fd
                if (!strcmp(path, ""))
                {
                                printf("No fd specified!\n");
                                return;
                }
                //converts the fd from a string to an int
                fd = atoi(path);
                //ensures there is a number of bytes to be read specified
                if (!strcmp(third, ""))
                {
                                printf("No byte amount specified!\n");
                                return;
                }

                //my_read will return the number of bytes that were actually read
                //this is important because the user may ask to print more bytes than there are
                actual = my_read(fd, buf, nbytes);

                if (actual == -1)
                {
                                printf("ERROR: Couldn't read file!\n");
                                strcpy(third, "");
                                return;
                }

                buf[actual] = '\0';
                printf("%s\n", buf);
                return actual;
}

//does the actual reading
//outputs to buf as an ouput parameter and returns the number of bytes that were read
int my_read(int fd, char *buf, int nbytes)
{
                MINODE *mip; //minode pointer
                OFT *oftp; //open field table pointer

                int count = 0;
                int lbk, blk, startByte, remain, ino;
                int avil;
                int *ip;

                int indirect_blk;
                int indirect_off;

                //int buffer for indirection
                int buf2[BLKSIZE];

                char *cq, *cp;
                char readbuf[1024];
                char temp[1024];

                //sets the oftp to the correct fd
                oftp = running->fd[fd];
                mip = oftp->inodeptr;

                //calculates the available amount of bytes to be read
                //this is from the size of the file minus the given offset
                avil = mip->INODE.i_size - oftp->offset;
                cq = buf;

                //loops while there are more bytes available and we want to read more bytes
                while(nbytes && avil)
                {
                                //mailmans to logical bloack number and startbyte in that block from offset
                                lbk = oftp->offset / BLKSIZE;
                                startByte = oftp->offset % BLKSIZE;

                                //If logical block is less than 12 we are only in the direct blocks
                                if(lbk < 12)
                                {
                                                blk = mip->INODE.i_block[lbk];
                                }
                                else if(lbk >= 12 && lbk < 256 + 12)
                                {
                                                printf("DOING INDIRECT\n");
                                                //indirect blocks
                                                //they are located at iblock 12
                                                get_block(mip->dev, mip->INODE.i_block[12], readbuf);

                                                ip = (int *)readbuf + lbk - 12;
                                                blk = *ip;
                                }
                                else
                                {
                                                printf("DOING DOUBLE INDIRECT\n");
                                                //double indirect blocks
                                                //they are located at iblock 13
                                                get_block(mip->dev, mip->INODE.i_block[13], readbuf);

                                                indirect_blk = (lbk - 256 - 12) / 256;
                                                indirect_off = (lbk - 256 - 12) % 256;

                                                ip = (int *)readbuf + indirect_blk;

                                                get_block(mip->dev, *ip, readbuf);

                                                ip = (int *)readbuf + indirect_off;
                                                blk = *ip;
                                }

                                //get the data block into readbuf[BLKSIZE]
                                get_block(mip->dev, blk, readbuf);

                                cp = readbuf + startByte;

                                remain = BLKSIZE - startByte;

                                //old way of doing this (one byte at a time)
                                while(remain > 0)
                                {
                                                *cq++ = *cp++;
                                                oftp->offset++;
                                                count++;
                                                avil--;
                                                nbytes--;
                                                remain--;
                                                if(nbytes <= 0 || avil <= 0)
                                                                break;
                                }

                                //if one data block is not enough, loop back to OUTER while for more...
                }
                printf("\nRead: read %d char from file descriptor %d\n", count, fd);
                printf("Remaining is: %d\n", remain);
                printf("nbytes is: %d\n", nbytes);
                return count;
}
