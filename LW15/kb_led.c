#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h> 
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/tty.h>                  /* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h>                   /* For KDSETLED */
#include <linux/vt.h>
#include <linux/console_struct.h>       /* For vc_cons */
#include <linux/vt_kern.h>

#define SYS_ENTRY_FILENAME "rw_test"

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Karlamachev R. H."); 
MODULE_DESCRIPTION("Keyboard led light by sys file system test");

static struct kobject *example_kobject;
static int foo = 0;

struct myDev
{
    struct timer_list my_timer;    
};

static int is_timer_initialised = 0;
static int cnt = 0;

struct timer_list my_timer;
struct tty_driver *my_driver;
int kbledstatus = 0;
#define BLINK_DELAY   HZ/5
#define ALL_LEDS_ON   0x07
#define RESTORE_LEDS  0xFF

static void my_timer_func(struct timer_list *t) 
{
    struct myDev *dev = from_timer(dev, t, my_timer);

    if (kbledstatus == ALL_LEDS_ON)
        kbledstatus = RESTORE_LEDS;
    else                
        kbledstatus = ALL_LEDS_ON;
    if(is_timer_initialised == 0) 
        return;
    (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, kbledstatus);
    dev->my_timer.expires = jiffies + BLINK_DELAY;
    add_timer(&dev->my_timer);
}

static void setup_timer(void)
{
    //Set up the LED blink timer the first time
    timer_setup(&my_timer, my_timer_func, 0);    
    my_timer.expires = jiffies + BLINK_DELAY; 
    add_timer(&my_timer);     
}

static void kbleds_init(void) 
{
    int i = 0;
    printk(KERN_INFO "kbleds: loading\n");
    printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);
    for (i = 0; i < MAX_NR_CONSOLES; i++) 
    {
        if (!vc_cons[i].d)
            break;
        printk(KERN_INFO "poet_atkm: console[%i/%i] #%i, tty %lx\n", i,
                MAX_NR_CONSOLES, vc_cons[i].d->vc_num,
                (unsigned long)vc_cons[i].d->port.tty);
    }
    printk(KERN_INFO "kbleds: finished scanning consoles\n");
    my_driver = vc_cons[fg_console].d->port.tty->driver;
    printk(KERN_INFO "kbleds: tty driver magic %x\n", my_driver->magic);    
}

static ssize_t foo_show(struct kobject *kobj, 
                        struct kobj_attribute *attr, 
                        char *buf) 
{    
    return sprintf(buf, "%d\n", foo);     
}

static ssize_t foo_store(struct kobject *kobj, 
                         struct kobj_attribute *attr, 
                         const char *buf, 
                         size_t count) 
{    
    printk(KERN_INFO "kbleds: cheked data from user space\n");
    sscanf(buf, "%du", &foo);
    if(!foo || (foo > 255 && foo < 7))
    {
        printk(KERN_INFO "Invalid data writed to buffer\n");
        return -EINVAL;
    }
    else 
    {
        if (foo == 7)
        {
            kbledstatus = foo;
            is_timer_initialised = 1;
            cnt++;
            setup_timer();
        }  
        else        
            is_timer_initialised = 0;
    }      
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
    kbleds_init();
    return error;
}

static void __exit mymodule_exit (void)
{
    printk(KERN_INFO "Cleanup_module runned\n");

    printk(KERN_INFO "kbleds: unloading...\n");
    if(cnt) 
    {
        del_timer(&my_timer);
        (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);
    }

    pr_debug ("Module un initialized successfully \n");
    kobject_put(example_kobject);
}

module_init(mymodule_init); 
module_exit(mymodule_exit); 