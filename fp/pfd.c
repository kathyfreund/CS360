//prints the open FD's
//displays the fd number, mode, offset, device and inode
void my_pfd(char *path)
{
                int i;
                OFT *ofp;
                printf("\n  fd   mode   offset   device   inode \n");
                printf("  --   ----   ------   ------   ----- \n");

                //loops through the OpenFileTable until we dont have any fd's to look at
                for(i = 0; i < NOFT; i++)
                {
                                ofp = &OpenFileTable[i];

                                if(ofp->refCount == 0)
                                {
                                                return;
                                }

                                printf("  %02d    ", i);

                                //prints the mode in chars
                                switch(ofp->mode)
                                {
                                case 0:
                                                printf("RD");
                                                break;
                                case 1:
                                                printf("WR");
                                                break;
                                case 2:
                                                printf("RW");
                                                break;
                                case 3:
                                                printf("AP");
                                                break;
                                default:
                                                printf("--");
                                                break;
                                }

                                //prints the offset, dev and ino
                                printf("    %6d     %2d     %5d\n", ofp->offset, ofp->inodeptr->dev, ofp->inodeptr->ino);
                }
                return 0;
}
