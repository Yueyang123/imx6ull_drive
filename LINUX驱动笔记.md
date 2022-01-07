# 字符类设备
在 Linux 中一切皆为文件，驱动加载成功以后会在“/dev”目录下生成一个相应的文件，应
用程序通过对这个名为“/dev/xxx”(xxx 是具体的驱动文件名字)的文件进行相应的操作即可实
现对硬件的操作。比如现在有个叫做/dev/led 的驱动文件，此文件是 led 灯的驱动文件。应用程
序使用 open 函数来打开文件/dev/led，使用完成以后使用 close 函数关闭/dev/led 这个文件。open
和 close 就是打开和关闭 led 驱动的函数，如果要点亮或关闭 led，那么就使用 write 函数来操
作，也就是向此驱动写入数据，这个数据就是要关闭还是要打开 led 的控制参数。如果要获取
led 灯的状态，就用 read 函数从驱动中读取相应的状态。
应用程序运行在用户空间，而 Linux 驱动属于内核的一部分，因此驱动运行于内核空间。
当我们在用户空间想要实现对内核的操作，比如使用 open 函数打开/dev/led 这个驱动，因为用
户空间不能直接对内核进行操作，因此必须使用一个叫做“系统调用”的方法来实现从用户空
间“陷入”到内核空间，这样才能实现对底层驱动的操作。open、close、write 和 read 等这些函
数是由 C 库提供的，在 Linux 系统中，系统调用作为 C 库的一部分。


其中关于 C 库以及如何通过系统调用“陷入”到内核空间这个我们不用去管，我们重点关
注的是应用程序和具体的驱动，应用程序使用到的函数在具体驱动程序中都有与之对应的函数，
比如应用程序中调用了 open 这个函数，那么在驱动程序中也得有一个名为 open 的函数。每一
个系统调用，在驱动中都有与之对应的一个驱动函数，在 Linux 内核文件 include/linux/fs.h 中
有个叫做 file_operations 的结构体，此结构体就是 Linux 内核驱动操作函数集合，内容如下所
示：

```c
 struct file_operations {
 struct module *owner;
 loff_t (*llseek) (struct file *, loff_t, int);
 ssize_t (*read) (struct file *, char __user *, size_t, loff_t 
*);
 ssize_t (*write) (struct file *, const char __user *, size_t,
loff_t *);
 ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
 ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
 int (*iterate) (struct file *, struct dir_context *);
 unsigned int (*poll) (struct file *, struct poll_table_struct 
*);
 long (*unlocked_ioctl) (struct file *, unsigned int, unsigned
long);
 long (*compat_ioctl) (struct file *, unsigned int, unsigned
long);
 int (*mmap) (struct file *, struct vm_area_struct *);
 int (*mremap)(struct file *, struct vm_area_struct *);
 int (*open) (struct inode *, struct file *);
 int (*flush) (struct file *, fl_owner_t id);
 int (*release) (struct inode *, struct file *);
 int (*fsync) (struct file *, loff_t, loff_t, int datasync);
 int (*aio_fsync) (struct kiocb *, int datasync);
 int (*fasync) (int, struct file *, int);
 int (*lock) (struct file *, int, struct file_lock *);
 ssize_t (*sendpage) (struct file *, struct page *, int, size_t,
loff_t *, int);
 unsigned long (*get_unmapped_area)(struct file *, unsigned long,
unsigned long, unsigned long, unsigned long);
 int (*check_flags)(int);
 int (*flock) (struct file *, int, struct file_lock *);
  ssize_t (*splice_write)(struct pipe_inode_info *, struct file *,
loff_t *, size_t, unsigned int);
 ssize_t (*splice_read)(struct file *, loff_t *, struct
pipe_inode_info *, size_t, unsigned int);
 int (*setlease)(struct file *, long, struct file_lock **, void
**);
 long (*fallocate)(struct file *file, int mode, loff_t offset,
off_t len);
 void (*show_fdinfo)(struct seq_file *m, struct file *f);
 #ifndef CONFIG_MMU
 unsigned (*mmap_capabilities)(struct file *);
 #endif
 };
```
第 1589 行，owner 拥有该结构体的模块的指针，一般设置为 THIS_MODULE。
第 1590 行，llseek 函数用于修改文件当前的读写位置。
第 1591 行，read 函数用于读取设备文件。
第 1592 行，write 函数用于向设备文件写入(发送)数据。
第 1596 行，poll 是个轮询函数，用于查询设备是否可以进行非阻塞的读写。
第 1597 行，unlocked_ioctl 函数提供对于设备的控制功能，与应用程序中的 ioctl 函数对应。
第 1598 行，compat_ioctl 函数与 unlocked_ioctl 函数功能一样，区别在于在 64 位系统上，
32 位的应用程序调用将会使用此函数。在 32 位的系统上运行 32 位的应用程序调用的是
unlocked_ioctl。
第 1599 行，mmap 函数用于将将设备的内存映射到进程空间中(也就是用户空间)，一般帧
缓冲设备会使用此函数，比如 LCD 驱动的显存，将帧缓冲(LCD 显存)映射到用户空间中以后应
用程序就可以直接操作显存了，这样就不用在用户空间和内核空间之间来回复制。
第 1601 行，open 函数用于打开设备文件。
第 1603 行，release 函数用于释放(关闭)设备文件，与应用程序中的 close 函数对应。
第 1604 行，fasync 函数用于刷新待处理的数据，用于将缓冲区中的数据刷新到磁盘中。
第 1605 行，aio_fsync 函数与 fasync 函数的功能类似，只是 aio_fsync 是异步刷新待处理的
数据。
在字符设备驱动开发中最常用的就是上面这些函数，关于其他的函数大家可以查阅相关文
档。我们在字符设备驱动开发中最主要的工作就是实现上面这些函数，不一定全部都要实现，
但是像 open、release、write、read 等都是需要实现的，当然了，具体需要实现哪些函数还是要
看具体的驱动要求。

## 驱动模块的加载和卸载

Linux 驱动有两种运行方式，第一种就是将驱动编译进 Linux 内核中，这样当 Linux 内核启
动的时候就会自动运行驱动程序。第二种就是将驱动编译成模块(Linux 下模块扩展名为.ko)，在
Linux 内核启动以后使用“insmod”命令加载驱动模块。在调试驱动的时候一般都选择将其编译
为模块，这样我们修改驱动以后只需要编译一下驱动代码即可，不需要编译整个 Linux 代码。
而且在调试的时候只需要加载或者卸载驱动模块即可，不需要重启整个系统。总之，将驱动编
译为模块最大的好处就是方便开发，当驱动开发完成，确定没有问题以后就可以将驱动编译进
Linux 内核中，当然也可以不编译进 Linux 内核中，具体看自己的需求。
模块有加载和卸载两种操作，我们在编写驱动的时候需要注册这两种操作函数，模块的加载和
卸载注册函数如下：
module_init(xxx_init); //注册模块加载函数
module_exit(xxx_exit); //注册模块卸载函数
module_init 函数用来向 Linux 内核注册一个模块加载函数，参数 xxx_init 就是需要注册的
具体函数，当使用“insmod”命令加载驱动的时候，xxx_init 这个函数就会被调用。module_exit()
函数用来向 Linux 内核注册一个模块卸载函数，参数 xxx_exit 就是需要注册的具体函数，当使
用“rmmod”命令卸载具体驱动的时候 xxx_exit 函数就会被调用。字符设备驱动模块加载和卸
载模板如下所示：


## 字符设备注册与注销


对于字符设备驱动而言，当驱动模块加载成功以后需要注册字符设备，同样，卸载驱动模
块的时候也需要注销掉字符设备。字符设备的注册和注销函数原型如下所示:
static inline int register_chrdev(unsigned int major, const char *name,
const struct file_operations *fops)
static inline void unregister_chrdev(unsigned int major, const char *name)
register_chrdev 函数用于注册字符设备，此函数一共有三个参数，这三个参数的含义如下：
major：主设备号，Linux 下每个设备都有一个设备号，设备号分为主设备号和次设备号两
部分，关于设备号后面会详细讲解。
name：设备名字，指向一串字符串。
fops：结构体 file_operations 类型指针，指向设备的操作函数集合变量。
unregister_chrdev 函数用户注销字符设备，此函数有两个参数，这两个参数含义如下：
major：要注销的设备对应的主设备号。
name：要注销的设备对应的设备名。

## 实现设备的具体操作函数


file_operations 结构体就是设备的具体操作函数，在示例代码 40.2.2.1 中我们定义了
file_operations结构体类型的变量test_fops，但是还没对其进行初始化，也就是初始化其中的open、
release、read 和 write 等具体的设备操作函数。本节小节我们就完成变量 test_fops 的初始化，设
置好针对 chrtest 设备的操作函数。在初始化 test_fops 之前我们要分析一下需求，也就是要对
chrtest 这个设备进行哪些操作，只有确定了需求以后才知道我们应该实现哪些操作函数。假设
对 chrtest 这个设备有如下两个要求：
1、能够对 chrtest 进行打开和关闭操作
设备打开和关闭是最基本的要求，几乎所有的设备都得提供打开和关闭的功能。因此我们
需要实现 file_operations 中的 open 和 release 这两个函数。
2、对 chrtest 进行读写操作
假设 chrtest 这个设备控制着一段缓冲区(内存)，应用程序需要通过 read 和 write 这两个函
数对 chrtest 的缓冲区进行读写操作。所以需要实现 file_operations 中的 read 和 write 这两个函
数。
在示例代码 40.2.3.1 中我们一开始编写了四个函数：chrtest_open、chrtest_read、chrtest_write
和 chrtest_release。这四个函数就是 chrtest 设备的 open、read、write 和 release 操作函数。第 29
行~35 行初始化 test_fops 的 open、read、write 和 release 这四个成员变量。

 static int chrtest_open(struct inode *inode, struct file *filp)
 static ssize_t chrtest_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
 static ssize_t chrtest_write(struct file *filp,const char __user *buf,size_t cnt, loff_t *offt)
 static int chrtest_release(struct inode *inode, struct file *filp)

## 添加 LICENSE 和作者信息
最后我们需要在驱动中加入 LICENSE 信息和作者信息，其中 LICENSE 是必须添加的，否
则的话编译的时候会报错，作者信息可以添加也可以不添加。LICENSE 和作者信息的添加使用
如下两个函数：
MODULE_LICENSE() //添加模块 LICENSE 信息
MODULE_AUTHOR() //添加模块作者信息

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YURI"); 


# Linux 设备号
为了方便管理，Linux 中每个设备都有一个设备号，设备号由主设备号和次设备号两部分
组成，主设备号表示某一个具体的驱动，次设备号表示使用这个驱动的各个设备。Linux 提供了
一个名为 dev_t 的数据类型表示设备号，dev_t 定义在文件 include/linux/types.h 里面，定义如下：
12 typedef __u32 __kernel_dev_t;
......
15 typedef __kernel_dev_t dev_t;
26 typedef unsigned int __u32;


综上所述，dev_t 其实就是 unsigned int 类型，是一个 32 位的数据类型。这 32 位的数据构成了主设备号
和次设备号两部分，其中高 12 位为主设备号，低 20 位为次设备号。因此 Linux
系统中主设备号范围为 0~4095，所以大家在选择主设备号的时候一定不要超过这个范围。在文
件 include/linux/kdev_t.h 中提供了几个关于设备号的操作函数(本质是宏)，如下所示：


6 #define MINORBITS 20
7 #define MINORMASK ((1U << MINORBITS) - 1)
9 #define MAJOR(dev) ((unsigned int) ((dev) >> MINORBITS))
10 #define MINOR(dev) ((unsigned int) ((dev) & MINORMASK))
11 #define MKDEV(ma,mi) (((ma) << MINORBITS) | (mi))
第 6 行，宏 MINORBITS 表示次设备号位数，一共是 20 位。
第 7 行，宏 MINORMASK 表示次设备号掩码。
第 9 行，宏 MAJOR 用于从 dev_t 中获取主设备号，将 dev_t 右移 20 位即可。
第 10 行，宏 MINOR 用于从 dev_t 中获取次设备号，取 dev_t 的低 20 位的值即可。
第 11 行，宏 MKDEV 用于将给定的主设备号和次设备号的值组合成 dev_t 类型的设备号。

##  设备号的分配


1、静态分配设备号
本小节讲的设备号分配主要是主设备号的分配。前面讲解字符设备驱动的时候说过了，注
册字符设备的时候需要给设备指定一个设备号，这个设备号可以是驱动开发者静态的指定一个
设备号，比如选择 200 这个主设备号。有一些常用的设备号已经被 Linux 内核开发者给分配掉
了，具体分配的内容可以查看文档 Documentation/devices.txt。并不是说内核开发者已经分配掉
的主设备号我们就不能用了，具体能不能用还得看我们的硬件平台运行过程中有没有使用这个
主设备号，使用“cat /proc/devices”命令即可查看当前系统中所有已经使用了的设备号。
2、动态分配设备号
静态分配设备号需要我们检查当前系统中所有被使用了的设备号，然后挑选一个没有使用
的。而且静态分配设备号很容易带来冲突问题，Linux 社区推荐使用动态分配设备号，在注册字
符设备之前先申请一个设备号，系统会自动给你一个没有被使用的设备号，这样就避免了冲突。
卸载驱动的时候释放掉这个设备号即可，设备号的申请函数如下：
int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)
函数 alloc_chrdev_region 用于申请设备号，此函数有 4 个参数：
dev：保存申请到的设备号。
baseminor：次设备号起始地址，alloc_chrdev_region 可以申请一段连续的多个设备号，这
些设备号的主设备号一样，但是次设备号不同，次设备号以 baseminor 为起始地址地址开始递
增。一般 baseminor 为 0，也就是说次设备号从 0 开始。
count：要申请的设备号数量。
name：设备名字。
注销字符设备之后要释放掉设备号，设备号释放函数如下：
void unregister_chrdev_region(dev_t from, unsigned count)
此函数有两个参数：
from：要释放的设备号。
count：表示从 from 开始，要释放的设备号数量。

# 字符类设备开发实验

## 字符设备驱动框架（静态）
```c
/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-05 19:35:25
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-05 19:51:57
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ide.h>
#define CHRDEV_MAJOR 200
#define CHRDEV_NAME "chr"

static char readbuf[20];
static char writebuf[20];
static char data[]={"kernel data !"};


static int chrdev_open(struct inode *inode ,struct file *filp){
    
}
/*
 * @description : 从设备读取数据
 * @param - filp : 要打开的设备文件(文件描述符)
 * @param - buf : 返回给用户空间的数据缓冲区
 * @param - cnt : 要读取的数据长度
 * @param - offt : 相对于文件首地址的偏移
 * @return : 读取的字节数，如果为负值，表示读取失败
 */
static ssize_t chrdev_read(struct file *filp ,char *buf,size_t cnt,loff_t *offt){
    
}
 /*
 * @description : 向设备写数据
 * @param - filp : 设备文件，表示打开的文件描述符
 * @param - buf : 要写给设备写入的数据
 * @param - cnt : 要写入的数据长度
 * @param - offt : 相对于文件首地址的偏移
 * @return : 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t chrdev_write(struct file *filp ,char *buf,size_t cnt,loff_t *offt){

}

static int chrdev_release(struct inode *inode ,struct file *filp){

}
/*
* 设备操作函数结构体
*/
 static struct file_operations chrdevbase_fops = {
    .owner = THIS_MODULE, 
    .open = chrdev_open,
    .read = chrdev_read,
    .write = chrdev_write,
    .release = chrdev_release,
 };

static int __init chrdevbase_init(void)
{
    return 0;
}
static void __exit chrdevbase_exit(void)
{

}

module_init(chrdevbase_init);
module_exit(chrdevbase_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("YURI");
```

## 驱动Makefile
KERNELDIR :=/home/yuri/SDK/IMX6ULL/IMX6ULL_SOURCE/MYLINUX/KERNEL/Linux_imx_yuri
CURRENT_PATH := $(shell pwd)
obj-m := chrdev.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean


# LED驱动实验
## 地址转换
Linux 内核启动的时候会初始化 MMU，设置好内存映射，设置好以后 CPU 访问的都是虚
拟 地 址 。 比 如 I.MX6ULL 的 GPIO1_IO03 引 脚 的 复 用 寄 存 器
IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03 的地址为 0X020E0068。如果没有开启 MMU 的话
直接向 0X020E0068 这个寄存器地址写入数据就可以配置 GPIO1_IO03 的复用功能。现在开启
了 MMU，并且设置了内存映射，因此就不能直接向 0X020E0068 这个地址写入数据了。我们必
须得到 0X020E0068 这个物理地址在 Linux 系统里面对应的虚拟地址，这里就涉及到了物理内
存和虚拟内存之间的转换，需要用到两个函数：ioremap 和 iounmap。
1、ioremap 函数
ioremap 函 数 用 于 获 取 指 定 物 理 地 址 空 间 对 应 的 虚 拟 地 址 空 间 ， 定 义 在
arch/arm/include/asm/io.h 文件中，定义如下：
ioremap 是个宏，有两个参数：cookie 和 size，真正起作用的是函数__arm_ioremap，此函
数有三个参数和一个返回值，这些参数和返回值的含义如下：
phys_addr：要映射给的物理起始地址。
size：要映射的内存空间大小。
mtype：ioremap 的类型，可以选择 MT_DEVICE、MT_DEVICE_NONSHARED、
MT_DEVICE_CACHED 和 MT_DEVICE_WC，ioremap 函数选择 MT_DEVICE。
返回值：__iomem 类型的指针，指向映射后的虚拟空间首地址。
假如我们要获取 I.MX6ULL 的 IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03 寄存器对应
的虚拟地址，使用如下代码即可
#define SW_MUX_GPIO1_IO03_BASE (0X020E0068)
static void __iomem* SW_MUX_GPIO1_IO03;
SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);


iounmap 只有一个参数 addr，此参数就是要取消映射的虚拟地址空间首地址。假如我们现
在要取消掉 IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03 寄存器的地址映射，使用如下代码
即可：
iounmap(SW_MUX_GPIO1_IO03);

