/* 
 * cmd_line_args_module.c - Demonstrates command line argument passing to a module. 
 */ 
#include <linux/init.h> 
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/moduleparam.h> 
#include <linux/stat.h> 
 
MODULE_LICENSE("GPL"); 

static short int myshort = 0; 
static int myint = 0; 
static long int mylong = 0; 
static char *mystring = "default"; 
static int myintarray[2] = { 0, 0 }; 
static int arr_argc = 2; 
 
/* module_param(foo, int, 0000) 
 * The first param is the parameters name. 
 * The second param is its data type. 
 * The final argument is the permissions bits, 
 * for exposing parameters in sysfs (if non-zero) at a later stage. 
 */ 

module_param(myshort, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); 
MODULE_PARM_DESC(myshort, "A short integer"); 

module_param(myint, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); 
MODULE_PARM_DESC(myint, "An integer"); 

module_param(mylong, long, S_IRUSR); 
MODULE_PARM_DESC(mylong, "A long integer"); 

module_param(mystring, charp, 0000); 
MODULE_PARM_DESC(mystring, "A character string"); 
 
/* module_param_array(name, type, num, perm); 
 * The first param is the parameter's (in this case the array's) name. 
 * The second param is the data type of the elements of the array. 
 * The third argument is a pointer to the variable that will store the number 
 * of elements of the array initialized by the user at module loading time. 
 * The fourth argument is the permission bits. 
 */ 

module_param_array(myintarray, int, &arr_argc, 0000); 
MODULE_PARM_DESC(myintarray, "An array of integers"); 
 
static int __init mod_init(void) 
{ 
    int i; 
 
    pr_info("Module parameters values are the following\n=============\n"); 
    pr_info("myshort is a short integer: %hd\n", myshort); 
    pr_info("myint is an integer: %d\n", myint); 
    pr_info("mylong is a long integer: %ld\n", mylong); 
    pr_info("mystring is a string: %s\n", mystring); 
 
    for (i = 0; i < ARRAY_SIZE(myintarray); i++) 
        pr_info("myintarray[%d] = %d\n", i, myintarray[i]); 
 
    pr_info("got %d arguments for myintarray.\n", arr_argc); 
    return 0; 
} 
 
static void __exit mod_exit(void) 
{ 
    pr_info("Goodbye command line demonstration\n"); 
} 
 
module_init(mod_init); 
module_exit(mod_exit);