dd if=/dev/zero of=disk bs=1024 count=1440
mkfs -b 1024 disk 1440
mount -o loop disk /mnt
(cd /mnt; mkdir dir shortNameDir veryLongNameDir longNameDir; ls -a -l)
umount /mnt

