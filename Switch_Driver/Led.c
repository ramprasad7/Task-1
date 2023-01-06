#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  
#include <linux/gpio.h>     
#include <linux/err.h>


#define GPIO_21 (21)




static int __init led_init(void){
	if(gpio_is_valid(GPIO_21) == false){
		printk("GPIO- %d is not valid\n",GPIO_21);
		gpio_free(GPIO_21);
		return -1;
	}
	if(gpio_request(GPIO_21,"GPIO_21") <0){
		printk("GPIO - %d failed to request\n",GPIO_21);
		gpio_free(GPIO_21);
		return -1;
	}	       
	if(gpio_direction_output(GPIO_21,0) < 0){
		printk("failed to set direction\n");
		gpio_free(GPIO_21);
		return -1;
	}
	printk("Module has been loaded\n");
	return 0;
}

static void __exit led_cleanup(void){
	gpio_free(GPIO_21);
	printk("Device driver has been removed succefully\n");
}


module_init(led_init);
module_exit(led_cleanup);



MODULE_AUTHOR("Ram");
MODULE_DESCRIPTION("GPIO Out Device Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");


