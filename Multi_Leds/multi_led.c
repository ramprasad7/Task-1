#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include<linux/version.h>
#include<linux/uaccess.h>
#include<linux/gpio.h>

#define MAX 3

/*GPIO pins*/

#define GPIO_21 21
#define GPIO_20 20
#define GPIO_16 16


static struct gpio leds_gpio[] = {
    {GPIO_21,GPIOF_OUT_INIT_LOW,"LED-1"},
    {GPIO_20,GPIOF_OUT_INIT_LOW,"LED-2"},
    {GPIO_16,GPIOF_OUT_INIT_LOW,"LED-3"}
};

dev_t device_number;
struct class* led_class;
struct cdev led_cdev[MAX];



static int minor;


int led_open(struct inode *pinode,struct file *pfile){
    printk("Device file opened...\n");
    minor = iminor(file_inode(pfile));
    if( minor == 0 ){
        if(gpio_is_valid(leds_gpio[0].gpio) == false){
            printk("GPIO - %d is not valid\n",leds_gpio[0].gpio);
            gpio_free(leds_gpio[0].gpio);
        }
        if(gpio_request(leds_gpio[0].gpio,"GPIO_21") < 0){
            printk("GPIO - %d failed to request\n",leds_gpio[0].gpio);
            gpio_free(leds_gpio[0].gpio);
        }
        if(gpio_direction_output(leds_gpio[0].gpio,0) < 0){
            printk("GPIO -%d failed to set direction\n",leds_gpio[0].gpio);
            gpio_free(leds_gpio[0].gpio);
        }
    }else if(minor == 1){
        if(gpio_is_valid(leds_gpio[1].gpio) == false){
            printk("GPIO - %d is not valid\n",leds_gpio[1].gpio);
            gpio_free(leds_gpio[1].gpio);
        }
        if(gpio_request(leds_gpio[1].gpio,"GPIO_20") < 0){
            printk("GPIO - %d failed to request\n",leds_gpio[1].gpio);
            gpio_free(leds_gpio[1].gpio);
        }
        if(gpio_direction_output(leds_gpio[1].gpio,0) < 0){
            printk("GPIO -%d failed to set direction\n",leds_gpio[1].gpio);
            gpio_free(leds_gpio[1].gpio);
        }  
    }else{
        if(gpio_is_valid(leds_gpio[2].gpio) == false){
            printk("GPIO - %d is not valid\n",leds_gpio[2].gpio);
            gpio_free(leds_gpio[2].gpio);
        }
        if(gpio_request(leds_gpio[2].gpio,"GPIO_16") < 0){
            printk("GPIO - %d failed to request\n",leds_gpio[2].gpio);
            gpio_free(leds_gpio[2].gpio);
        }
        if(gpio_direction_output(leds_gpio[2].gpio,0) < 0){
            printk("GPIO -%d failed to set direction\n",leds_gpio[2].gpio);
            gpio_free(leds_gpio[2].gpio);   
        }
    }
    return 0;
}


int led_close(struct inode *pinode,struct file *pfile){
    printk("Device file closed!\n");
    if( minor == 0 ){
        gpio_free(leds_gpio[0].gpio);
    }else if(minor == 1){
        gpio_free(leds_gpio[1].gpio);
    }else{
        gpio_free(leds_gpio[2].gpio);   
    }
    return 0;
}

ssize_t led_read(struct file* pfile,char* __user buf,size_t len,loff_t *loff){
    printk("Device file opened to read %ld\n",len);
    return len;
}

ssize_t led_write(struct file* pfile,const char* __user buf,size_t len,loff_t *loff){
    uint8_t dev_buf[10] = {0};
    if(copy_from_user(dev_buf,buf,len) > 0){
        printk("Couldn't write all the given bytes\n");
        return -1;
    }
    if(minor == 0){
        if(dev_buf[0] == '0'){
            gpio_set_value(leds_gpio[0].gpio,0);
        }else if(dev_buf[0] == '1'){
            gpio_set_value(leds_gpio[0].gpio,1);
        }else{
            printk("Please write either 1 or 0\n");
            return -1;
        }
   }else if( minor == 1){
        if(dev_buf[0] == '0'){
            gpio_set_value(leds_gpio[1].gpio,0);
        }else if(dev_buf[0] == '1'){
            gpio_set_value(leds_gpio[1].gpio,1);
        }else{
            printk("Please write either 1 or 0\n");
            return -1;
        }
   }else{
    if(dev_buf[0] == '0'){
        gpio_set_value(leds_gpio[2].gpio,0);
        }else if(dev_buf[0] == '1'){
            gpio_set_value(leds_gpio[2].gpio,1);
        }else{
            printk("Please write either 1 or 0\n");
            return -1;
        }
   }
    printk("Device file opened to write %ld number of bytes\n",len);
    return len;
}

struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_close,
};

static int __init led_init(void){
    int err = alloc_chrdev_region(&device_number,0,MAX,"led_devices");
    dev_t dev_num;
    int major,i;
    if(err < 0){
        printk("failed to allocate major number\n");
        unregister_chrdev_region(device_number,1);
        return -1;
    }
    major = MAJOR(device_number);
    printk("Device register with <MAJOR> = <%d>\n",MAJOR(device_number));
    led_class = class_create(THIS_MODULE,"led_class");
    
    if(IS_ERR(led_class)){
        printk("failed to create class\n");
        class_destroy(led_class);
        unregister_chrdev_region(device_number,1);
        return -2;
    }
    for(i=0;i<MAX;i++){
        dev_num = MKDEV(major,i);
        printk("1st minor number = %d\n",MINOR(dev_num));
        cdev_init(&led_cdev[i],&fops);
        if(cdev_add(&led_cdev[i],dev_num,1) < 0){
            printk("Failed at cdev_add\n");
            cdev_del(&led_cdev[i]);
            return -3;
        }
        if(IS_ERR(device_create(led_class,NULL,dev_num,NULL,"led_device-%d",i))){
            printk("Failed to create device file in /dev directory\n");
            device_destroy(led_class,dev_num);
            return -4;
        }
    }
    printk("Module loaded succesfully\n");
    return 0;
}



static void __exit led_cleanup(void){
    dev_t dev_num;
    int i,major;
    major = MAJOR(device_number);
    for(i=0;i<MAX;i++){
        dev_num = MKDEV(major ,i);
        cdev_del(&led_cdev[i]);
        device_destroy(led_class,dev_num);
    }
    class_destroy(led_class);
    unregister_chrdev_region(device_number,MAX);
    printk("Device driver has been removed succefully\n");
}


module_init(led_init);
module_exit(led_cleanup);

MODULE_AUTHOR("Ram");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple gpio driver for multiple LEDs");
MODULE_VERSION("1.0");
