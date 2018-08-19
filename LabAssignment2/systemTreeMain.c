#include "systemTreeFunctions.c"

void main()
{
	initialize();
	while(quitting)
	{
		printf("- - - - - - - - - - - - - - -\n");
		//Clear globals?
		strcpy(line, "");
		strcpy(command, "");
		strcpy(pathname, "");	

		printf("Please input a command: \n");
		printf("NOTE: You can type in 'menu' any any time to display supported commands.\n - ");

		fgets(line, 128, stdin);
		sscanf(line, "%s %s", command, pathname);
		printf("Command: %s\nPathname (if applicable): %s\n\n", command, pathname);
		

		int ID = findCmd(command);		

		switch(ID)
		{
			case 0 :
				menu();
				break;
			case 1 :
				if(strcmp(pathname, "") == 0)
				{	
					printf("[!] ERROR: pathname required\n");
					break;
				}
				mkdir();//make new directory
				break;
			case 2 : 
				if(strcmp(pathname, "") == 0)
				{
					printf("[!] ERROR: pathname required\n");
					break;
				}
				rmdir();//rm directory if empty
				break;
			case 3 :
				ls();//list directory contents
				break;
			case 4 :
				cd();//change directory
				break;
			case 5 :
				pwd();//display pathname
				break;
			case 6 :
				if(strcmp(pathname, "") == 0)
				{
					printf("[!] ERROR: pathname required\n");
					break;
				} 
				creat();//create File node
				break;
			case 7 : 
				if(strcmp(pathname, "") == 0)
				{
					printf("[!] ERROR: pathname required\n");
					break;
				}
				rm();//rm a File node
				break;
			case 8 :
				if(strcmp(pathname, "") == 0)
				{
					printf("[!] ERROR: filename required\n");
					break;
				} 
				reload();//re-initialize file system
				break;
			case 9 : 
				if(strcmp(pathname, "") == 0)
				{
					printf("[!] ERROR: filename required\n");
					break;
				} 
				save();//save file system
				break;
			case 10 : 
				quit();//save file system, then terminate
				break;
			default:
				printf("[!] ERROR: Command does not exist\n");
		};
		printf("\n");	
	}

}
