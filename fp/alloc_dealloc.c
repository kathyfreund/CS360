//Sets a bit using OR
//Used in ialloc
void set_bit(char *buf, int bit)
{
                int i, j;
                //mailmans algorithm from class
                i = bit / 8; //divide bit by 8
                j = bit % 8; //mod by 8 to get the bytes
                buf[i] |= (1 << j); //or command
}

//decrements the amount of free inodes on the device
//this is used to ensure we dont use more inodes than we have room for.
void decFreeInodes(int dev)
{
                char buf[BLKSIZE]; //defined in header

                //dec free inodes count in SUPER and Group Descriptor block
                get_block(dev, 1, buf); //built in function
                sp = (SUPER *)buf; //super block pointer
                sp->s_free_inodes_count--; //pointer member, decrease number of free inodes
                put_block(dev, 1, buf); //built in function, adds block
                //add another block
                get_block(dev, 2, buf); //get the second block
                gp = (GD *)buf; //group descriptor pointer
                gp->bg_free_inodes_count--; //pointer memeber, decrease number free inodes
                put_block(dev, 2, buf); //built in function
}

int tst_bit(char *buf, int bit) //test bits
{
                int i, j;
                //mailmans
                i = bit / 8; //divide by 8
                j = bit % 8; //mod by 8

                if(buf[i] & (1 << j))
                                return 1;

                return 0;
}

//end of helper functions

//allocates a free inode; returns ino
int ialloc(int dev)
{
                int i;
                char buf[BLKSIZE];

                //read imap
                get_block(dev, imap, buf);

                for(i = 0; i < ninodes; i++)
                {
                                if(tst_bit(buf, i) == 0)
                                {
                                                set_bit(buf, i);
                                                decFreeInodes(dev);
                                                put_block(dev, imap, buf);
                                                return i + 1;
                                }
                }
                printf("[!] ERROR - No free inodes.\n");
                return 0;
}

//allocates a free block; returns bno
int balloc(int dev)
{
                int i;
                char buf[BLKSIZE]; 

                //read bmap
                get_block(dev, bmap, buf);

                for(i = 0; i < nblocks; i++)
                {
                                if(tst_bit(buf, i) == 0) 
                                {
                                                set_bit(buf, i); 
                                                decFreeInodes(dev); 
                                                put_block(dev, bmap, buf); 
                                                return i;
                                }
                }
                printf("[!] ERROR - No free blocks.\n");
                return 0;
}




//deallocates an inode for a given ino on the dev
//this is used when we remove things
//once dealocated, we increment the free inodes in the SUPER and in the group descriptor
int idealloc(int dev, int ino)
{
                char buf[1024];
                int byte;
                int bit;

                //clear bit(bmap, bno)
                get_block(dev, imap, buf);

                //mailmans to where it is, from lecture
                byte = ino / 8;
                bit = ino % 8;

                //negate it
                buf[byte] &= ~(1 << bit);

                put_block(dev, imap, buf); //built in function

                //set free blocks
                get_block(dev, 1, buf); //built in function
                sp = (SUPER *)buf; //super block pointer
                sp->s_free_blocks_count++; //pointer member, increase number of inodes since we have deallocated it
                put_block(dev, 1, buf); //built in function, adds another block

                get_block(dev, 2, buf); //same as above
                gp = (GD *)buf;
                gp->bg_free_blocks_count++;
                put_block(dev, 2, buf);
}

//deallocate a block
//once deallocated we also increment the number of free blocks, basically opposite of above function
int bdealloc(int dev, int bno)
{
                char buf[1024];
                int byte;
                int bit;

                //clear bit(bmap, bno)
                get_block(dev, bmap, buf);

                //mailmans from class
                byte = bno / 8;
                bit = bno % 8;

                buf[byte] &= ~(1 << bit);

                put_block(dev, bmap, buf); //built in function

                //set free blocks
                get_block(dev, 1, buf);
                sp = (SUPER *)buf;
                sp->s_free_blocks_count++;
                put_block(dev, 1, buf);

                get_block(dev, 2, buf);
                gp = (GD *)buf;
                gp->bg_free_blocks_count++;
                put_block(dev, 2, buf);

                return 0;
}
