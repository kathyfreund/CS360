
void init()
{
                int i;

                running = malloc(sizeof(PROC));

                proc[0].pid = 1;
                proc[0].uid = 0;
                proc[0].cwd = 0;

                proc[1].pid = 2; 
                proc[1].uid = 1; 
                proc[1].cwd = 0; 

                running = &proc[0];

                for(i = 0; i < 100; i++)
                {
                                minode[i].refCount = 0;
                }

                root = 0;
}

void mount_root(char device_name[64])
{
                char buf[1024]; 
                dev = open(device_name, O_RDWR); 

                if(dev < 0)
                {
                                printf("ERROR: Could not open %s!\n", device_name);
                                exit(0);
                }

                get_block(dev, SUPERBLOCK, buf); 
                sp = (SUPER *)buf; //super
                //EXT2 FS
                if(sp->s_magic != 0xEF53) 
                {
                                printf("NOT AN EXT2 FS!\n");
                                exit(1);
                }

                ninodes = sp->s_inodes_count; 
                nblocks = sp->s_blocks_count; 

                get_block(dev, GDBLOCK, buf); 
                gp = (GD *)buf; 

                imap = gp->bg_inode_bitmap; 
                bmap = gp->bg_block_bitmap; 

                inodeBeginBlock = gp->bg_inode_table; 
                root = iget(dev, 2); //root is always 2
                proc[0].cwd = root;
                proc[1].cwd = root;

                root->refCount = 3;

                printf("%s has been mounted sucessfully!\n", device_name);
}


int ls_file(MINODE *mip, char *name)
{
  int k;
  int count = 0;
  u16 mode, mask;
  char mydate[32], *s, *cp, ss[32];

  mode = mip->INODE.i_mode;
  if (S_ISDIR(mode))
      putchar('d');
  else if (S_ISLNK(mode))
      putchar('l');
  else
      putchar('-');


   mask = 000400;
   for (k=0; k<3; k++){
      if (mode & mask)
         putchar('r');
      else
         putchar('-');
      mask = mask >> 1;

     if (mode & mask)
        putchar('w');
     else
        putchar('-');
        mask = mask >> 1;

     if (mode & mask)
        putchar('x');
     else
        putchar('-');
        mask = mask >> 1;

     }
     printf("	%4d", mip->INODE.i_links_count);
     printf("	%4d", mip->INODE.i_uid);
     printf("  %4d", mip->INODE.i_gid);
     printf("  ");

     s = mydate;
     s = (char *)ctime(&mip->INODE.i_ctime);
     s = s + 4;
     strncpy(ss, s, 12);
     ss[12] = 0;

     printf("%s", ss);
     printf("%8ld",   mip->INODE.i_size);

     printf("    %s", name);

     if (S_ISLNK(mode))
        printf(" -> %s", (char *)mip->INODE.i_block);
     printf("\n");

}

int ls_dir(MINODE *mip)
{
  int i;
  char sbuf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  MINODE *dip;

  for (i=0; i<12; i++){ /* search direct blocks only */
     //printf("i_block[%d] = %d\n\n", i, mip->INODE.i_block[i]);
     if (mip->INODE.i_block[i] == 0) 
         return 0;

     get_block(mip->dev, mip->INODE.i_block[i], sbuf);
     dp = (DIR *)sbuf;
     cp = sbuf;
     //        printf("   i_number rec_len name_len   name\n");

     while (cp < sbuf + BLKSIZE){
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;
        //  printf("%8d%8d%8u        %s\n", 
        //        dp->inode, dp->rec_len, dp->name_len,temp);
	/************
        if (strcmp(temp, ".")==0 || strcmp(temp, "..")==0){
           cp += dp->rec_len;
           dp = (DIR *)cp;
	   continue;
	}
	************/
        dip = iget(dev, dp->inode); 
        ls_file(dip, temp);
        iput(dip);

        cp += dp->rec_len;
        dp = (DIR *)cp;
     }
  }
}

int ls(char *pathname)
{
  MINODE *mip;
  u16 mode;
  int dev, ino;
  printf("================================================================\n");

  if (pathname[0] == 0)
    ls_dir(running->cwd);
  else{
    dev = root->dev;
    ino = getino(dev, pathname);
    if (ino==0){
      printf("no such file %s\n", pathname);
      return -1;
    }
    mip = iget(dev, ino);
    mode = mip->INODE.i_mode;
    if (!S_ISDIR(mode))
      ls_file(mip, (char *)basename(pathname));
    else
      ls_dir(mip);
    iput(mip);
  }
  printf("=================================================================\n");
}

void cd(char *pathname)
{
                int ino = 0;

                MINODE *mip = running->cwd; 
                MINODE *newmip = NULL;

                if (!strcmp(pathname, "")) 
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
                ino = getino(mip, pathname); 
                if(ino == 0)
                {
                                printf("The directory %s does not exist.\n", pathname); 
                                return;
                }

                newmip = iget(dev, ino); 
                if(!S_ISDIR(newmip->INODE.i_mode))
                {
                                printf("%s is not a directory.\n", pathname); //can't cd to a file
				
                                iput(newmip);
                                return;
                }

                running->cwd = newmip; 
                iput(newmip); 
                return;
}

int quit()
{
                int i;

                for(i = 0; i < NMINODE; i++)
                {
                                if(minode[i].refCount > 0 && minode[i].dirty == 1)
                                {
                                                minode[i].refCount = 1;
                                                iput(&minode[i]);
                                }
                }
                printf("Exiting...\n");
                exit(0);
}
