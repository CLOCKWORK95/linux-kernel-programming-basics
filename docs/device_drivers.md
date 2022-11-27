# Device Drivers

## Introduction: Device Driver, Device File, major and minor numbers
One class of module is the **device driver**.
On Unix, **each piece of hardware**, like a hard drive or a sound card, **is represented by a** file located in */dev* named a **device file**, which provides a way to communicate with the hardware. 
A sound card device driver might connect the /dev/sound device file to the specific sound card. 
A userspace program can use the device file without ever knowing what kind of sound card is installed.

Let’s have a look at some device files. 
Here are device files which represent the first three partitions on the primary master IDE hard drive:
```
$ ls -l /dev/hda[1-3]
brw-rw----  1 root  disk  3, 1 Jul  5  2000 /dev/hda1
brw-rw----  1 root  disk  3, 2 Jul  5  2000 /dev/hda2
brw-rw----  1 root  disk  3, 3 Jul  5  2000 /dev/hda3
```
Notice the column of numbers separated by a comma: 
- The first number is called the **device’s major number**. The major number **tells you which driver is used to access the hardware**.
Each driver is assigned a unique major number. **All device files with the same major number are controlled by the same driver**. 
All the above major numbers are '3', because they’re all controlled by the same driver.
- The second number is the **minor number**. The minor number is **used by the driver to distinguish between the various hardware it controls**. 
Returning to the example above, although all three devices are handled by the same driver they have unique minor numbers because the driver sees them as being different pieces of "the same" hardware.


## Character Devices and Block Devices
Devices are divided into two types: **character devices** and **block devices**. 
The difference is that **block devices have a buffer for requests**, so they can choose the best order in which to respond to the requests. 
This is important in the case of storage devices, where it is faster to read or write sectors which are close to each other, rather than those which are further apart. Another difference is that **block devices can only accept input and return output in blocks** (whose size can vary according to the device), whereas **character devices are allowed to use as many or as few bytes as they like**. 
**Most devices** in the world **are character**, because they don’t need this type of buffering, and they don’t operate with a fixed block size. 
You can tell whether a device file is for a block device or a character device by looking at the first character in the output of ls -l . 
If it is ‘b’ then it is a block device, and if it is ‘c’ then it is a character device. 
The devices you see above are block devices. Here are some character devices (the serial ports):
```
crw-rw----  1 root  dial 4, 64 Feb 18 23:34 /dev/ttyS0
crw-r-----  1 root  dial 4, 65 Nov 17 10:26 /dev/ttyS1
crw-rw----  1 root  dial 4, 66 Jul  5  2000 /dev/ttyS2
crw-rw----  1 root  dial 4, 67 Jul  5  2000 /dev/ttyS3
```

To create a new char device named coffee with major/minor number X and Y, simply do:
```
mknod /dev/coffee c X Y . 
```

You do not have to put your device files into /dev, but it is done by convention. 
However, when creating a device file for testing purposes, it is probably OK to place it in your working directory where you compile the kernel module. 

When a device file is accessed, the kernel uses the major number of the file to determine which driver should be used to handle the access. 
This means that the kernel doesn’t really need to use or even know about the minor number. 
The driver itself is the only thing that cares about the minor number. It uses the minor number to distinguish between different pieces of hardware.

"Hardware" can be something a bit more abstract than a PCI card that you can hold in your hand. 
Look at these two device files:
```
$ ls -l /dev/sda /dev/sdb
brw-rw---- 1 root disk 8,  0 Jan  3 09:02 /dev/sda
brw-rw---- 1 root disk 8, 16 Jan  3 09:02 /dev/sdb
By now you can look at these two device files and know instantly that they are block devices and are handled by same driver (block major 8). 
Sometimes two device files with the same major but different minor number can actually represent the same piece of physical hardware!
So just be aware that the word “hardware” in our discussion can mean something very abstract.
```

## Character Device Drivers
