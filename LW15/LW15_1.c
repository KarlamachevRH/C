/*  МЕТОДИЧЕСКИЕ УКАЗАНИЯ.

  Написать модуль ядра, который будет обмениваться информацией с userspace через /dev/mydevl. */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Karlamachev R. H.");
MODULE_DESCRIPTION("Linux kernel module for get/send data from/to user space by creating device symbolic name in /dev/... folder.");

#define DEVICE_NAME "mydevl"
#define DEVICE_FULL_NAME "/dev/mydevl"
#define EXAMPLE_MSG "Hello, World!"
#define MSG_BUFFER_LEN 1024

/* Prototypes for device functions */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static int major_num;
static int device_open_count = 0;
static char msg_buffer[MSG_BUFFER_LEN];
static char *msg_ptr;


/* This structure points to all of the device functions */
static struct file_operations file_ops = {
 .read = device_read,
 .write = device_write,
 .open = device_open,
 .release = device_release
};

/* When a process reads from our device, this gets called. */
static ssize_t device_read(struct file *filp, char *buffer, size_t len, loff_t *offset) {
  int bytes_read = 0;
  /* If we’re at the end, loop back to the beginning */
  if (*msg_ptr == 0) {
    msg_ptr = msg_buffer;
  }

 /* Put data in the buffer */
  while (len && *msg_ptr) {
    /* 
    * Buffer is in user data, not kernel, so you can’t just reference
    * with a pointer. The function put_user handles this for us 
    */
    put_user(*(msg_ptr++), buffer++);
    len--;
    bytes_read++;
  }
  return bytes_read;
}

/* Called when a process tries to write to our device */
static ssize_t device_write(struct file *filp, const char *buffer, size_t len, loff_t *offset) {
  
  int bytes_writed = 0;
  int i = 0;

   while (len && i < MSG_BUFFER_LEN-1) {   
    /* Get data from user space */
    get_user(msg_buffer[i], buffer++);
    len--;
    i++;
    bytes_writed++;
  }
  return bytes_writed;
}

/* Called when a process opens our device */
static int device_open(struct inode *inode, struct file *file) {
  /* If device is open, return busy */
  if (device_open_count) {
    return -EBUSY;
  }
  device_open_count++;
  try_module_get(THIS_MODULE);
  return 0;
}

/* Called when a process closes our device */
static int device_release(struct inode *inode, struct file *file) {  
  /* Decrement the open counter and usage count. Without this, the module would not unload. */
  device_open_count--;
  module_put(THIS_MODULE);
  return 0;
}

static int __init mydevl_init(void) {

  /* Fill buffer with temporary message */
  strncpy(msg_buffer, EXAMPLE_MSG, strlen(EXAMPLE_MSG)+1);

  /* Set the msg_ptr to the buffer */
  msg_ptr = msg_buffer;

  /* Try to register character device */
  major_num = register_chrdev(0, DEVICE_NAME, &file_ops);
  if (major_num < 0) {
    printk(KERN_ALERT "Could not register device: %d\n", major_num);
    return major_num;
  } else {
    printk(KERN_INFO "mydevl module loaded with device major number %d\n", major_num);
    return 0;
  }

  /* Try to create device pseudo file */
  if(sys_mknod(DEVICE_FULL_NAME, S_IFCHR|0666, MKDEV(major_num,1)) < 0){
    printk(KERN_INFO "Failed to create DEVICE_FULL_NAME device file\n");
    return -1;
  }
}

static void __exit mydevl_exit(void) { 
  
  /* Try to remove device pseudo file */
  /* if(sys_unlink(DEVICE_FULL_NAME) < 0){
    printk(KERN_INFO "Failed to remove DEVICE_FULL_NAME device file\n");    
  } */

  /* Unregister the character device. */
  unregister_chrdev(major_num, DEVICE_NAME); 
  printk(KERN_INFO "Goodbye, World!\n");  
}

/* Register module functions */
module_init(mydevl_init);
module_exit(mydevl_exit);