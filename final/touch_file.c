
void touch_file(char path[124])
{
        int ino;
        int newmode = 0;
        MINODE *mip = running->cwd;
        MINODE *touchmip = NULL;
        INODE *ip = NULL;
        char fullpath[128];
        strcpy(fullpath, path);

        //checks
        if (!strcmp(path, "")) 
        {
                printf("No file name given!\n");
                return;
        }

        printf("Path = %s\n", path);
        ino = getino(running->cwd, path);
        if (ino != 0) //exists
        {
                printf("The target exists, touching...\n");
                touchmip = iget(dev, ino);
                ip = &touchmip->INODE;

                ip->i_mtime = time(0L);
                touchmip->dirty = 1;

                iput(touchmip);
                return;
        }
        else //creat file
        {
                printf("The target does not exists, creating file...\n");
                creat_file(fullpath);
        }

        return;
}
