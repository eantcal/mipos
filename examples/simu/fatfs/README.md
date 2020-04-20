You can create a blank disk image by typing the following command:

```
  mkfs.msdos -C disk.img 100
```

where 100 is the size in KB

On some systems /mkfs.msdos is under /sbin directory. 
Afterwards you can mount it as loopback device by using the following commands:

```
  sudo mkdir -p /media/mydisk
  sudo mount -o loop,rw,sync ./disk.img /media/mydisk/
```

To unmount, you can use the command

```
  sudo umount /media/mydisk/
```

