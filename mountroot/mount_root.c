void init()
{
                //2 PROCs, P0 with uid=0, P1 with uid=1, all PROC.cwd = 0
                //MINODE minode[100]; all with refCount=0
                //MINODE *root = 0;
                int i;

                //a. allocate memory for running process
                running = malloc(sizeof(PROC));

                proc[0].pid = 1; //process location 0's process identification number = 1
                proc[0].uid = 0; //process location 0's user identification number = 0
                proc[0].cwd = 0; //process location 0's current working directory = 0

                proc[1].pid = 2; //same as above
                proc[1].uid = 1; //same as above
                proc[1].cwd = 0; //same as above

                running = &proc[0];

                //b. Add MINODE minode[100] all with refCount=0
                // the minode is the in-memory inode, set all references to 0
                for(i = 0; i < 100; i++)
                {
                                minode[i].refCount = 0;
                }

                //MINODE *root = 0;
                root = 0;
}

//2. mount root file system, establish / (home directory) and CWDs
void mount_root(char device_name[64])
{
                char buf[1024]; //allocate space for buffer
                //open device for RW
                dev = open(device_name, O_RDWR); //dev = device

                //check if open() worked
                if(dev < 0)
                {
                                printf("ERROR: Could not open %s!\n", device_name);
                                exit(0);
                }

                //read super block to verify it's an EXT2 FS
                get_block(dev, SUPERBLOCK, buf); //built in function
                sp = (SUPER *)buf; //super pointer
                //verify if it's an EXT2 FS
                if(sp->s_magic != 0xEF53) //make sure majic number is the EXT2 value
                {
                                printf("NOT AN EXT2 FS!\n");
                                exit(1);
                }

                //set some variables
                ninodes = sp->s_inodes_count; //get the super's inode count, use pointer member in strut
                nblocks = sp->s_blocks_count; //get the super block count, use the pointer member in strut

                //read group block for info
                get_block(dev, GDBLOCK, buf); //built in function for getting block
                gp = (GD *)buf; //group descripter pointer

                imap = gp->bg_inode_bitmap; //get the group descripter's inodes bitmap
                bmap = gp->bg_block_bitmap; //get the gd block bit map

                inodeBeginBlock = gp->bg_inode_table; //get the indoe table

                //get root inode
                root = iget(dev, 2); //root is location 2 of device, built in function from util.c

                //let cwd of both p0 and p1 point at the root minode (refCount=3)
                proc[0].cwd = root;
                proc[1].cwd = root;

                root->refCount = 3;

                printf("%s has been mounted sucessfully!\n", device_name);
                //device has now been mounted
}

//print information for file or dir
void print_info(MINODE *mip, char *name)
{
                int i;
                INODE *ip = &mip->INODE; //create inode pointer

                char *permissions = "rwxrwxrwx"; //r = read, w = write, x = run

                //information for each file
                u16 mode   = ip->i_mode; //file or dir mode
                u16 links  = ip->i_links_count; //links
                u16 uid    = ip->i_uid; //user id number
                u16 gid    = ip->i_gid; //group id number
                u32 size   = ip->i_size; //size of file or dir

                char *time = ctime( (time_t*)&ip->i_mtime); //get the times from the files or dirs
                //remove \r from time
                time[strlen(time) - 1] = 0;

                switch(mode & 0xF000)
                {
                case 0x8000:  putchar('-');     break; // 0x8 = 1000
                case 0x4000:  putchar('d');     break; // 0x4 = 0100
                case 0xA000:  putchar('l');     break; // 0xA = 1010
                default:      putchar('?');     break;
                }

                //print the permissions
                for(i = 0; i < strlen(permissions); i++)
                                putchar(mode & (1 << (strlen(permissions) - 1 - i)) ? permissions[i] : '-');
                                //binary if then statement, if this do this, if not, do this

                //print the rest
                printf("%4hu %4hu %4hu %8u %26s  %s\n", links, gid, uid, size, time, name);
                strcat(teststr, name);
                strcat(teststr, " ");

                //done
                return;
}

