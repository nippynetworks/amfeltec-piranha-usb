/**
 * gpio_access.c . create a "file" in /proc
 *
 */
#include <linux/module.h> /* Specifically, a module */
#include <linux/kernel.h> /* We're doing kernel work */
#include <linux/proc_fs.h> /* Necessary because we use the proc fs */
#include <asm/uaccess.h> /* for copy_from_user */
#include <linux/list.h>		/* For lists */

//#include <linux/gpio.h>
//#include <asm-generic/gpio.h>

#define DRIVER_VERSION		"0.2"
#define MOD_AUTHOR			"Amfeltec"
#define MOD_DESCRIPTION	"gpio access IO driver"
#define MOD_LICENSE			"GPL"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amfeltec");

#define PROCFS_MAX_SIZE (1024*32)
#define PROCFS_NAME "amf_access"

#define IMX_GPIO_NR(p,pin) (p*32 + pin)


/**
 * This structure hold information about the /proc file
 *
 */
static struct proc_dir_entry *Proc_File;
/**
 * The buffer used to store character for this module
 *
 */
static char *procfs_buffer;
/**
 * The size of the buffer
 *
 */
static unsigned long procfs_buffer_size = 0;

/*
struct proc_dir_entry *proc_file_entry;

static const struct file_operations proc_file_fops = {
 .owner = THIS_MODULE,
 .open  = open_callback,
 .read  = read_callback,
};

int __init init_module(void){
  proc_file_entry = proc_create("proc_file_name", 0, NULL, &proc_file_fops);
  if(proc_file_entry == NULL)
   return -ENOMEM;
  return 0;
}
*/


/**
 * This function is called then the /proc file is read
 *
 */
int
procfile_read(char *buffer,
		char **buffer_location,
		off_t offset, int buffer_length, int *eof, void *data)
{
	int ret;
	printk(KERN_INFO "procfile_read (/proc/%s) called\n", PROCFS_NAME);

        printk(KERN_INFO "offset: %d         buffer_length: %d\n", offset, buffer_length);

	if (offset > 0) {
		/* we have finished to read, return 0 */
		ret = 0;
	} else {
		/* fill the buffer, return the buffer size */

printk(KERN_INFO "buf res : %s\n", procfs_buffer);

		ret = sprintf(procfs_buffer, "Jiffies frequency is: %d Hz\n", HZ);

//		memcpy(buffer, procfs_buffer, procfs_buffer_size);
//		ret = procfs_buffer_size;
		 memcpy(buffer, procfs_buffer, ret);
	}


	return ret;
}


/**
 * This function is called with the /proc file is written
 *
 */

int procfile_write(struct file *file, const char *buffer, unsigned long count,
		void *data)
{
	int i, gpio, val;
	/* get buffer size */
printk(KERN_INFO "len : %d\n", count);
	procfs_buffer_size = count;
	if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}

	/* write data to the buffer */
/*
	if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}
*/
	return procfs_buffer_size;
}
/**
 *This function is called when the module is loaded
 *
 */
int init_module()
{
	/* create the /proc file */
	Proc_File = create_proc_entry(PROCFS_NAME, 0644, NULL);
//        Proc_File = proc_create("proc_file_name", 0, NULL, &proc_file_fops);

printk(KERN_INFO "\n\n\n\n\n\n\n\nRS>>> amf_debug  HZ=%d\n\n\n\n\n\n", HZ);
 	if (Proc_File == NULL) {
//		remove_proc_entry(PROCFS_NAME, Proc_File);
		remove_proc_entry(PROCFS_NAME, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
				PROCFS_NAME);
		return -ENOMEM;
	}
	Proc_File->read_proc = procfile_read;
	Proc_File->write_proc = procfile_write;
//	Proc_File->owner = THIS_MODULE;
	Proc_File->mode = S_IFREG | S_IRUGO;
	Proc_File->uid = 0;
	Proc_File->gid = 0;
	Proc_File->size = 37;
	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
	procfs_buffer=kmalloc(PROCFS_MAX_SIZE, GFP_KERNEL);
printk(KERN_INFO "RS>>> Allocated: %d\n", PROCFS_MAX_SIZE);

	return 0; /* everything is ok */
}
/**
 *This function is called when the module is unloaded
 *
 */
void cleanup_module()
{
//	remove_proc_entry(PROCFS_NAME, Proc_File);
	remove_proc_entry(PROCFS_NAME, NULL);
	kfree(procfs_buffer);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}

