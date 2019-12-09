
/*  МЕТОДИЧЕСКИЕ УКАЗАНИЯ.

	Написать модуль ядра, который будет обмениваться информацией с userspace через proc, sysfs. Продемонстрировать работу модуля. */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include<linux/sched.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define PROC_ENTRY_FILENAME "rw_test"

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Karlamachev R. H."); 
MODULE_DESCRIPTION("Proc file system read/write test");

int len = 0, temp = 0;

char *msg = NULL;

ssize_t read_proc(struct file *filp, char *buf, size_t count, loff_t *offp) {

  if(count > temp) {
    count = temp;
  }

  temp = temp-count;

  copy_to_user(buf, msg, count);

  if(count == 0)
    temp = len;
    
  return count;
}

ssize_t write_proc(struct file *filp, const char *buf, size_t count, loff_t *offp) {

  int len = strlen(buf) + 1;

  msg = kmalloc(len*sizeof(char), GFP_KERNEL);

  copy_from_user(msg, buf, count);

  len = count;
  temp = len;

  return count;
}

struct file_operations proc_fops = {
  read: read_proc,  
  write: write_proc
};

int proc_init (void) {

  proc_create(PROC_ENTRY_FILENAME, 0, NULL, &proc_fops);

  return 0;
}

void proc_cleanup(void) {

  remove_proc_entry(PROC_ENTRY_FILENAME, NULL);
  kfree(msg);

}

module_init(proc_init);
module_exit(proc_cleanup);