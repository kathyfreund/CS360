#include <libgen.h>

void get_block(int fd, int blk, char buf[BLKSIZE]) 
{
                lseek(fd, (long)(blk*BLKSIZE), 0); //finds block and puts it in fd
                read(fd, buf, BLKSIZE); //reads block into buf
}

void put_block(int fd, int blk, char buf[BLKSIZE])
{
                lseek(fd, (long)(blk*BLKSIZE), 0); //finds block and puts it in fd
                write(fd, buf, BLKSIZE); //writes block into buf
}

MINODE *iget(int dev, int ino) // load INODE of (dev,ino) into a minode[]; return mip->minode[]
{
                int ipos = 0;
                int i = 0;
                int offset = 0;
                char buf[1024];
                INODE *ip = NULL;
                MINODE *mip = malloc(sizeof(MINODE));

                for (i = 0; i < NMINODE; i++) //loop through inodes
                {
                                mip = &minode[i];
				//is it part of the device and is it our ino?
                                if(mip->dev == dev && mip->ino == ino)
                                {
                                                mip->refCount++;
                                                return mip;
                                }
                }

                //mailman algorithm
                ipos = (ino - 1)/8 + INODE_START_POS;
                offset = (ino - 1) % 8;
                get_block(dev, ipos, buf);
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

//tokenize pathname and put parts into name[64][64]
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

//spits back corresponding ino for given dir in mip
int getino(MINODE *mip, char pathname[64])
{
                int ino = 0, i = 0, n = 0;
                int inumber, offset;
                char path[64];
                char name[64][64];
                char *temp;
                char buf[1024];

		//root
                if(!strcmp(pathname, "/"))
		{
                                return 2;  //root ino
		}

		//starts at root
                if(pathname[0] == '/')
                {
                                mip = root; //start at root
                }

                //pathname
                if(pathname)
                {
                                strcat(pathname, "/"); //for good measure
                                temp = strtok(pathname, "/"); //parse

                                while(temp != NULL)
                                {
						//store
                                                strcpy(name[i], temp);
                                                temp = strtok(NULL, "/");
                                                i++; //next dir storage space
                                                n++; //what we need for next step
                                }
                }

                for(i = 0; i < n; i++) //loop through
                {
                                ino = search(mip, name[i]); 

                                if(ino == 0) //no name
                                {
						iput(mip);
						printf("Name %s does not exist\n.", name[i]);
                                                return 0;
                                }
				iput(mip);
                                mip = iget(dev, ino);
                }
		iput(mip);
                return ino;
}

//checks mip's contents to see if it contains inode with given name
//returns found inode's ino
int search(MINODE *mip, char *name)
{

                int i;
                char buf[BLKSIZE], *cp;
                char dir_name[64];
                DIR *dp;
		//printf("searching for %s\n", name);
                //directory? doesn't hurt to double check
                if(!S_ISDIR(mip->INODE.i_mode))
                {
                                printf("[!] ERROR - Not a directory.\n");
                                return 0;
                }

                //i_block[0] to i_block[11], which point to direct blocks.
                for(i = 0; i < 12; i++)
                {
                                if(mip->INODE.i_block[i]) //not empty?
                                {
						//get block
                                                get_block(dev, mip->INODE.i_block[i], buf);
						//conver to dir type
                                                dp = (DIR *)buf;
                                                cp = buf;

                                                while(cp < buf + BLKSIZE)
                                                {
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
                                                                //does dir_name = name?
								//printf("At %s.\n", dir_name);
                                                                if(strcmp(name, dir_name) == 0)
								{
                                                                                return dp->inode; 
								}
                                                                cp += dp->rec_len;
                                                                dp = (DIR *)cp;
                                                }
                                }
                }
                printf("%s does not exist.\n", name);
                strcpy(teststr, "Name ");
                strcat(teststr, name);
                strcat(teststr, " does not exist.");
                return 0;
}

 // dispose of a minode[] pointed by mip
void iput(MINODE *mip) 
{
                int ino = 0;
                int offset, ipos;
                char buf[1024];

                ino = mip->ino;

                mip->refCount--;

                //check refcount to see if anyone using it
                //dirty = 1 if changed
                if (mip->refCount > 0 || !mip->dirty)
                {
                                return;
                }

		/* write INODE back to disk */
                //mailman's
                ipos = (ino - 1) / 8 + INODE_START_POS;
                offset = (ino -1) % 8;

                get_block(mip->dev, ipos, buf);

		// copy INODE into *ip
                ip = (INODE*)buf + offset;
                *ip = mip->INODE;

                put_block(mip->dev, ipos, buf);
                mip->dirty = 0;
}

int findmyname(MINODE *parent, int myino, char *myname)
{

                int i;
                INODE *ip;
                char buf[BLKSIZE];
                char *cp;
                DIR *dp;

                if(myino == root->ino)
                {
                                strcpy(myname, "/");
                                return 0;
                }

                if(!parent)
                {
                                printf("ERROR: No parent!\n");
                                return 1;
                }

                ip = &parent->INODE;

                if(!S_ISDIR(ip->i_mode))
                {
                                printf("ERROR: Not a directory!\n");
                                return 1;
                }

                for(i = 0; i < 12; i++)
                {
                                if(ip->i_block[i])
                                {
                                                get_block(dev, ip->i_block[i], buf);
                                                dp = (DIR*)buf;
                                                cp = buf;

                                                while(cp < buf + BLKSIZE)
                                                {
                                                                if(dp->inode == myino)
                                                                {
                                                                                strncpy(myname, dp->name, dp->name_len);
                                                                                myname[dp->name_len] = 0;
                                                                                return 0;
                                                                }
                                                                else
                                                                {
                                                                                cp += dp->rec_len;
                                                                                dp = (DIR*)cp;
                                                                }
                                                }
                                }
                }
                return 1;
}

int findino(MINODE *mip, int *myino, int *parentino)
{

                INODE *ip;
                char buf[1024];
                char *cp;
                DIR *dp;

                //check if exists
                if(!mip)
                {
                                printf("[!] ERROR - ino non-existant.\n");
                                return 1;
                }

                //point ip to minode's inode
                ip = &mip->INODE;

                //check if directory
                if(!S_ISDIR(ip->i_mode))
                {
                                printf("[!] ERROR - ino not a directory.\n");
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