//print directory
//goes through the directory and runs print info for every dp
void print_dir(MINODE *mip)
{
                int i;
                DIR *dp; //directory pointer
                char *cp; //helper pointer
                char buf[1024], temp[1024];
                INODE *ip = &mip->INODE; //inode pointer
                MINODE *temp_mip;

                printf("Printing directory...\n");

                for(i = 0; i < ip->i_size/1024; i++)
                {
                                if(ip->i_block[i] == 0) //no contents in directoru
                                                break;

                                get_block(dev, ip->i_block[i], buf); //load contents of device into buffer
                                dp = (DIR*)buf; //descripter pointer
                                cp = buf; //cp now equals buffer, can walk through buffer and print contents

                                //print the contents
                                while(cp < buf + BLKSIZE)
                                {
                                                strncpy(temp, dp->name, dp->name_len); //built in strut members
                                                temp[dp->name_len] = 0;

                                                temp_mip = iget(dev, dp->inode); //built in function from util.c
                                                if(temp_mip)
                                                {
                                                                print_info(temp_mip, temp); //calls printinfo on temp
                                                                iput(temp_mip);
                                                }
                                                else
                                                                printf("ERROR: Cannot print info for Minode!\n");

                                                memset(temp, 0, 1024);
                                                cp += dp->rec_len;
                                                dp = (DIR*)cp;
                                }
                }

                printf("\n");
}

//prints everything located at the pathname
//calls printdir which calls printinfo for a given pathname
//the main function of ls is to get the pathname and check for certain cases
//finds the ino of the pathname
void ls(char *pathname)
{
                int ino, offset;
                MINODE *mip = running->cwd; //get the minode MIP
                char name[64][64], temp[64];
                char buf[1024];

                //ls cwd
                if(!strcmp(pathname, ""))
                {
                                //print_dir(mip->INODE);
                                print_dir(mip);
                                return;
                }

                //ls root dir
                if(!strcmp(pathname, "/"))
                {
                                //print_dir(root->INODE);
                                print_dir(root);
                                return;
                }

                //if there's a pathname, ls pathname
                if(pathname)
                {
                                //check if path starts at root
                                if(pathname[0] == '/')
                                {
                                                mip = root;
                                }

                                //search for path to print
                                ino = getino(mip, pathname); //built in function from util.c
                                if(ino == 0)
                                {
                                                return;
                                }

                                mip = iget(dev, ino); //from util.c
                                if(!S_ISDIR(mip->INODE.i_mode))
                                {
                                                printf("%s is not a directory!\n", pathname);
                                                strcpy(teststr, pathname);
                                                strcat(teststr, " is not a directory!");
                                                iput(mip);
                                                return;
                                }

                                //print_dir(mip->INODE);
                                print_dir(mip);
                                iput(mip); //from util.c
                }
                else
                {
                                //print root dir
                                //print_dir(root->INODE);
                                print_dir(root);
                }
}

//change cwd to pathname. If no pathname, set cwd to root.
//checks some casses, ultimatly sets running->cwd to the inode of the pathname
void cd(char *pathname)
{
                int ino = 0;

                MINODE *mip = running->cwd; //same as above, running is temp value for use with cd
                MINODE *newmip = NULL;

                if (!strcmp(pathname, "")) //if pathname is empty CD to root
                {
                                running->cwd = root; //set cwd to root
                                return;
                }

                if (!strcmp(pathname, "/"))
                {
                                running->cwd = root; //set cwd to root
                                return;
                }

                printf("Path = %s\n", pathname);
                ino = getino(mip, pathname); //load pathname into ino, from util.c
                if(ino == 0)
                {
                                printf("The directory %s does not exist!\n", pathname); //make sure aren't cd'ing to dir that doesn't exist
                                return;
                }

                newmip = iget(dev, ino); //load pathname from ino into new process, from util.c
                if(!S_ISDIR(newmip->INODE.i_mode))
                {
                                printf("%s is not a directory!\n", pathname); //can't cd to a file
                                iput(newmip);
                                return;
                }

                running->cwd = newmip; //put pathname back into running's cwd
                iput(newmip); //built in function from util.c
                return;
}

//iput all still loaded minodes before shutdown
int quit()
{
                int i;

                //go through all the minodes
                for(i = 0; i < NMINODE; i++)
                {
                                //check if used and dirty
                                if(minode[i].refCount > 0 && minode[i].dirty == 1)
                                {
                                                minode[i].refCount = 1;
                                                iput(&minode[i]); //built in from util.c
                                }
                }
                printf("Exiting...\n");
                exit(0);
}
