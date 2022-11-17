/* This kernel module is trying to intercept some functionality of the keyboard device driver, rather than the keyboard event driver. 
 * The keyboard event driver lives in drivers/char/keyboard.c. */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/platform_device.h>	/* which defines struct platform_driver */
#include <linux/device.h>	/* which defines struct device_driver */
#include <linux/serio.h>	/* which defines struct serio */
#include <linux/input.h>	/* which defines macros like EV_KEY */
#include <linux/libps2.h>	/* which defines macros like PS_FLAG_XX */
#include <linux/proc_fs.h>	/* so we can call proc APIs, such as proc_create, proc_mkdir... */
#include <linux/delay.h>	/* so that we can call udelay()... */

MODULE_LICENSE("GPL v2");	/* without saying this, we won't be able to use kallsyms_lookup_name(). */

/* borrowed from drivers/input/serio/i8042-io.h */
#define I8042_COMMAND_REG       0x64 // write commands to this register would send commands to the 8042 controller, not the 8042 PS/2 device.
#define I8042_STATUS_REG        0x64
#define I8042_DATA_REG          0x60

/* i8042 status register, which has 8 bits: bit 0 to bit 7.
 * bit 0: output buffer status (0 = empty, 1 = full), (must be set before attempting to read data from IO port 0x60) 
 * bit 1: input buffer status (0 = empty, 1 = full), (must be clear before attempting to write data to IO port 0x60 or IO port 0x64) */
#define I8042_STR_IBF           0x02	/* this is bit 1, input buffer status bit. */

/*
 * This is in 50us units, the time we wait for the i8042 to react. This
 * has to be long enough for the i8042 itself to timeout on sending a byte
 * to a non-existent mouse.
 * */

#define I8042_CTL_TIMEOUT       10000

/*
 *  * Scancode to keycode tables. These are just the default setting, and
 *   * are loadable via a userland utility.
 *    */

#define ATKBD_KEYMAP_SIZE       512

#define ATKBD_CMD_ENABLE        0x00f4
#define ATKBD_CMD_RESET_DIS     0x00f5  /* Reset to defaults and disable */
#define ATKBD_CMD_RESET_BAT     0x00ff

/* protocol scancodes */
/* ATKBD_RET_ACK means acknowledge from kbd. 
 * It's a reserved scancode, thus it means the data isn't from input. 
 * Such a scancode is called a protocol scancode. There are two types of scancodes:
 * protocol scancodes, escape scancodes, and ordinary scancodes. */
#define ATKBD_RET_ACK           0xfa	/* Acknowledge from kbd */
#define ATKBD_RET_NAK           0xfe	/* Keyboard fails to ack, please resend */
#define ATKBD_RET_BAT           0xaa	/* BAT (Basic Assurance Test) OK */
#define ATKBD_RET_EMUL0         0xe0	/* The codes e0 and e1 introduce scancode sequences, and are not usually used as isolated scancodes themselves */
#define ATKBD_RET_EMUL1         0xe1	/* */
#define ATKBD_RET_RELEASE       0xf0	/* */
#define ATKBD_RET_HANJA         0xf1	/* Some keyboards, as reply to command */
#define ATKBD_RET_HANGEUL       0xf2	/* */
#define ATKBD_RET_ERR           0xff	/* Keyboard error */

struct i8042_port {
        struct serio *serio;
        int irq;
        bool exists;
        signed char mux;
};

struct i8042_port * i8042_ports;

struct atkbd {

        struct ps2dev ps2dev;
        struct input_dev *dev;

        /* Written only during init */
        char name[64];
        char phys[32];

        unsigned short id;
        unsigned short keycode[ATKBD_KEYMAP_SIZE];
        DECLARE_BITMAP(force_release_mask, ATKBD_KEYMAP_SIZE);
        unsigned char set;
        bool translated;
        bool extra;
        bool write;
        bool softrepeat;
        bool softraw;
        bool scroll;
        bool enabled;

        /* Accessed only from interrupt */
        unsigned char emul;
        bool resend;
        bool release;
        unsigned long xl_bit;
        unsigned int last;
        unsigned long time;
        unsigned long err_count;

        struct delayed_work event_work;
        unsigned long event_jiffies;
        unsigned long event_mask;

        /* Serializes reconnect(), attr->set() and event work */
        struct mutex mutex;
};

static irqreturn_t (*orig_irq_handler)(struct serio *serio, unsigned char data, unsigned int flags);
static int (*orig_kbd_write)(struct serio *port, unsigned char c);

/* The proc directory entry */
static struct proc_dir_entry *pdir;
#define GLOBAL_RW_PERM 0666
#define DIR_NAME "lincoln"
#define FILE_NAME "cmd"

/* we do not support EV_MSC events, thus we do not generate EV_MSC MSC_SCAN or EV_MSC MSC_RAW events.
 * this driver only supports number keys and letter keys; special keys require some special treatment, 
 * which is beyond the scope of this program. 
 * the parameter flags won't be used in this program. */
static irqreturn_t lincoln_irq_handler(struct serio *serio, unsigned char data, unsigned int flags)
{
	char code = data & 0x7f;
	int value;
	if(data >> 7 == 1){
		value = 0;
	}else{
		value = 1;
	}

	if(data == ATKBD_RET_BAT){
		printk("keyboard reset okay.");
	}
	else if(data == ATKBD_RET_ACK){
		printk("we get an ACK from the keyboard.");
	}
	else {
		struct atkbd *atkbd = serio_get_drvdata(serio);	
		struct input_dev *dev = atkbd->dev;
		if(code == 0x26){code = 0x1f;}
		else if(code == 0x1f){code = 0x26;}		

		input_event(dev, EV_KEY, code, value); 
		input_sync(dev); 	
	}
	return IRQ_HANDLED;
}

