#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

struct node
{
	char name[64];
	char type;
	struct node *childPtr, *siblingPtr, *parentPtr;

}typedef Node;


Node *root = NULL, *cwd = NULL, *curr = NULL;

char line[128];
char command[16], pathname[64];
char dir_name[64], base_name[64];
int quitting = 1;

char *cmd[] = {"menu", "mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm", "reload", "save", "quit", 0};


char *filepwd()
{
	Node *foo = malloc(sizeof(Node));
	Node *foo2 = malloc(sizeof(Node));
	int flag = 0;
	char string1[128], string2[128] = "/0";
	foo = cwd;
	foo2 = cwd;
	while(foo->parentPtr != NULL)
	{
		strcpy(string1, foo->name);
		if(flag == 1)
		{
			strcat(string1, "/");
		}
		strcat(string1, string2);
		strcpy(string2, string1);
		foo = foo->parentPtr;
		flag = 1;
	}
	strcpy(string1, foo->name);
	strcat(string1, string2);
	strcpy(string2, string1);
	if(foo2->type == 'D')
	{
		strcpy(string2, "D");
		strcat(string2, " ");
		strcat(string2, string1);
		strcpy(string1, string2);
	}	
	else
	{
		strcpy(string2, "F");
		strcat(string2, " ");	
		strcat(string2, string1);
		strcpy(string1, string2);
	}
	printf("%s", string1);
	printf("\n");	
	return string1;
}


void divide()
{
	char *s = strtok(pathname, "/");
	char *last;
	while(s)
	{
		last = s;
		s = strtok(NULL, "/");

		if(s)
		{
			strcat(dir_name, last);
			strcat(dir_name, "/");
		}
		else
		{
			strcpy(base_name, last);
		}
	}	
}


void initialize()
{
	root = malloc(sizeof(Node));
	strcpy(root->name, "root");
	//do i need to initialize children if 
	//there's nothing there to begin with?
	root->childPtr = NULL;
	root->siblingPtr = NULL;
	root->parentPtr = NULL;

	root->type = 'D';

	cwd = root;
}




int findCmd(char *command)
{
	int i = 0;
	while(cmd[i])
	{
		if(strcmp(command, cmd[i]) == 0)
		{
			return i;
		}
		i++;
	}
	return -1;
}



/////////////////////////////////////////////DONE
void menu()
{
	printf("Here are the available commands:\n");
	printf("NOTE: pathnames marked with [] are optional\n\n");
	printf("mkdir pathname :	make a new directory\n");
	printf("rmdir pathname :	remove directory if empty\n");
	printf("cd [pathname] :		change CWD to pathname, or to / if no pathname\n");
	printf("ls [pathname] : 	list directory contents (pathname or CWD)\n");
	printf("pwd :			print (absolute) pathname of CWD\n");
	printf("creat pathname :	create a FILE node\n");
	printf("rm pathname:		rempve a FILE node\n");
	printf("save filename : 	save the current file system tree in a file\n");
	printf("reload filename :	re-initialize the file system tree from a file\n");
	printf("quit :			save the file system tree, then terminate\n\n");
}




/////////////////////////////////////////////DONE


void mkdir()
{
	//break up pathname into dirname and basename
	divide();

	if(pathname[0] == '/') //absolute
	{
		char *temp = strdup(dir_name);
		strcpy(dir_name, "/");
		strcat(dir_name, temp);
		curr = root;
	}
	else //relative
	{
		curr = cwd;
	}


	//search for dirname node
	printf("dir_name->%s, and base_name->%s\n", dir_name, base_name);
	char *s = strtok(dir_name, "/");
	while(s)
	{
		curr = curr->childPtr;
		
		while(curr != NULL)
		{
			if(strcmp(curr->name, s) == 0) //found
			{
				if(curr->type == 'F') //file, not directory
				{
					printf("ERROR: %s IS A FILE TYPE\n", s);
					return;
				}
				break;
			}
			else
			{
				curr = curr->siblingPtr;
			}
		}
		if(curr == NULL)
		{
			printf("ERROR: %s Directory does not exist\n", s);
			return;
		}
		s = strtok(NULL, "/");
	}

	//if dirname exists and is a dir
		//search for basename

	Node *foo = curr->childPtr; //traverse through siblings
	while(foo != NULL)
	{
		if (strcmp(foo->name, base_name) == 0)
		{
			printf("ERROR: %s Directory already exists", s);
			return;
		}
		foo = foo->siblingPtr;
	}

	//add new dir
	Node *newD = malloc(sizeof(Node));
	strcpy(newD->name, base_name);
	newD->type = 'D';
	newD->parentPtr = curr;
	newD->siblingPtr = NULL;
	newD->childPtr = NULL;

	if(curr->childPtr == NULL) //current has no child - newD only child
	{
		curr->childPtr = newD;
	}
	else //current already has a child - newD has a sibling(s)
	{
		curr = curr->childPtr;
		while(curr->siblingPtr != NULL)
		{
			curr = curr->siblingPtr;
		}
		curr->siblingPtr = newD; //add it to the list of siblings
	}

	printf("[!] New Directory '%s' created\n", pathname);

}




