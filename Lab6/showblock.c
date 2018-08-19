/********* showblock.c code ***************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h> // Used for strtok
#include <sys/stat.h> // Used for S_ISDIR

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

#define BLKSIZE 1024

/******************* in <ext2fs/ext2_fs.h>*******************************
struct ext2_super_block {
  u32  s_inodes_count;       // total number of inodes
  u32  s_blocks_count;       // total number of blocks
  u32  s_r_blocks_count;     
  u32  s_free_blocks_count;  // current number of free blocks
  u32  s_free_inodes_count;  // current number of free inodes 
  u32  s_first_data_block;   // first data block in this group
  u32  s_log_block_size;     // 0 for 1KB block size
  u32  s_log_frag_size;
  u32  s_blocks_per_group;   // 8192 blocks per group 
  u32  s_frags_per_group;
  u32  s_inodes_per_group;    
  u32  s_mtime;
  u32  s_wtime;
  u16  s_mnt_count;          // number of times mounted 
  u16  s_max_mnt_count;      // mount limit
  u16  s_magic;              // 0xEF53
  // A FEW MORE non-essential fields
};
**********************************************************************/

///////////////////////////////////////////////////////////////
// Vars for getblock()
char buf[BLKSIZE];
int fd;

// Gets a block from the file descriptor
int get_block(int fd, int blk, char buf[ ]) {
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
}
#define BLOC_OFFSET(block) (BLKSIZE + block-1)*BLKSIZE

void get_inode(int fd, int ino, int inode_table,INODE *inode) {
	lseek(fd, BLOC_OFFSET(inode_table) + (ino - 1) * sizeof(INODE), 0);
	read(fd, inode, sizeof(INODE));
}

///////////////////////////////////////////////////////////////
// Checks to make sure that the open fs is ext2
verifyext2fs() {

  printf("FILE descriptor: %d", fd);
  printf("buf: %d", buf);

  // read SUPER block
  get_block(fd, 1, buf);  
  sp = (SUPER *)buf;

  // check for EXT2 magic number:
  printf("VALID EXT2 FS: s_magic = %x\n", sp->s_magic);
  if (sp->s_magic != 0xEF53) {
    printf("NOT an EXT2 FS\n");
    exit(1);
  }

  printf("\n************  super block info: *************\n");

  printf("inodes_count		%d\n", sp->s_inodes_count);
  printf("blocks_count		%d\n", sp->s_blocks_count);
  printf("r_blocks_count		%d\n", sp->s_r_blocks_count);
  printf("free_blocks_count	%d\n", sp->s_free_blocks_count);
  printf("free_inodes_count	%d\n", sp->s_free_inodes_count);
  printf("log_blk_size		%d\n", sp->s_log_block_size);
  printf("first_data_block	%d\n", sp->s_first_data_block);
  printf("magic number		%x\n", sp->s_magic);
  printf("rev_level		%d\n", sp->s_rev_level);
  printf("inode_size		%d\n", sp->s_inode_size);
  //printf("blocks_per_group	%d\n", sp->s_blocks_per_group);
  printf("blksize			%d\n", BLKSIZE);
  printf("inodes_per_group	%d\n", sp->s_inodes_per_group);

  printf("---------------------------------------------\n");

}

int InodesBeginBlock = 0;

// Set GP ptr to group descriptor
get_group_descriptor_get_inodebegin() {
  get_block(fd, 2, buf);
  gp = (SUPER *)buf;

  InodesBeginBlock = gp->bg_inode_table;
  //printf("\nInodesBeginBlock: %d\n", InodesBeginBlock);
}

read_InodeBeginBlock() {
  printf("\nPrinting InodeBeginBlock:\n-------------------------\n - InodesBeginBlock=%d\n", InodesBeginBlock);

  // get inode start block     
  get_block(fd, InodesBeginBlock, buf);

  ip = (INODE *)buf + 1;         // ip points at 2nd INODE
  
  printf(" - mode=%4x ", ip->i_mode);
  printf("  uid=%d  gid=%d\n", ip->i_uid, ip->i_gid);
  printf(" - size=%d\n", ip->i_size);
  printf(" - time=%s", ctime(&ip->i_ctime));
  printf(" - link=%d\n", ip->i_links_count);
  printf(" - i_block[0]=%d\n", ip->i_block[0]);
}

char *name[128];
char *pathname = "/";
int i = 0, n = 0;

get_tokens_from_pathname() {
  printf("\nPathname: %s\n", pathname);

  // May have to remove an initial '/'
  // Get first token
  name[0] = strtok(pathname, "/");
  printf(" - name[0]: %s\n", name[0]);

  while (name[i] != NULL) {
    i++;
    name[i] = strtok(NULL, "/");
    printf(" - name[%d]: %s\n", i, name[i]);
  }

  n = i;
  printf(" - n = %d\n", n);
}


char dbuf[1024];

