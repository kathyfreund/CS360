
void chmod_file(char path[124])
{
        int ino = 0;
        int newmode = 0;
        MINODE *mip = running->cwd;
        INODE *ip = NULL; 

        if (!strcmp(third, ""))
        {
                printf("No mode given!\n");
                return;
        }
        //converts to octal int
        newmode = (int) strtol(third, (char **)NULL, 8);

        printf("Path = %s\n", path);
        ino = getino(mip, path);

        if (ino)
                mip = iget(dev, ino);
        else
        {
                strcpy(third, "");
                return;
        }
        ip = &mip->INODE;

        ip->i_mode = (ip->i_mode & 0xF000) | newmode;

        mip->dirty = 1;
        iput(mip);
        strcpy(third, "");
}
