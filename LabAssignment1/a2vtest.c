#include "a2-myprint.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

typedef unsigned char u8;
typedef unsigned int u32;
int fd = 0, i = 0, nr = 0, count = 0;
struct partition *p, *ep;
char buf[512];
int x = 0;

struct partition
	{
		u8 drive;
		u8 head; 		//starting head
		u8 sector;		//starting sector
		u8 cylinder;		//starting cylinder
		u8 sys_type;		//partition type
		u8 end_head;		//end head
		u8 end_sector;		//end sector
		u8 end_cylinder;	//end cylinder
		u32 start_sector;	//starting sector counting from 8
		u32 nr_sectors;		//nr of sectors in partition
	};

char *dev = "vdisk";
int read(), lseek();



int print_raw(struct partition *p)
{
	for (i = 0; i < 4; i++)
	{
		printf("%2x ", p->drive);
		
		printf("%4d ", p->head);
		printf("%4d ", p->sector);
		printf("%4d ", p->cylinder);
		
		printf("%2x ", p->sys_type);

		printf("%4d ", p->end_head);
		printf("%4d ", p->end_sector);
		printf("%4d ", p->end_cylinder + 1);
	
		printf("%8d ", p->start_sector);
		printf("%8d ", p->nr_sectors);
		printf("\n");
		
		p++;
	}
}



int print_fdisk(struct partition *p)
{	
		printf("%4d	", p->cylinder + 1);
		printf("%8d   ", p->end_cylinder + 1);
		printf("%8d", p->nr_sectors);
		printf("\n");
}


int ext_part(struct partition *p, unsigned long base_sector)
{
	unsigned long s = 0;
	if (count == 3)
	{
		return;
	}
	printf("Relative Sector: %lu\n", p->start_sector);

	if (base_sector == 0)
	{
		base_sector = p->start_sector;
		s = p->start_sector;
	}
	else
	{
		s = p->start_sector + base_sector;
	}
	/*
	if((nr = read(fd, buf, sizeof(buf))) == -1)
	{
		perror("Read");
		exit(1);
	}
	*/


	printf("Absolute Sector: %lu\n", s);
	lseek(fd, (long)(s*512), 0);


	for (i = 0; i < 2; i++)
	{
		p = (struct partition *)(buf + 0x1BE + (16 * x));
		print_fdisk(p);
		x++;
	}
	
	count++;
	ext_part(p, base_sector);
}


int main(int argc, char *argv[])
{

	fd = open("vdisk", O_RDONLY);          // open disk iamge file for READ
	read(fd, buf, 512);                // read FIRST 512 bytes into buf[ ]
   	p = (struct partition *)(&buf[0x1BE]); // p points at Ptable in buf[ ]

   	// p->cylinder, p->sector, etc.


	printf("-----RAW FORM-----\n");
	print_raw(p);

	printf("\n-----LINUX FDISK FORM-----\n");
	printf("start_cyl   end_cyl   size\n");
	for (i = 0; i < 4; i++)
	{
		print_fdisk(p);
		if (p->sys_type == 5)
		{
			//extended partition
			printf("-----EXTENDED PARTITION-----\n");
			ext_part(p, 0);	
		}
		p++;
	}

	// Assume you want to read sector 10 (counting from 0)
   	//sector = 10;
   	//lseek(fd, (long)(sector*512), 0);     // seek to sector 10           
   	//read(fd, buf, 512);                   // read sector 10 into buf[ ], etc.

}















