//*****Given*****
typedef unsigned int u32;
char *ctable = "0123456789ABCDEF";
int  BASE = 10;	// for decimal numbers
int rpu(u32 x)
{  
	char c;
	if (x)
	{
		BASE = 10;
		c = ctable[x % BASE];
       		rpu(x / BASE);
       		putchar(c);
    	}
}

int printu(u32 x) //unsigned
{
	BASE = 10;
	(x==0)? putchar('0') : rpu(x);
	putchar(' ');
}



//*****2.1 - DONE*****
int prints(char *s)
{
	char *c = s;
	while(*c)
	{
		putchar(*c++);
	}
}




//*****2.2 - DONE*****
int printd(int x) //integers - may be negative
{
	if (x == 0)
	{
		putchar('0');
	}
	else if ( x < 0)
	{
		putchar('-');
		BASE = 10;
		rpu(-x);
	}
	else
	{
		BASE = 10;
		rpu(x);
	}
}

int printo(u32 x)
{
	putchar('0');	//start with 0
	if (x == 0)
	{
			//nothing? another 0?
	}
	else
	{
		BASE = 8;
		rpu(x);
	}
}

int printx(u32 x)
{
	putchar('0');
	putchar('x');
	if (x == 0)
	{
		putchar('0');
	}
	else
	{
		BASE = 16;
		rpu(x);
	}	
}



//*****3.1 - DONE*****
int myprintf(char *fmt, ...) 	//3.4.1 from book
{
	char *cp = fmt; 	//pointer
	int *ip = &fmt + 1;	//item
	while (*cp)
	{
		if (*cp != '%')
		{
			putchar(*cp);
			if (*cp == '\n')
			{
				putchar('\r');
			}
			cp++; continue;
		}
		cp++;
		switch (*cp)
		{
			case 'c':putchar(*ip);	break;		//char
			case 's':prints((char*)(*ip));	break;	//strings
			case 'u':printu(*ip);	break;		//unsigned
			case 'd':printd(*ip);	break;		//int
			case 'o':printo(*ip);	break;		//octal
			case 'x':printx(*ip);	break;		//hex
		}
		cp++; ip++;
	}
}