/////////////////////////////////////////////DONE
void rmdir()
{
	//separate dname and bname
	divide();

	if(pathname[0] == '/')
	{
		char *temp = strdup(dir_name);
		strcpy(dir_name, "/");
		strcat(dir_name, temp);
		curr = root;
	}
	else
	{
		curr = cwd;
	}

	char *s = strtok(dir_name, "/");
	while(s) //search
	{
		curr = curr->childPtr;
		
		while(curr != NULL)
		{
			if(strcmp(curr->name, s) == 0) //found
			{
				if(curr->type == 'F') //file, not directory
				{
					printf("ERROR: %s IS A FILE TYPE\n", s);
					return;
				}
				break;
			}
			else
			{
				curr = curr->siblingPtr;
			}
		}
		if(curr == NULL)
		{
			printf("ERROR: %s Directory does not exist\n", s);
			return;
		}
		s = strtok(NULL, "/");
	}


	Node *foo = curr->childPtr; //go to basename

	if(foo->childPtr != NULL) //check if empty
	{
		printf("ERROR: Directory is not empty");
		return;
	}

	Node *last;
	int del = 0;

	while(del == 0)
	{
		if(strcmp(foo->name, base_name) == 0) 
		{
			if(foo->siblingPtr != NULL) //sibling present
			{
				if(last != NULL)//middle
				{
					last->siblingPtr = foo->siblingPtr;
				}
		 		else//first
				{
					curr->childPtr = foo->siblingPtr;
				}
				if(last != NULL)
				{
					
				}
				if(cwd == foo)
				{
					cwd = root;
				}
				return;
			}
			else //no siblings - KILL
			{
				curr->childPtr = NULL;
				printf("Killing off %s's child", curr->name);
				if(cwd == foo)
				{
					cwd = root;
				}
				return;
			}
		}
		else
		{
			last = foo;
			foo = foo->siblingPtr;
		}
	}
	printf("[!] Directory '%s' removed\n", pathname);

}




/////////////////////////////////////////////DONE
void ls()
{
	if(cwd == root)
	{
		printf("/\n");
	}	

	Node *foo = cwd;
	foo = foo->childPtr;

	while(foo != NULL)
	{
		printf("%s", foo->name);
		printf("	");
		foo = foo->siblingPtr;
	}
}


/////////////////////////////////////////////DONE
void cd()
{
	int found = 0;
	curr = cwd;
	divide();

	if(pathname[0] == '/') //absolute
	{
		char *temp = strdup(dir_name);
		strcpy(dir_name, "/");
		strcat(dir_name, temp);
		curr = root;
	}
	else //relative
	{
		curr = cwd;
	}

	if(strcmp(pathname, "/") == 0) //no path
	{
		cwd = root;
	}
	else
	{
		char *s = strtok(pathname, "/");
		while(s) //search
		{
			curr = curr->childPtr;
		
			while(curr != NULL)
			{
				if(strcmp(curr->name, s) == 0) //found
				{
					if(curr->type == 'F') //file, not directory
					{
						printf("ERROR: %s IS A FILE TYPE\n", s);
						return;
					}
					break;
				}
				else
				{
					curr = curr->siblingPtr;
				}
			}
			if(curr == NULL)
			{
				printf("ERROR: %s Directory does not exist\n", s);
				return;
			}
			s = strtok(NULL, "/");
		}
		if((cwd->name, base_name) != 0)
		{
			cwd = curr->childPtr;
		}
		cwd = curr;
	}
	printf("[!] Directory changed to '%s'\n", cwd->name);
}



/////////////////////////////////////////////DONE
void rpwd(Node *p)
{
	if(strcmp(p->name, "root") == 0)
	{
		printf("root");
		return;
	}
	else
	{	
		if(p != root)
		{
			rpwd(p->parentPtr);
		}	
		printf("/%s", p->name);
		return;
	}
}
void pwd()
{

	rpwd(cwd);
	printf("\n");
}





/////////////////////////////////////////////DONE
void creat()
{	

	divide();	

	if(pathname[0] == '/')
	{
		char *temp = strdup(dir_name);
		strcpy(dir_name, "/");
		strcat(dir_name, temp);
		curr = root;
	}
	else
	{
		curr = cwd;
	}	

	printf("pathname->%s\n", pathname);
	char *s = strtok(dir_name, "/");
	printf("s->%s\n", s);
	while(s) //search
	{
		curr = curr->childPtr;
		while(curr != NULL)
		{
			if(strcmp(curr->name, s) == 0) //found
			{
				if(curr->type == 'F') //file, not directory
				{
					printf("ERROR: %s IS A FILE TYPE\n", s);
					return;
				}
				break;
			}
			else
			{
				curr = curr->siblingPtr;
			}
		}
		if(curr == NULL)
		{
			printf("ERROR: %s Directory does not exist\n", s);
			return;
		}
		s = strtok(NULL, "/");
	}	

	//s = strtok(0, "\n"); //file to create

	Node *newF = malloc(sizeof(Node));
	strcpy(newF->name, base_name);
	newF->type = 'F';
	newF->parentPtr = curr;
	newF->siblingPtr = NULL;
	newF->childPtr = NULL;
	
	if(curr->childPtr != NULL) //current already has a child - newF has a sibling(s)
	{
		curr = curr->childPtr;
		while(curr->siblingPtr!= NULL)
		{
			curr = curr->siblingPtr;
		}
		curr->siblingPtr = newF; //add it to the list of siblings
	}
	else
	{
		curr->childPtr = newF;
	}
	printf("[!] File '%s' created\n", newF->name);

}




