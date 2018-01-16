#include <stdio.h>

int *FP; //a global pointer
main(int argc, char *argv[], char *env[])
{
	int a, b, c;
	printf("enter main\n\n");

	printf("*****Helpful Things*****\n");
	printf("Address of argc ->%x\n", &argc);
	printf("Address of argv ->%x\n", argv);
	printf("Address of env ->%x\n\n", env);

	printf("Address of A -> %8x\n", &a);
	printf("Address of B -> %8x\n", &b);
	printf("Address of C -> %8x\n\n", &c);

	a=1; b=2, c=3;
	A(a,b);
	printf("exit main\n");
}
int A(int x, int y)
{
	int d,e,f;
	printf("enter A\n\n");

	printf("Address of D -> %8x\n", &d);
	printf("Address of E -> %8x\n", &e);
	printf("Address of F -> %8x\n\n", &f);

	d=4; e=5; f=6;
	B(d,e);
	printf("exit A\n");
}
int B(int x, int y)
{
	int u,v,w,i,*p;
	printf("enter B\n\n");

	printf("Address of U -> %8x\n", &u);
	printf("Address of V -> %8x\n", &v);
	printf("Address of W -> %8x\n\n", &w);

	u=7; v=8; w=9;
	asm("movl %ebp, FP"); //set FP=CPU's %ebp register

	//Write C code to Do (1)-(3) AS SPECIFIED BELOW

	//1 - DONE
	printf("*****Problem 1*****\n");	
	p = FP;
	while(p != 0)
	{
		printf("%8x -> ", p);
		p = *p;
	}
	printf("%8x\n\n", p);


	//2 - DONE

	i = -8;	
	FP -= 8;
	printf("*****Problem 2*****\n");
	while(i < 92)
	{
		printf("%8x     %8x\n", FP, *FP);
		FP++;
		i++;
	}

	
	printf("\nexit B\n");
}

