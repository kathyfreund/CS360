void make_dir(char path[124])
{
                int i, ino;
                MINODE *pmip; 
                INODE *pip; 

                char buf[1024];
                char temp1[1024], temp2[1024];
                char parent_name[1024], child_name[1024];

              
                strcpy(temp1, path);
                strcpy(temp2, path);

                strcpy(parent_name, dirname(temp1));
                strcpy(child_name, basename(temp2));
		printf("Parent = %s\nChild = %s\n", parent_name, child_name);

                ino = getino(running->cwd, parent_name); //get parent ino

                printf("Ino = %d\n", ino);
                pmip = iget(dev, ino);
                pip = &pmip->INODE;

                //check if parent exists
                if(!pmip)
                {
                                printf("ERROR: Parent does not exist!\n");
                                return;
                }

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

                //change parent links and time
                pip->i_links_count++;
                pip->i_atime = time(0L);
                pmip->dirty = 1;


                iput(pmip);

                return;
}

//puts name into dir
int enter_name(MINODE *mip, int myino, char *myname)
{
                int i;
                INODE *parent_ip = &mip->INODE; 

                char buf[1024];
                char *cp; 
                DIR *dp; 

                int need_len = 0, ideal = 0, remain = 0;
                int bno = 0, block_size = 1024;

                for(i = 0; i < parent_ip->i_size / BLKSIZE; i++) //12 direct
                {
                                if(parent_ip->i_block[i] == 0) 
                                                break;

                                bno = parent_ip->i_block[i]; //get block number

                                get_block(dev, bno, buf); 

                             

                                //All DIR entries in a data block have rec_len = IDEAL_LEN
				//IDEAL_LEN = 4*[ (8 + name_len + 3)/4 ].
                                need_len = 4 * ( (8 + strlen(myname) + 3) / 4);

                                //Step to the last entry in a data block
				dp = (DIR*)buf; 
                                cp = buf; 
                                while(cp + dp->rec_len < buf + BLKSIZE)
                                {
                                                cp += dp->rec_len;
                                                dp = (DIR*)cp;
                                }

                                printf("Last entry is %s.\n", dp->name);
                                cp = (char*)dp;

                                //ideal length uses name len of last dir entry
                                ideal = 4 * ( (8 + dp->name_len + 3) / 4);

				//Let remain = LAST entry's rec_len - its IDEAL_LENGTH;
                                remain = dp->rec_len - ideal;
                                printf("Remaining is %d.\n", remain);


				/*
EXAMPLE:

                                 |LAST entry 
  --|-4---2----2--|----|---------|--------- rlen ->------------------------|
    |ino rlen nlen NAME|.........|ino rlen nlen|NAME                       |
  --------------------------------------------------------------------------
                                                    |     NEW entry
  --|-4---2----2--|----|---------|----ideal_len-----|--- rlen=remain ------|
    |ino rlen nlen NAME|.........|ino rlen nlen|NAME|myino rlen nlen myname|
  --------------------------------------------------------------------------


				*/

                                if(remain >= need_len)
                                {
                                                dp->rec_len = ideal;

                                                cp += dp->rec_len;
                                                dp = (DIR*)cp;

                                                dp->inode = myino;
                                                dp->rec_len = block_size - ((u32)cp - (u32)buf);
                                                printf("rec len is %d\n", dp->rec_len);
                                                dp->name_len = strlen(myname);
                                                dp->file_type = EXT2_FT_DIR; //EXT2 type dir
                                                //sets dp name to given name
                                                strcpy(dp->name, myname);

                                                put_block(dev, bno, buf);

                                                return 1;
                                }
                }

                printf("Number is %d.\n", i);

                //no space in existing data blocks
                bno = balloc(dev);
                parent_ip->i_block[i] = bno;

                parent_ip->i_size += BLKSIZE;
                mip->dirty = 1;

                get_block(dev, bno, buf);

                dp = (DIR*)buf;
                cp = buf;

                printf("Dir name is %s\n", dp->name);

                dp->inode = myino; 
                dp->rec_len = 1024; 
                dp->name_len = strlen(myname); 
                dp->file_type = EXT2_FT_DIR; 
                strcpy(dp->name, myname); 

                put_block(dev, bno, buf); 
                return 1;
}

//mkdir function
void my_mkdir(MINODE *pmip, char *child_name)
{
                int ino = ialloc(dev); //allocate space
                int bno = balloc(dev);
                int i;

                printf("Device is %d\n", dev);
                printf("Ino is %d.\nBno is %d.\n", ino, bno);

                MINODE *mip = iget(dev, ino);
                INODE *ip = &mip->INODE;

                char *cp, buf[1024];
                DIR *dp;

                ip->i_mode = 0x41ED; //OR 040755: DIR type and permissions
                ip->i_uid  = running->uid; //owner uid
                ip->i_gid  = running->gid; //group id
                //we set the size to blksize to because that is the size of a dir
                ip->i_size = BLKSIZE; //size in bytes
                ip->i_links_count = 2; //links count=2 because of . and ..
                ip->i_atime = time(0L); //set access time to current time
                ip->i_ctime = time(0L); //set creation time to current time
                ip->i_mtime = time(0L); //set modify time to current time

                // . and ..
                ip->i_blocks = 2;          
                ip->i_block[0] = bno;      

                //all blocks to 0
                for(i = 1; i < 15; i++)
                {
                                ip->i_block[i] = 0;
                }

                mip->dirty = 1; 
                iput(mip);

                //create data block for new DIR containing . and ..
                get_block(dev, bno, buf);

                dp = (DIR*)buf;
                cp = buf;

                //first dir (.)
                dp->inode = ino; 
                dp->rec_len = 4 * (( 8 + 1 + 3) / 4); 
                dp->name_len = strlen("."); 
                dp->file_type = (u8)EXT2_FT_DIR; 
                dp->name[0] = '.'; 

                cp += dp->rec_len;
                dp = (DIR*)cp;

                // parent (..)
                dp->inode = pmip->ino;
                dp->rec_len = 1012; 
                dp->name_len = strlen(".."); 
                dp->file_type = (u8)EXT2_FT_DIR; 
                dp->name[0] = '.'; 
                dp->name[1] = '.'; //second period now makes (..)

                put_block(dev, bno, buf);

                enter_name(pmip, ino, child_name);

                return 1;
}
