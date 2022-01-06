/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-05 19:35:25
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-05 22:19:09
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ide.h>
#define CHRDEV_MAJOR 200
#define CHRDEV_NAME "chrdev"

static char readbuf[20];
static char writebuf[20];
static char data[]={"kernel data !"};


static int chrdev_open(struct inode *inode ,struct file *filp){
    printk("DEVICE OPEN ! \r\n");
    return 0;
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
    int ret=0;
    memcpy(readbuf,data,sizeof(data));
    ret=copy_to_user(buf,readbuf,cnt);
    if(ret==0)printk("SEND OK !\r\n");
    else printk("SEND FAILED ! \r\n");
    return 0;
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
    int ret=0;
    ret=copy_from_user(writebuf,buf,cnt);
    if(ret==0)printk("WRITE OK :%s!\r\n",writebuf);
    else printk("WRITE FAILED ! \r\n");
    return 0;
}

static int chrdev_release(struct inode *inode ,struct file *filp){
    printk("RELEASE！\r\n");
    return 0;
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
    int retvalue;
    retvalue = register_chrdev(CHRDEV_MAJOR, CHRDEV_NAME,&chrdevbase_fops);
    if(retvalue<0){
        printk("RIGISTER ERROR\r\n");
    }else{
        printk("RIGISTER SUCCESS\r\n");
    }
    return 0;
}
static void __exit chrdevbase_exit(void)
{
    unregister_chrdev(CHRDEV_MAJOR, CHRDEV_NAME);
    printk("EXIT CHRDEV \r\n");
}

module_init(chrdevbase_init);
module_exit(chrdevbase_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("YURI");