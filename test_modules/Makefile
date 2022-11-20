obj-m+=helloworld1.o 
obj-m+=cmd_line_args_module.o

PWD:=$(CURDIR) 
 
all: 
	make -C	/lib/modules/$(shell uname -r)/build M=$(PWD) modules 
 
clean: 
	make -C	/lib/modules/$(shell uname -r)/build M=$(PWD) clean