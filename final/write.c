
int my_write(int fd, char buf[], int nbytes)
{
                int i, j;
                int *ip;
                int counter = 0;
                int remain;
                int lbk, startByte;
                int blk;
                int indirect_blk, indirect_off;

                OFT *oftp;
                MINODE *mip;
                char write_buf[1024];
                char *cp, *cq = buf;

                if(fd < 0 || fd >= NFD)
                {
                                printf("[!] ERROR - Invalid file descriptor.\n");
                                return;
                }

                //loop OFT
                for(i = 0; i < NOFT; i++)
                {
                                if(OpenFileTable[i].inodeptr == running->fd[fd]->inodeptr) 
                                {
                                                oftp = running->fd[fd]; 
                                                break;
                                }
                }

                if(!oftp || (oftp->mode != 1 && oftp->mode != 2 && oftp->mode != 3)) 
                {
                                printf("[!] ERROR - Mode not set to write.\n");
                                return;
                }

                mip = oftp->inodeptr;

                printf("Writing to file...\n");

                //while there are bytes to be written
                while(nbytes > 0)
                {
                                //mailmans
                                lbk = oftp->offset / BLKSIZE; 
                                startByte = oftp->offset % BLKSIZE;

                                if(lbk < 12) //direct
                                {
                                                if(mip->INODE.i_block[lbk] == 0) //allocate
                                                {
                                                                mip->INODE.i_block[lbk] = balloc(mip->dev);
                                                }
                                                blk = mip->INODE.i_block[lbk]; 
                                }
                                else if(lbk >= 12 && lbk < 256 + 12)
                                {
                                                //indirect
                                                
                                                if(!mip->INODE.i_block[12])
                                                {
                                                                mip->INODE.i_block[12] = balloc(mip->dev); //allocate

                                                                //zero out
                                                                get_block(mip->dev, mip->INODE.i_block[12], write_buf);
                                                                for(i = 0; i < BLKSIZE; i++)
                                                                                write_buf[i] = 0;
                                                                put_block(mip->dev, mip->INODE.i_block[12], write_buf);
                                                }
                                                get_block(mip->dev, mip->INODE.i_block[12], write_buf);

                                                ip = (int*)write_buf + lbk - 12;
                                                blk = *ip;

                                                //allocate
                                                if(blk == 0)
                                                {
                                                                *ip = balloc(mip->dev);
                                                                blk = *ip;
                                                }
                                }
                                else
                                {
                                                //double indirect
                                                if(mip->INODE.i_block[13] == 0)
                                                {
                                                                mip->INODE.i_block[13] = balloc(mip->dev);
                                                                //fill it with 0's
                                                                get_block(mip->dev, mip->INODE.i_block[13], write_buf);
                                                                for(i = 0; i < BLKSIZE; i++)
                                                                                write_buf[i] = 0;
                                                                put_block(mip->dev, mip->INODE.i_block[13], write_buf);
                                                }

                                                get_block(mip->dev, mip->INODE.i_block[13], write_buf);

                                                //mailmans
                                                indirect_blk = (lbk - 256 - 12) / 256;
                                                indirect_off = (lbk - 256 - 12) % 256;

                                                ip = (int *)write_buf + indirect_blk;
                                                blk = *ip;

                                                //allocate
                                                if(!blk)
                                                {
                                                                ip = balloc(mip->dev); 
                                                                blk = *ip;

                                                                //zero out
                                                                get_block(mip->dev, blk, write_buf);
                                                                for(i = 0; i < BLKSIZE; i++)
                                                                                write_buf[i] = 0;
                                                                put_block(mip->dev, blk, write_buf);
                                                }

                                                get_block(mip->dev, blk, write_buf);

                                                ip = (int*)write_buf + indirect_off;
                                                blk = *ip;

                                                if(!blk)
                                                {
                                                                *ip = balloc(mip->dev);
                                                                blk = *ip;
                                                                put_block(mip->dev, blk, write_buf);
                                                }
                                }

                                get_block(mip->dev, blk, write_buf);
                                cp = write_buf + startByte;
                                remain = BLKSIZE - startByte;

                                //byte by byte
                                while(remain > 0)
                                {
                                                *cp++ = *cq++;
                                                nbytes--;
                                                counter++;
                                                remain--;
                                                oftp->offset++;
                                                if(oftp->offset > mip->INODE.i_size)
                                                                mip->INODE.i_size++;
                                                if(nbytes <= 0)
                                                                break;
                                }

                                put_block(mip->dev, blk, write_buf);
                }

                mip->dirty = 1;
                printf("Wrote %d char into file descripter fd = %d\n", counter, fd);
                return nbytes;
}

void do_write(char *path)
{
                int i, fd, nbytes;
                char *buf = (char*)malloc( (strlen(third) + 1) * sizeof(char*) );

                OFT *ofp;

                //checks
                if(!path)
                {
                                printf("[!] ERROR - No file name given.\n");
                                return;
                }

                if(!third)
                {
                                printf("[!] ERROR - No text to write.\n");
                                return;
                }

                fd = atoi(path); 
                for(i = 0; i < NOFT; i++)
                {
                                
                                ofp = &OpenFileTable[i];

                                if(ofp->refCount == 0)
                                {
                                                printf("[!] ERROR - Bad fd.\n");
                                                return;
                                }

                                //check mode
                                if(i == fd)
                                {
                                                if(ofp->mode == 1 || ofp->mode == 2 || ofp->mode == 3)
                                                                break;
                                                else
                                                {
                                                                printf("[!] ERROR - Wrong mode for writing.\n");
                                                                return;
                                                }
                                }
                }

                strcpy(buf, third);
                nbytes = strlen(buf);
                printf("fd is %d\n", fd);
                my_write(fd, buf, nbytes);
                return;
}
