/*********** util.c file ****************/

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   
int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
} 

MINODE *iget(int dev, int ino)
{
                int ipos = 0;
                int i = 0;
                int offset = 0;
                char buf[1024];
                INODE *ip = NULL;
                MINODE *mip = malloc(sizeof(MINODE));

                for (i = 0; i < NMINODE; i++)
                {
                                mip = &minode[i];

                                if(mip->dev == dev && mip->ino == ino)
                                {
                                                mip->refCount++;
                                                return mip;
                                }
                }

                //mailman's
                ipos = (ino - 1)/8 + INODE_START_POS;
                offset = (ino - 1) % 8;

                get_block(dev, ipos, buf);
                //load inode
                ip = (INODE *)buf + offset;

                for (i = 0; i < NMINODE; i++)
                {
                                mip = &minode[i];

                                if (mip->refCount == 0)
                                {
                                                mip->INODE = *ip;
                                                mip->dev = dev;
                                                mip->ino = ino;
                                                mip->refCount++;

                                                return mip;
                                }
                }
}

void tokenize(char pathname[256])
{
                char path_copy[256] = "";
                char *temp;
                int i = 0;

                for(i = 0; i < 64; i++)
                {
                                strcpy(names[i], "");
                }

                i = 0;

                //copy pathname so we don't destroy the original pathname
                strcpy(path_copy, pathname);
                strcat(path_copy, "/");
                temp = strtok(path_copy, "/");

                //populate names
                while(temp != NULL)
                {
                                strcpy(names[i], temp);
                                temp = strtok(NULL, "/");
                                i++;
                }
}

//returns the ino for a given pathname
int getino(MINODE *mip, char pathname[64])
{
                int ino = 0, i = 0, n = 0;
                int inumber, offset;
                char path[64];
                char name[64][64];
                char *temp;
                char buf[1024];

                //check if it is root
                if(!strcmp(pathname, "/"))
                                return 2;  //return root ino

                //check if starts at root
                if(pathname[0] == '/')
                {
                                mip = root;
                }

                //if there's a pathname, then parse it
                if(pathname)
                {
                                //parse the string and put it into name
                                strcat(pathname, "/");
                                temp = strtok(pathname, "/");

                                while(temp != NULL)
                                {
                                                strcpy(name[i], temp);
                                                temp = strtok(NULL, "/");
                                                i++;
                                                n++;
                                }
                                //parsing complete
                }

                //time to do the searching
                for(i = 0; i < n; i++)
                {
                                ino = search(mip, name[i]);

                                if(ino == 0)
                                {
                                                //can't find name[i]
                                                return 0;
                                }
                                mip = iget(dev, ino);
                }

                return ino;
}




int search(MINODE *mip, char *name)
{
                //search for name in the data blocks of this INODE
                //if found, return name's ino;
                //return 0

                int i;
                char buf[BLKSIZE], *cp;
                char dir_name[64];
                DIR *dp;

                //make sure it's a directory before we do anything
                if(!S_ISDIR(mip->INODE.i_mode))
                {
                                printf("ERROR: Not a directory!\n");
                                return 0;
                }

                //search through the direct blocks
                for(i = 0; i < 12; i++)
                {
                                //if the data block has stuff in it
                                if(mip->INODE.i_block[i])
                                {
                                                //get the block
                                                get_block(dev, mip->INODE.i_block[i], buf);

                                                dp = (DIR *)buf;
                                                cp = buf;

                                                while(cp < buf + BLKSIZE)
                                                {
                                                                //null terminate dp->name for strcmp
                                                                if(dp->name_len < 64)
                                                                {
                                                                                strncpy(dir_name, dp->name, dp->name_len);
                                                                                dir_name[dp->name_len] = 0;
                                                                }
                                                                else
                                                                {
                                                                                strncpy(dir_name, dp->name, 64);
                                                                                dir_name[63] = 0;
                                                                }
                                                                //check if it's the name we're looking for
                                                                if(strcmp(name, dir_name) == 0)
                                                                                return dp->inode;  //name matches, return the inode
                                                                cp += dp->rec_len;
                                                                dp = (DIR *)cp;
                                                }
                                }
                }
                //name does not exist, print error message
                printf("Name %s does not exist...\n", name);
                strcpy(teststr, "Name ");
                strcat(teststr, name);
                strcat(teststr, " does not exist...");
                return 0;
}



void iput(MINODE *mip)
{
                int ino = 0;
                int offset, ipos;
                char buf[1024];

                ino = mip->ino;

                //decrement refCount by 1
                mip->refCount--;

                //check refcount to see if anyone using it
                //check dirty to see if it's been changed, dirty == 1 if changed
                //if refCount > 0 and dirty return
                if (mip->refCount == 0 && mip->dirty == 0)
                {
                                return;
                }

                //mailman's to determine disk block and which inode in that block
                ipos = (ino - 1) / 8 + INODE_START_POS;
                offset = (ino -1) % 8;

                //read that block in
                get_block(mip->dev, ipos, buf);

                //copy minode's inode into the inode area in that block
                ip = (INODE*)buf + offset;
                *ip = mip->INODE;

                //write block back to disk
                put_block(mip->dev, ipos, buf);
                mip->dirty = 0;
}


int findino(MINODE *mip, u32 *myino) 
{
  INODE *ip;
                char buf[1024];
                char *cp;
                DIR *dp;

                //check if exists
                if(!mip)
                {
                                printf("ERROR: INO does not exist!\n");
                                return 1;
                }

                //point ip to minode's inode
                ip = &mip->INODE;

                //check if directory
                if(!S_ISDIR(ip->i_mode))
                {
                                printf("ERROR: INO is not a directory\n");
                                return 1;
                }

                get_block(dev, ip->i_block[0], buf);
                dp = (DIR*)buf;
                cp = buf;

                //.
                *myino = dp->inode;

                cp += dp->rec_len;
                dp = (DIR*)cp;

                //..
                *parentino = dp->inode;

return 0;
}


int findmyname(MINODE *parent, u32 myino, char *myname) 
{
 int i;
 char buf[BLKSIZE], temp[256], *cp;  
 DIR    *dp;
 MINODE *mip = parent;

 //**********  search for a file name ***************
 for (i=0; i<12; i++){ // search direct blocks only
     if (mip->INODE.i_block[i] == 0) 
           return -1;

     get_block(mip->dev, mip->INODE.i_block[i], buf);
     dp = (DIR *)buf;
     cp = buf;

     while (cp < buf + BLKSIZE){
       strncpy(temp, dp->name, dp->name_len);
       temp[dp->name_len] = 0;
       //printf("%s  ", temp);

       if (dp->inode == myino){
           strncpy(myname, dp->name, dp->name_len);
           myname[dp->name_len] = 0;
           return 0;
       }
       cp += dp->rec_len;
       dp = (DIR *)cp;
     }
 }
 return -1;
}


int findino(MINODE *mip, u32 *myino) 
{
  char buf[BLKSIZE], *cp;   
  DIR *dp;

  get_block(mip->dev, mip->INODE.i_block[0], buf);
  cp = buf; 
  dp = (DIR *)buf;
  *myino = dp->inode;
  cp += dp->rec_len;
  dp = (DIR *)cp;
  return dp->inode;
}