## I/O 内存访问函数
这里说的 I/O 是输入/输出的意思，并不是我们学习单片机的时候讲的 GPIO 引脚。这里涉
及到两个概念：I/O 端口和 I/O 内存。当外部寄存器或内存映射到 IO 空间时，称为 I/O 端口。
当外部寄存器或内存映射到内存空间时，称为 I/O 内存。但是对于 ARM 来说没有 I/O 空间这个
概念，因此 ARM 体系下只有 I/O 内存(可以直接理解为内存)。使用 ioremap 函数将寄存器的物
理地址映射到虚拟地址以后，我们就可以直接通过指针访问这些地址，但是 Linux 内核不建议
这么做，而是推荐使用一组操作函数来对映射后的内存进行读写操作。

读操作函数
读操作函数有如下几个：
示例代码 41.1.2.1 读操作函数
1 u8 readb(const volatile void __iomem *addr)
2 u16 readw(const volatile void __iomem *addr)
3 u32 readl(const volatile void __iomem *addr)
readb、readw 和 readl 这三个函数分别对应 8bit、16bit 和 32bit 读操作，参数 addr 就是要
读取写内存地址，返回值就是读取到的数据。

写操作函数
写操作函数有如下几个：
示例代码 41.1.2.2 写操作函数
1 void writeb(u8 value, volatile void __iomem *addr)
2 void writew(u16 value, volatile void __iomem *addr)
3 void writel(u32 value, volatile void __iomem *addr)
writeb、writew 和 writel 这三个函数分别对应 8bit、16bit 和 32bit 写操作，参数 value 是要
写入的数值，addr 是要写入的地址。

```c
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define LED_MAJOR		200		/* 主设备号 */
#define LED_NAME		"led" 	/* 设备名字 */

#define LEDOFF 	0				/* 关灯 */
#define LEDON 	1				/* 开灯 */
 
/* 寄存器物理地址 */
#define CCM_CCGR1_BASE				(0X020C406C)	
#define SW_MUX_GPIO1_IO03_BASE		(0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE		(0X020E02F4)
#define GPIO1_DR_BASE				(0X0209C000)
#define GPIO1_GDIR_BASE				(0X0209C004)

/* 映射后的寄存器虚拟地址指针 */
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

/*
 * @description		: LED打开/关闭
 * @param - sta 	: LEDON(0) 打开LED，LEDOFF(1) 关闭LED
 * @return 			: 无
 */
void led_switch(u8 sta)
{
	u32 val = 0;
	if(sta == LEDON) {
		val = readl(GPIO1_DR);
		val &= ~(1 << 3);	
		writel(val, GPIO1_DR);
	}else if(sta == LEDOFF) {
		val = readl(GPIO1_DR);
		val|= (1 << 3);	
		writel(val, GPIO1_DR);
	}	
}

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int led_open(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 * @description		: 从设备读取数据 
 * @param - filp 	: 要打开的设备文件(文件描述符)
 * @param - buf 	: 返回给用户空间的数据缓冲区
 * @param - cnt 	: 要读取的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 读取的字节数，如果为负值，表示读取失败
 */
static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	return 0;
}

/*
 * @description		: 向设备写数据 
 * @param - filp 	: 设备文件，表示打开的文件描述符
 * @param - buf 	: 要写给设备写入的数据
 * @param - cnt 	: 要写入的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1];
	unsigned char ledstat;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	ledstat = databuf[0];		/* 获取状态值 */

	if(ledstat == LEDON) {	
		led_switch(LEDON);		/* 打开LED灯 */
	} else if(ledstat == LEDOFF) {
		led_switch(LEDOFF);	/* 关闭LED灯 */
	}
	return 0;
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
 */
static int led_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = 	led_release,
};

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init led_init(void)
{
	int retvalue = 0;
	u32 val = 0;

	/* 初始化LED */
	/* 1、寄存器地址映射 */
  	IMX6U_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);
	SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
  	SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
	GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);
	GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE, 4);

	/* 2、使能GPIO1时钟 */
	val = readl(IMX6U_CCM_CCGR1);
	val &= ~(3 << 26);	/* 清楚以前的设置 */
	val |= (3 << 26);	/* 设置新值 */
	writel(val, IMX6U_CCM_CCGR1);

	/* 3、设置GPIO1_IO03的复用功能，将其复用为
	 *    GPIO1_IO03，最后设置IO属性。
	 */
	writel(5, SW_MUX_GPIO1_IO03);
	
	/*寄存器SW_PAD_GPIO1_IO03设置IO属性
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 00 默认下拉
     *bit [13]: 0 kepper功能
     *bit [12]: 1 pull/keeper使能
     *bit [11]: 0 关闭开路输出
     *bit [7:6]: 10 速度100Mhz
     *bit [5:3]: 110 R0/6驱动能力
     *bit [0]: 0 低转换率
	 */
	writel(0x10B0, SW_PAD_GPIO1_IO03);

	/* 4、设置GPIO1_IO03为输出功能 */
	val = readl(GPIO1_GDIR);
	val &= ~(1 << 3);	/* 清除以前的设置 */
	val |= (1 << 3);	/* 设置为输出 */
	writel(val, GPIO1_GDIR);

	/* 5、默认关闭LED */
	val = readl(GPIO1_DR);
	val |= (1 << 3);	
	writel(val, GPIO1_DR);

	/* 6、注册字符设备驱动 */
	retvalue = register_chrdev(LED_MAJOR, LED_NAME, &led_fops);
	if(retvalue < 0){
		printk("register chrdev failed!\r\n");
		return -EIO;
	}
	return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit led_exit(void)
{
	/* 取消映射 */
	iounmap(IMX6U_CCM_CCGR1);
	iounmap(SW_MUX_GPIO1_IO03);
	iounmap(SW_PAD_GPIO1_IO03);
	iounmap(GPIO1_DR);
	iounmap(GPIO1_GDIR);

	/* 注销字符设备驱动 */
	unregister_chrdev(LED_MAJOR, LED_NAME);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("YURI");

```

# 动态使用设备号（动态）
## 申请设备号
使用 register_chrdev 函数注册字符设备的时候只需要给定一个主设备号即可，但是这样会
带来两个问题：
①、需要我们事先确定好哪些主设备号没有使用。
②、会将一个主设备号下的所有次设备号都使用掉，比如现在设置 LED 这个主设备号为
200，那么 0~1048575(2^20-1)这个区间的次设备号就全部都被 LED 一个设备分走了。这样太浪
费次设备号了！一个 LED 设备肯定只能有一个主设备号，一个次设备号。
解决这两个问题最好的方法就是要使用设备号的时候向 Linux 内核申请，需要几个就申请
几个，由 Linux 内核分配设备可以使用的设备号。这个就是我们在 40.3.2 小节讲解的设备号的
分配，如果没有指定设备号的话就使用如下函数来申请设备号：
int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)
如果给定了设备的主设备号和次设备号就使用如下所示函数来注册设备号即可：
int register_chrdev_region(dev_t from, unsigned count, const char *name)
参数 from 是要申请的起始设备号，也就是给定的设备号；参数 count 是要申请的数量，一
般都是一个；参数 name 是设备名字。
注 销 字 符 设 备 之 后 要 释 放 掉 设 备 号 ， 不 管 是 通 过 alloc_chrdev_region 函 数 还 是
register_chrdev_region 函数申请的设备号，统一使用如下释放函数：
void unregister_chrdev_region(dev_t from, unsigned count)

新字符设备驱动下，设备号分配示例代码如下：
示例代码 42.1.1.1 新字符设备驱动下设备号分配
1 int major; /* 主设备号 */
2 int minor; /* 次设备号 */
3 dev_t devid; /* 设备号 */
4 
5 if (major) { /* 定义了主设备号 */
6 devid = MKDEV(major, 0); /* 大部分驱动次设备号都选择 0 */
7 register_chrdev_region(devid, 1, "test");
8 } else { /* 没有定义设备号 */
9 alloc_chrdev_region(&devid, 0, 1, "test"); /* 申请设备号 */
10 major = MAJOR(devid); /* 获取分配号的主设备号 */
11 minor = MINOR(devid); /* 获取分配号的次设备号 */
12 }
第 1~3 行，定义了主/次设备号变量 major 和 minor，以及设备号变量 devid。
第 5 行，判断主设备号 major 是否有效，在 Linux 驱动中一般给出主设备号的话就表示这
个设备的设备号已经确定了，因为次设备号基本上都选择 0，这算个 Linux 驱动开发中约定俗
成的一种规定了。
第 6 行，如果 major 有效的话就使用 MKDEV 来构建设备号，次设备号选择 0。
第 7 行，使用 register_chrdev_region 函数来注册设备号。
第 9~11 行，如果 major 无效，那就表示没有给定设备号。此时就要使用 alloc_chrdev_region
函数来申请设备号。设备号申请成功以后使用 MAJOR 和 MINOR 来提取出主设备号和次设备
号，当然了，第 10 和 11 行提取主设备号和次设备号的代码可以不要。
如果要注销设备号的话，使用如下代码即可：
示例代码 42.1.1.2 cdev 结构体
1 unregister_chrdev_region(devid, 1); /* 注销设备号 */
注销设备号的代码很简单。

## 用申请到的设备号注册设备
### 1、字符设备结构
在 Linux 中使用 cdev 结构体表示一个字符设备，cdev 结构体在 include/linux/cdev.h 文件中
的定义如下：
示例代码 42.1.2.1 cdev 结构体
1 struct cdev {
2 struct kobject kobj;
3 struct module *owner;
4 const struct file_operations *ops;
5 struct list_head list;
6 dev_t dev;
7 unsigned int count;
8 };
在 cdev 中有两个重要的成员变量：ops 和 dev，这两个就是字符设备文件操作函数集合
file_operations 以及设备号 dev_t。编写字符设备驱动之前需要定义一个 cdev 结构体变量，这个
变量就表示一个字符设备，如下所示：
struct cdev test_cdev;
### 2、cdev_init 函数
定义好 cdev 变量以后就要使用 cdev_init 函数对其进行初始化，cdev_init 函数原型如下：
void cdev_init(struct cdev *cdev, const struct file_operations *fops)
参数 cdev 就是要初始化的 cdev 结构体变量，参数 fops 就是字符设备文件操作函数集合。
使用 cdev_init 函数初始化 cdev 变量的示例代码如下：
示例代码 42.1.2.2 cdev_init 函数使用示例代码
1 struct cdev testcdev;
2 
3 /* 设备操作函数 */
4 static struct file_operations test_fops = {
5 .owner = THIS_MODULE,
6 /* 其他具体的初始项 */
7 };
8 
9 testcdev.owner = THIS_MODULE;
10 cdev_init(&testcdev, &test_fops); /* 初始化 cdev 结构体变量 */
### 3、cdev_add 函数
cdev_add 函数用于向 Linux 系统添加字符设备(cdev 结构体变量)，首先使用 cdev_init 函数
完成对 cdev 结构体变量的初始化，然后使用 cdev_add 函数向 Linux 系统添加这个字符设备。
cdev_add 函数原型如下：
int cdev_add(struct cdev *p, dev_t dev, unsigned count)
参数 p 指向要添加的字符设备(cdev 结构体变量)，参数 dev 就是设备所使用的设备号，参
数 count 是要添加的设备数量。完善示例代码 42.1.2.2，加入 cdev_add 函数，内容如下所示：
示例代码 42.1.2.2 cdev_add 函数使用示例
1 struct cdev testcdev;
2 
3 /* 设备操作函数 */
4 static struct file_operations test_fops = {
5 .owner = THIS_MODULE,
6 /* 其他具体的初始项 */
7 };
8 
9 testcdev.owner = THIS_MODULE;
10 cdev_init(&testcdev, &test_fops); /* 初始化 cdev 结构体变量 */
11 cdev_add(&testcdev, devid, 1); /* 添加字符设备 */
示例代码 42.1.2.2 就是新的注册字符设备代码段，Linux 内核中大量的字符设备驱动都是采
用这种方法向 Linux 内核添加字符设备。如果在加上示例代码 42.1.1.1 中分配设备号的程序，
那么就它们一起实现的就是函数 register_chrdev 的功能。
### 4、cdev_del 函数
卸载驱动的时候一定要使用 cdev_del 函数从 Linux 内核中删除相应的字符设备，cdev_del
函数原型如下：
void cdev_del(struct cdev *p)
参数 p 就是要删除的字符设备。如果要删除字符设备，参考如下代码：
示例代码 42.1.2.3 cdev_del 函数使用示例
1 cdev_del(&testcdev); /* 删除 cdev */
cdev_del 和 unregister_chrdev_region 这两个函数合起来的功能相当于 unregister_chrdev 函
数。

## 自动创建设备节点
在前面的 Linux 驱动实验中，当我们使用 modprobe 加载驱动程序以后还需要使用命令
“mknod”手动创建设备节点。本节就来讲解一下如何实现自动创建设备节点，在驱动中实现
自动创建设备节点的功能以后，使用 modprobe 加载驱动模块成功的话就会自动在/dev 目录下
创建对应的设备文件。
###  mdev 机制
udev 是一个用户程序，在 Linux 下通过 udev 来实现设备文件的创建与删除，udev 可以检
测系统中硬件设备状态，可以根据系统中硬件设备状态来创建或者删除设备文件。比如使用
modprobe 命令成功加载驱动模块以后就自动在/dev 目录下创建对应的设备节点文件,使用
rmmod 命令卸载驱动模块以后就删除掉/dev 目录下的设备节点文件。使用 busybox 构建根文件
系统的时候，busybox 会创建一个 udev 的简化版本—mdev，所以在嵌入式 Linux 中我们使用
mdev 来实现设备节点文件的自动创建与删除，Linux 系统中的热插拔事件也由 mdev 管理，在
/etc/init.d/rcS 文件中如下语句：
echo /sbin/mdev > /proc/sys/kernel/hotplug
上述命令设置热插拔事件由 mdev 来管理，关于 udev 或 mdev 更加详细的工作原理这里就
不详细探讨了，我们重点来学习一下如何通过 mdev 来实现设备文件节点的自动创建与删除。
### 创建和删除类
自动创建设备节点的工作是在驱动程序的入口函数中完成的，一般在 cdev_add 函数后面添
加自动创建设备节点相关代码。首先要创建一个 class 类，class 是个结构体，定义在文件
include/linux/device.h 里面。class_create 是类创建函数，class_create 是个宏定义，内容如下：
示例代码 42.2.1.1 class_create 函数
1 #define class_create(owner, name) \
2 ({ \
3 static struct lock_class_key __key; \
4 __class_create(owner, name, &__key); \
5 })
6
7 struct class *__class_create(struct module *owner, const char *name,
8 struct lock_class_key *key)
根据上述代码，将宏 class_create 展开以后内容如下：
struct class *class_create (struct module *owner, const char *name)
class_create 一共有两个参数，参数 owner 一般为 THIS_MODULE，参数 name 是类名字。
返回值是个指向结构体 class 的指针，也就是创建的类。
卸载驱动程序的时候需要删除掉类，类删除函数为 class_destroy，函数原型如下：
void class_destroy(struct class *cls);
参数 cls 就是要删除的类。
### 创建设备
上一小节创建好类以后还不能实现自动创建设备节点，我们还需要在这个类下创建一个设
备。使用 device_create 函数在类下面创建设备，device_create 函数原型如下：
struct device *device_create(struct class *class, 
struct device *parent,
dev_t devt, 
void *drvdata, 
const char *fmt, ...)
device_create 是个可变参数函数，参数 class 就是设备要创建哪个类下面；参数 parent 是父
设备，一般为 NULL，也就是没有父设备；参数 devt 是设备号；参数 drvdata 是设备可能会使用
的一些数据，一般为 NULL；参数 fmt 是设备名字，如果设置 fmt=xxx 的话，就会生成/dev/xxx
这个设备文件。返回值就是创建好的设备。
同样的，卸载驱动的时候需要删除掉创建的设备，设备删除函数为 device_destroy，函数原
型如下：
void device_destroy(struct class *class, dev_t devt)
参数 class 是要删除的设备所处的类，参数 devt 是要删除的设备号。

## 设置文件私有数据
每个硬件设备都有一些属性，比如主设备号(dev_t)，类(class)、设备(device)、开关状态(state)
等等，在编写驱动的时候你可以将这些属性全部写成变量的形式，如下所示：
示例代码 42.3.1 变量形式的设备属性
dev_t devid; /* 设备号 */
struct cdev cdev; /* cdev */
struct class *class; /* 类 */
struct device *device; /* 设备 */
int major; /* 主设备号 */
int minor; /* 次设备号 */
这样写肯定没有问题，但是这样写不专业！对于一个设备的所有属性信息我们最好将其做
成一个结构体。编写驱动 open 函数的时候将设备结构体作为私有数据添加到设备文件中，如下
所示：
/* 设备结构体 */
1 struct test_dev{
2 dev_t devid; /* 设备号 */
3 struct cdev cdev; /* cdev */
4 struct class *class; /* 类 */
5 struct device *device; /* 设备 */
6 int major; /* 主设备号 */
7 int minor; /* 次设备号 */
8 };
9 
10 struct test_dev testdev;
11 
12 /* open 函数 */
13 static int test_open(struct inode *inode, struct file *filp)
14 {
15 filp->private_data = &testdev; /* 设置私有数据 */
16 return 0;
17 }
在 open 函数里面设置好私有数据以后，在 write、read、close 等函数中直接读取 private_data
即可得到设备结构体。


