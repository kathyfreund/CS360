
void set_bit(char *buf, int bit)
{
                int i, j;
                //mailmans
                i = bit / 8; 
                j = bit % 8; 
                buf[i] |= (1 << j); 
}

void decFreeInodes(int dev)
{
                char buf[BLKSIZE]; 

                get_block(dev, 1, buf); 
                sp = (SUPER *)buf; 
                sp->s_free_inodes_count--; 
                put_block(dev, 1, buf);

                //add block
                get_block(dev, 2, buf);
                gp = (GD *)buf; 
                gp->bg_free_inodes_count--; 
                put_block(dev, 2, buf);
}

int tst_bit(char *buf, int bit) 
{
                int i, j;
                //mailmans
                i = bit / 8;
                j = bit % 8; 

                if(buf[i] & (1 << j))
                                return 1;

                return 0;
}


int ialloc(int dev)
{
                int i;
                char buf[BLKSIZE]; 

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

//allocates block
int balloc(int dev)
{
                int i;
                char buf[BLKSIZE]; 

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

//deallocates an inode
int idealloc(int dev, int ino)
{
                char buf[1024];
                int byte;
                int bit;

                get_block(dev, imap, buf);

                byte = ino / 8;
                bit = ino % 8;

                buf[byte] &= ~(1 << bit);

                put_block(dev, imap, buf); 

                get_block(dev, 1, buf);
                sp = (SUPER *)buf; 
                sp->s_free_blocks_count++; 
                put_block(dev, 1, buf); 

                get_block(dev, 2, buf); 
                gp = (GD *)buf;
                gp->bg_free_blocks_count++;
                put_block(dev, 2, buf);
}

//deallocate block
int bdealloc(int dev, int bno)
{
                char buf[1024];
                int byte;
                int bit;

                get_block(dev, bmap, buf);

                //mailmans
                byte = bno / 8;
                bit = bno % 8;

                buf[byte] &= ~(1 << bit);

                put_block(dev, bmap, buf);

                //free blocks
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
