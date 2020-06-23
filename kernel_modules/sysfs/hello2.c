/* 
 *  Модуль ядра, который обменивается информацией с userspace через sysfs.
 *  Передать в переменную модуля значение (число) / считать это значение.
 *  При передаче числа в модуль записывается сообщение в /var/log/kern.log
 *  При считывании из модуля выводится записанное ранее число.
 *  Создано для ядра 4.15.0-106-generic в дистрибутиве Ubuntu 16.04.1
 */

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h> 
#include <linux/fs.h>
#include <linux/string.h>

#define SYS_ENTRY_FILENAME "rw_test2"

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Karlamachev R. H."); 
MODULE_DESCRIPTION("Sys file system read/write test");

static struct kobject *example_kobject;
static int foo;

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) 
{
    return sprintf(buf, "%d\n", foo);
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    sscanf(buf, "%du", &foo);
    printk("rw_test2: recieved digit from user: %d", foo);
    return count;
}

static struct kobj_attribute foo_attribute = __ATTR(foo, 0660, foo_show, foo_store);

static int __init mymodule_init (void) 
{
    int error = 0;
    pr_debug("Module initialized successfully \n");
    example_kobject = kobject_create_and_add(SYS_ENTRY_FILENAME, kernel_kobj);
    if(!example_kobject)
        return -ENOMEM;
    error = sysfs_create_file(example_kobject, &foo_attribute.attr);
    if (error)
    {
        pr_debug("failed to create the SYS_ENTRY_FILENAME file in /sys/kernel/ \n");
    }
    return error;
}

static void __exit mymodule_exit (void)
{
    pr_debug ("Module un initialized successfully \n");
    kobject_put(example_kobject);
}

module_init(mymodule_init); 
module_exit(mymodule_exit); 