## 示例
```c
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define NEWCHRLED_CNT			1		  	/* 设备号个数 */
#define NEWCHRLED_NAME			"newchrled"	/* 名字 */
#define LEDOFF 					0			/* 关灯 */
#define LEDON 					1			/* 开灯 */
 
/* 寄存器物理地址 */
#define CCM_CCGR1_BASE				(0X020C406C)	
#define SW_MUX_GPIO1_IO03_BASE		(0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE		(0X020E02F4)
#define GPIO1_DR_BASE				(0X0209C000)
#define GPIO1_GDIR_BASE				(0X0209C004)

/* 映射后的寄存器虚拟地址指针 */
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

/* newchrled设备结构体 */
struct newchrled_dev{
	dev_t devid;			/* 设备号 	 */
	struct cdev cdev;		/* cdev 	*/
	struct class *class;		/* 类 		*/
	struct device *device;	/* 设备 	 */
	int major;				/* 主设备号	  */
	int minor;				/* 次设备号   */
};

struct newchrled_dev newchrled;	/* led设备 */

/*
 * @description		: LED打开/关闭
 * @param - sta 	: LEDON(0) 打开LED，LEDOFF(1) 关闭LED
 * @return 			: 无
 */
void led_switch(u8 sta)
{
	u32 val = 0;
	if(sta == LEDON) {
		val = readl(GPIO1_DR);
		val &= ~(1 << 3);	
		writel(val, GPIO1_DR);
	}else if(sta == LEDOFF) {
		val = readl(GPIO1_DR);
		val|= (1 << 3);	
		writel(val, GPIO1_DR);
	}	
}

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int led_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &newchrled; /* 设置私有数据 */
	return 0;
}

/*
 * @description		: 从设备读取数据 
 * @param - filp 	: 要打开的设备文件(文件描述符)
 * @param - buf 	: 返回给用户空间的数据缓冲区
 * @param - cnt 	: 要读取的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 读取的字节数，如果为负值，表示读取失败
 */
static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	return 0;
}

/*
 * @description		: 向设备写数据 
 * @param - filp 	: 设备文件，表示打开的文件描述符
 * @param - buf 	: 要写给设备写入的数据
 * @param - cnt 	: 要写入的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1];
	unsigned char ledstat;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	ledstat = databuf[0];		/* 获取状态值 */

	if(ledstat == LEDON) {	
		led_switch(LEDON);		/* 打开LED灯 */
	} else if(ledstat == LEDOFF) {
		led_switch(LEDOFF);	/* 关闭LED灯 */
	}
	return 0;
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
 */
static int led_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations newchrled_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = 	led_release,
};

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init led_init(void)
{
	u32 val = 0;

	/* 初始化LED */
	/* 1、寄存器地址映射 */
  	IMX6U_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);
	SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
  	SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
	GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);
	GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE, 4);

	/* 2、使能GPIO1时钟 */
	val = readl(IMX6U_CCM_CCGR1);
	val &= ~(3 << 26);	/* 清楚以前的设置 */
	val |= (3 << 26);	/* 设置新值 */
	writel(val, IMX6U_CCM_CCGR1);

	/* 3、设置GPIO1_IO03的复用功能，将其复用为
	 *    GPIO1_IO03，最后设置IO属性。
	 */
	writel(5, SW_MUX_GPIO1_IO03);
	
	/*寄存器SW_PAD_GPIO1_IO03设置IO属性
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 00 默认下拉
     *bit [13]: 0 kepper功能
     *bit [12]: 1 pull/keeper使能
     *bit [11]: 0 关闭开路输出
     *bit [7:6]: 10 速度100Mhz
     *bit [5:3]: 110 R0/6驱动能力
     *bit [0]: 0 低转换率
	 */
	writel(0x10B0, SW_PAD_GPIO1_IO03);

	/* 4、设置GPIO1_IO03为输出功能 */
	val = readl(GPIO1_GDIR);
	val &= ~(1 << 3);	/* 清除以前的设置 */
	val |= (1 << 3);	/* 设置为输出 */
	writel(val, GPIO1_GDIR);

	/* 5、默认关闭LED */
	val = readl(GPIO1_DR);
	val |= (1 << 3);	
	writel(val, GPIO1_DR);

	/* 注册字符设备驱动 */
	/* 1、创建设备号 */
	if (newchrled.major) {		/*  定义了设备号 */
		newchrled.devid = MKDEV(newchrled.major, 0);
		register_chrdev_region(newchrled.devid, NEWCHRLED_CNT, NEWCHRLED_NAME);
	} else {						/* 没有定义设备号 */
		alloc_chrdev_region(&newchrled.devid, 0, NEWCHRLED_CNT, NEWCHRLED_NAME);	/* 申请设备号 */
		newchrled.major = MAJOR(newchrled.devid);	/* 获取分配号的主设备号 */
		newchrled.minor = MINOR(newchrled.devid);	/* 获取分配号的次设备号 */
	}
	printk("newcheled major=%d,minor=%d\r\n",newchrled.major, newchrled.minor);	
	
	/* 2、初始化cdev */
	newchrled.cdev.owner = THIS_MODULE;
	cdev_init(&newchrled.cdev, &newchrled_fops);
	
	/* 3、添加一个cdev */
	cdev_add(&newchrled.cdev, newchrled.devid, NEWCHRLED_CNT);

	/* 4、创建类 */
	newchrled.class = class_create(THIS_MODULE, NEWCHRLED_NAME);
	if (IS_ERR(newchrled.class)) {
		return PTR_ERR(newchrled.class);
	}

	/* 5、创建设备 */
	newchrled.device = device_create(newchrled.class, NULL, newchrled.devid, NULL, NEWCHRLED_NAME);
	if (IS_ERR(newchrled.device)) {
		return PTR_ERR(newchrled.device);
	}
	
	return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit led_exit(void)
{
	/* 取消映射 */
	iounmap(IMX6U_CCM_CCGR1);
	iounmap(SW_MUX_GPIO1_IO03);
	iounmap(SW_PAD_GPIO1_IO03);
	iounmap(GPIO1_DR);
	iounmap(GPIO1_GDIR);

	/* 注销字符设备驱动 */
	cdev_del(&newchrled.cdev);/*  删除cdev */
	unregister_chrdev_region(newchrled.devid, NEWCHRLED_CNT); /* 注销设备号 */

	device_destroy(newchrled.class, newchrled.devid);
	class_destroy(newchrled.class);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("YURI");

```

# LINUX设备树
因为在新版本的 Linux 中，ARM 相关的驱动全部采用了设备树(也有支持老式驱动
的，比较少)，最新出的 CPU 其驱动开发也基本都是基于设备树的，比如 ST 新出的 STM32MP157、
NXP 的 I.MX8 系列等。我们所使用的Linux版本为 4.1.15，其支持设备树，所以正点原子 I.MX6UALPHA 开发板的所有 Linux 驱动都是基于设备树的。本章我们就来了解一下设备树的起源、重
点学习一下设备树语法。

## 什么是设备树？
设备树(Device Tree)，将这个词分开就是“设备”和“树”，描述设备树的文件叫做 DTS(Device 
Tree Source)，这个 DTS 文件采用树形结构描述板级设备，也就是开发板上的设备信息，比如
CPU 数量、 内存基地址、IIC 接口上接了哪些设备、SPI 接口上接了哪些设备等等，如图 43.1.1
所示：

在图 43.1.1 中，树的主干就是系统总线，IIC 控制器、GPIO 控制器、SPI 控制器等都是接
到系统主线上的分支。IIC 控制器有分为 IIC1 和 IIC2 两种，其中 IIC1 上接了 FT5206 和 AT24C02
这两个 IIC 设备，IIC2 上只接了 MPU6050 这个设备。DTS 文件的主要功能就是按照图 43.1.1
所示的结构来描述板子上的设备信息，DTS 文件描述设备信息是有相应的语法规则要求的，稍
后我们会详细的讲解 DTS 语法规则。

在 3.x 版本(具体哪个版本笔者也无从考证)以前的 Linux 内核中 ARM 架构并没有采用设备
树。在没有设备树的时候 Linux 是如何描述 ARM 架构中的板级信息呢？在 Linux 内核源码中
大量的 arch/arm/mach-xxx 和 arch/arm/plat-xxx 文件夹，这些文件夹里面的文件就是对应平台下
的板级信息。比如在 arch/arm/mach-smdk2440.c 中有如下内容(有缩减)：


上述代码中的结构体变量 smdk2440_fb_info 就是描述 SMDK2440 这个开发板上的 LCD 信
息的，结构体指针数组 smdk2440_devices 描述的 SMDK2440 这个开发板上的所有平台相关信
息。这个仅仅是使用 2440 这个芯片的 SMDK2440 开发板下的 LCD 信息，SMDK2440 开发板
还有很多的其他外设硬件和平台硬件信息。使用 2440 这个芯片的板子有很多，每个板子都有描
述相应板级信息的文件，这仅仅只是一个 2440。随着智能手机的发展，每年新出的 ARM 架构
芯片少说都在数十、数百款，Linux 内核下板级信息文件将会成指数级增长！这些板级信息文件
都是.c 或.h 文件，都会被硬编码进 Linux 内核中，导致 Linux 内核“虚胖”。就好比你喜欢吃自
助餐，然后花了 100 多到一家宣传看着很不错的自助餐厅，结果你想吃的牛排、海鲜、烤肉基
本没多少，全都是一些凉菜、炒面、西瓜、饮料等小吃，相信你此时肯定会脱口而出一句“F*k!”、
“骗子！”。同样的，当 Linux 之父 linus 看到 ARM 社区向 Linux 内核添加了大量“无用”、冗余
的板级信息文件，不禁的发出了一句“This whole ARM thing is a f*cking pain in the ass”。从此以
后 ARM 社区就引入了 PowerPC 等架构已经采用的设备树(Flattened Device Tree)，将这些描述
板级硬件信息的内容都从 Linux 内中分离开来，用一个专属的文件格式来描述，这个专属的文
件就叫做设备树，文件扩展名为.dts。一个 SOC 可以作出很多不同的板子，这些不同的板子肯
定是有共同的信息，将这些共同的信息提取出来作为一个通用的文件，其他的.dts 文件直接引
用这个通用文件即可，这个通用文件就是.dtsi 文件，类似于 C 语言中的头文件。一般.dts 描述
板级信息(也就是开发板上有哪些 IIC 设备、SPI 设备等)，.dtsi 描述 SOC 级信息(也就是 SOC 有
几个 CPU、主频是多少、各个外设控制器信息等)。


## DTS、DTB 和 DTC
上一小节说了，设备树源文件扩展名为.dts，但是我们在前面移植 Linux 的时候却一直在使
用.dtb 文件，那么 DTS 和 DTB 这两个文件是什么关系呢？DTS 是设备树源码文件，DTB 是将
DTS 编译以后得到的二进制文件。将.c 文件编译为.o 需要用到 gcc 编译器，那么将.dts 编译为.dtb
需要什么工具呢？需要用到 DTC 工具！DTC 工具源码在 Linux 内核的 scripts/dtc 目录下，
scripts/dtc/Makefile 文件内容如下：
1 hostprogs-y := dtc
2 always := $(hostprogs-y)
3
4 dtc-objs:= dtc.o flattree.o fstree.o data.o livetree.o treesource.o \
5 srcpos.o checks.o util.o
6 dtc-objs += dtc-lexer.lex.o dtc-parser.tab.o

可以看出，DTC 工具依赖于 dtc.c、flattree.c、fstree.c 等文件，最终编译并链接出 DTC 这
个主机文件。如果要编译 DTS 文件的话只需要进入到 Linux 源码根目录下，然后执行如下命
令：
make all
或者：
make dtbs
“make all”命令是编译 Linux 源码中的所有东西，包括 zImage，.ko 驱动模块以及设备
树，如果只是编译设备树的话建议使用“make dtbs”命令。
基于 ARM 架构的 SOC 有很多种，一种 SOC 又可以制作出很多款板子，每个板子都有一
个对应的 DTS 文件，那么如何确定编译哪一个 DTS 文件呢？我们就以 I.MX6ULL 这款芯片对
应的板子为例来看一下，打开 arch/arm/boot/dts/Makefile，有如下内容：

可以看出，当选中 I.MX6ULL 这个 SOC 以后(CONFIG_SOC_IMX6ULL=y)，所有使用到
I.MX6ULL 这个 SOC 的板子对应的.dts 文件都会被编译为.dtb。如果我们使用 I.MX6ULL 新做
了一个板子，只需要新建一个此板子对应的.dts 文件，然后将对应的.dtb 文件名添加到 dtb-
$(CONFIG_SOC_IMX6ULL)下，这样在编译设备树的时候就会将对应的.dts 编译为二进制的.dtb
文件。
示例代码 43.2.2 中第 422 和 423 行就是我们在给正点原子的 I.MX6U-ALPHA 开发板移植
Linux 系统的时候添加的设备树。关于.dtb 文件怎么使用这里就不多说了，前面讲解 Uboot 移
植、Linux 内核移植的时候已经无数次的提到如何使用.dtb 文件了(uboot 中使用 bootz 或 bootm
命令向 Linux 内核传递二进制设备树文件(.dtb))。

## DTS 语法
虽然我们基本上不会从头到尾重写一个.dts 文件，大多时候是直接在 SOC 厂商提供的.dts
文件上进行修改。但是 DTS 文件语法我们还是需要详细的学习一遍，因为我们肯定需要修改.dts
文件。大家不要看到要学习新的语法就觉得会很复杂，DTS 语法非常的人性化，是一种 ASCII
文本文件，不管是阅读还是修改都很方便。

### dtsi 头文件
和 C 语言一样，设备树也支持头文件，设备树的头文件扩展名为.dtsi。在 imx6ull-alientekemmc.dts 中有如下所示内容：
示例代码 43.3.1.1 imx6ull-alientek-emmc.dts 文件代码段
12 #include <dt-bindings/input/input.h>
13 #include "imx6ull.dtsi"
第 12 行，使用“#include”来引用“input.h”这个.h 头文件。
第 13 行，使用“#include”来引用“imx6ull.dtsi”这个.dtsi 头文件。
看到这里，大家可能会疑惑，不是说设备树的扩展名是.dtsi 吗？为什么也可以直接引用 C
语言中的.h 头文件呢？这里并没有错，.dts 文件引用 C 语言中的.h 文件，甚至也可以引用.dts 文
件，打开 imx6ull-14x14-evk-gpmi-weim.dts 这个文件，此文件中有如下内容：

可以看出，示例代码 43.3.1.2 中直接引用了.dts 文件，因此在.dts 设备树文件中，可以通过
“#include”来引用.h、.dtsi 和.dts 文件。只是，我们在编写设备树头文件的时候最好选择.dtsi 后
缀。
一般.dtsi 文件用于描述 SOC 的内部外设信息，比如 CPU 架构、主频、外设寄存器地址范
围，比如 UART、IIC 等等。比如 imx6ull.dtsi 就是描述 I.MX6ULL 这颗 SOC 内部外设情况信息
的，内容如下：

## 设备节点
设备树是采用树形结构来描述板子上的设备信息的文件，每个设备都是一个节点，叫做设
备节点，每个节点都通过一些属性信息来描述节点信息，属性就是键—值对。以下是从
imx6ull.dtsi 文件中缩减出来的设备树文件内容：

示例代码 43.3.2.1 设备树模板
1 / {
2 aliases {
3 can0 = &flexcan1;
4 };
5 
6 cpus {
7 #address-cells = <1>;
8 #size-cells = <0>;
9 
10 cpu0: cpu@0 {
11 compatible = "arm,cortex-a7";
12 device_type = "cpu";
13 reg = <0>;
14 };
15 };
16
17 intc: interrupt-controller@00a01000 {
18 compatible = "arm,cortex-a7-gic";
19 #interrupt-cells = <3>;
20 interrupt-controller;
21 reg = <0x00a01000 0x1000>,
22 <0x00a02000 0x100>;
23 };
24 }
第 1 行，“/”是根节点，每个设备树文件只有一个根节点。细心的同学应该会发现，imx6ull.dtsi
和 imx6ull-alientek-emmc.dts 这两个文件都有一个“/”根节点，这样不会出错吗？不会的，因为
这两个“/”根节点的内容会合并成一个根节点。
第 2、6 和 17 行，aliases、cpus 和 intc 是三个子节点，在设备树中节点命名格式如下：
node-name@unit-address
其中“node-name”是节点名字，为 ASCII 字符串，节点名字应该能够清晰的描述出节点的
功能，比如“uart1”就表示这个节点是 UART1 外设。“unit-address”一般表示设备的地址或寄
存器首地址，如果某个节点没有地址或者寄存器的话“unit-address”可以不要，比如“cpu@0”、
“interrupt-controller@00a01000”。
但是我们在示例代码 43.3.2.1 中我们看到的节点命名却如下所示：
cpu0:cpu@0
上述命令并不是“node-name@unit-address”这样的格式，而是用“：”隔开成了两部分，“：”
前面的是节点标签(label)，“：”后面的才是节点名字，格式如下所示：
label: node-name@unit-address
引入 label 的目的就是为了方便访问节点，可以直接通过&label 来访问这个节点，比如通过
&cpu0 就可以访问“cpu@0”这个节点，而不需要输入完整的节点名字。再比如节点 “intc: 
interrupt-controller@00a01000”，节点 label 是 intc，而节点名字就很长了，为“interruptcontroller@00a01000”。很明显通过&intc 来访问“interrupt-controller@00a01000”这个节点要方
便很多！
第 10 行，cpu0 也是一个节点，只是 cpu0 是 cpus 的子节点。
每个节点都有不同属性，不同的属性又有不同的内容，属性都是键值对，值可以为空或任
意的字节流。设备树源码中常用的几种数据形式如下所示：

①、字符串 
compatible = "arm,cortex-a7";
上述代码设置 compatible 属性的值为字符串“arm,cortex-a7”。
②、32 位无符号整数
reg = <0>;
上述代码设置 reg 属性的值为 0，reg 的值也可以设置为一组值，比如：
reg = <0 0x123456 100>;
③、字符串列表
属性值也可以为字符串列表，字符串和字符串之间采用“,”隔开，如下所示：
compatible = "fsl,imx6ull-gpmi-nand", "fsl, imx6ul-gpmi-nand";
上述代码设置属性 compatible 的值为“fsl,imx6ull-gpmi-nand”和“fsl, imx6ul-gpmi-nand”。


### 1、标准属性
节点是由一堆的属性组成，节点都是具体的设备，不同的设备需要的属性不同，用户可以
自定义属性。除了用户自定义属性，有很多属性是标准属性，Linux 下的很多外设驱动都会使用
这些标准属性，本节我们就来学习一下几个常用的标准属性。
1、compatible 属性
compatible 属性也叫做“兼容性”属性，这是非常重要的一个属性！compatible 属性的值是
一个字符串列表，compatible 属性用于将设备和驱动绑定起来。字符串列表用于选择设备所要
使用的驱动程序，compatible 属性的值格式如下所示：
"manufacturer,model"
其中 manufacturer 表示厂商，model 一般是模块对应的驱动名字。比如 imx6ull-alientekemmc.dts 中 sound 节点是 I.MX6U-ALPHA 开发板的音频设备节点，I.MX6U-ALPHA 开发板上
的音频芯片采用的欧胜(WOLFSON)出品的 WM8960，sound 节点的 compatible 属性值如下：
compatible = "fsl,imx6ul-evk-wm8960","fsl,imx-audio-wm8960";
属性值有两个，分别为“fsl,imx6ul-evk-wm8960”和“fsl,imx-audio-wm8960”，其中“fsl”
表示厂商是飞思卡尔，“imx6ul-evk-wm8960”和“imx-audio-wm8960”表示驱动模块名字。sound
这个设备首先使用第一个兼容值在 Linux 内核里面查找，看看能不能找到与之匹配的驱动文件，
如果没有找到的话就使用第二个兼容值查。
一般驱动程序文件都会有一个 OF 匹配表，此 OF 匹配表保存着一些 compatible 值，如果设
备节点的 compatible 属性值和 OF 匹配表中的任何一个值相等，那么就表示设备可以使用这个
驱动。比如在文件 imx-wm8960.c 中有如下内容：