/////////////////////////////////////////////DONE
void rm()
{
	divide();	

	if(pathname[0] == '/')
	{
		char *temp = strdup(dir_name);
		strcpy(dir_name, "/");
		strcat(dir_name, temp);
		curr = root;
	}
	else
	{
		curr = cwd;
	}	

	char *s = strtok(dir_name, "/");
	while(s) //search
	{
		curr = curr->childPtr;
		while(curr != NULL)
		{
			if(strcmp(curr->name, s) == 0) //found
			{
				if(curr->type == 'F') //file, not directory
				{
					printf("ERROR: %s IS A FILE TYPE\n", s);
					return;
				}
				break;
			}
			else
			{
				curr = curr->siblingPtr;
			}
		}
		if(curr == NULL)
		{
			printf("ERROR: '%s' Directory does not exist\n", s);
			return;
		}
		s = strtok(NULL, "/");
	}

	Node *foo = curr->childPtr;
	Node *temp;
	int found = 0;

	while(found == 0)
	{
		if(strcmp(base_name, foo->name) == 0) //match - remove
		{
			if(foo->type == 'D')
			{
				printf("ERROR: '%s' is a directory\n", foo->name);
			}
			else if(curr->childPtr != foo) //has siblings
			{
				temp = foo->parentPtr;
				temp->siblingPtr = foo->siblingPtr;
				foo->parentPtr = NULL;
				found = 1;
				break;
			}
			else
			{
				curr->childPtr = NULL;
				foo->parentPtr = NULL;
				temp = foo->siblingPtr;
				if(temp != NULL)
				{
					curr->childPtr = temp;
				}
				foo->siblingPtr = NULL;
				found = 1;
				break;
			}
		}	
		foo = foo->siblingPtr;
	}
	if(found == 0)
	{
		printf("ERROR: File name %s does not exist\n", s);
		return;
	}
	printf("[!] File '%s' deleted\n", base_name);
}


/////////////////////////////////////////////DONE
void reload()
{
	FILE *f = fopen(pathname, "r");
	char string1[128];
	int flag = 0;
	if(strcmp(pathname, "") == 0)
	{
		printf("ERROR: No file name given.\n");
	}
	if(f == NULL)
	{
		printf("ERROR: file cannot be opened.\n");
		return;
	}
	while (!feof(f))
	{
		memset(line, 0, strlen(line));
		memset(command, 0, strlen(command));
		memset(pathname, 0, strlen(pathname));
		memset(dir_name, 0, strlen(dir_name));
		memset(base_name, 0, strlen(base_name));
		fscanf(f, "%s", string1);
		if(strcmp(string1, "") == 0)
		{
			return;
		}
		if(strcmp(string1, "D") == 0)
		{
			fscanf(f, "%s", string1);
			if (strcmp(string1, "/") != 0)
			{
				strcpy(pathname, string1);
				mkdir();
			}
		}
		else if(strcmp(string1, "F") == 0)
		{
			fscanf(f, "%s", string1);
			strcpy(pathname, string1);
			creat();
		}
		printf("\n");
	}
	fclose(f);
}


/////////////////////////////////////////////DONE
void save()
{
	Node *foo = malloc(sizeof(Node));
	FILE *f = fopen(pathname, "w+");
	char string1[128];
	int flag = 0;
	if(strcmp(pathname, "") == 0)
	{
		printf("ERROR: no file name given\n");
	}
	if(f == NULL)
	{
		printf("ERROR: file cannot be opened\n");
		return;
	}

	foo = cwd;
	cwd = root;
	while(cwd != NULL)
	{
		if(cwd->childPtr != NULL && flag == 0)
		{	
			strcpy(string1, filepwd());
			fprintf(f, "%s\n", string1);
			cwd = cwd->childPtr;
		}
		else if(cwd->siblingPtr != NULL)
		{
			if(flag == 0)
			{
				strcpy(string1, filepwd());
				fprintf(f, "%s\n", string1);
			}
			cwd = cwd->siblingPtr;
			flag = 0;
		}
		else
		{
			if(flag == 0)
			{
				strcpy(string1, filepwd());
				fprintf(f, "%s\n",  string1);
			}
			cwd = cwd->parentPtr;
			flag = 1;
		}
	}	
	fclose(f);
	cwd = foo;
}



/////////////////////////////////////////////DONE
void quit()
{
	quitting = 0;
}


















