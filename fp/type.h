#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>

#include <math.h>


/*

1KB      0    1    2    3    4  | 5 . ........... 27| 28 ..................|
BLOCK: |Boot|Super| Gd |Bmap|Imap|Inodes blocks .....|....  data blocks ....|
                                 |    INODEs         |
                              ino|1,2,3,4 .......... |

struct ext2_super_block ********* SUPER
{
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
	u16	s_state;		// File system state 
	u16	s_errors;		// Behaviour when detecting errors 
	u16	s_minor_rev_level; 	// minor revision level 
	u32	s_lastcheck;		// time of last check 
	u32	s_checkinterval;	// max. time between checks 
	u32	s_creator_os;		// OS 
	u32	s_rev_level;		// Revision level 
	u16	s_def_resuid;		// Default uid for reserved blocks 
	u16	s_def_resgid;		// Default gid for reserved blocks 
	u32	s_first_ino; 		// First non-reserved inode 
====>	u16     s_inode_size; 		// size of inode structure 
};

struct ext2_group_desc ********** GD
{
  u32  bg_block_bitmap;          // Bmap block number
  u32  bg_inode_bitmap;          // Imap block number
  u32  bg_inode_table;           // Inodes begin block number
  u16  bg_free_blocks_count;     
  u16  bg_free_inodes_count;
  u16  bg_used_dirs_count;        
};



struct ext2_inode ******* INODE
{
  u16  i_mode;          // same as st_mode in stat() syscall
  u16  i_uid;                     // ownerID
  u32  i_size;                    // file size in bytes
  u32  i_atime;                   // access time
  u32  i_ctime;			  // creation time
  u32  i_mtime;			  // mod time
  u32  i_dtime;
  u16  i_gid;                     // groupID
  u16  i_links_count;             // link count
  u32  i_blocks;                  // IGNORE
  u32  i_flags;                   // IGNORE
  u32  i_reserved1;               // IGNORE
  u32  i_block[15];               // see below

//The fields i_block[15] record the disk blocks (numbers) of a file, which are 

    DIRECT blocks : i_block[0] to i_block[11], which point to direct blocks.
    INDIRECT block: I_block[12] points to a block, which contians 256 (ulong) 
                    block numbers.
    DOUBLE INDIRECT block: 
                    I_block[13] points to a block, which points to 256 blocks,
                    each of which point to 256 blocks.
    TRIPLE INDIRECT block: 
                    I_block[14] points to a block, which points to 256 blocks,
                    each of which points to 256 blocks, each of which points to
                    256 blocks.
}

struct ext2_dir_entry_2  *************** DIR
{
	u32  inode;        // Inode number; count from 1, NOT from 0
	u16  rec_len;      // This entry length in bytes
	u8   name_len;     // Name length in bytes
	u8   file_type;    // for future use
	char name[EXT2_NAME_LEN];  // File name: 1-255 chars, no NULL byte
};

*/


#define SUPERBLOCK        1 // Block#1: Super Block: (At byte offset 1024 in hard disk partitions) B1 is the Superblock, which is described by the super_block structure, where only the essential fields are explained. You may ignore the other fields.

#define GDBLOCK           2 // Block#2: Group Descriptor Block (in s_first_data_block+1 on hard disk) EXT2 divides disk blocks into GROUPs. Each group contains 8192 (contiguous) blocks. Each group is described by a group_desc structure of 32 bytes:

#define BBITMAP           3 // Block#3: Block Bitmap (Bmap): A bitmap is a sequence of bits used to represent some kind of items, e.g. disk blocks or inode structures. Bitmaps are used to allocate/deallocate items. In a bitmap, a 0 bit means the corresponding item is FREE, and a 1 bit means the corresponding item is in USE. A FD has 1440 blocks but block#0 is NOT used by the file system. So, the Bmap has only 1439 valid bits in B3. The exact values of Bmap will be shown later.

#define IBITMAP           4 // Block#4: Inode Bitmap (Imap) An inode is a data structure used to represent a file. A file system is created with a finite number of inodes. The status of the inodes are represented by a bitmap, called the Imap, in B4.  In an EXT2 FS, the first 10 inodes are reserved. Thus, the Imap of an empty FS starts with TEN 1's, followed by 0's. The Imap has only ninodes VALID bits. Invalid bits are set to 1, indicating that they are NOT available.

#define INODEBLOCK        5 // Block#5: Inodes (begin) Block In Unix, every file is represented by a UNIQUE inode structure of 128 bytes (256 bytes in EXT4), defined as follows.

#define BLOCK_SIZE        1024
#define BLKSIZE           1024

#define BITS_PER_BLOCK    (8*BLOCK_SIZE)
#define INODES_PER_BLOCK  (BLOCK_SIZE/sizeof(INODE))

#define ROOT_INODE        2
#define INODE_START_POS   10

#define DIR_MODE          0040777
#define FILE_MODE         0100644
#define SUPER_MAGIC       0xEF53
#define SUPER_USER        0

#define FREE              0
#define BUSY              1
#define READY             2
#define KILLED            3

#define NMINODE           64
#define NMOUNT            10
#define NPROC             10
#define NFD               10
#define NOFT              50

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;  

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

typedef struct Oft
{
  int   mode;
  int   refCount;
  struct Minode *inodeptr;
  long  offset;
} OFT;

typedef struct Proc
{
  int   uid; // uid = 0 for SUPERUSER; non-zero for ordinary user
  int   pid; // pid = 0 to NPROC-1 ----- process ID = first process = 1 [NOT BY INDEX]
  int   gid; 
  int   ppid; // parent pid 
  int   status; // PROC status (0-3 -> 0:FREE, 1:READY, 2:ZOMBIE, 3:SLEEP)

  struct Minode *cwd; // CWD pointer -> CWD INODE in memory 
  OFT   *fd[NFD];

  struct Proc *next;
  struct Proc *parent;
  struct Proc *child;
  struct Proc *sibling;
} PROC;



typedef struct Minode  // In-memory inodes structure
{
  INODE INODE; // disk inode    
  int   dev, ino; //device, inode id

  int   refCount; //dependents
  int   dirty; //used?
  int   mounted;
  struct Mount *mountptr;
  char     name[128];          
} MINODE;



typedef struct Mount
{
        int    ninodes;
        int    nblocks;
        int    dev, busy;
        struct Minode *mounted_inode;
        char   name[256];
        char   mount_name[64];
} MOUNT;

MINODE minode[NMINODE]; //stores all nodes
MINODE *root;
PROC   proc[NPROC], *running; //running is current process address; proc is max 10
MOUNT  mounttab[5];

char names[64][128],*name[64];
char third [64];
OFT OpenFileTable[NOFT];
int fd, dev, n; //file descriptor, dev = open(device_name, O_RDWR)
int nblocks, ninodes, bmap, imap, inode_start, iblock;
int inodeBeginBlock;
char pathname[256], parameter[256];
char teststr[1024] = "";
int DEBUG;