632 static const struct of_device_id imx_wm8960_dt_ids[] = {
633 { .compatible = "fsl,imx-audio-wm8960", },
634 { /* sentinel */ }
635 };
636 MODULE_DEVICE_TABLE(of, imx_wm8960_dt_ids);
637
638 static struct platform_driver imx_wm8960_driver = {
639 .driver = {
640 .name = "imx-wm8960",
641 .pm = &snd_soc_pm_ops,
642 .of_match_table = imx_wm8960_dt_ids,
643 },
644 .probe = imx_wm8960_probe,
645 .remove = imx_wm8960_remove,
646 };
第 632~635 行的数组 imx_wm8960_dt_ids 就是 imx-wm8960.c 这个驱动文件的匹配表，此
匹配表只有一个匹配值“fsl,imx-audio-wm8960”。如果在设备树中有哪个节点的 compatible 属
性值与此相等，那么这个节点就会使用此驱动文件。
第 642 行，wm8960 采用了 platform_driver 驱动模式，关于 platform_driver 驱动后面会讲
解。此行设置.of_match_table 为 imx_wm8960_dt_ids，也就是设置这个 platform_driver 所使用的
OF 匹配表。
### 2、model 属性
model 属性值也是一个字符串，一般 model 属性描述设备模块信息，比如名字什么的，比
如：
model = "wm8960-audio";

### 3、status 属性
status 属性看名字就知道是和设备状态有关的，status 属性值也是字符串，字符串是设备的
状态信息，可选的状态如表 43.3.3.1 所示：

### 4、#address-cells 和#size-cells 属性
这两个属性的值都是无符号 32 位整形，#address-cells 和#size-cells 这两个属性可以用在任
何拥有子节点的设备中，用于描述子节点的地址信息。#address-cells 属性值决定了子节点 reg 属
性中地址信息所占用的字长(32 位)，#size-cells 属性值决定了子节点 reg 属性中长度信息所占的
字长(32 位)。#address-cells 和#size-cells 表明了子节点应该如何编写 reg 属性值，一般 reg 属性
都是和地址有关的内容，和地址相关的信息有两种：起始地址和地址长度，reg 属性的格式一为：
reg = <address1 length1 address2 length2 address3 length3……>
每个“address length”组合表示一个地址范围，其中 address 是起始地址，length 是地址长
度，#address-cells 表明 address 这个数据所占用的字长，#size-cells 表明 length 这个数据所占用
的字长，比如

1 spi4 {
2 compatible = "spi-gpio";
3 #address-cells = <1>;
4 #size-cells = <0>;
5 
6 gpio_spi: gpio_spi@0 {
7 compatible = "fairchild,74hc595";
8 reg = <0>;
9 };
10 };
11
12 aips3: aips-bus@02200000 {
13 compatible = "fsl,aips-bus", "simple-bus";
14 #address-cells = <1>;
15 #size-cells = <1>;
16
17 dcp: dcp@02280000 {
18 compatible = "fsl,imx6sl-dcp";
19 reg = <0x02280000 0x4000>;
20 };
21 };
第 3，4 行，节点 spi4 的#address-cells = <1>，#size-cells = <0>，说明 spi4 的子节点 reg 属
性中起始地址所占用的字长为 1，地址长度所占用的字长为 0。
第 8 行，子节点 gpio_spi: gpio_spi@0 的 reg 属性值为 <0>，因为父节点设置了#addresscells = <1>，#size-cells = <0>，因此 addres=0，没有 length 的值，相当于设置了起始地址，而没
有设置地址长度。
第 14，15 行，设置 aips3: aips-bus@02200000 节点#address-cells = <1>，#size-cells = <1>，
说明 aips3: aips-bus@02200000 节点起始地址长度所占用的字长为 1，地址长度所占用的字长也
为 1。
第 19 行，子节点 dcp: dcp@02280000 的 reg 属性值为<0x02280000 0x4000>，因为父节点设
置了#address-cells = <1>，#size-cells = <1>，address= 0x02280000，length= 0x4000，相当于设置
了起始地址为 0x02280000，地址长度为 0x40000。

### 5、rreg 属性
reg 属性前面已经提到过了，reg 属性的值一般是(address，length)对。reg 属性一般用于描
述设备地址空间资源信息，一般都是某个外设的寄存器地址范围信息，比如在 imx6ull.dtsi 中有
如下内容：
323 uart1: serial@02020000 {
324 compatible = "fsl,imx6ul-uart",
325 "fsl,imx6q-uart", "fsl,imx21-uart";
326 reg = <0x02020000 0x4000>;
327 interrupts = <GIC_SPI 26 IRQ_TYPE_LEVEL_HIGH>;
328 clocks = <&clks IMX6UL_CLK_UART1_IPG>,
329 <&clks IMX6UL_CLK_UART1_SERIAL>;
330 clock-names = "ipg", "per";
331 status = "disabled";
332 };
上述代码是节点 uart1，uart1 节点描述了 I.MX6ULL 的 UART1 相关信息，重点是第 326 行
的 reg 属性。其中 uart1 的父节点 aips1: aips-bus@02000000 设置了#address-cells = <1>、#sizecells = <1>，因此 reg 属性中 address=0x02020000，length=0x4000。查阅《I.MX6ULL 参考手册》
可知，I.MX6ULL 的 UART1 寄存器首地址为 0x02020000，但是 UART1 的地址长度(范围)并没
有 0x4000 这么多，这里我们重点是获取 UART1 寄存器首地址。


### 6、ranges 属性
ranges属性值可以为空或者按照(child-bus-address,parent-bus-address,length)格式编写的数字
矩阵，ranges 是一个地址映射/转换表，ranges 属性每个项目由子地址、父地址和地址空间长度
这三部分组成：
child-bus-address：子总线地址空间的物理地址，由父节点的#address-cells 确定此物理地址
所占用的字长。
parent-bus-address：父总线地址空间的物理地址，同样由父节点的#address-cells 确定此物
理地址所占用的字长。
length：子地址空间的长度，由父节点的#size-cells 确定此地址长度所占用的字长。
如果 ranges 属性值为空值，说明子地址空间和父地址空间完全相同，不需要进行地址转换，
对于我们所使用的 I.MX6ULL 来说，子地址空间和父地址空间完全相同，因此会在 imx6ull.dtsi
中找到大量的值为空的 ranges 属性，如下所示：

第 142 行定义了 ranges 属性，但是 ranges 属性值为空。
ranges 属性不为空的示例代码如下所示：
示例代码 43.3.3.5 ranges 属性不为空
1 soc {
2 compatible = "simple-bus";
3 #address-cells = <1>;
4 #size-cells = <1>;
5 ranges = <0x0 0xe0000000 0x00100000>;
6 
7 serial {
8 device_type = "serial";
9 compatible = "ns16550";
10 reg = <0x4600 0x100>;
11 clock-frequency = <0>;
12 interrupts = <0xA 0x8>;
13 interrupt-parent = <&ipic>;
14 };
15 };
第 5 行，节点 soc 定义的 ranges 属性，值为<0x0 0xe0000000 0x00100000>，此属性值指定
了一个 1024KB(0x00100000)的地址范围，子地址空间的物理起始地址为 0x0，父地址空间的物
理起始地址为 0xe0000000。
第 10 行，serial 是串口设备节点，reg 属性定义了 serial 设备寄存器的起始地址为 0x4600，
寄存器长度为 0x100。经过地址转换，serial 设备可以从 0xe0004600 开始进行读写操作，
0xe0004600=0x4600+0xe0000000。

7、name 属性
name 属性值为字符串，name 属性用于记录节点名字，name 属性已经被弃用，不推荐使用
name 属性，一些老的设备树文件可能会使用此属性。
8、device_type 属性
device_type 属性值为字符串，IEEE 1275 会用到此属性，用于描述设备的 FCode，但是设
备树没有 FCode，所以此属性也被抛弃了。此属性只能用于 cpu 节点或者 memory 节点。
imx6ull.dtsi 的 cpu0 节点用到了此属性，内容如下所示：


## 使用设备树之前设备匹配方法
在没有使用设备树以前，uboot 会向 Linux 内核传递一个叫做 machine id 的值，machine id
也就是设备 ID，告诉 Linux 内核自己是个什么设备，看看 Linux 内核是否支持。Linux 内核是
支持很多设备的，针对每一个设备(板子)，Linux内核都用MACHINE_START和MACHINE_END
来定义一个 machine_desc 结构体来描述这个设备，比如在文件 arch/arm/mach-imx/machmx35_3ds.c 中有如下定义：
613 MACHINE_START(MX35_3DS, "Freescale MX35PDK")
614 /* Maintainer: Freescale Semiconductor, Inc */
615 .atag_offset = 0x100,
616 .map_io = mx35_map_io,
617 .init_early = imx35_init_early,
618 .init_irq = mx35_init_irq,
619 .init_time = mx35pdk_timer_init,
620 .init_machine = mx35_3ds_init,
621 .reserve = mx35_3ds_reserve,
622 .restart = mxc_restart,
623 MACHINE_END

上述代码就是定义了“Freescale MX35PDK”这个设备，其中 MACHINE_START 和
MACHINE_END 定义在文件 arch/arm/include/asm/mach/arch.h 中，内容如下：
示例代码 43.3.4.3 MACHINE_START 和 MACHINE_END 宏定义
#define MACHINE_START(_type,_name) \
static const struct machine_desc __mach_desc_##_type \
__used \
__attribute__((__section__(".arch.info.init"))) = { \
 .nr = MACH_TYPE_##_type, \
 .name = _name,
#define MACHINE_END \
};
根据 MACHINE_START 和 MACHINE_END 的宏定义，将示例代码 43.3.4.2 展开后如下所
示：
示例代码 43.3.4.3 展开以后
1 static const struct machine_desc __mach_desc_MX35_3DS \
2 __used \
3 __attribute__((__section__(".arch.info.init"))) = {
4 .nr = MACH_TYPE_MX35_3DS, 
5 .name = "Freescale MX35PDK",
6 /* Maintainer: Freescale Semiconductor, Inc */
7 .atag_offset = 0x100,
8 .map_io = mx35_map_io,
9 .init_early = imx35_init_early,
10 .init_irq = mx35_init_irq,
11 .init_time = mx35pdk_timer_init,
12 .init_machine = mx35_3ds_init,
13 .reserve = mx35_3ds_reserve,
14 .restart = mxc_restart,
15 };
从示例代码 43.3.4.3 中可以看出，这里定义了一个 machine_desc 类型的结构体变量
__mach_desc_MX35_3DS ， 这 个 变 量 存 储 在 “ .arch.info.init ” 段 中 。 第 4 行 的
MACH_TYPE_MX35_3DS 就 是 “ Freescale MX35PDK ” 这 个 板 子 的 machine id 。
MACH_TYPE_MX35_3DS 定义在文件 include/generated/mach-types.h 中，此文件定义了大量的
machine id，内容如下所示：
示例代码 43.3.4.3 mach-types.h 文件中的 machine id
15 #define MACH_TYPE_EBSA110 0
16 #define MACH_TYPE_RISCPC 1
17 #define MACH_TYPE_EBSA285 4
18 #define MACH_TYPE_NETWINDER 5
19 #define MACH_TYPE_CATS 6
20 #define MACH_TYPE_SHARK 15
21 #define MACH_TYPE_BRUTUS 16
22 #define MACH_TYPE_PERSONAL_SERVER 17
......
287 #define MACH_TYPE_MX35_3DS 1645
......
1000 #define MACH_TYPE_PFLA03 4575
第 287 行就是 MACH_TYPE_MX35_3DS 的值，为 1645。
前面说了，uboot 会给 Linux 内核传递 machine id 这个参数，Linux 内核会检查这个 machine
id，其实就是将 machine id 与示例代码 43.3.4.3 中的这些 MACH_TYPE_XXX 宏进行对比，看
看有没有相等的，如果相等的话就表示 Linux 内核支持这个设备，如果不支持的话那么这个设
备就没法启动 Linux 内核。

## 使用设备树以后的设备匹配方法
当 Linux 内 核 引 入 设 备 树 以 后 就 不 再 使 用 MACHINE_START 了 ， 而 是 换 为 了
DT_MACHINE_START。DT_MACHINE_START 也定义在文件 arch/arm/include/asm/mach/arch.h
里面，定义如下

