
void my_cat(char *path)
{
                int n, i;
                int fd = 0;
                char buf[1024];
                int size = 1024;

                //check for path
                if(!path)
                {
                                printf("ERROR: Missing file name!\n");
                                return;
                }

                strcpy(third, "0");

                fd = open_file(path);
                my_pfd("");

                while((n = my_read(fd, buf, size)))
                {
                                buf[n] = '\0';
                                i = 0;
                                while(buf[i])
                                {
                                                putchar(buf[i]);
                                                if(buf[i] == '\n')
                                                                putchar('\r');
                                                i++;
                                }
                }

                printf("\n\r"); 
                close_file(fd);

                return;
}
