//cat a file
void my_cat(char *path)
{
                int n, i;
                int fd = 0;
                char buf[1024];
                int size = 1024;

                //check for path, path would be the file name
                if(!path)
                {
                                printf("ERROR: Missing file name!\n");
                                return;
                }

                //make sure open mode is read
                strcpy(third, "0");

                //open with 0 for RD
                fd = open_file(path);
                my_pfd("");

                while((n = my_read(fd, buf, size)))
                {
                                //null terminate the buffer
                                buf[n] = '\0';
                                i = 0;
                                //print each char in the buffer, this is to handle \n
                                while(buf[i])
                                {
                                                putchar(buf[i]);
                                                if(buf[i] == '\n')
                                                                putchar('\r');
                                                i++;
                                }
                }

                printf("\n\r"); //add the termination characters in file
                close_file(fd); //call the close file command from close.c

                return;
}
