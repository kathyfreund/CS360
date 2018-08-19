#include <libgen.h>

void get_block(int fd, int blk, char buf[BLKSIZE])
{
                lseek(fd, (long)(blk*BLKSIZE), 0);
                read(fd, buf, BLKSIZE);
}

void put_block(int fd, int blk, char buf[BLKSIZE])
{
                lseek(fd, (long)(blk*BLKSIZE), 0);
                write(fd, buf, BLKSIZE);
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

                //get block
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

                
                strcpy(path_copy, pathname);
                strcat(path_copy, "/");
                temp = strtok(path_copy, "/");

                while(temp != NULL)
                {
                                strcpy(names[i], temp);
                                temp = strtok(NULL, "/");
                                i++;
                }
}

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
                                return 2;  

                //starts at root
                if(pathname[0] == '/')
                {
                                mip = root;
                }

                //parse it
                if(pathname)
                {
                                strcat(pathname, "/");
                                temp = strtok(pathname, "/");

                                while(temp != NULL)
                                {
                                                strcpy(name[i], temp);
                                                temp = strtok(NULL, "/");
                                                i++;
                                                n++;
                                }
                }

                for(i = 0; i < n; i++)
                {
				printf("Searching for %s...\n", name[i]);
                                ino = search(mip, name[i]);

                                if(ino == 0) //no name
                                {
                                                return 0;
                                }
                                mip = iget(dev, ino);
                }

                return ino;
}

int search(MINODE *mip, char *name)
{
                
                int i;
                char buf[BLKSIZE], *cp;
                char dir_name[64];
                DIR *dp;

                //dir?
                if(!S_ISDIR(mip->INODE.i_mode))
                {
                                printf("ERROR: Not a directory!\n");
                                return 0;
                }

                for(i = 0; i < 12; i++)
                {
                                if(mip->INODE.i_block[i])
                                {
                                                get_block(dev, mip->INODE.i_block[i], buf);

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
                                                                if(strcmp(name, dir_name) == 0)
                                                                                return dp->inode;  
                                                                cp += dp->rec_len;
                                                                dp = (DIR *)cp;
                                                }
                                }
                }
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

                mip->refCount--;

                
                if (mip->refCount == 0 && mip->dirty == 0)
                {
                                return;
                }

                //mailman's
                ipos = (ino - 1) / 8 + INODE_START_POS;
                offset = (ino -1) % 8;

                //read
                get_block(mip->dev, ipos, buf);

                ip = (INODE*)buf + offset;
                *ip = mip->INODE;

                //write
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
                                printf("[!] ERROR - INO doesn't exist.\n");
                                return 1;
                }

                ip = &mip->INODE;

                //dir?
                if(!S_ISDIR(ip->i_mode))
                {
                                printf("[!] ERROR - INO not a directory.\n");
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
