#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h> 

/*                  ----- linux/init.h -----
    Each kernel module has to define both 'init' and 'exit' functions:
    'module_init' and 'module_exit' are 2 macros defined in header
    linux/init.h. Module init is the function which is going to be 
    called at module loading time (dynamic module) or at system-boot 
    (static module).
    Module exit is the 'clean-up' function, called at module 
    removal time.
    
                    ----- linux/module.h -----
    Includes support for dynamic loading of module into the kernel.
    It includes also definition of special macros i.e. the one 
    used in this file, 'MODULE_LICENSE'. 
    The latter is used to tell the kernel the module is using a free 
    license. Without this, the kernel would complain when the module 
    is loaded.
*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gianmarco Bencivenni"); 
MODULE_DESCRIPTION("A sample helloworld loadable kernel module"); 

static int __init whatever_init_name(void){
    /* module's init fuction*/
    pr_info("Hello kernel 1\n");
    /* A non 0 return means init_module failed; module can't be loaded. */ 
    return 0;
}

static void __exit whatever_exit_name(void){
    /* module's clean-up function*/
    pr_info("Goodbye kernel 1\n");
}

module_init(whatever_init_name);

module_exit(whatever_exit_name);
