# Overview

In this assignment, we will write a Linux kernel module called lexus. Note that, you will only be able to test this assignment on a Linux machine where you have root privilege. A VMware-based CentOS 7 (64 bit) VM image is provided. Later on we will refer to this VM as the cs452 VM (username/password: cs452/cs452, run commands with sudo to gain root privilege). You can also download a CentOS 7 (64 bit) and install it by yourself, and you can also use VirtualBox.

## Important notes

You MUST build against the kernel version (3.10.0-957.el7.x86_64) installed on the cs452 VM. You will need to use root in this project is to load and unload the kernel module.

# Specification

Your will develop a lottery scheduler in a Linux system. Your scheduler will work as a kernel module.
