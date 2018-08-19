all: balloc bmap gd ialloc imap inode super dir

balloc: balloc.c
	gcc -m32 -o balloc balloc.c -I. -g -w

bmap: bmap.c
	gcc -m32 -o bmap bmap.c -I. -g -w

gd: gd.c
	gcc -m32 -o gd gd.c -I. -g -w

ialloc: ialloc.c
	gcc -m32 -o ialloc ialloc.c -I. -g -w

imap: imap.c
	gcc -m32 -o imap imap.c -I. -g -w

inode: inode.c
	gcc -m32 -o inode inode.c -I. -g -w

super: super.c
	gcc -m32 -o super super.c -I. -g -w

dir: dir.c
	gcc -m32 -o dir dir.c -I. -g -w