#define DT_MACHINE_START(_name, _namestr) \
static const struct machine_desc __mach_desc_##_name \
__used \
__attribute__((__section__(".arch.info.init"))) = { \
 .nr = ~0, \
 .name = _namestr,
可以看出，DT_MACHINE_START 和 MACHINE_START 基本相同，只是.nr 的设置不同，
在 DT_MACHINE_START 里面直接将.nr 设置为~0。说明引入设备树以后不会再根据 machine
id 来检查 Linux 内核是否支持某个设备了。
打开文件 arch/arm/mach-imx/mach-imx6ul.c，有如下所示内容：
208 static const char *imx6ul_dt_compat[] __initconst = {
209 "fsl,imx6ul",
210 "fsl,imx6ull",
211 NULL,
212 };
213
214 DT_MACHINE_START(IMX6UL, "Freescale i.MX6 Ultralite (Device Tree)")
215 .map_io = imx6ul_map_io,
216 .init_irq = imx6ul_init_irq,
217 .init_machine = imx6ul_init_machine,
218 .init_late = imx6ul_init_late,
219 .dt_compat = imx6ul_dt_compat,
220 MACHINE_END
machine_desc 结构体中有个.dt_compat 成员变量，此成员变量保存着本设备兼容属性，示
例代码 43.3.4.5 中设置.dt_compat = imx6ul_dt_compat，imx6ul_dt_compat 表里面有"fsl,imx6ul"
和"fsl,imx6ull"这两个兼容值。只要某个设备(板子)根节点“/”的 compatible 属性值与
imx6ul_dt_compat 表中的任何一个值相等，那么就表示 Linux 内核支持此设备。imx6ull-alientekemmc.dts 中根节点的 compatible 属性值如下：
compatible = "fsl,imx6ull-14x14-evk", "fsl,imx6ull";
其中“fsl,imx6ull”与 imx6ul_dt_compat 中的“fsl,imx6ull”匹配，因此 I.MX6U-ALPHA 开
发板可以正常启动 Linux 内核。如果将 imx6ull-alientek-emmc.dts 根节点的 compatible 属性改为
其他的值，比如：
compatible = "fsl,imx6ull-14x14-evk", "fsl,imx6ullll"
重新编译 DTS，并用新的 DTS 启动 Linux 内核，结果如图 43.3.4.1 所示的错误提示：


# 向节点追加或修改内容
产品开发过程中可能面临着频繁的需求更改，比如第一版硬件上有一个 IIC 接口的六轴芯
片 MPU6050，第二版硬件又要把这个 MPU6050 更换为 MPU9250 等。一旦硬件修改了，我们
就要同步的修改设备树文件，毕竟设备树是描述板子硬件信息的文件。假设现在有个六轴芯片
fxls8471，fxls8471 要接到 I.MX6U-ALPHA 开发板的 I2C1 接口上，那么相当于需要在 i2c1 这
个节点上添加一个 fxls8471 子节点。先看一下 I2C1 接口对应的节点，打开文件 imx6ull.dtsi 文
件，找到如下所示内容：


示例代码 43.3.5.1 就是 I.MX6ULL 的 I2C1 节点，现在要在 i2c1 节点下创建一个子节点，
这个子节点就是 fxls8471，最简单的方法就是在 i2c1 下直接添加一个名为 fxls8471 的子节点，
如下所示：
示例代码 43.3.5.2 添加 fxls8471 子节点
937 i2c1: i2c@021a0000 {
938 #address-cells = <1>;
939 #size-cells = <0>;
940 compatible = "fsl,imx6ul-i2c", "fsl,imx21-i2c";
941 reg = <0x021a0000 0x4000>;
942 interrupts = <GIC_SPI 36 IRQ_TYPE_LEVEL_HIGH>;
943 clocks = <&clks IMX6UL_CLK_I2C1>;
944 status = "disabled";
945946 //fxls8471 子节点
947 fxls8471@1e {
948 compatible = "fsl,fxls8471";
949 reg = <0x1e>;
950 };
951 };

第 947~950 行就是添加的 fxls8471 这个芯片对应的子节点。但是这样会有个问题！i2c1 节
点是定义在 imx6ull.dtsi 文件中的，而 imx6ull.dtsi 是设备树头文件，其他所有使用到 I.MX6ULL
这颗 SOC 的板子都会引用 imx6ull.dtsi 这个文件。直接在 i2c1 节点中添加 fxls8471 就相当于在
其他的所有板子上都添加了 fxls8471 这个设备，但是其他的板子并没有这个设备啊！因此，按
照示例代码 43.3.5.2 这样写肯定是不行的。
这里就要引入另外一个内容，那就是如何向节点追加数据，我们现在要解决的就是如何向
i2c1 节点追加一个名为 fxls8471 的子节点，而且不能影响到其他使用到 I.MX6ULL 的板子。
I.MX6U-ALPHA 开发板使用的设备树文件为 imx6ull-alientek-emmc.dts，因此我们需要在
imx6ull-alientek-emmc.dts 文件中完成数据追加的内容，方式如下：

示例代码 43.3.5.3 节点追加数据方法
1 &i2c1 {
2 /* 要追加或修改的内容 */
3 };
第 1 行，&i2c1 表示要访问 i2c1 这个 label 所对应的节点，也就是 imx6ull.dtsi 中的“i2c1: 
i2c@021a0000”。
第 2 行，花括号内就是要向 i2c1 这个节点添加的内容，包括修改某些属性的值。
打开 imx6ull-alientek-emmc.dts，找到如下所示内容：

224 &i2c1 {
225 clock-frequency = <100000>;
226 pinctrl-names = "default";
227 pinctrl-0 = <&pinctrl_i2c1>;
228 status = "okay";
229
230 mag3110@0e {
231 compatible = "fsl,mag3110";
232 reg = <0x0e>;
233 position = <2>;
234 };
235
236 fxls8471@1e {
237 compatible = "fsl,fxls8471";
238 reg = <0x1e>;
239 position = <0>;
240 interrupt-parent = <&gpio5>;
241 interrupts = <0 8>;
242 };
示例代码 43.3.5.4 就是向 i2c1 节点添加/修改数据，比如第 225 行的属性“clock-frequency”
就表示 i2c1 时钟为 100KHz。“clock-frequency”就是新添加的属性。
第 228 行，将 status 属性的值由原来的 disabled 改为 okay。
第 230~234 行，i2c1 子节点 mag3110，因为 NXP 官方开发板在 I2C1 上接了一个磁力计芯
片 mag3110，正点原子的 I.MX6U-ALPHA 开发板并没有使用 mag3110。
第 236~242 行，i2c1 子节点 fxls8471，同样是因为 NXP 官方开发板在 I2C1 上接了 fxls8471
这颗六轴芯片。
因为示例代码 43.3.5.4 中的内容是 imx6ull-alientek-emmc.dts 这个文件内的，所以不会对
使用 I.MX6ULL 这颗 SOC 的其他板子造成任何影响。这个就是向节点追加或修改内容，重点
就是通过&label 来访问节点，然后直接在里面编写要追加或者修改的内容。

## Linux 内核解析 DTB 文件
Linux 内核在启动的时候会解析 DTB 文件，然后在/proc/device-tree 目录下生成相应的设备
树节点文件。接下来我们简单分析一下 Linux 内核是如何解析 DTB 文件的，流程如图 43.7.1 所
示：在 start_kernel 函数中完成了设备树节点解析的工作，最终实际工
作的函数为 unflatten_dt_node。

比如我们现在要想在 I.MX6ULL 这颗 SOC 的 I2C 下添加一个节点，那么就可以查看
Documentation/devicetree/bindings/i2c/i2c-imx.txt，此文档详细的描述了 I.MX 系列的 SOC 如何
在设备树中添加 I2C 设备节点，文档内容如下所示：


Optional properties:
- clock-frequency : Constains desired I2C/HS-I2C bus clock frequency in 
Hz.
 The absence of the propoerty indicates the default frequency 100 kHz.
- dmas: A list of two dma specifiers, one for each entry in dma-names.
- dma-names: should contain "tx" and "rx".
Examples:
i2c@83fc4000 { /* I2C2 on i.MX51 */
 compatible = "fsl,imx51-i2c", "fsl,imx21-i2c";
 reg = <0x83fc4000 0x4000>;
 interrupts = <63>;
};
i2c@70038000 { /* HS-I2C on i.MX51 */
 compatible = "fsl,imx51-i2c", "fsl,imx21-i2c";
 reg = <0x70038000 0x4000>;
 interrupts = <64>;
 clock-frequency = <400000>;
};
i2c0: i2c@40066000 { /* i2c0 on vf610 */
 compatible = "fsl,vf610-i2c";
 reg = <0x40066000 0x1000>;
 interrupts =<0 71 0x04>;
 dmas = <&edma0 0 50>,
 <&edma0 0 51>;
 dma-names = "rx","tx";
};

## 设备树常用 OF 操作函数
设备树描述了设备的详细信息，这些信息包括数字类型的、字符串类型的、数组类型的，
我们在编写驱动的时候需要获取到这些信息。比如设备树使用 reg 属性描述了某个外设的寄存
器地址为 0X02005482，长度为 0X400，我们在编写驱动的时候需要获取到 reg 属性的
0X02005482 和 0X400 这两个值，然后初始化外设。Linux 内核给我们提供了一系列的函数来获
取设备树中的节点或者属性信息，这一系列的函数都有一个统一的前缀“of_”，所以在很多资
料里面也被叫做 OF 函数。这些 OF 函数原型都定义在 include/linux/of.h 文件中。

与查找节点有关的 OF 函数有 5 个，我们依次来看一下。
### 1、of_find_node_by_name 函数
of_find_node_by_name 函数通过节点名字查找指定的节点，函数原型如下：
struct device_node *of_find_node_by_name(struct device_node *from,
const char *name);
函数参数和返回值含义如下：
from：开始查找的节点，如果为 NULL 表示从根节点开始查找整个设备树。
name：要查找的节点名字。
返回值：找到的节点，如果为 NULL 表示查找失败。
### 2、of_find_node_by_type 函数
of_find_node_by_type 函数通过 device_type 属性查找指定的节点，函数原型如下：
struct device_node *of_find_node_by_type(struct device_node *from, const char *type)
函数参数和返回值含义如下：
from：开始查找的节点，如果为 NULL 表示从根节点开始查找整个设备树。
type：要查找的节点对应的 type 字符串，也就是 device_type 属性值。
返回值：找到的节点，如果为 NULL 表示查找失败。
### 3、of_find_compatible_node 函数
of_find_compatible_node 函数根据 device_type 和 compatible 这两个属性查找指定的节点，
函数原型如下：
struct device_node *of_find_compatible_node(struct device_node *from,
const char *type, 
const char *compatible)
函数参数和返回值含义如下：
from：开始查找的节点，如果为 NULL 表示从根节点开始查找整个设备树。
type：要查找的节点对应的 type 字符串，也就是 device_type 属性值，可以为 NULL，表示
忽略掉 device_type 属性。
compatible：要查找的节点所对应的 compatible 属性列表。
返回值：找到的节点，如果为 NULL 表示查找失败
### 4、of_find_matching_node_and_match 函数
of_find_matching_node_and_match 函数通过 of_device_id 匹配表来查找指定的节点，函数原
型如下：
struct device_node *of_find_matching_node_and_match(struct device_node *from,
 const struct of_device_id *matches,
 const struct of_device_id **match)
函数参数和返回值含义如下：
from：开始查找的节点，如果为 NULL 表示从根节点开始查找整个设备树。
matches：of_device_id 匹配表，也就是在此匹配表里面查找节点。
match：找到的匹配的 of_device_id。
返回值：找到的节点，如果为 NULL 表示查找失败
### 5、of_find_node_by_path 函数
of_find_node_by_path 函数通过路径来查找指定的节点，函数原型如下：
inline struct device_node *of_find_node_by_path(const char *path)
函数参数和返回值含义如下：
path：带有全路径的节点名，可以使用节点的别名，比如“/backlight”就是 backlight 这个
节点的全路径。
返回值：找到的节点，如果为 NULL 表示查找失败

### 查找父/子节点的 OF 函数
Linux 内核提供了几个查找节点对应的父节点或子节点的 OF 函数，我们依次来看一下。
1、of_get_parent 函数
of_get_parent 函数用于获取指定节点的父节点(如果有父节点的话)，函数原型如下：
struct device_node *of_get_parent(const struct device_node *node)
函数参数和返回值含义如下：node：要查找的父节点的节点。
返回值：找到的父节点。
2、of_get_next_child 函数
of_get_next_child 函数用迭代的方式查找子节点，函数原型如下：
struct device_node *of_get_next_child(const struct device_node *node,
 struct device_node *prev)
函数参数和返回值含义如下：
node：父节点。
prev：前一个子节点，也就是从哪一个子节点开始迭代的查找下一个子节点。可以设置为
NULL，表示从第一个子节点开始。
返回值：找到的下一个子节点。


# 设备树LED实验
在根节点“/”下创建一个名为“yuri_led”的子节点，打开 imx6ull-yuri-emmc.dts 文件，
在根节点“/”最后面输入如下所示内容：
	yuri_led{
		#address-cells = <1>;
		#size-cells = <1>;
		compatible="yuri_led";
		status = "okay";
		reg=<0X020C406C 0X04 /* CCM_CCGR1_BASE */
			0X020E0068 0X04 /* SW_MUX_GPIO1_IO03_BASE */
			0X020E02F4 0X04 /* SW_PAD_GPIO1_IO03_BASE */
			0X0209C000 0X04 /* GPIO1_DR_BASE */
			0X0209C004 0X04 >;
	};

第 2、3 行，属性#address-cells 和#size-cells 都为 1，表示 reg 属性中起始地址占用一个字长
(cell)，地址长度也占用一个字长(cell)。
第 4 行，属性 compatbile 设置 alphaled 节点兼容性为“atkalpha-led”。
第 5 行，属性 status 设置状态为“okay”。
第 6~10 行，reg 属性，非常重要！reg 属性设置了驱动里面所要使用的寄存器物理地址，比
如第 6 行的“0X020C406C 0X04”表示 I.MX6ULL 的 CCM_CCGR1 寄存器，其中寄存器首地
址为 0X020C406C，长度为 4 个字节。
设备树修改完成以后输入如下命令重新编译一下 imx6ull-alientek-emmc.dts：


编写驱动
```c
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DTSLED_CNT			1		  	/* 设备号个数 */
#define DTSLED_NAME			"dtsled"	/* 名字 */
#define LEDOFF 					0			/* 关灯 */
#define LEDON 					1			/* 开灯 */

/* 映射后的寄存器虚拟地址指针 */
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

/* dtsled设备结构体 */
struct dtsled_dev{
	dev_t devid;			/* 设备号 	 */
	struct cdev cdev;		/* cdev 	*/
	struct class *class;		/* 类 		*/
	struct device *device;	/* 设备 	 */
	int major;				/* 主设备号	  */
	int minor;				/* 次设备号   */
	struct device_node	*nd; /* 设备节点 */
};

struct dtsled_dev dtsled;	/* led设备 */

/*
 * @description		: LED打开/关闭
 * @param - sta 	: LEDON(0) 打开LED，LEDOFF(1) 关闭LED
 * @return 			: 无
 */
void led_switch(u8 sta)
{
	u32 val = 0;
	if(sta == LEDON) {
		val = readl(GPIO1_DR);
		val &= ~(1 << 3);	
		writel(val, GPIO1_DR);
	}else if(sta == LEDOFF) {
		val = readl(GPIO1_DR);
		val|= (1 << 3);	
		writel(val, GPIO1_DR);
	}	
}

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int led_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &dtsled; /* 设置私有数据 */
	return 0;
}

/*
 * @description		: 从设备读取数据 
 * @param - filp 	: 要打开的设备文件(文件描述符)
 * @param - buf 	: 返回给用户空间的数据缓冲区
 * @param - cnt 	: 要读取的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 读取的字节数，如果为负值，表示读取失败
 */
static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	return 0;
}

/*
 * @description		: 向设备写数据 
 * @param - filp 	: 设备文件，表示打开的文件描述符
 * @param - buf 	: 要写给设备写入的数据
 * @param - cnt 	: 要写入的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1];
	unsigned char ledstat;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	ledstat = databuf[0];		/* 获取状态值 */

	if(ledstat == LEDON) {	
		led_switch(LEDON);		/* 打开LED灯 */
	} else if(ledstat == LEDOFF) {
		led_switch(LEDOFF);	/* 关闭LED灯 */
	}
	return 0;
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
 */
static int led_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations dtsled_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = 	led_release,
};

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init led_init(void)
{
	u32 val = 0;
	int ret;
	u32 regdata[14];
	const char *str;
	struct property *proper;

	/* 获取设备树中的属性数据 */
	/* 1、获取设备节点：alphaled */
	dtsled.nd = of_find_node_by_path("/yuri_led");
	if(dtsled.nd == NULL) {
		printk("yuri_led node nost find!\r\n");
		return -EINVAL;
	} else {
		printk("yuri_led node find!\r\n");
	}

	/* 2、获取compatible属性内容 */
	proper = of_find_property(dtsled.nd, "compatible", NULL);
	if(proper == NULL) {
		printk("compatible property find failed\r\n");
	} else {
		printk("compatible = %s\r\n", (char*)proper->value);
	}

	/* 3、获取status属性内容 */
	ret = of_property_read_string(dtsled.nd, "status", &str);
	if(ret < 0){
		printk("status read failed!\r\n");
	} else {
		printk("status = %s\r\n",str);
	}

	/* 4、获取reg属性内容 */
	ret = of_property_read_u32_array(dtsled.nd, "reg", regdata, 10);
	if(ret < 0) {
		printk("reg property read failed!\r\n");
	} else {
		u8 i = 0;
		printk("reg data:\r\n");
		for(i = 0; i < 10; i++)
			printk("%#X ", regdata[i]);
		printk("\r\n");
	}

	/* 初始化LED */
#if 0
	/* 1、寄存器地址映射 */
	IMX6U_CCM_CCGR1 = ioremap(regdata[0], regdata[1]);
	SW_MUX_GPIO1_IO03 = ioremap(regdata[2], regdata[3]);
  	SW_PAD_GPIO1_IO03 = ioremap(regdata[4], regdata[5]);
	GPIO1_DR = ioremap(regdata[6], regdata[7]);
	GPIO1_GDIR = ioremap(regdata[8], regdata[9]);
#else
	IMX6U_CCM_CCGR1 = of_iomap(dtsled.nd, 0);
	SW_MUX_GPIO1_IO03 = of_iomap(dtsled.nd, 1);
  	SW_PAD_GPIO1_IO03 = of_iomap(dtsled.nd, 2);
	GPIO1_DR = of_iomap(dtsled.nd, 3);
	GPIO1_GDIR = of_iomap(dtsled.nd, 4);
#endif

	/* 2、使能GPIO1时钟 */
	val = readl(IMX6U_CCM_CCGR1);
	val &= ~(3 << 26);	/* 清楚以前的设置 */
	val |= (3 << 26);	/* 设置新值 */
	writel(val, IMX6U_CCM_CCGR1);

	/* 3、设置GPIO1_IO03的复用功能，将其复用为
	 *    GPIO1_IO03，最后设置IO属性。
	 */
	writel(5, SW_MUX_GPIO1_IO03);
	
	/*寄存器SW_PAD_GPIO1_IO03设置IO属性
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 00 默认下拉
     *bit [13]: 0 kepper功能
     *bit [12]: 1 pull/keeper使能
     *bit [11]: 0 关闭开路输出
     *bit [7:6]: 10 速度100Mhz
     *bit [5:3]: 110 R0/6驱动能力
     *bit [0]: 0 低转换率
	 */
	writel(0x10B0, SW_PAD_GPIO1_IO03);

	/* 4、设置GPIO1_IO03为输出功能 */
	val = readl(GPIO1_GDIR);
	val &= ~(1 << 3);	/* 清除以前的设置 */
	val |= (1 << 3);	/* 设置为输出 */
	writel(val, GPIO1_GDIR);

	/* 5、默认关闭LED */
	val = readl(GPIO1_DR);
	val |= (1 << 3);	
	writel(val, GPIO1_DR);

	/* 注册字符设备驱动 */
	/* 1、创建设备号 */
	if (dtsled.major) {		/*  定义了设备号 */
		dtsled.devid = MKDEV(dtsled.major, 0);
		register_chrdev_region(dtsled.devid, DTSLED_CNT, DTSLED_NAME);
	} else {						/* 没有定义设备号 */
		alloc_chrdev_region(&dtsled.devid, 0, DTSLED_CNT, DTSLED_NAME);	/* 申请设备号 */
		dtsled.major = MAJOR(dtsled.devid);	/* 获取分配号的主设备号 */
		dtsled.minor = MINOR(dtsled.devid);	/* 获取分配号的次设备号 */
	}
	printk("dtsled major=%d,minor=%d\r\n",dtsled.major, dtsled.minor);	
	
	/* 2、初始化cdev */
	dtsled.cdev.owner = THIS_MODULE;
	cdev_init(&dtsled.cdev, &dtsled_fops);
	
	/* 3、添加一个cdev */
	cdev_add(&dtsled.cdev, dtsled.devid, DTSLED_CNT);

	/* 4、创建类 */
	dtsled.class = class_create(THIS_MODULE, DTSLED_NAME);
	if (IS_ERR(dtsled.class)) {
		return PTR_ERR(dtsled.class);
	}

	/* 5、创建设备 */
	dtsled.device = device_create(dtsled.class, NULL, dtsled.devid, NULL, DTSLED_NAME);
	if (IS_ERR(dtsled.device)) {
		return PTR_ERR(dtsled.device);
	}
	
	return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit led_exit(void)
{
	/* 取消映射 */
	iounmap(IMX6U_CCM_CCGR1);
	iounmap(SW_MUX_GPIO1_IO03);
	iounmap(SW_PAD_GPIO1_IO03);
	iounmap(GPIO1_DR);
	iounmap(GPIO1_GDIR);

	/* 注销字符设备驱动 */
	cdev_del(&dtsled.cdev);/*  删除cdev */
	unregister_chrdev_region(dtsled.devid, DTSLED_CNT); /* 注销设备号 */

	device_destroy(dtsled.class, dtsled.devid);
	class_destroy(dtsled.class);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zuozhongkai");

```


# GPIO子系统

## pinctl子系统
Linux 驱动讲究驱动分离与分层，pinctrl 和 gpio 子系统就是驱动分离与分层思想下的产物，
驱动分离与分层其实就是按照面向对象编程的设计思想而设计的设备驱动框架，关于驱动的分
离与分层我们后面会讲。本来 pinctrl 和 gpio 子系统应该放到驱动分离与分层章节后面讲解，但
是不管什么外设驱动，GPIO 驱动基本都是必须的，而 pinctrl 和 gpio 子系统又是 GPIO 驱动必
须使用的，所以就将 pintrcl 和 gpio 子系统这一章节提前了。
我们先来回顾一下上一章是怎么初始化 LED 灯所使用的 GPIO，步骤如下：
①、修改设备树，添加相应的节点，节点里面重点是设置 reg 属性，reg 属性包括了 GPIO
相关寄存器。
② 、 获 取 reg 属 性 中 IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03 和
IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03 这两个寄存器地址，并且初始化这两个寄存器，这
两个寄存器用于设置 GPIO1_IO03 这个 PIN 的复用功能、上下拉、速度等。
③、在②里面将 GPIO1_IO03 这个 PIN 复用为了 GPIO 功能，因此需要设置 GPIO1_IO03
这个 GPIO 相关的寄存器，也就是 GPIO1_DR 和 GPIO1_GDIR 这两个寄存器。
总结一下，②中完成对 GPIO1_IO03 这个 PIN 的初始化，设置这个 PIN 的复用功能、上下
拉等，比如将 GPIO_IO03 这个 PIN 设置为 GPIO 功能。③中完成对 GPIO 的初始化，设置 GPIO
为输入/输出等。如果使用过 STM32 的话应该都记得，STM32 也是要先设置某个 PIN 的复用功
能、速度、上下拉等，然后再设置 PIN 所对应的 GPIO。其实对于大多数的 32 位 SOC 而言，引
脚的设置基本都是这两方面，因此 Linux 内核针对 PIN 的配置推出了 pinctrl 子系统，对于 GPIO
的配置推出了 gpio 子系统。本节我们来学习 pinctrl 子系统，下一节再学习 gpio 子系统。
大多数 SOC 的 pin 都是支持复用的，比如 I.MX6ULL 的 GPIO1_IO03 既可以作为普通的
GPIO 使用，也可以作为 I2C1 的 SDA 等等。此外我们还需要配置 pin 的电气特性，比如上/下
拉、速度、驱动能力等等。传统的配置 pin 的方式就是直接操作相应的寄存器，但是这种配置
方式比较繁琐、而且容易出问题(比如 pin 功能冲突)。pinctrl 子系统就是为了解决这个问题而引
入的，pinctrl 子系统主要工作内容如下：
①、获取设备树中 pin 信息。
②、根据获取到的 pin 信息来设置 pin 的复用功能
③、根据获取到的 pin 信息来设置 pin 的电气特性，比如上/下拉、速度、驱动能力等。
对于我们使用者来讲，只需要在设备树里面设置好某个 pin 的相关属性即可，其他的初始
化工作均由 pinctrl 子系统来完成，pinctrl 子系统源码目录为 drivers/pinctrl。

## I.MX6ULL 的 pinctrl 子系统驱动
1、PIN 配置信息详解

要使用 pinctrl 子系统，我们需要在设备树里面设置 PIN 的配置信息，毕竟 pinctrl 子系统要
根据你提供的信息来配置 PIN 功能，一般会在设备树里面创建一个节点来描述 PIN 的配置信
息。打开 imx6ull.dtsi 文件，找到一个叫做 iomuxc 的节点，如下所示：

示例代码 45.1.2.2 就是向 iomuxc 节点追加数据，不同的外设使用的 PIN 不同、其配置也不
同，因此一个萝卜一个坑，将某个外设所使用的所有 PIN 都组织在一个子节点里面。示例代码
45.1.2.2 中 pinctrl_hog_1 子节点就是和热插拔有关的 PIN 集合，比如 USB OTG 的 ID 引脚。
pinctrl_flexcan1 子节点是 flexcan1 这个外设所使用的 PIN，pinctrl_wdog 子节点是 wdog 外设所
使用的 PIN。如果需要在 iomuxc 中添加我们自定义外设的 PIN，那么需要新建一个子节点，然
后将这个自定义外设的所有 PIN 配置信息都放到这个子节点中。
将其与示例代码 45.1.2.1 结合起来就可以得到完成的 iomuxc 节点，如下所示：
示例代码 45.1.2.3 完整的 iomuxc 节点
1 iomuxc: iomuxc@020e0000 {
2 compatible = "fsl,imx6ul-iomuxc";
3 reg = <0x020e0000 0x4000>;
4 pinctrl-names = "default";
5 pinctrl-0 = <&pinctrl_hog_1>;
6 imx6ul-evk {
7 pinctrl_hog_1: hoggrp-1 {
8 fsl,pins = <
9 MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 0x17059
10 MX6UL_PAD_GPIO1_IO05__USDHC1_VSELECT 0x17059
11 MX6UL_PAD_GPIO1_IO09__GPIO1_IO09 0x17059
12 MX6UL_PAD_GPIO1_IO00__ANATOP_OTG1_ID 0x13058
13 >;
......
16 };
17 };
18 };
第 2 行，compatible 属性值为“fsl,imx6ul-iomuxc”，前面讲解设备树的时候说过，Linux 内
核会根据 compatbile 属性值来查找对应的驱动文件，所以我们在 Linux 内核源码中全局搜索字
符串“fsl,imx6ul-iomuxc”就会找到 I.MX6ULL 这颗 SOC 的 pinctrl 驱动文件。稍后我们会讲解
这个 pinctrl 驱动文件。
第 9~12 行，pinctrl_hog_1 子节点所使用的 PIN 配置信息，我们就以第 9 行的 UART1_RTS_B
这个 PIN 为例，讲解一下如何添加 PIN 的配置信息，UART1_RTS_B 的配置信息如下：
MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 0x17059
首先说明一下，UART1_RTS_B 这个 PIN 是作为 SD 卡的检测引脚，也就是通过此 PIN 就
可 以 检 测 到 SD 卡 是 否 有 插 入 。 UART1_RTS_B 的 配 置 信 息 分 为 两 部 分 ：
MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 和 0x17059
我们重点来看一下这两部分是什么含义，前面说了，对于一个 PIN 的配置主要包括两方面，
一个是设置这个 PIN 的复用功能，另一个就是设置这个 PIN 的电气特性。所以我们可以大胆的
猜测 UART1_RTS_B 的这两部分配置信息一个是设置 UART1_RTS_B 的复用功能，一个是用来
设置 UART1_RTS_B 的电气特性。
首先来看一下 MX6UL_PAD_UART1_RTS_B__GPIO1_IO19，这是一个宏定义，定义在文件
arch/arm/boot/dts/imx6ul-pinfunc.h 中，imx6ull.dtsi 会引用 imx6ull-pinfunc.h 这个头文件，而
imx6ull-pinfunc.h 又会引用 imx6ul-pinfunc.h 这个头文件（绕啊绕！）。从这里可以看出，可以在
设备树中引用 C 语言中.h 文件中的内容。MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 的宏定
义内容如下：

示例代码 45.1.2.4 中一共有 8 个以“MX6UL_PAD_UART1_RTS_B”开头的宏定义，大家
仔细观察应该就能发现，这 8 个宏定义分别对应 UART1_RTS_B 这个 PIN 的 8 个复用 IO。查
阅《I.MX6ULL 参考手册》可以知 UART1_RTS_B 的可选复用 IO 如图 45.1.2.1 所示：
示 例 代 码 196 行 的 宏 定 义 MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 表示将
UART1_RTS_B 这个 IO 复用为 GPIO1_IO19。此宏定义后面跟着 5 个数字，也就是这个宏定义
的具体值，如下所示：
0x0090 0x031C 0x0000 0x5 0x0
这 5 个值的含义如下所示：
<mux_reg conf_reg input_reg mux_mode input_val>
综上所述可知：
0x0090：mux_reg 寄存器偏移地址，设备树中的 iomuxc 节点就是 IOMUXC 外设对应的节
点 ， 根 据 其 reg 属 性 可 知 IOMUXC 外 设 寄 存 器 起 始 地 址 为 0x020e0000 。 因 此
0x020e0000+0x0090=0x020e0090，IOMUXC_SW_MUX_CTL_PAD_UART1_RTS_B 寄存器地址
正 好 是 0x020e0090 ， 大 家 可 以 在 《 IMX6ULL 参 考 手 册 》 中 找 到
IOMUXC_SW_MUX_CTL_PAD_UART1_RTS_B 这个寄存器的位域图，如图 45.1.2.2 所示：

因此可知，0x020e0000+mux_reg 就是 PIN 的复用寄存器地址。
0x031C：conf_reg 寄存器偏移地址，和 mux_reg 一样，0x020e0000+0x031c=0x020e031c，
这个就是寄存器 IOMUXC_SW_PAD_CTL_PAD_UART1_RTS_B 的地址。
0x0000：input_reg 寄存器偏移地址，有些外设有 input_reg 寄存器，有 input_reg 寄存器的
外设需要配置 input_reg 寄存器。没有的话就不需要设置，UART1_RTS_B 这个 PIN 在做
GPIO1_IO19 的时候是没有 input_reg 寄存器，因此这里 intput_reg 是无效的。
0x5 ： mux_reg 寄 存 器 值 ， 在 这 里 就 相 当 于 设 置
IOMUXC_SW_MUX_CTL_PAD_UART1_RTS_B 寄存器为 0x5，也即是设置 UART1_RTS_B 这
个 PIN 复用为 GPIO1_IO19。
0x0：input_reg 寄存器值，在这里无效。
这就是宏 MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 的含义，看的比较仔细的同学应该
会发现并没有 conf_reg 寄存器的值，config_reg 寄存器是设置一个 PIN 的电气特性的，这么重
要的寄存器怎么没有值呢？回到示例代码 45.1.2.3 中，第 9 行的内容如下所示：
MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 0x17059 

MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 我们上面已经分析了，就剩下了一个 0x17059，
反应快的同学应该已经猜出来了，0x17059 就是 conf_reg 寄存器值！此值由用户自行设置，通
过此值来设置一个 IO 的上/下拉、驱动能力和速度等。在这里就相当于设置寄存器
IOMUXC_SW_PAD_CTL_PAD_UART1_RTS_B 的值为 0x17059。

## gpio 子系统
上一小节讲解了 pinctrl 子系统，pinctrl 子系统重点是设置 PIN(有的 SOC 叫做 PAD)的复用
和电气属性，如果 pinctrl 子系统将一个 PIN 复用为 GPIO 的话，那么接下来就要用到 gpio 子系
统了。gpio 子系统顾名思义，就是用于初始化 GPIO 并且提供相应的 API 函数，比如设置 GPIO
为输入输出，读取 GPIO 的值等。gpio 子系统的主要目的就是方便驱动开发者使用 gpio，驱动
开发者在设备树中添加 gpio 相关信息，然后就可以在驱动程序中使用 gpio 子系统提供的 API
函数来操作 GPIO，Linux 内核向驱动开发者屏蔽掉了 GPIO 的设置过程，极大的方便了驱动开
发者使用 GPIO。

1、设备树中的 gpio 信息
I.MX6ULL-ALPHA 开发板上的 UART1_RTS_B 做为 SD 卡的检测引脚，UART1_RTS_B 复
用为 GPIO1_IO19，通过读取这个 GPIO 的高低电平就可以知道 SD 卡有没有插入。首先肯定是
将 UART1_RTS_B 这个 PIN 复用为 GPIO1_IO19，并且设置电气属性，也就是上一小节讲的
pinctrl 节点。打开 imx6ull-alientek-emmc.dts， UART1_RTS_B 这个 PIN 的 pincrtl 设置如下：
示例代码 45.2.2.1 SD 卡 CD 引脚 PIN 配置参数
316 pinctrl_hog_1: hoggrp-1 {
317 fsl,pins = <
318 MX6UL_PAD_UART1_RTS_B__GPIO1_IO19 0x17059 /* SD1 CD */
......
322 >;
323 };
第 318 行，设置 UART1_RTS_B 这个 PIN 为 GPIO1_IO19。
pinctrl 配置好以后就是设置 gpio 了，SD 卡驱动程序通过读取 GPIO1_IO19 的值来判断 SD
卡有没有插入，但是 SD 卡驱动程序怎么知道 CD 引脚连接的 GPIO1_IO19 呢？肯定是需要设
备树告诉驱动啊！在设备树中 SD 卡节点下添加一个属性来描述 SD 卡的 CD 引脚就行了，SD
卡驱动直接读取这个属性值就知道 SD 卡的 CD 引脚使用的是哪个 GPIO 了。SD 卡连接在
I.MX6ULL 的 usdhc1 接口上，在 imx6ull-alientek-emmc.dts 中找到名为“usdhc1”的节点，这个
节点就是 SD 卡设备节点，如下所示：
示例代码 45.2.2.2 设备树中 SD 卡节点
760 &usdhc1 {
761 pinctrl-names = "default", "state_100mhz", "state_200mhz";
762 pinctrl-0 = <&pinctrl_usdhc1>;
763 pinctrl-1 = <&pinctrl_usdhc1_100mhz>;
764 pinctrl-2 = <&pinctrl_usdhc1_200mhz>;
765 /* pinctrl-3 = <&pinctrl_hog_1>; */
766 cd-gpios = <&gpio1 19 GPIO_ACTIVE_LOW>;
767 keep-power-in-suspend;
768 enable-sdio-wakeup;
769 vmmc-supply = <&reg_sd1_vmmc>;
770 status = "okay";
771 };
第 765 行，此行本来没有，是作者添加的，usdhc1 节点作为 SD 卡设备总节点，usdhc1 节
点需要描述 SD 卡所有的信息，因为驱动要使用。本行就是描述 SD 卡的 CD 引脚 pinctrl 信息
所在的子节点，因为 SD 卡驱动需要根据 pincrtl 节点信息来设置 CD 引脚的复用功能等。762~764
行的 pinctrl-0~2 都是 SD 卡其他 PIN 的 pincrtl 节点信息。但是大家会发现，其实在 usdhc1 节点
中并没有“pinctrl-3 = <&pinctrl_hog_1>”这一行，也就是说并没有指定 CD 引脚的 pinctrl 信息，
那么 SD 卡驱动就没法设置 CD 引脚的复用功能啊？这个不用担心，因为在“iomuxc”节点下
引用了 pinctrl_hog_1 这个节点，所以 Linux 内核中的 iomuxc 驱动就会自动初始化 pinctrl_hog_1
节点下的所有 PIN。
第 766 行，属性“cd-gpios”描述了 SD 卡的 CD 引脚使用的哪个 IO。属性值一共有三个，
我们来看一下这三个属性值的含义，“&gpio1”表示 CD 引脚所使用的 IO 属于 GPIO1 组，“19”
表示 GPIO1 组的第 19 号 IO，通过这两个值 SD 卡驱动程序就知道 CD 引脚使用了 GPIO1_IO19
这 GPIO。“GPIO_ACTIVE_LOW”表示低电平有效，如果改为“GPIO_ACTIVE_HIGH”就表
示高电平有效。
根据上面这些信息，SD 卡驱动程序就可以使用 GPIO1_IO19 来检测 SD 卡的 CD 信号了，
打开 imx6ull.dtsi，在里面找到如下所示内容：


### gpio 子系统 API 函数
对于驱动开发人员，设置好设备树以后就可以使用 gpio 子系统提供的 API 函数来操作指定
的 GPIO，gpio 子系统向驱动开发人员屏蔽了具体的读写寄存器过程。这就是驱动分层与分离
的好处，大家各司其职，做好自己的本职工作即可。gpio 子系统提供的常用的 API 函数有下面
几个：
1、gpio_request 函数
gpio_request 函数用于申请一个 GPIO 管脚，在使用一个 GPIO 之前一定要使用 gpio_request
进行申请，函数原型如下：
int gpio_request(unsigned gpio, const char *label)
函数参数和返回值含义如下：
gpio：要申请的 gpio 标号，使用 of_get_named_gpio 函数从设备树获取指定 GPIO 属性信
息，此函数会返回这个 GPIO 的标号。
label：给 gpio 设置个名字。
返回值：0，申请成功；其他值，申请失败。
2、gpio_free 函数
如果不使用某个 GPIO 了，那么就可以调用 gpio_free 函数进行释放。函数原型如下：
void gpio_free(unsigned gpio)
函数参数和返回值含义如下：
gpio：要释放的 gpio 标号。
返回值：无。
3、gpio_direction_input 函数
此函数用于设置某个 GPIO 为输入，函数原型如下所示：
int gpio_direction_input(unsigned gpio)
函数参数和返回值含义如下：
gpio：要设置为输入的 GPIO 标号。
返回值：0，设置成功；负值，设置失败。
4、gpio_direction_output 函数
此函数用于设置某个 GPIO 为输出，并且设置默认输出值，函数原型如下：
int gpio_direction_output(unsigned gpio, int value)
函数参数和返回值含义如下：
gpio：要设置为输出的 GPIO 标号。
value：GPIO 默认输出值。
返回值：0，设置成功；负值，设置失败。
5、gpio_get_value 函数
此函数用于获取某个 GPIO 的值(0 或 1)，此函数是个宏，定义所示：
#define gpio_get_value __gpio_get_value
int __gpio_get_value(unsigned gpio)
函数参数和返回值含义如下：
gpio：要获取的 GPIO 标号。
返回值：非负值，得到的 GPIO 值；负值，获取失败。
6、gpio_set_value 函数
此函数用于设置某个 GPIO 的值，此函数是个宏，定义如下
#define gpio_set_value __gpio_set_value
void __gpio_set_value(unsigned gpio, int value)
函数参数和返回值含义如下：
gpio：要设置的 GPIO 标号。
value：要设置的值。
返回值：无
关于 gpio 子系统常用的 API 函数就讲这些，这些是我们用的最多的。

### 修改设备树文件
1、添加 pinctrl 节点
I.MX6U-ALPHA 开发板上的 LED 灯使用了 GPIO1_IO03 这个 PIN，打开 imx6ull-alientekemmc.dts，在 iomuxc 节点的 imx6ul-evk 子节点下创建一个名为“pinctrl_led”的子节点，节点
内容如下所示：
示例代码 45.4.1.1 GPIO1_IO03 pinctrl 节点
1 pinctrl_led: ledgrp {
2 fsl,pins = <
3 MX6UL_PAD_GPIO1_IO03__GPIO1_IO03 0x10B0 /* LED0 */
4 >;
5 };
第 3 行，将 GPIO1_IO03 这个 PIN 复用为 GPIO1_IO03，电气属性值为 0X10B0。
2、添加 LED 设备节点
在根节点“/”下创建 LED 灯节点，节点名为“gpioled”，节点内容如下：
示例代码 45.4.1.2 创建 LED 灯节点
1 gpioled {
2 #address-cells = <1>;
3 #size-cells = <1>;
4 compatible = "atkalpha-gpioled";
5 pinctrl-names = "default";
6 pinctrl-0 = <&pinctrl_led>;
7 led-gpio = <&gpio1 3 GPIO_ACTIVE_LOW>;
8 status = "okay";
9 };
第 6 行，pinctrl-0 属性设置 LED 灯所使用的 PIN 对应的 pinctrl 节点。
第 7 行，led-gpio 属性指定了 LED 灯所使用的 GPIO，在这里就是 GPIO1 的 IO03，低电平
有效。稍后编写驱动程序的时候会获取 led-gpio 属性的内容来得到 GPIO 编号，因为 gpio 子系
统的 API 操作函数需要 GPIO 编号。
3、检查 PIN 是否被其他外设使用
这一点非常重要！！！
很多初次接触设备树的驱动开发人员很容易因为这个小问题栽了大跟头！因为我们所使用
的设备树基本都是在半导体厂商提供的设备树文件基础上修改而来的，而半导体厂商提供的设
备树是根据自己官方开发板编写的，很多 PIN 的配置和我们所使用的开发板不一样。比如 A 这
个引脚在官方开发板接的是 I2C 的 SDA，而我们所使用的硬件可能将 A 这个引脚接到了其他
的外设，比如 LED 灯上，接不同的外设，A 这个引脚的配置就不同。一个引脚一次只能实现一
个功能，如果 A 引脚在设备树中配置为了 I2C 的 SDA 信号，那么 A 引脚就不能再配置为 GPIO，
否则的话驱动程序在申请 GPIO 的时候就会失败。检查 PIN 有没有被其他外设使用包括两个方
面：
①、检查 pinctrl 设置。
②、如果这个 PIN 配置为 GPIO 的话，检查这个 GPIO 有没有被别的外设使用。
在本章实验中 LED 灯使用的 PIN 为 GPIO1_IO03，因此先检查 GPIO_IO03 这个 PIN 有没
有被其他的 pinctrl 节点使用，在 imx6ull-alientek-emmc.dts 中找到如下内容：
pinctrl_tsc 节点是 TSC(电阻触摸屏接口)的 pinctrl 节点，从第 484 行可以看出，默认情况下
GPIO1_IO03 作为了 TSC 外设的 PIN。所以我们需要将第 484 行屏蔽掉！和 C 语言一样，在要
屏蔽的内容前后加上“/*”和“*/”符号即可。其实在 I.MX6U-ALPHA 开发板上并没有用到 TSC
接口，所以第 482~485 行的内容可以全部屏蔽掉。
因为本章实验我们将 GPIO1_IO03 这个 PIN 配置为了 GPIO，所以还需要查找一下有没有
其他的外设使用了 GPIO1_IO03，在 imx6ull-alientek-emmc.dts 中搜索“gpio1 3”，找到如下内
容：设备树编写完成以后使用“make dtbs”命令重新编译设备树，然后使用新编译出来的
imx6ull-alientek-emmc.dtb 文件启动 Linux 系统。启动成功以后进入“/proc/device-tree”目录中
查看“gpioled”节点是否存在，如果存在的话就说明设备树基本修改成功(具体还要驱动验证)，


具体使用驱动
```c
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名		: gpioled.c
作者	  	: 左忠凯
版本	   	: V1.0
描述	   	: 采用pinctrl和gpio子系统驱动LED灯。
其他	   	: 无
论坛 	   	: www.openedv.com
日志	   	: 初版V1.0 2019/7/13 左忠凯创建
***************************************************************/
#define GPIOLED_CNT			1		  	/* 设备号个数 */
#define GPIOLED_NAME		"gpioled"	/* 名字 */
#define LEDOFF 				0			/* 关灯 */
#define LEDON 				1			/* 开灯 */

/* gpioled设备结构体 */
struct gpioled_dev{
	dev_t devid;			/* 设备号 	 */
	struct cdev cdev;		/* cdev 	*/
	struct class *class;	/* 类 		*/
	struct device *device;	/* 设备 	 */
	int major;				/* 主设备号	  */
	int minor;				/* 次设备号   */
	struct device_node	*nd; /* 设备节点 */
	int led_gpio;			/* led所使用的GPIO编号		*/
};

struct gpioled_dev gpioled;	/* led设备 */

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int led_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &gpioled; /* 设置私有数据 */
	return 0;
}

/*
 * @description		: 从设备读取数据 
 * @param - filp 	: 要打开的设备文件(文件描述符)
 * @param - buf 	: 返回给用户空间的数据缓冲区
 * @param - cnt 	: 要读取的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 读取的字节数，如果为负值，表示读取失败
 */
static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	return 0;
}

/*
 * @description		: 向设备写数据 
 * @param - filp 	: 设备文件，表示打开的文件描述符
 * @param - buf 	: 要写给设备写入的数据
 * @param - cnt 	: 要写入的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1];
	unsigned char ledstat;
	struct gpioled_dev *dev = filp->private_data;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	ledstat = databuf[0];		/* 获取状态值 */

	if(ledstat == LEDON) {	
		gpio_set_value(dev->led_gpio, 0);	/* 打开LED灯 */
	} else if(ledstat == LEDOFF) {
		gpio_set_value(dev->led_gpio, 1);	/* 关闭LED灯 */
	}
	return 0;
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
 */
static int led_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations gpioled_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = 	led_release,
};

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init led_init(void)
{
	int ret = 0;

	/* 设置LED所使用的GPIO */
	/* 1、获取设备节点：gpioled */
	gpioled.nd = of_find_node_by_path("/gpioled");
	if(gpioled.nd == NULL) {
		printk("gpioled node not find!\r\n");
		return -EINVAL;
	} else {
		printk("gpioled node find!\r\n");
	}

	/* 2、 获取设备树中的gpio属性，得到LED所使用的LED编号 */
	gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "led-gpio", 0);
	if(gpioled.led_gpio < 0) {
		printk("can't get led-gpio");
		return -EINVAL;
	}
	printk("led-gpio num = %d\r\n", gpioled.led_gpio);

	/* 3、设置GPIO1_IO03为输出，并且输出高电平，默认关闭LED灯 */
	ret = gpio_direction_output(gpioled.led_gpio, 1);
	if(ret < 0) {
		printk("can't set gpio!\r\n");
	}

	/* 注册字符设备驱动 */
	/* 1、创建设备号 */
	if (gpioled.major) {		/*  定义了设备号 */
		gpioled.devid = MKDEV(gpioled.major, 0);
		register_chrdev_region(gpioled.devid, GPIOLED_CNT, GPIOLED_NAME);
	} else {						/* 没有定义设备号 */
		alloc_chrdev_region(&gpioled.devid, 0, GPIOLED_CNT, GPIOLED_NAME);	/* 申请设备号 */
		gpioled.major = MAJOR(gpioled.devid);	/* 获取分配号的主设备号 */
		gpioled.minor = MINOR(gpioled.devid);	/* 获取分配号的次设备号 */
	}
	printk("gpioled major=%d,minor=%d\r\n",gpioled.major, gpioled.minor);	
	
	/* 2、初始化cdev */
	gpioled.cdev.owner = THIS_MODULE;
	cdev_init(&gpioled.cdev, &gpioled_fops);
	
	/* 3、添加一个cdev */
	cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT);

	/* 4、创建类 */
	gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);
	if (IS_ERR(gpioled.class)) {
		return PTR_ERR(gpioled.class);
	}

	/* 5、创建设备 */
	gpioled.device = device_create(gpioled.class, NULL, gpioled.devid, NULL, GPIOLED_NAME);
	if (IS_ERR(gpioled.device)) {
		return PTR_ERR(gpioled.device);
	}
	return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit led_exit(void)
{
	/* 注销字符设备驱动 */
	cdev_del(&gpioled.cdev);/*  删除cdev */
	unregister_chrdev_region(gpioled.devid, GPIOLED_CNT); /* 注销设备号 */

	device_destroy(gpioled.class, gpioled.devid);
	class_destroy(gpioled.class);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zuozhongkai");
```

# LINUX中断实验
## Linux 中断简介
### Linux 中断 API 函数
先来回顾一下裸机实验里面中断的处理方法：
①、使能中断，初始化相应的寄存器。
②、注册中断服务函数，也就是向 irqTable 数组的指定标号处写入中断服务函数
②、中断发生以后进入 IRQ 中断服务函数，在 IRQ 中断服务函数在数组 irqTable 里面查找
具体的中断处理函数，找到以后执行相应的中断处理函数。
在 Linux 内核中也提供了大量的中断相关的 API 函数，我们来看一下这些跟中断有关的
API 函数：
1、中断号
每个中断都有一个中断号，通过中断号即可区分不同的中断，有的资料也把中断号叫做中
断线。在 Linux 内核中使用一个 int 变量表示中断号，关于中断号我们已经在第十七章讲解过
了。
2、request_irq 函数
在 Linux 内核中要想使用某个中断是需要申请的，request_irq 函数用于申请中断，request_irq
函数可能会导致睡眠，因此不能在中断上下文或者其他禁止睡眠的代码段中使用 request_irq 函
数。request_irq 函数会激活(使能)中断，所以不需要我们手动去使能中断，request_irq 函数原型
如下：
int request_irq(unsigned int irq, 
irq_handler_t handler, 
unsigned long flags,
 const char *name, 
void *dev)
函数参数和返回值含义如下：
irq：要申请中断的中断号。
handler：中断处理函数，当中断发生以后就会执行此中断处理函数。
flags：中断标志，可以在文件 include/linux/interrupt.h 里面查看所有的中断标志，这里我们
介绍几个常用的中断标志
比如 I.MX6U-ALPHA 开发板上的 KEY0 使用 GPIO1_IO18，按下 KEY0 以后为低电平，因
此可以设置为下降沿触发，也就是将 flags 设置为 IRQF_TRIGGER_FALLING。表 51.1.1.1 中的
这些标志可以通过“|”来实现多种组合。
name：中断名字，设置以后可以在/proc/interrupts 文件中看到对应的中断名字。
dev：如果将 flags 设置为 IRQF_SHARED 的话，dev 用来区分不同的中断，一般情况下将
dev 设置为设备结构体，dev 会传递给中断处理函数 irq_handler_t 的第二个参数。
返回值：0 中断申请成功，其他负值 中断申请失败，如果返回-EBUSY 的话表示中断已经
被申请了。
3、free_irq 函数
使用中断的时候需要通过 request_irq 函数申请，使用完成以后就要通过 free_irq 函数释放
掉相应的中断。如果中断不是共享的，那么 free_irq 会删除中断处理函数并且禁止中断。free_irq
函数原型如下所示：
void free_irq(unsigned int irq, 
void *dev)
函数参数和返回值含义如下：
irq：要释放的中断。
dev：如果中断设置为共享(IRQF_SHARED)的话，此参数用来区分具体的中断。共享中断
只有在释放最后中断处理函数的时候才会被禁止掉。
返回值：无。
4、中断处理函数
使用 request_irq 函数申请中断的时候需要设置中断处理函数，中断处理函数格式如下所示：
irqreturn_t (*irq_handler_t) (int, void *)
第一个参数是要中断处理函数要相应的中断号。第二个参数是一个指向 void 的指针，也就
是个通用指针，需要与 request_irq 函数的 dev 参数保持一致。用于区分共享中断的不同设备，
dev 也可以指向设备数据结构。中断处理函数的返回值为 irqreturn_t 类型，irqreturn_t 类型定义
如下所示：
return IRQ_RETVAL(IRQ_HANDLED)
5、中断使能与禁止函数
常用的中断使用和禁止函数如下所示：
void enable_irq(unsigned int irq)
void disable_irq(unsigned int irq)
enable_irq 和 disable_irq 用于使能和禁止指定的中断，irq 就是要禁止的中断号。disable_irq
函数要等到当前正在执行的中断处理函数执行完才返回，因此使用者需要保证不会产生新的中
断，并且确保所有已经开始执行的中断处理程序已经全部退出。在这种情况下，可以使用另外
一个中断禁止函数：
void disable_irq_nosync(unsigned int irq)
disable_irq_nosync 函数调用以后立即返回，不会等待当前中断处理程序执行完毕。上面三
个函数都是使能或者禁止某一个中断，有时候我们需要关闭当前处理器的整个中断系统，也就
是在学习 STM32 的时候常说的关闭全局中断，这个时候可以使用如下两个函数：
local_irq_enable()
local_irq_disable()
local_irq_enable 用于使能当前处理器中断系统，local_irq_disable 用于禁止当前处理器中断
系统。假如 A 任务调用 local_irq_disable 关闭全局中断 10S，当关闭了 2S 的时候 B 任务开始运
行，B 任务也调用 local_irq_disable 关闭全局中断 3S，3 秒以后 B 任务调用 local_irq_enable 函
数将全局中断打开了。此时才过去 2+3=5 秒的时间，然后全局中断就被打开了，此时 A 任务要
关闭 10S 全局中断的愿望就破灭了，然后 A 任务就“生气了”，结果很严重，可能系统都要被
A 任务整崩溃。为了解决这个问题，B 任务不能直接简单粗暴的通过 local_irq_enable 函数来打
开全局中断，而是将中断状态恢复到以前的状态，要考虑到别的任务的感受，此时就要用到下
面两个函数：
local_irq_save(flags)
local_irq_restore(flags)
这两个函数是一对，local_irq_save 函数用于禁止中断，并且将中断状态保存在 flags 中。
local_irq_restore 用于恢复中断，将中断到 flags 状态。

### 上半部与下半部

在有些资料中也将上半部和下半部称为顶半部和底半部，都是一个意思。我们在使用
request_irq 申请中断的时候注册的中断服务函数属于中断处理的上半部，只要中断触发，那么
中断处理函数就会执行。我们都知道中断处理函数一定要快点执行完毕，越短越好，但是现实
往往是残酷的，有些中断处理过程就是比较费时间，我们必须要对其进行处理，缩小中断处理
函数的执行时间。比如电容触摸屏通过中断通知 SOC 有触摸事件发生，SOC 响应中断，然后
通过 IIC 接口读取触摸坐标值并将其上报给系统。但是我们都知道 IIC 的速度最高也只有
400Kbit/S，所以在中断中通过 IIC 读取数据就会浪费时间。我们可以将通过 IIC 读取触摸数据
的操作暂后执行，中断处理函数仅仅相应中断，然后清除中断标志位即可。这个时候中断处理
过程就分为了两部分：
上半部：上半部就是中断处理函数，那些处理过程比较快，不会占用很长时间的处理就可
以放在上半部完成。
下半部：如果中断处理过程比较耗时，那么就将这些比较耗时的代码提出来，交给下半部
去执行，这样中断处理函数就会快进快出。
1、软中断
一开始 Linux 内核提供了“bottom half”机制来实现下半部，简称“BH”。后面引入了软中
断和 tasklet 来替代“BH”机制，完全可以使用软中断和 tasklet 来替代 BH，从 2.5 版本的 Linux
内核开始 BH 已经被抛弃了。Linux 内核使用结构体 softirq_action 表示软中断， softirq_action
结构体定义在文件 include/linux/interrupt.h 中
2、tasklet
tasklet 是利用软中断来实现的另外一种下半部机制，在软中断和 tasklet 之间，建议大家使
用 tasklet。Linux 内核使用 tasklet_struct 结构体来表示 tasklet
3、工作队列
工作队列是另外一种下半部执行方式，工作队列在进程上下文执行，工作队列将要推后的
工作交给一个内核线程去执行，因为工作队列工作在进程上下文，因此工作队列允许睡眠或重
新调度。因此如果你要推后的工作可以睡眠那么就可以选择工作队列，否则的话就只能选择软
中断或 tasklet


# LINUX非阻塞IO

 阻塞和非阻塞简介
这里的“IO”并不是我们学习 STM32 或者其他单片机的时候所说的“GPIO”(也就是引脚)。
这里的 IO 指的是 Input/Output，也就是输入/输出，是应用程序对驱动设备的输入/输出操作。当
应用程序对设备驱动进行操作的时候，如果不能获取到设备资源，那么阻塞式 IO 就会将应用程
序对应的线程挂起，直到设备资源可以获取为止。对于非阻塞 IO，应用程序对应的线程不会挂
起，它要么一直轮询等待，直到设备资源可以使用，要么就直接放弃。阻塞式 IO 如图 52.1.1.1
所示：
应用程序使用非阻塞访问方式从设备读取数据，当设备不可用或
数据未准备好的时候会立即向内核返回一个错误码，表示数据读取失败。应用程序会再次重新
读取数据，这样一直往复循环，直到数据读取成功。

1 int fd;
2 int data = 0;
3
4 fd = open("/dev/xxx_dev", O_RDWR); /* 阻塞方式打开 */
5 ret = read(fd, &data, sizeof(data)); /* 读取数据 */

对于设备驱动文件的默认读取方式就是阻塞式的，所以我
们前面所有的例程测试 APP 都是采用阻塞 IO。
如果应用程序要采用非阻塞的方式来访问驱动设备文件，可以使用如下所示代码：

1 int fd;
2 int data = 0;
3
4 fd = open("/dev/xxx_dev", O_RDWR | O_NONBLOCK); /* 非阻塞方式打开 */
5 ret = read(fd, &data, sizeof(data)); /* 读取数据 */


## 轮询
如果用户应用程序以非阻塞的方式访问设备，设备驱动程序就要提供非阻塞的处理方式，
也就是轮询。poll、epoll 和 select 可以用于处理轮询，应用程序通过 select、epoll 或 poll 函数来
查询设备是否可以操作，如果可以操作的话就从设备读取或者向设备写入数据。当应用程序调
用 select、epoll 或 poll 函数的时候设备驱动程序中的 poll 函数就会执行，因此需要在设备驱动
程序中编写 poll 函数。我们先来看一下应用程序中使用的 select、poll 和 epoll 这三个函数。

1、select 函数
select 函数原型如下：
int select(int nfds, 
fd_set *readfds, 
fd_set *writefds,
fd_set *exceptfds, 
struct timeval *timeout)
函数参数和返回值含义如下：
nfds：所要监视的这三类文件描述集合中，最大文件描述符加 1。
readfds、writefds 和 exceptfds：这三个指针指向描述符集合，这三个参数指明了关心哪些
描述符、需要满足哪些条件等等，这三个参数都是 fd_set 类型的，fd_set 类型变量的每一个位
都代表了一个文件描述符。readfds 用于监视指定描述符集的读变化，也就是监视这些文件是否
可以读取，只要这些集合里面有一个文件可以读取那么 seclect 就会返回一个大于 0 的值表示文
件可以读取。如果没有文件可以读取，那么就会根据 timeout 参数来判断是否超时。可以将 readfs
设置为 NULL，表示不关心任何文件的读变化。writefds 和 readfs 类似，只是 writefs 用于监视
这些文件是否可以进行写操作。exceptfds 用于监视这些文件的异常。
比如我们现在要从一个设备文件中读取数据，那么就可以定义一个 fd_set 变量，这个变量
要传递给参数 readfds。当我们定义好一个 fd_set 变量以后可以使用如下所示几个宏进行操作：
void FD_ZERO(fd_set *set)
void FD_SET(int fd, fd_set *set)
void FD_CLR(int fd, fd_set *set)
int FD_ISSET(int fd, fd_set *set)
FD_ZERO 用于将 fd_set 变量的所有位都清零，FD_SET 用于将 fd_set 变量的某个位置 1，
也就是向 fd_set 添加一个文件描述符，参数 fd 就是要加入的文件描述符。FD_CLR 用于将 fd_set

变量的某个位清零，也就是将一个文件描述符从 fd_set 中删除，参数 fd 就是要删除的文件描述
符。FD_ISSET 用于测试一个文件是否属于某个集合，参数 fd 就是要判断的文件描述符。
timeout:超时时间，当我们调用 select 函数等待某些文件描述符可以设置超时时间，超时时
间使用结构体 timeval 表示，结构体定义如下所示：
struct timeval {
long tv_sec; /* 秒 */
long tv_usec; /* 微妙 */
};
当 timeout 为 NULL 的时候就表示无限期的等待。
返回值：0，表示的话就表示超时发生，但是没有任何文件描述符可以进行操作；-1，发生
错误；其他值，可以进行操作的文件描述符个数。
使用 select 函数对某个设备驱动文件进行读非阻塞访问的操作示例如下所示：
示例代码 52.1.3.1 select 函数非阻塞读访问示例
1 void main(void)
2 {
3 int ret, fd; /* 要监视的文件描述符 */
4 fd_set readfds; /* 读操作文件描述符集 */
5 struct timeval timeout; /* 超时结构体 */
6 
7 fd = open("dev_xxx", O_RDWR | O_NONBLOCK); /* 非阻塞式访问 */
8 
9 FD_ZERO(&readfds); /* 清除 readfds */
10 FD_SET(fd, &readfds); /* 将 fd 添加到 readfds 里面 */
11 
12 /* 构造超时时间 */
13 timeout.tv_sec = 0;
14 timeout.tv_usec = 500000; /* 500ms */
15 
16 ret = select(fd + 1, &readfds, NULL, NULL, &timeout);
17 switch (ret) {
18 case 0: /* 超时 */
19 printf("timeout!\r\n");
20 break;
21 case -1: /* 错误 */
22 printf("error!\r\n");
23 break;
24 default: /* 可以读取数据 */
25 if(FD_ISSET(fd, &readfds)) { /* 判断是否为 fd 文件描述符 */
26 /* 使用 read 函数读取数据 */
27 }
28 break;
29 } 
30 }

2、poll 函数
在单个线程中，select 函数能够监视的文件描述符数量有最大的限制，一般为 1024，可以
修改内核将监视的文件描述符数量改大，但是这样会降低效率！这个时候就可以使用 poll 函数，
poll 函数本质上和 select 没有太大的差别，但是 poll 函数没有最大文件描述符限制，Linux 应用
程序中 poll 函数原型如下所示：
int poll(struct pollfd *fds, 
 nfds_t nfds, 
 int timeout)
函数参数和返回值含义如下：
fds：要监视的文件描述符集合以及要监视的事件,为一个数组，数组元素都是结构体 pollfd
类型的，pollfd 结构体如下所示：
struct pollfd {
int fd; /* 文件描述符 */
short events; /* 请求的事件 */
 short revents; /* 返回的事件 */
};
fd 是要监视的文件描述符，如果 fd 无效的话那么 events 监视事件也就无效，并且 revents
返回 0。events 是要监视的事件，可监视的事件类型如下所示：
POLLIN 有数据可以读取。
POLLPRI 有紧急的数据需要读取。
POLLOUT 可以写数据。
POLLERR 指定的文件描述符发生错误。
POLLHUP 指定的文件描述符挂起。
POLLNVAL 无效的请求。
POLLRDNORM 等同于 POLLIN
revents 是返回参数，也就是返回的事件，由 Linux 内核设置具体的返回事件。
nfds：poll 函数要监视的文件描述符数量。
timeout：超时时间，单位为 ms。
返回值：返回 revents 域中不为 0 的 pollfd 结构体个数，也就是发生事件或错误的文件描述
符数量；0，超时；-1，发生错误，并且设置 errno 为错误类型。
使用 poll 函数对某个设备驱动文件进行读非阻塞访问的操作示例如下所示：
示例代码 52.1.3.2 poll 函数读非阻塞访问示例
1 void main(void)
2 {
3 int ret; 
4 int fd; /* 要监视的文件描述符 */
5 struct pollfd fds; 
6 
7 fd = open(filename, O_RDWR | O_NONBLOCK); /* 非阻塞式访问 */
8 
9 /* 构造结构体 */
10 fds.fd = fd;
11 fds.events = POLLIN; /* 监视数据是否可以读取 */
12 
13 ret = poll(&fds, 1, 500); /* 轮询文件是否可操作，超时 500ms */
14 if (ret) { /* 数据有效 */
15 ......
16 /* 读取数据 */
17 ......
18 } else if (ret == 0) { /* 超时 */
19 ......
20 } else if (ret < 0) { /* 错误 */
21 ......
22 }
23 }
3、epoll 函数
传统的 selcet 和 poll 函数都会随着所监听的 fd 数量的增加，出现效率低下的问题，而且
poll 函数每次必须遍历所有的描述符来检查就绪的描述符，这个过程很浪费时间。为此，epoll
应运而生，epoll 就是为处理大并发而准备的，一般常常在网络编程中使用 epoll 函数。应用程
序需要先使用 epoll_create 函数创建一个 epoll 句柄，epoll_create 函数原型如下：
int epoll_create(int size)
函数参数和返回值含义如下：
size：从 Linux2.6.8 开始此参数已经没有意义了，随便填写一个大于 0 的值就可以。
返回值：epoll 句柄，如果为-1 的话表示创建失败。
epoll 句柄创建成功以后使用 epoll_ctl 函数向其中添加要监视的文件描述符以及监视的事
件，epoll_ctl 函数原型如下所示：
int epoll_ctl(int epfd, 
 int op, 
 int fd,
 struct epoll_event *event)
函数参数和返回值含义如下：
epfd：要操作的 epoll 句柄，也就是使用 epoll_create 函数创建的 epoll 句柄。
op：表示要对 epfd(epoll 句柄)进行的操作，可以设置为：
EPOLL_CTL_ADD 向 epfd 添加文件参数 fd 表示的描述符。
EPOLL_CTL_MOD 修改参数 fd 的 event 事件。
EPOLL_CTL_DEL 从 epfd 中删除 fd 描述符。
fd：要监视的文件描述符。
event：要监视的事件类型，为 epoll_event 结构体类型指针，epoll_event 结构体类型如下所
示：
struct epoll_event {
uint32_t events; /* epoll 事件 */
epoll_data_t data; /* 用户数据 */
};
结构体 epoll_event 的 events 成员变量表示要监视的事件，可选的事件如下所示：
EPOLLIN 有数据可以读取。
EPOLLOUT 可以写数据。
EPOLLPRI 有紧急的数据需要读取。
EPOLLERR 指定的文件描述符发生错误。
EPOLLHUP 指定的文件描述符挂起。
EPOLLET 设置 epoll 为边沿触发，默认触发模式为水平触发。
EPOLLONESHOT 一次性的监视，当监视完成以后还需要再次监视某个 fd，那么就需要将
fd 重新添加到 epoll 里面。
上面这些事件可以进行“或”操作，也就是说可以设置监视多个事件。
返回值：0，成功；-1，失败，并且设置 errno 的值为相应的错误码。
一切都设置好以后应用程序就可以通过 epoll_wait 函数来等待事件的发生，类似 select 函
数。epoll_wait 函数原型如下所示：
int epoll_wait(int epfd, 
struct epoll_event *events,
int maxevents, 
int timeout)
函数参数和返回值含义如下：
epfd：要等待的 epoll。
events：指向 epoll_event 结构体的数组，当有事件发生的时候 Linux 内核会填写 events，调
用者可以根据 events 判断发生了哪些事件。
maxevents：events 数组大小，必须大于 0。
timeout：超时时间，单位为 ms。
返回值：0，超时；-1，错误；其他值，准备就绪的文件描述符数量。
epoll 更多的是用在大规模的并发服务器上，因为在这种场合下 select 和 poll 并不适合。当
设计到的文件描述符(fd)比较少的时候就适合用 selcet 和 poll，本章我们就使用 sellect 和 poll 这
两个函数。

# 异步通知
我们首先来回顾一下“中断”，中断是处理器提供的一种异步机制，我们配置好中断以后就
可以让处理器去处理其他的事情了，当中断发生以后会触发我们事先设置好的中断服务函数，
在中断服务函数中做具体的处理。比如我们在裸机篇里面编写的 GPIO 按键中断实验，我们通
过按键去开关蜂鸣器，采用中断以后处理器就不需要时刻的去查看按键有没有被按下，因为按
键按下以后会自动触发中断。同样的，Linux 应用程序可以通过阻塞或者非阻塞这两种方式来
访问驱动设备，通过阻塞方式访问的话应用程序会处于休眠态，等待驱动设备可以使用，非阻
塞方式的话会通过 poll 函数来不断的轮询，查看驱动设备文件是否可以使用。这两种方式都需
要应用程序主动的去查询设备的使用情况，如果能提供一种类似中断的机制，当驱动程序可以
访问的时候主动告诉应用程序那就最好了。
“信号”为此应运而生，信号类似于我们硬件上使用的“中断”，只不过信号是软件层次上
的。算是在软件层次上对中断的一种模拟，驱动可以通过主动向应用程序发送信号的方式来报
告自己可以访问了，应用程序获取到信号以后就可以从驱动设备中读取或者写入数据了。整个
过程就相当于应用程序收到了驱动发送过来了的一个中断，然后应用程序去响应这个中断，在
整个处理过程中应用程序并没有去查询驱动设备是否可以访问，一切都是由驱动设备自己告诉
给应用程序的。
阻塞、非阻塞、异步通知，这三种是针对不同的场合提出来的不同的解决方法，没有优劣
之分，在实际的工作和学习中，根据自己的实际需求选择合适的处理方法即可。
异步通知的核心就是信号，在 arch/xtensa/include/uapi/asm/signal.h 文件中定义了 Linux 所支
持的所有信号，这些信号如下所示：
示例代码 53.1.1.1 Linux 信号
34 #define SIGHUP 1 /* 终端挂起或控制进程终止 */
35 #define SIGINT 2 /* 终端中断(Ctrl+C 组合键) */
36 #define SIGQUIT 3 /* 终端退出(Ctrl+\组合键) */
37 #define SIGILL 4 /* 非法指令 */
38 #define SIGTRAP 5 /* debug 使用，有断点指令产生 */
39 #define SIGABRT 6 /* 由 abort(3)发出的退出指令 */
40 #define SIGIOT 6 /* IOT 指令 */
41 #define SIGBUS 7 /* 总线错误 */
42 #define SIGFPE 8 /* 浮点运算错误 */
43 #define SIGKILL 9 /* 杀死、终止进程 */
44 #define SIGUSR1 10 /* 用户自定义信号 1 */
45 #define SIGSEGV 11 /* 段违例(无效的内存段) */
46 #define SIGUSR2 12 /* 用户自定义信号 2 */
47 #define SIGPIPE 13 /* 向非读管道写入数据 */
48 #define SIGALRM 14 /* 闹钟 */
49 #define SIGTERM 15 /* 软件终止 */
50 #define SIGSTKFLT 16 /* 栈异常 */
51 #define SIGCHLD 17 /* 子进程结束 */
52 #define SIGCONT 18 /* 进程继续 */
53 #define SIGSTOP 19 /* 停止进程的执行，只是暂停 */
54 #define SIGTSTP 20 /* 停止进程的运行(Ctrl+Z 组合键) */
55 #define SIGTTIN 21 /* 后台进程需要从终端读取数据 */
56 #define SIGTTOU 22 /* 后台进程需要向终端写数据 */
57 #define SIGURG 23 /* 有"紧急"数据 */
58 #define SIGXCPU 24 /* 超过 CPU 资源限制 */
59 #define SIGXFSZ 25 /* 文件大小超额 */
60 #define SIGVTALRM 26 /* 虚拟时钟信号 */
61 #define SIGPROF 27 /* 时钟信号描述 */
62 #define SIGWINCH 28 /* 窗口大小改变 */
63 #define SIGIO 29 /* 可以进行输入/输出操作 */
64 #define SIGPOLL SIGIO 
65 /* #define SIGLOS 29 */
66 #define SIGPWR 30 /* 断点重启 */
67 #define SIGSYS 31 /* 非法的系统调用 */
68 #define SIGUNUSED 31 /* 未使用信号 */
 在示例代码 53.1.1.1 中的这些信号中，除了 SIGKILL(9)和 SIGSTOP(19)这两个信号不能被
忽略外，其他的信号都可以忽略。这些信号就相当于中断号，不同的中断号代表了不同的中断，
不同的中断所做的处理不同，因此，驱动程序可以通过向应用程序发送不同的信号来实现不同
的功能。
我们使用中断的时候需要设置中断处理函数，同样的，如果要在应用程序中使用信号，那
么就必须设置信号所使用的信号处理函数，在应用程序中使用 signal 函数来设置指定信号的处
理函数，signal 函数原型如下所示：
sighandler_t signal(int signum, sighandler_t handler)
函数参数和返回值含义如下：
signum：要设置处理函数的信号。
handler：信号的处理函数。
返回值：设置成功的话返回信号的前一个处理函数，设置失败的话返回 SIG_ERR。
信号处理函数原型如下所示：
typedef void (*sighandler_t)(int)
我们前面讲解的使用“kill -9 PID”杀死指定进程的方法就是向指定的进程(PID)发送
SIGKILL 这个信号。当按下键盘上的 CTRL+C 组合键以后会向当前正在占用终端的应用程序发
出 SIGINT 信号，SIGINT 信号默认的动作是关闭当前应用程序。这里我们修改一下 SIGINT 信
号的默认处理函数，当按下 CTRL+C 组合键以后先在终端上打印出“SIGINT signal！”这行字
符串，然后再关闭当前应用程序。新建 signaltest.c 文件，然后输入如下所示内容：
示例代码 53.1.1.2 信号测试
1 #include "stdlib.h"
2 #include "stdio.h"
3 #include "signal.h"
4 
5 void sigint_handler(int num)
6 {
7 printf("\r\nSIGINT signal!\r\n");
8 exit(0);
9 }
10
11 int main(void)
12 {
13 signal(SIGINT, sigint_handler);
14 while(1);
15 return 0;
16 }
在示例代码 53.1.1.2 中我们设置 SIGINT 信号的处理函数为 sigint_handler，当按下 CTRL+C
向 signaltest 发送 SIGINT 信号以后 sigint_handler 函数就会执行，此函数先输出一行“SIGINT 
signal!”字符串，然后调用 exit 函数关闭 signaltest 应用程序。
使用如下命令编译 signaltest.c：
gcc signaltest.c -o signaltest
然后输入“./signaltest”命令打开 signaltest 这个应用程序，然后按下键盘上的 CTRL+C 组
合键，结果如图 53.1.1.1 所示：
2、fasync 函数
如果要使用异步通知，需要在设备驱动中实现 file_operations 操作集中的 fasync 函数，此
函数格式如下所示：
int (*fasync) (int fd, struct file *filp, int on)
fasync 函数里面一般通过调用 fasync_helper 函数来初始化前面定义的 fasync_struct 结构体
指针，fasync_helper 函数原型如下：
int fasync_helper(int fd, struct file * filp, int on, struct fasync_struct **fapp)
fasync_helper 函数的前三个参数就是 fasync 函数的那三个参数，第四个参数就是要初始化
的 fasync_struct 结构体指针变量。当应用程序通过“fcntl(fd, F_SETFL, flags | FASYNC)”改变
fasync 标记的时候，驱动程序 file_operations 操作集中的 fasync 函数就会执行。
驱动程序中的 fasync 函数参考示例如下：
示例代码 53.1.2.3 驱动中 fasync 函数参考示例
1 struct xxx_dev {
2 ......
3 struct fasync_struct *async_queue; /* 异步相关结构体 */
4 };
5
6 static int xxx_fasync(int fd, struct file *filp, int on)
7 {
8 struct xxx_dev *dev = (xxx_dev)filp->private_data;
9 
10 if (fasync_helper(fd, filp, on, &dev->async_queue) < 0)
11 return -EIO;
12 return 0;
13 }
14
15 static struct file_operations xxx_ops = {
16 ......
17 .fasync = xxx_fasync,
18 ......
19 };
在关闭驱动文件的时候需要在 file_operations 操作集中的 release 函数中释放 fasync_struct，
fasync_struct 的释放函数同样为 fasync_helper，release 函数参数参考实例如下：
示例代码 53.1.2.4 释放 fasync_struct 参考示例
1 static int xxx_release(struct inode *inode, struct file *filp)
2 {
3 return xxx_fasync(-1, filp, 0); /* 删除异步通知 */
4 }
5
6 static struct file_operations xxx_ops = {
7 ......
8 .release = xxx_release,
9 };
第 3 行通过调用示例代码 53.1.2.3 中的 xxx_fasync 函数来完成 fasync_struct 的释放工作，
但是，其最终还是通过 fasync_helper 函数完成释放工作。
1、kill_fasync 函数
当设备可以访问的时候，驱动程序需要向应用程序发出信号，相当于产生“中断”。kill_fasync
函数负责发送指定的信号，kill_fasync 函数原型如下所示：
void kill_fasync(struct fasync_struct **fp, int sig, int band)
函数参数和返回值含义如下：
fp：要操作的 fasync_struct。
sig：要发送的信号。
band：可读时设置为 POLL_IN，可写时设置为 POLL_OUT。
返回值：无。

应用程序对异步通知的处理
应用程序对异步通知的处理包括以下三步：
1、注册信号处理函数
应用程序根据驱动程序所使用的信号来设置信号的处理函数，应用程序使用 signal 函数来
设置信号的处理函数。前面已经详细的讲过了，这里就不细讲了。
2、将本应用程序的进程号告诉给内核
使用 fcntl(fd, F_SETOWN, getpid())将本应用程序的进程号告诉给内核。
3、开启异步通知
使用如下两行程序开启异步通知：
flags = fcntl(fd, F_GETFL); /* 获取当前的进程状态 */
fcntl(fd, F_SETFL, flags | FASYNC); /* 开启当前进程异步通知功能 */
重点就是通过 fcntl 函数设置进程状态为 FASYNC，经过这一步，驱动程序中的 fasync 函
数就会执行。

# PLATFORM驱动
