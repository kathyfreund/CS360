
int my_pwd()
{
        char path[128] = "";
        char temp[128] = "";
        char name[64] = "";
        int ino, parent_ino;
        MINODE *mip = running->cwd; 

        //check if root
        if(mip == root)
        {
                printf("/\n");
                strcpy(teststr, "/");
                return;
        }

        while(mip != root)
        {
                findino(mip, &ino, &parent_ino);
                mip = iget(dev, parent_ino);
                findmyname(mip, ino, name);

                strcpy(temp, path);
                strcpy(path, "");
                strcat(path, "/");
                strcat(path, name);
                strcat(path, temp);
        }
        printf("%s\n", path);
        strcpy(teststr, path);

        return 1;
}
