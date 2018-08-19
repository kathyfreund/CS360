//write function
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

                //check to make sure the fd can exist
                if(fd < 0 || fd >= NFD)
                {
                                printf("ERROR: Invalid file descriptor!\n");
                                return;
                }

                //goes through the OpenFileTable to find the fd
                for(i = 0; i < NOFT; i++)
                {
                                if(OpenFileTable[i].inodeptr == running->fd[fd]->inodeptr) //if the file ptr matches the running processes file ptr
                                {
                                                oftp = running->fd[fd]; //it has been found, add to open file table
                                                break;
                                }
                }

                //ensures the fd is open for write
                if(!oftp || (oftp->mode != 1 && oftp->mode != 2 && oftp->mode != 3)) //checking for write only caused issues
                {
                                printf("ERROR: Wrong file mode for write!\n");
                                return;
                }

                //gets the minode
                mip = oftp->inodeptr;

                printf("Writing to file...\n");

                //writes while there are more bytes to be written
                while(nbytes > 0)
                {
                                //mailmans
                                lbk = oftp->offset / BLKSIZE; //last block
                                startByte = oftp->offset % BLKSIZE;

                                if(lbk < 12) //should be less than 12 as first 10 are reserved
                                {
                                                if(mip->INODE.i_block[lbk] == 0) //make sure the block is available
                                                {
                                                                mip->INODE.i_block[lbk] = balloc(mip->dev); //if it is available, allocate
                                                }
                                                blk = mip->INODE.i_block[lbk]; //set block to newly allocated block
                                }
                                else if(lbk >= 12 && lbk < 256 + 12)
                                {
                                                //indirect
                                                //check if indirect block, else we need to allocate it
                                                if(!mip->INODE.i_block[12])
                                                {
                                                                mip->INODE.i_block[12] = balloc(mip->dev); //allocate block space

                                                                //fill the new block with 0's
                                                                get_block(mip->dev, mip->INODE.i_block[12], write_buf);
                                                                for(i = 0; i < BLKSIZE; i++)
                                                                                write_buf[i] = 0;
                                                                put_block(mip->dev, mip->INODE.i_block[12], write_buf);
                                                }
                                                get_block(mip->dev, mip->INODE.i_block[12], write_buf);

                                                ip = (int*)write_buf + lbk - 12;
                                                blk = *ip;

                                                //if data block does not exist yet, have to allocate
                                                if(blk == 0)
                                                {
                                                                *ip = balloc(mip->dev);
                                                                blk = *ip;
                                                }
                                }
                                else
                                {
                                                //double indirect
                                                //same stuff, if it doesn't exist, we need to allocate it
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

                                                //if no block yet, have to allocate it
                                                if(!blk)
                                                {
                                                                ip = balloc(mip->dev); //allocate block
                                                                blk = *ip;

                                                                //fill with 0's
                                                                get_block(mip->dev, blk, write_buf);
                                                                for(i = 0; i < BLKSIZE; i++)
                                                                                write_buf[i] = 0;
                                                                put_block(mip->dev, blk, write_buf);
                                                }

                                                get_block(mip->dev, blk, write_buf); //get the block from device

                                                ip = (int*)write_buf + indirect_off;
                                                blk = *ip;

                                                if(!blk)
                                                {
                                                                *ip = balloc(mip->dev); //more allocation
                                                                blk = *ip;
                                                                put_block(mip->dev, blk, write_buf);
                                                }
                                }

                                get_block(mip->dev, blk, write_buf);
                                cp = write_buf + startByte;
                                remain = BLKSIZE - startByte;

                                //writes byte by byte
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

//write function, this is the one that gets called in main
void do_write(char *path)
{
                int i, fd, nbytes;
                char *buf = (char*)malloc( (strlen(third) + 1) * sizeof(char*) );
                //open file table
                OFT *ofp;

                //checks
                if(!path)
                {
                                printf("ERROR: No file name given!\n");
                                return;
                }

                if(!third)
                {
                                printf("ERROR: No text to write!\n");
                                return;
                }

                //after making sure there's the right number of inputs,
                //take in the fd
                fd = atoi(path); //path is just file name

                for(i = 0; i < NOFT; i++)
                {
                                //make sure it's in the global open file table
                                ofp = &OpenFileTable[i];

                                if(ofp->refCount == 0)
                                {
                                                printf("ERROR: Bad fd!\n");
                                                return;
                                }

                                //have to verify the fd and its mode
                                if(i == fd)
                                {
                                                if(ofp->mode == 1 || ofp->mode == 2 || ofp->mode == 3)
                                                                break;
                                                else
                                                {
                                                                printf("ERROR: Wrong mode for writing!\n");
                                                                return;
                                                }
                                }
                }

                //copy the text to be written to a buffer
                strcpy(buf, third);
                nbytes = strlen(buf);
                printf("fd is %d\n", fd);
                my_write(fd, buf, nbytes);
                return;
}
