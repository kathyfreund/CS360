
int read_file(char *path)
{
                int nbytes = atoi(third), actual = 0;
                int fd = 0;

                char buf[nbytes + 1];

                MINODE *mip;
                INODE* ip;

                strcpy(buf, "");

                printf("Reading file...\n");
                //check fd
                if (!strcmp(path, ""))
                {
                                printf("No fd specified!\n");
                                return;
                }
                fd = atoi(path);
		//check byte amount
                if (!strcmp(third, ""))
                {
                                printf("No byte amount specified!\n");
                                return;
                }

                actual = my_read(fd, buf, nbytes);
		//check if my_read successfull
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

//actual function
int my_read(int fd, char *buf, int nbytes)
{
                MINODE *mip;
                OFT *oftp; 
                int count = 0;
                int lbk, blk, startByte, remain, ino;
                int avil;
                int *ip;

                int indirect_blk;
                int indirect_off;

                int buf2[BLKSIZE];

                char *cq, *cp;
                char readbuf[1024];
                char temp[1024];

                oftp = running->fd[fd];
                mip = oftp->inodeptr;

                avil = mip->INODE.i_size - oftp->offset;
                cq = buf;

                //loops while there are mote bytes to read
                while(nbytes && avil)
                {
                                //mailmans 
                                lbk = oftp->offset / BLKSIZE;
                                startByte = oftp->offset % BLKSIZE;

                                //direct
                                if(lbk < 12)
                                {
                                                blk = mip->INODE.i_block[lbk];
                                }
                                else if(lbk >= 12 && lbk < 256 + 12)
                                {
                                                printf("DOING INDIRECT\n");
                                                get_block(mip->dev, mip->INODE.i_block[12], readbuf);

                                                ip = (int *)readbuf + lbk - 12;
                                                blk = *ip;
                                }
                                else
                                {
                                                printf("DOING DOUBLE INDIRECT\n");
                                                get_block(mip->dev, mip->INODE.i_block[13], readbuf);

                                                indirect_blk = (lbk - 256 - 12) / 256;
                                                indirect_off = (lbk - 256 - 12) % 256;

                                                ip = (int *)readbuf + indirect_blk;

                                                get_block(mip->dev, *ip, readbuf);

                                                ip = (int *)readbuf + indirect_off;
                                                blk = *ip;
                                }

                                get_block(mip->dev, blk, readbuf);

                                cp = readbuf + startByte;

                                remain = BLKSIZE - startByte;

                                //byte by byte
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

                                
                }
                printf("\nRead: read %d char from file descriptor %d\n", count, fd);
                printf("Remaining is: %d\n", remain);
                printf("nbytes is: %d\n", nbytes);
                return count;
}