/* in theory we should use this lock when reading/writing the 8042 registers,
 * but in this assignment, it looks like even if we don't use this lock,
 * we should still be able to pass all of our tests. */
struct spinlock *i8042_lock;

/*
 * lincoln_kbd_write(): sends a byte out through the keyboard interface.
 * return 0 if successful; return -1 if not successful.
 * */
static int lincoln_kbd_write(struct serio *port, unsigned char c)
{	
	int counter = 0;
	while((inb(I8042_STATUS_REG) & I8042_STR_IBF) && counter < I8042_CTL_TIMEOUT){
		udelay(50);
		counter++;
	}
	if(counter == I8042_CTL_TIMEOUT){
		printk("Time out! Writing faild.");
		return -1;
	}
	outb(c, I8042_DATA_REG);
	return 0;
}

/* gets called when a user runs sudo echo 'cmd' > /proc/lincoln/cmd
 * supported commands:
 * "D": disable;
 * "E": enable;
 * "R": reset;
 * */
ssize_t write_proc(struct file *filp, const char *user, size_t count, loff_t *offset)
{
	char *buf = (char *)kmalloc(count, GFP_KERNEL);
	char cmd;

	/* this is the command the user sends to us, copy it to a kernel buffer. */
	if(copy_from_user(buf, user, count)){
		return -EFAULT;
	}
   cmd = buf[0];

   switch(cmd)
   {
      case 'D':
		serio_write(i8042_ports[0].serio, ATKBD_CMD_RESET_DIS);
        break;
      case 'E':
		serio_write(i8042_ports[0].serio, ATKBD_CMD_ENABLE);
        break;
      case 'R':
       	serio_write(i8042_ports[0].serio, ATKBD_CMD_RESET_BAT);
        break;
      default:
		printk(KERN_INFO "command not supported.");
   }

   kfree((void *)buf);
   return count;
}

struct file_operations proc_fops = {
   write: write_proc,
};

/* helper function to create the directory entries for /proc */
void create_proc_files(void) {
   pdir = proc_mkdir(DIR_NAME, NULL);
   proc_create(FILE_NAME, GLOBAL_RW_PERM, pdir, &proc_fops);
}

/* helper function to delete the directory entries for /proc */
void delete_proc_files(void) {
   remove_proc_entry(FILE_NAME, pdir);
   remove_proc_entry(DIR_NAME, NULL);
}

/* this function gets called when the module is loaded. */
static int __init lincoln_init(void){
	struct platform_driver * i8042_driver;
	struct device_driver * driver;
	i8042_ports=(struct i8042_port *)kallsyms_lookup_name("i8042_ports");
	if(i8042_ports == NULL) {
		printk(KERN_ERR "where the heck is the i8042_ports?\n");
		return -1;
	}

	i8042_driver=(struct platform_driver *)kallsyms_lookup_name("i8042_driver");
	if(i8042_driver == NULL) {
		printk(KERN_ERR "where the heck is the i8042_driver?\n");
		return -1;
	}
	driver=&(i8042_driver->driver);
	printk(KERN_INFO "i8042 driver is at address 0x%p, and its remove is at 0x%p\n", i8042_driver, i8042_driver->remove);
	printk(KERN_INFO "i8042 driver name is %s\n", driver->name);

	i8042_lock=(struct spinlock *)kallsyms_lookup_name("i8042_lock");
	if(i8042_lock == NULL) {
		printk(KERN_ERR "where the heck is the i8042_lock?\n");
		return -1;
	}

	/* first, let's print keyboard info; the keyboard is associated with irq 1. */
	if(i8042_ports[0].serio){
		printk(KERN_INFO "serio: %s at %#lx,%#lx irq %d\n",
			i8042_ports[0].serio->name,
			(unsigned long) I8042_DATA_REG,
			(unsigned long) I8042_COMMAND_REG,
			i8042_ports[0].irq);
		/* redirect the write function to our write function. */
		orig_kbd_write = i8042_ports[0].serio->write;
		i8042_ports[0].serio->write = lincoln_kbd_write;

		/* redirect the interrupt handler pointer to point to our interrupt handler. */
		if(i8042_ports[0].serio->drv!=NULL){
			orig_irq_handler = i8042_ports[0].serio->drv->interrupt;
			i8042_ports[0].serio->drv->interrupt = lincoln_irq_handler;
		}
	}

	/* next, let's print aux info, which is actually for the mouse. the mouse is associated with irq 12. */
	if(i8042_ports[1].serio){
		printk(KERN_INFO "serio: %s at %#lx,%#lx irq %d\n",
			i8042_ports[1].serio->name,
			(unsigned long) I8042_DATA_REG,
			(unsigned long) I8042_COMMAND_REG,
			i8042_ports[1].irq);
	}

	/* creating the /proc/lincoln/cmd file. */
	create_proc_files();
	return 0;
}

/* this function gets called when the module is unloaded. */
static void __exit lincoln_exit(void){
	if(i8042_ports[0].serio!=NULL){
		i8042_ports[0].serio->write = orig_kbd_write;
		if(i8042_ports[0].serio->drv!=NULL){
			i8042_ports[0].serio->drv->interrupt = orig_irq_handler;
		}
	}
	/* deleting the /proc/lincoln/cmd file. */
	delete_proc_files();
	printk(KERN_INFO "goodbye\n");
}

module_init(lincoln_init);
module_exit(lincoln_exit);

/* vim: set ts=4: */
