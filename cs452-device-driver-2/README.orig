# Overview

In this assignment, we will write a Linux kernel module called lincoln. This module will serve as a keyboard device driver. You should still use the cs452 VM (username:cs452, password: cs452) which you used for your tesla, lexus, infiniti, and toyota, as loading and unloading the kernel module requires the root privilege. 

## Learning Objectives

- Learning how to interact with an I/O device.
- Gaining a better understanding of interrupts, and interrupt handlers.
- Understanding the difference between polling and interrupts.

## Important Notes

You MUST build against the kernel version (3.10.0-1160.el7.x86_64), which is the default version of the kernel installed on the cs452 VM.

In this README, you will see the term **host**. It is used to refer to the computer to which the keyboard is connected.

## Book References

Operating Systems: Three Easy Pieces: [I/O Devices](https://pages.cs.wisc.edu/~remzi/OSTEP/file-devices.pdf).

This textbook chapter explains what roles I/O devices play in a computer system, and how device drivers work in general. In particular, it describes how an operating system interacts with I/O devices, and how interrupts work and why interrupts can lower the CPU's overhead. The chapter also explains what an interrupt handler is - in this assignment, a part of your job is to implement an interrupt handler for a PS/2 keyboard, which is the default keyboard used in the provided virtual machine.

## Background

### The Linux Input Subsystem

According to chapter 7 (Input Drivers) of the book "Essential Linux Device Drivers", written by Sreekrishnan Venkateswaran, the Linux kernel has an input subsystem which "was created to unify scattered drivers that handle diverse classes of data-input devices such as keyboards, mice, trackballs, joysticks, roller wheels, touch screens, accelerometers, and tablets." The input subsystem looks like this:

![alt text](linux-input.jpg "The Linux Input Subsystem")

The chapter says the above figure "illustrates the operation of the input subsystem. The subsystem contains two classes of drivers that work in tandem: event drivers and device drivers. Event drivers are responsible for interfacing with applications, whereas device drivers are responsible for low-level communication with input devices. The mouse event generator, mousedev, is an example of the former, and the PS/2 mouse driver is an example of the latter. Both event drivers and device drivers can avail the services of an efficient, bug-free, reusable core, which lies at the heart of the input subsystem."

You are recommended to read the chapter, but to briefly explain this figure and relate it to this assignment: the part we are working on is a keyboard device driver, which belongs to the **input device drivers** class, and we will interact with the keyboard event driver, which is already existing in the Linux kernel and belongs to the **input event drivers** class. An input device driver does not directly interact with applications, rather it interacts with an input event driver, and that input event driver interacts with applications. Therefore, when a key press/release event occurs, we, as the keyboard device driver, capture this event and report this event to the keyboard event driver. See the [Reaction to Ordinary Scan Codes](#reaction-to-ordinary-scan-codes) section for the APIs we can use to report events.

### The Intel 8042 Controller

The provided virtual machine has an (emulated) Intel 8042 controller, which allows a PS/2 keyboard and a PS/2 mouse to connect to the machine.

**side note**: it is my own understanding that most laptops contain an Intel 8042 controller and the keyboard comes with the laptop is considered as a PS/2 keyboard. Correct me if this is not the case on your laptop. However, no matter you are using a PS/2 keyboard or not, it should not affect you complete this assignment, because the provided virtual machine would always treat your keyboard as a PS/2 keyboard. Even if your real keyboard is not a PS/2 keyboard, the virtual machine will make it act like a PS/2 keyboard.

The following picture, downloaded from the [osdev.org](https://wiki.osdev.org/%228042%22_PS/2_Controller) website, shows the structure of an Intel 8042 controller.

![alt text](8042.png "Intel 8042 Controller")

As we can see from the picture, the PS/2 keyboard has two I/O ports, whose addresses are 0x60 and 0x64. You can interpret these two ports as two 8-bit registers. 0x60 represents the data register, a register which allows us to transfer data to and from the keyboard. 0x64 represents the status register, which also has 8 bits. A PS/2 keyboard typically has two buffers, input buffer and output buffer. And certain bits of the status register tells us the status of these two buffers, more specifically:

1. bit 0 of this status register indicates the output buffer status (0=empty, 1=full), when this bit is 1, it means there is something for us to read from the data register.
2. bit 1 of this status register indicates the input buffer status (0=empty, 1=full), when this bit is 0, it means the input buffer has space, and writing data to the data register is therefore allowed.

As the textbook chapter describes: "By reading and writing these registers, the operating system can control device behavior".

# The Starter Code

The starter code looks like this:

```console
[cs452@xyno cs452-device-driver-2]$ ls
8042.png  lincoln.c  linux-input.jpg  Makefile  README.md  README.template
```

You will be completing the lincoln.c file.

The starter code already provides you with the code for a kernel module called lincoln. To install the module, run *make* and then *sudo insmod lincoln.ko*; to remove it, run *sudo rmmod lincoln*. Yes, in rmmod, whether or not you specify ko does not matter; but in insmod, you must have that ko.

## The Proc Interface

The starter code creates a proc interface for users to communicate with the keyboard. More specifically, when the module is loaded, a file called */proc/lincoln/cmd* will be created. Users can write commands into this file, and the commands are expected to pass to the keyboard, via a function you are going to implement - *lincoln_kbd_write*().

## Intercepting the Default Keyboard Interrupt Handler

The starter code also hijacks the default keyboard driver, so that when an interrupt occurs, the default interrupt handler will not be called, rather, it is an interrupt handler that you are going to implement - *lincoln_irq_handler*(), which will be called.

# Specification

You are required to implement the following functions.

```c
static int lincoln_kbd_write(struct serio *port, unsigned char c);
```

this function writes a single byte to the keyboard. It should return 0 if the writing is successful, and return -1 if the writing is not successful. The first parameter *port* will not be used in this assignment.

A PS/2 keyboard supports 17 host-to-keyboard commands. For example, command *0xf5* means disabling the keyboard, and command *0xf4* means enabling the keyboard, and command 0xff means resetting the keyboard. In this assignment, we want to allow the user to disable, enable and reset the keyboard. More specifically, when the user runs this:

```console
# sudo echo 'D' > /proc/lincoln/cmd
```

we need to disable the keyboard. In other words, we want to send the *0xf5* command to the keyboard - write this 0xf5 to the data register. The expected effect of this command is, the keyboard will become unresponsive.

And then when the user runs this:

```console
# sudo echo 'E' > /proc/lincoln/cmd
```

we need to enable the keyboard. In other words, we want to send the *0xf4* command to the keyboard - write this 0xf4 to the data register. The expected effect of this command is, the keyboard will become responsive again.

**Question**: when the keyboard is not responsive at all, how can the user even enter the second command - which enables the keyboard?

And then when the user runs this:

```console
# sudo echo 'R' > /proc/lincoln/cmd
```

we want to keyboard to reset. In other words, we want to send the *0xff* command to the keyboard - write this 0xf4 to the data register. The expected effect of this command is, the keyboard will first respond with "ack" (0xfa), and then it will perform a BAT test. When the BAT test is complete, the keyboard will send either 0xAA (BAT successful) or 0xFC (Error) to the host. Keep reading this README and you will soon find out what BAT is.

**Note**: the starter code is implemented in such a way that when the user run the above *sudo echo* commands, your *lincoln_kbd_write*() will get called, and the command is passed as the second parameter of your function, i.e., *unsigned char c*, which is one byte. Your *lincoln_kbd_write*() achieves its goal of writing the command to the keyboard, via writing this one byte command into the keyboard's data register, which, as introduced above, is located at address 0x60.

```c
static irqreturn_t lincoln_irq_handler(struct serio *serio, unsigned char data, unsigned int flags);
```

this is the interrupt handler. Every time the keyboard raises an interrupt, this function will get called. There are two situations when a keyboard raises an interrupts:

1. User input. This is the most obvious reason. As the computer user, you type something from the keyboard, the keyboard needs to send a code (known as a scan code) corresponding to the key (you just pressed or released) to the upper layer of the system, and eventually the application will receive that key. Here, the second parameter of the interrupt handler, which is *data*, stores the scan code.

2. Sometimes the user does not input anything, but the keyboard may still want to tell the CPU that something is happening. In this case, the keyboard also produces a scan code, which is known as a protocol scan code - in contrast, a scan code produced in the above situation is called an ordinary scan code. Still, the second parameter of the interrupt handler, which is *data*, stores the scan code. Some examples of the protocol scan code include:

  - 0xaa. This is called the BAT successful code. When you boot your computer, the keyboard performs a diagnostic self-test referred to as BAT (Basic Assurance Test) and configure the keyboard to its default values. When entering BAT, the keyboard enables its three LED indicators, and turns them off when BAT has completed. At this time, a BAT completion code of either 0xAA (BAT successful) or 0xFC (Error) is sent to the host. Besides power-on, a software reset would also trigger the keyboard to perform the BAT test.

  - 0xfa. This is called the acknowledge code, or "ack" code. When the host sends a command to the keyboard, the keyboard may respond with an **ack** code, indicating the command is received by the keyboard.

A typical keyboard also defines other protocol scan codes. In this assignment, your interrupt handler needs to handle protocol scan codes, as well as ordinary scan codes.

In total we have the following protocol scan codes defined:

```c
#define ATKBD_RET_ACK           0xfa	/* Acknowledge from kbd */
#define ATKBD_RET_NAK           0xfe	/* Keyboard fails to ack, please resend */
#define ATKBD_RET_BAT           0xaa	/* BAT (Basic Assurance Test) OK */
#define ATKBD_RET_EMUL0         0xe0	/* The codes e0 and e1 introduce scancode sequences, and are not usually used as isolated scancodes themselves */
#define ATKBD_RET_EMUL1         0xe1	/* */
#define ATKBD_RET_RELEASE       0xf0	/* */
#define ATKBD_RET_HANJA         0xf1	/* Some keyboards, as reply to command */
#define ATKBD_RET_HANGEUL       0xf2	/* */
#define ATKBD_RET_ERR           0xff	/* Keyboard error */
```

**Special Requirement**: Your interrupt handler must achieve this: when the user types every key other than *l* or *s*, the user should observe normal behaviors; but when the user types *l*, it should be interpreted as *s*, and displayed as *s*; when the user types *s*, it should be interpreted as *l*, and displayed as *l*. Note: the scan code of *l* is 0x26, the scan code of *s* is 0x1f.

## Reaction to Protocol Scan Codes

When the keyboard produces a protocol scan code, the interrupt handler should react as following:

1. if the produced protocol scan code is ATKBD_RET_BAT, print a message to the kernel log saying:"keyboard reset okay.", and return **IRQ_HANDLED**.
2. if the produced protocol scan code is ATKBD_RET_ACK, print a message to the kernel log saying:"we get an ACK from the keyboard.", and return **IRQ_HANDLED**.
3. for all the other protocol scan codes, your interrupt handler can just return **IRQ_HANDLED**.

## Reaction to Ordinary Scan Codes

When the keyboard produces an ordinary scan code, the interrupt handler should pass it to the keyboard event driver, which will then pass it to the corresponding application. Passing the ordinary scan code to the keyboard event driver is considered as reporting an event. And to report events to the event drivers layer (which includes the keyboard event driver and other event drivers), you can call *input_event*() and *input_sync*(). Their prototypes are:

```c
void input_event(struct input_dev *dev, unsigned int type, unsigned int code, int value);
static inline void input_sync(struct input_dev *dev);
```

And you can call them like this:

```c
struct atkbd *atkbd = serio_get_drvdata(serio);	// here serio is the first parameter of the interrupt handler function.
struct input_dev *dev = atkbd->dev;
input_event(dev, EV_KEY, code, value); // this function is used to report new input event, the first parameter dev means the device that generated the event.
input_sync(dev); // this function indicates that the input subsystem can collect previously generated events into an evdev packet and send it to user space via /dev/input/inputX
```

Here *code* basically tells the Linux input subsystem which key is pressed or released, and *value* tells the Linux input subsystem that the action is a key press or a key release. If the action is a key press, then *value* must be 1, if the action is a key release, then *value* must be 0. We can derive both the *code* and the *value* from the second parameter (i.e., *data*) of the interrupt handler function. The parameter *data*, as an unsigned char data type, has 8 bits, and:

1. bit 7 represents the action: 1==release, 0==press. (I know this is very confusing, read this very carefully: if bit 7 of *data* is 1, it means this action is release, and therefore you should set *value* to 0 and pass it as the fourth parameter of the function *input_event*(); if bit 7 of *data* is 0, it means this action is press, and therefore you should set *value* to 1 and pass it as the fourth parameter of the function *input_event*()).
2. bit 6 to bit 0 represents the *code* that you are going to pass as the third parameter to the function *input_event*().

More explanation of EV_KEY. The second parameter of *input_event*() tells the input subsystem what event type is generated. The Linux input subsystem defines several event types: the type EV_KEY is used to describe state changes of keyboards, buttons, or other key-like devices; the type EV_REL is used to describe relative axis value changes, e.g. moving the mouse 5 units to the left; the type EV_ABS is used to describe absolute axis value changes, e.g. describing the coordinates of a touch on a touchscreen; the type EV_LED is used to turn LEDs on devices on and off; the type EV_SND is used to output sound to devices. If you want to know more about these events, see the [documentation](https://www.kernel.org/doc/Documentation/input/event-codes.txt) comes with the Linux kernel source code.

Once the ordinary scan code is passed to the keyboard event driver, your interrupt handler function can just return **IRQ_HANDLED**.

## Accessing the Status and Data Registers

The textbook chapter describes:"on x86, the *in* and *out* instructions can be used to communicate with devices". Indeed, in this assignment, we will use the *in* instruction to inquire the status of our device - which in this assignment, means the keyboard, and we will use the *out* instruction to send our command to the device.

In this assignment, we do not intend to use the *in* and *out* assembly instructions directly, rather, we use C functions provided by the Linux kernel:

1. *inb*(): this function encapsulates the *in* assembly instruction and it reads one byte from one specific (8-bit) register. For example, if we want to read one byte from a register whose address is at 0x64, then we call:

```c
inb(0x64)
```

2. *outb*(): this function encapsulates the *out* assembly instruction and it write one byte to one specific (8-bit) register. For example, if we want to write one byte *c* to a register whose address is at 0x60, then we call:

```c
outb(c, 0x60)
```

Similar to what the textbook chapter presents, your *lincoln_kbd_write*() should have the following polling logic:

```c
while(STATUS == BUSY)
;
Write data to DATA register
```

However, just like the textbook chapter says, a while loop like this "seems inefficient; specifically, it wastes a great deal of CPU time just waiting for the (potentially slow) device to complete its activity". Therefore, this is not recommended. A better way is, poll the device every 50 micro-seconds. In other words, inquire the status of the device, if it's busy - busy means that the input buffer is not available, then sleep for 50 micro-seconds, and then try again. You can call *udelay*() like this to sleep for 50 micro-seconds.

```c
udelay(50);
```

If after polling 10,000 times, the device is still busy, your *lincoln_kbd_write*() should print an error message and stop the writing attempt, in other words, your *lincoln_kbd_write*() returns -1, indicating the writing is not successful. To serve this purpose, the starter code therefore has the following macro:

```c
#define I8042_CTL_TIMEOUT       10000
```

## Testing

### Part 1.

When the module is loaded, the user, in the starter code directory, types *sl* from a local console inside the virtual machine, should see *ls* and the result of the *ls* command:

```console
[cs452@xyno cs452-device-driver-2]$ ls
8042.png  lincoln.c  Makefile  README.md  README.template
```

**Note**: here we see *ls*, but I actually typed *sl*.

when the user actually types *ls*, the user should see *sl*, and get the following:

```console
[cs452@xyno cs452-device-driver-2]$ sl
bash: sl: command not found...
Similar command is: 'ls'
```

Other keys should work as expected:

```console
[cs452@xyno cs452-device-driver-2]$ pwd
/home/cs452/cs452-device-driver-2
[cs452@xyno cs452-device-driver-2]$ who
cs452    pts/0        2022-09-15 10:25 (192.168.56.1)
[cs452@xyno cs452-device-driver-2]$ whoami
cs452
```

**Note**: we do not need to test special keys, such as ESC, Left SHIFT, Right SHIFT etc.

### Part 2.

When the user runs:

```console
# sudo echo 'D' > /proc/lincoln/cmd
```

The keyboard becomes unresponsive.

When the user then runs:

```console
# sudo echo 'E' > /proc/lincoln/cmd
```

The keyboard becomes responsive again.

When the user runs:

```console
# sudo echo 'R' > /proc/lincoln/cmd
```

After the above three *echo* commands, we must see this message printed in the kernel log at least once:

```console
we get an ACK from the keyboard.
```
On some computers it could be three times, at least on my computer, each of the above *echo* commands would trigger the keyboard to send an "ack" message back; but this may not be the standard behavior, so if you only get this message once, that is acceptable too. According to [The PS/2 Mouse/Keyboard Protocol](https://www.avrfreaks.net/sites/default/files/PS2%20Keyboard.pdf), written by Adam Chapweske, when the host sends a reset command to the keyboard, the keyboard responds with an "ack", and then performs the BAT test. However, the same documentation does not say the keyboard would respond with an "ack" when receiving the disable or enable command.


In addition, because of the BAT test, we should also observe this message to be printed in the kernel log once and only once:

```console
keyboard reset okay.
```

And these messages must be printed by your interrupt handler function.

## Submission

Due: 23:59pm, November 20th, 2022. Late submission will not be accepted/graded.

## Project Layout

All files necessary for compilation and testing need to be submitted, this includes source code files, header files, and Makefile. The structure of the submission folder should be the same as what was given to you.

## Grading Rubric (Undergraduate and Graduate)

Grade: /100

- [ 80 pts] Functional Requirements:
  - *ls* shows (and runs) as *sl*, and *sl* shows (and runs) as *ls*. /20
  - Commands *pwd*, *who*, and *whoami* work as expected. /20
  - Keyboard disable and then enable works. /20
  - Keyboard reset works and we get that ack message, as well as the "reset okay" message, in the kernel log. /10
  - Module can be installed and removed without crashing the system: /10
    - You won't get these points if your module doesn't implement any of the above functional requirements.

- [10 pts] Compiler warnings:
  - Each compiler warning will result in a 3 point deduction.
  - You are not allowed to suppress warnings.
  - You won't get these points if you didn't implement any of the above functional requirements.

- [10 pts] Documentation:
  - README.md file (rename this current README file to README.orig and rename the README.template to README.md.)
  - You are required to fill in every section of the README template, missing 1 section will result in a 2-point deduction.

## Special Notes

If you want to use the QEMU/KVM virtual machine hosted on onyx nodes to test your keyboard driver, you need to enable the VNC service. VNC stands for Virtual Network Computing, which is a protocol that allows us to remotely access the graphic user interface of another computer. More specifically, if previously this was how you boot the VM:

```console
./qemu-3.0.0/x86_64-softmmu/qemu-system-x86_64 -machine accel=kvm -hda cs452.img -boot c -device e1000,netdev=net0 -netdev user,id=net0,hostfwd=tcp::5022-:22 -name cs452 -m 2G -serial stdio -curses -s
```

Now it needs to be like this:

```console
./qemu-3.0.0/x86_64-softmmu/qemu-system-x86_64 -machine accel=kvm -hda cs452.img -boot c -device e1000,netdev=net0 -netdev user,id=net0,hostfwd=tcp::5022-:22 -name cs452 -m 2G -serial stdio -s -vnc onyxnode64.boisestate.edu:0
```

Here we assume the VM is running on onyxnode64. The only difference between these two commands is replacing '-curses' with '-vnc onyxnode64.boisestate.edu:0'. The curses option means displaying the VM's video output in a text mode; whereas the vnc option means starting a VNC server and redirect the VM's display to this VNC server, which by default listens on port 5900.

Once your VM is booted with this VNC server running, you can use a VNC client, also known as a VNC viewer, to connect to your VM, just connect to onyxnode64.boisestate.edu:5900. Once again, here we assume the VM is running on onyxnode64. The following are two examples of using a VNC client to access the VM.

1. using "Remote Desktop Viewer".

![alt text](vnc-rdv.png "Remote Desktop Viewer")

2. using "TigerVNC Viewer", which be default is installed on all onyx machines.

![alt text](vnc-tiger.png "TigerVNC Viewer")
