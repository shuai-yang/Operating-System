## Project Number/Title 

* Author: Shuai Yang
* Class: CS452/CS552 [Operating Systems] Section #01

## Overview

I implemented a Linux kernel module called lincoln which serves as a keyboard device driver. Please note that my program also supports the left shift key.  

## Manifest

--- lincoln.c</br>
--- Makefile</br>
--- linux-input.jpg</br>
--- 8042.png</br>
--- vnc-rdv.png</br>
--- vnc-tiger.png</br>

I only modified _lincoln.c_. Those _.jpg_ and _.png_ photos are snapshots of background and testing performed on QEMU/KVM virtual machine hosted on onyx nodes. 

## Building the project

To build the program, run the following commands:</br>
make</br>
sudo insmod lincoln.ko </br>
sudo rmmod lincoln </br>
 
## Features and usage
An special feature of this program is to hijack the default keyboard driver using our interrupt handler, so the default one will not be called. Specifically, when the user types  **l**, it should be interpreted as **s**, and displayed as **s**; when the user types **s**, it should be interpreted as **l**, and displayed as **l**. Another feature is this driver not only supports lower case letters, but also capital letters using **Left SHIFT** key. 

## Testing

There are two parts of testing. </br>

First part is testing the special features mentioned above and makes sure that implementation doesn't impact the normal behaviors of other keys. Note that in order to watch the required messages printed in the kernel log, I opened another terminal window and ran **sudo tail -f /var/log/messages** before loading the module. Once the module was loaded, the **ls** command seen on terminal was actually the output of my typing **sl** from keyboard. Then I typed **ls** and I saw **sl**. After that to make sure other keys work as normal, I tested commands **pwd**, **who**, **whoami**. 

Second part is testing the keyboard disabling, enabling, and resetting using the specific echo commands. Note that I created a batch script named run.sh to run these commands, where the program firstly ran Disable and lasted for 3 seconds until Enabled. The command to run my script is **./run.sh**. Pay attention that as module was loaded, I actually typed **./run.lh** to see this command. Once the keyboard was enabled again after 3 seconds, I ran command **sudo rmmod lincoln** by actually typing  **ludo rmmod sincosn** to remove the module. Once the moduel was removed, I really typed **lsmod** and saw **lsmod** to check the status of modules in the Linux kernel.</br>

Below is the snapshot of my test results and the required messages printed in the kernel log.</br> 
![](./test_result.PNG)

## Known Bugs

None

## Reflection and Self Assessment

On Wednesday night, when I decided to start coding for this project, my VM didn't work at all unexpectedly. I rebooted many times but still couldn't make it running. so I decided to reinstall the VM again and luckily the new VM works and even performs faster than the old one. Although I started the program not earlier, I am not worried too much because during the class, I've already thought about how to solve the program in my mind, I knew which part is the most challenging to me, and I estimated whether or not I could finish it in 1-2 days if I am really fully focused right before the due date. For this project which I started my first line of code last night and now I finished the project in 24 hours. There are two classes each week and I attended both. That means before I started writing the first line of code on terminal, I've already spent at least 2.5 hours fully focused on this project already and wrote lines of pseudo code on my notes. Therefore, I think I am a person who can even perform better under pressure with high-level concentration driven by a strong internal motivation and determination. Stress doesn't scare me but a kind of friend of me at some extent because that drives me be highly-concentrated on one thing during a period of time. </br>

I began to be interested in OS particular computer security via taking this course and being a Kernel Software Engineer probably in the future. Many projects are kind of related to security topic and really practical. I am glad that I have one more option of linux kernel programming no matter for professional career or for academic research in the future than those who didn't take this course. 

## Sources Used

class notes </br>

