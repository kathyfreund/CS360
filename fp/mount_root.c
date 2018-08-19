void init()
{
                int i;
                running = malloc(sizeof(PROC));

                proc[0].pid = 1; //first process
                proc[0].uid = 0; //current index
                proc[0].cwd = 0; 

                proc[1].pid = 2; //second process
                proc[1].uid = 1; //current index
                proc[1].cwd = 0; 

                running = &proc[0]; //start at 0

                //MINODE minode[64]; all with refCount=0
                for(i = 0; i < 64; i++)
                {
                                minode[i].refCount = 0;
                }
                root = 0;
}

/*
PROC* running           MINODE *root                          
      |                          |                 ||*********************
      V                          |  MINODE         || 
    PROC[0]                      V minode[100]     ||         Disk dev
 =============  |-pointerToCWD-> ==== 0 =====      ||   ==================
 |nextProcPtr|  |                |  INODE   |      ||   |     INODEs   
 |pid = 1    |  |                | -------  |      ||   ================== 
 |uid = 0    |  |                | (dev,2)  |      || 
 |cwd --------->|                | refCount=3      ||*********************
 |           |                   | dirty=0  |
 |fd[16]     |                   | mounted=1|         
 | ------    |                   | mptr->mtable[0]
 |           |                   |----------| 
 | - ALL 0 - |                   |==========|         
 | ------    |                   |  INODE   |          
 | ------    |                   | -------  |   
 =============                   | (dev,ino)|   
                                 | refCount |  
   PROC[1]          ^            | dirty    |   
    pid=2           |            |          |
    uid=1           |            |          |
    cwd ----> root minode        |==========|  


             ---------0--------|----1----|---2------ 
   mtable[]= |dev=fd           |  dev=0  |
             |ninodes,nblocks  |         |
             |bmap,imap,iblk   |         |
             |mntDirPtr=root   |         |
             |devName="mydidsk"|         | 
             |mntName="/"      |         |
             ----------------------------------------
*/

void mount_root(char device_name[64]) //updates ninodes, nblocks, sp, gp, imap, bmap, inodeBeginBlock, root, root->refCount, 
{
                char buf[1024];
                dev = open(device_name, O_RDWR); //if opened successfully, dev should be > 1

                if(dev < 0) 
                {
                                printf("[!] ERROR - Could not open device.\n");
                                exit(0); //failed to open dev, go back to main
                }

                //is it an EXT2 FS?
                get_block(dev, SUPERBLOCK, buf); //util.c; superblock = block 1
                sp = (SUPER *)buf; //change it over to a supernode type
                if(sp->s_magic != 0xEF53) //must match in order to be considered ext2
                {
                                printf("[!] ERROR - device not an ext2 filesystem\n");
                                exit(1); //not ext2, back to main to try again
                }


                ninodes = sp->s_inodes_count; //get number of inodes
                nblocks = sp->s_blocks_count; //get number blocks
		printf("s_inodes_count = %u\ns_blocks_count = %u\n", ninodes, nblocks); 


                get_block(dev, GDBLOCK, buf); //util.c; gdblock = block 2
                gp = (GD *)buf; //change it over to GD type
                imap = gp->bg_inode_bitmap; //get imap block number
                bmap = gp->bg_block_bitmap; //get bmap block number
		inodeBeginBlock = gp->bg_inode_table; //get inodes begin block number
		printf("bg_inode_bitmap = %u\nbg_block_bitmap = %u\n", imap, bmap);  
		printf("bg_inode_table = %u\n", inodeBeginBlock);
              
		//Let cwd of both P0 and P1 point at the root minode (refCount=3)
                root = iget(dev, 2); //util.c; root ino should always be 2
                proc[0].cwd = root;
                proc[1].cwd = root;
                root->refCount = 3; //refCount should alwasy be one+ of exisitng dependents

                printf("Mount sucessfull.\n", device_name);
}