//search for node/ prints out dir
int search(INODE * inodePtr, char * name) 
  {
	//printf("\nSEARCHING FOR: %s", name);
	for (int i = 0; i < 12; i++) 
	{
		if (inodePtr->i_block[i] == 0)
		{
			return 0;
		}
		get_block(fd, inodePtr->i_block[i], dbuf);

		DIR *dp = (SUPER *)dbuf;
		char *cp = dbuf;
		int temp = 0;
		int found = 0;
		printf("********* root dir entries ***********\n");
		printf("i_number  rec_len   name_len	name\n");
		while (cp < &dbuf[1024])
		{
			printf("%d	  %d	    %d		%s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);
			if (strcmp(name, dp->name) == 0)
			{
				temp = dp->inode;
				found = 1;
			}
			cp += dp->rec_len;
			dp = (DIR *)cp;

		}
		if(found)
		{
			printf("\n\n - Found at INODE: %d\n", temp);
			return temp;
		}
		printf(" - Not Found\n");
		return 0;
	}
}




///////////////////////////////////////////////////////////////
int inumber;

// Actual code for this assignment
showblock() {
  //1. Open the device for READ (DONE IN MAINLINE). Read in Superblock, verify it is ext2
  // Verify that the opened FS is ext2
  verifyext2fs();
  
  //2. Read in group descriptor block, determine where INODEs begin on the disk. Call it the InodesBeginBlock
  get_group_descriptor_get_inodebegin();  

  //3. Read in InodeBeginBlock to get the inode of /, which is INODE #2. NOTE: inode number counts from 1.
  // Also points ip to the 2nd Inode, which should be the root inode
  read_InodeBeginBlock();
  // HOW DO THIS? <---------------------------------------------------------------------------------------------------------------------------------- NEEDS TO BE DONE

  //4. Break up pathname into components and let the number of components be n, Denote the components by name[0] name[1] name[n-1]
  get_tokens_from_pathname();  

  //5. No step five

  //6-7. Inside step 8

  //8. Since Steps 6-7 will be repeated n times, you should implement a function
  //search(/*INODE * inodePtr, char * name)*/); // <--------------------------------------------------------------------------------------------------- THIS CALL SHOULD BE DOWN IN 7.#2

  //7.#2 Then, all you have to do is call search() n times, as sketched below.
  //Assume:    n,  name[0], ...., name[n-1]   are globals and ip --> INODE of /

  for (i = 0; i < n; i++){
    inumber = search(ip, name[i]);
    //can't find name[i], BOMB OUT!  
    if (inumber == 0) {
      printf("\nCan't find name[%d]: '%s'\n", i, name[i]);
      exit(1);
    } 
    
    //-------------------------------------------------------
    //use inumber to read in its INODE and let ip --> this INODE
    int INODES_PER_BLOCK = BLKSIZE / sizeof(INODE);

  	get_block(fd, (((inumber-1)/INODES_PER_BLOCK)+InodesBeginBlock), buf);
    ip = (INODE *)buf + ((inumber-1)%INODES_PER_BLOCK);

    
    printf("\nPrinting Found Inode:\n-------------------------\n - inode=%d\n", inumber);
    printf(" - InodesPerBlock: %d\n", INODES_PER_BLOCK);
    printf(" - Found in Block: %d\n", (((inumber-1)/INODES_PER_BLOCK)+InodesBeginBlock));
    printf(" - Found @ Offset: %d\n", ((inumber - 1) % INODES_PER_BLOCK));
    printf(" - mode=%4x ", ip->i_mode);
    printf("  uid=%d  gid=%d\n", ip->i_uid, ip->i_gid);
    printf(" - size=%d\n", ip->i_size);
    printf(" - time=%s", ctime(&ip->i_ctime));
    printf(" - link=%d\n", ip->i_links_count);

    printf("****************  DISK BLOCKS  *******************\n");
    printf(" - i_block[0]=%d\n", ip->i_block[0]);
  	

    if(S_ISDIR(ip->i_mode)) {
      //printf("\n\n\nDIRECTORY!\n\n\n");
    }
    else {
      if(i == n-1) {        
        //printf("\n\n\nLAST ELEMENT! AND NOT A DIR\n\n\n");
      }
      else {
        //printf("\n\n\nNOT DIRECTORY!\n\n\n");
        printf("\nname[%d]: '%s' is not a directory, exiting.\n", i, name[i]);
        exit(1);
      }
    }
  }

  int blocks[256],dblocks[256],dblocks2[256];
  printf("\n=========== Direct Blocks ===========\n", inumber);
   for(int i = 0; i < 14;i++)
   {
	   if (ip->i_block[i] != 0)
		   printf("%d \n", ip->i_block[i]);
	   else
		   continue;

   }
}




// Vars for mainline
// Name of disk to open
char *disk = "mydisk";

// Mainline handles opening of disk, then calls showblock()
main(int argc, char *argv[ ]) { 
  // If given a diskname, use it instead of mydisk - DEFAULT: "mydisk"
  if (argc > 1) {
    disk = argv[1];
  }
  // If given a pathname, set pathname - DEFAULT: "/"
  if (argc > 2) {
    pathname = argv[2];
  }
  // Open disk for read only
  fd = open(disk, O_RDONLY);
  if (fd < 0) {
    printf("Open failed\n");
    exit(1);
  }
  printf("Opened '%s' for RDONLY\n", disk);
  
  // Call main function
  showblock();
}

