
void make_dir(char path[124])
{
                int i, ino;
                MINODE *pmip; //minode pointer
                INODE *pip; //inode pointer

                char buf[1024];
                char temp1[1024], temp2[1024];
                char parent_name[1024], child_name[1024];

                //copy path so we don't destroy it
                strcpy(temp1, path);
                strcpy(temp2, path);

                //get parent and child name
                strcpy(parent_name, dirname(temp1));
                strcpy(child_name, basename(temp2));

                //get parent's ino
                ino = getino(running->cwd, parent_name);
                printf("ino is %d\n", ino);
                pmip = iget(dev, ino);
                pip = &pmip->INODE;

                //check if parent exists
                if(!pmip)
                {
                                printf("ERROR: Parent does not exist!\n");
                                return;
                }

                if(pmip == root)
                                printf("In root...\n");

                //check if dir
                if(!S_ISDIR(pip->i_mode))
                {
                                printf("ERROR: Parent is not a directory!\n");
                                return;
                }

                //check if dir already exists
                if(getino(running->cwd, path) != 0)
                {
                                printf("ERROR: %s already exists!\n", path);
                                return;
                }

                //call my_mkdir to make the dir
                my_mkdir(pmip, child_name);

                //increment the parents link count and adjust the time
                pip->i_links_count++;
                pip->i_atime = time(0L);
                pmip->dirty = 1;
                //set dirty to true and iput

                iput(pmip);

                return;
}

//puts the name into the parent directory
int enter_name(MINODE *mip, int myino, char *myname)
{
                int i;
                INODE *parent_ip = &mip->INODE; //inode parent pointer

                char buf[1024];
                char *cp; //pointer
                DIR *dp; //dir pointer

                int need_len = 0, ideal = 0, remain = 0;
                int bno = 0, block_size = 1024;

                for(i = 0; i < parent_ip->i_size / BLKSIZE; i++)
                {
                                if(parent_ip->i_block[i] == 0) //if no iblocks, break
                                                break;

                                bno = parent_ip->i_block[i]; //get block number

                                get_block(dev, bno, buf); //get the block from device buffer

                                dp = (DIR*)buf; //redefine pointer
                                cp = buf; //set helper pointer to buffer

                                //need length
                                need_len = 4 * ( (8 + strlen(myname) + 3) / 4);

                                //step into last dir entry
                                while(cp + dp->rec_len < buf + BLKSIZE)
                                {
                                                cp += dp->rec_len;
                                                dp = (DIR*)cp;
                                }

                                printf("Last entry is %s...\n", dp->name);
                                cp = (char*)dp;

                                //ideal length uses name len of last dir entry
                                ideal = 4 * ( (8 + dp->name_len + 3) / 4);

                                remain = dp->rec_len - ideal;
                                printf("Remaining is %d...\n", remain);

                                if(remain >= need_len)
                                {
                                                //set rec_len to ideal
                                                dp->rec_len = ideal;

                                                cp += dp->rec_len;
                                                dp = (DIR*)cp;

                                                //sets the dirpointer inode to the given myino
                                                dp->inode = myino;
                                                dp->rec_len = block_size - ((u32)cp - (u32)buf);
                                                printf("rec len is %d\n", dp->rec_len);
                                                dp->name_len = strlen(myname);
                                                dp->file_type = EXT2_FT_DIR; //sets it to EXT2 type dir
                                                //sets the dp name to the given name
                                                strcpy(dp->name, myname);

                                                //puts the block
                                                put_block(dev, bno, buf);

                                                return 1;
                                }
                }

                printf("Number is %d...\n", i);

                //no space in existing data blocks, time to allocate in next block
                bno = balloc(dev); //allocate blocks
                parent_ip->i_block[i] = bno; //add to parent

                parent_ip->i_size += BLKSIZE; //modify inode size
                mip->dirty = 1;

                get_block(dev, bno, buf);

                dp = (DIR*)buf; //dir pointer modified
                cp = buf;

                printf("Dir name is %s\n", dp->name);

                dp->inode = myino; //set inode to myino
                dp->rec_len = 1024; //reset length to 1024
                dp->name_len = strlen(myname); //set name to myname
                dp->file_type = EXT2_FT_DIR; //set dir type to EXT2 compatible
                strcpy(dp->name, myname); //set the dir pointer name to myname

                put_block(dev, bno, buf); //add the block

                return 1;
}

//mkdir function
void my_mkdir(MINODE *pmip, char *child_name)
{
                int ino = ialloc(dev);
                int bno = balloc(dev);
                int i;

                printf("Device is %d\n", dev);
                printf("Ino is %d...\nBno is %d...\n", ino, bno);

                MINODE *mip = iget(dev, ino); //get the inodes
                INODE *ip = &mip->INODE;

                char *cp, buf[1024];
                DIR *dp; //dir pointer

                ip->i_mode = 0x41ED; //OR 040755: DIR type and permissions
                ip->i_uid  = running->uid; //owner uid
                ip->i_gid  = running->gid; //group id
                //we set the size to blksize to because that is the size of a dir
                ip->i_size = BLKSIZE; //size in bytes
                ip->i_links_count = 2; //links count=2 because of . and ..
                ip->i_atime = time(0L); //set access time to current time
                ip->i_ctime = time(0L); //set creation time to current time
                ip->i_mtime = time(0L); //set modify time to current time

                //this is for . and ..
                ip->i_blocks = 2;          //blocks count in 512-byte chunks
                ip->i_block[0] = bno;      //new DIR has one data block

                //set all blocks to 0
                for(i = 1; i < 15; i++)
                {
                                ip->i_block[i] = 0;
                }

                mip->dirty = 1; //set dirty to true and iput
                iput(mip);

                //create data block for new DIR containing . and ..
                get_block(dev, bno, buf);

                dp = (DIR*)buf;
                cp = buf;

                //first dir (.)
                dp->inode = ino; //set dir pointer inodes to ino, ino allocated above, new inodes
                dp->rec_len = 4 * (( 8 + 1 + 3) / 4); //should result in 4 * 3 which is 12
                dp->name_len = strlen("."); //set the name length
                dp->file_type = (u8)EXT2_FT_DIR; //set it to EXT2 dir type
                dp->name[0] = '.'; //make the name .

                cp += dp->rec_len;
                dp = (DIR*)cp;

                //this portion is for the parent (..)
                dp->inode = pmip->ino;
                dp->rec_len = 1012; //will always be 12 in this case
                printf("Rec_len is %d\n", dp->rec_len); //set it to 12
                dp->name_len = strlen(".."); //set name length
                dp->file_type = (u8)EXT2_FT_DIR; //EXT2 dir type
                dp->name[0] = '.'; //for the first period
                dp->name[1] = '.'; //second period now makes (..)

                //write buf to disk block bno
                put_block(dev, bno, buf);

                //enter name entry into parent's directory
                enter_name(pmip, ino, child_name);

                return 1;
}