//prints out info from given inode/name of inode
void print_info(MINODE *mip, char *name)
{
                int i;
                INODE *ip = &mip->INODE;
                char *permissions = "rwxrwxrwx"; //r = read, w = write, x = run

                u16 mode   = ip->i_mode; 
                u16 links  = ip->i_links_count; 
                u16 uid    = ip->i_uid;
                u16 gid    = ip->i_gid; 
                u32 size   = ip->i_size; 

                char *time = ctime( (time_t*)&ip->i_mtime); 
                time[strlen(time) - 1] = 0;

                switch(mode & 0xF000)
                {
                case 0x8000:  putchar('-');     break; 
                case 0x4000:  putchar('d');     break; 
                case 0xA000:  putchar('l');     break;
                default:      putchar('?');     break;
                }

                for(i = 0; i < strlen(permissions); i++)
		{
                                putchar(mode & (1 << (strlen(permissions) - 1 - i)) ? permissions[i] : '-');
		}
  
                printf("%4hu %4hu %4hu %8u %26s  %s\n", links, gid, uid, size, time, name);
                strcat(teststr, name);
                strcat(teststr, " ");

                return;
}


void print_dir(MINODE *mip) //takes in inode whose contents we want to see
{
                int i;
                DIR *dp; 
                char *cp; 
                char buf[1024], temp[1024];
                INODE *ip = &mip->INODE; 
                MINODE *temp_mip;

                for(i = 0; i < ip->i_size/1024; i++) //i_size = file size in bytes
                {
                                if(ip->i_block[i] == 0) //empty
				{
                                                break; 
				}
                                get_block(dev, ip->i_block[i], buf); 
                                dp = (DIR*)buf; //convert into dir
                                cp = buf; 
                                
                                while(cp < buf + BLKSIZE)
                                {
                                                strncpy(temp, dp->name, dp->name_len); 
                                                temp[dp->name_len] = 0;

                                                temp_mip = iget(dev, dp->inode); //returns inode of given ino
                                                if(temp_mip)
                                                {
                                                                print_info(temp_mip, temp);
                                                                iput(temp_mip);
                                                }
                                                else
						{
                                                                printf("[!] ERROR - cannot print info.\n");
						}
                                                memset(temp, 0, 1024);
                                                cp += dp->rec_len;
                                                dp = (DIR*)cp; //next
                                }
                }

                printf("\n");
}

void ls(char *pathname)
{
                int ino, offset;
                MINODE *mip = running->cwd; //get the minode MIP
                char name[64][64], temp[64];
                char buf[1024];

                //nothing
                if(!strcmp(pathname, ""))
                {
                                print_dir(mip);
                                return;
                }

                //root
                if(!strcmp(pathname, "/"))
                {
                                print_dir(root);
                                return;
                }

                //actual path
                if(pathname)
                {
                                //absolute
                                if(pathname[0] == '/')
                                {
                                                mip = root;
                                }
                                ino = getino(mip, pathname);

                                if(ino == 0) //does not exist
                                {
                                                return; //return to main to try again
                                }

                                mip = iget(dev, ino); //util.c; returns inode of given ino
                                if(!S_ISDIR(mip->INODE.i_mode))
                                {
                                                printf("[!] ERROR - %s is not a directory.\n", pathname);
                                                strcpy(teststr, pathname);
                                                strcat(teststr, " is not a directory.");
                                                iput(mip); //why?
                                                return;
                                }
                                print_dir(mip);
                                iput(mip); //why?
                }
                else
                {
                                print_dir(root);
                }
}

void cd(char *pathname)
{
                int ino = 0;

                MINODE *mip = running->cwd;
                MINODE *newmip = NULL;

                if (!strcmp(pathname, "")) //root
                {
                                running->cwd = root; 
                                return;
                }

                if (!strcmp(pathname, "/")) //root again
                {
                                running->cwd = root; 
                                return;
                }

                //printf("Path = %s\n", pathname);
                ino = getino(mip, pathname); //returns ino of mip

                if(ino == 0) // does it exist?
                {
                                printf("[!] ERROR - Directory %s does not exist.\n", pathname);
                                return;
                }

                newmip = iget(dev, ino); //returns inode of given ino
                if(!S_ISDIR(newmip->INODE.i_mode)) //is it a dir?
                {
                                printf("[!] ERROR - %s is not a directory.\n", pathname);
                                iput(newmip);
                                return;
                }

                running->cwd = newmip; //actually change our position
                iput(newmip); 
                return;
}

//iput() all minodes with (refCount > 0 && DIRTY);
int quit()
{
                int i;
                for(i = 0; i < NMINODE; i++)
                {
                                //check ref/dirty
                                if(minode[i].refCount > 0 && minode[i].dirty == 1)
                                {
                                                minode[i].refCount = 1;
                                                iput(&minode[i]);
                                }
                }
                printf("Exiting...\n");
                exit(0);
}
