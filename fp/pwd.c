/*
HOW TO pwd()?    pwd(running->cwd):

   pwd(MINODE *wd){
      if (wd == root) print "/"
      else
         rpwd(wd);
   }
   rpwd(MINODE *wd)
   {
     if (wd==root) return;
     from i_block[0] of wd->INODE: get my_ino, parent_ino
     pip = iget(dev, parent_ino);
     from pip->INODE.i_block[0]: get my_name string as LOCAL

     rpwd(pip);  // recursive call rpwd() with pip

     print "/%s", my_name;
   }
*/


int my_pwd()
{
        char path[128] = "";
        char temp[128] = "";
        char name[64] = "";
        int ino, parent_ino;
        MINODE *mip = running->cwd; //get cwd 

        if(mip == root) //root
        {
                printf("/\n"); //easy 
                strcpy(teststr, "/");
                return;
        }

        while(mip != root) //rpw - recursive until root is found
        {
                findino(mip, &ino, &parent_ino);//find ino
                mip = iget(dev, parent_ino);
                findmyname(mip, ino, name); //find name

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

