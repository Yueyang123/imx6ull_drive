/*
 * @Description: 
 * @Autor: YURI
 * @Date: 2022-01-05 21:57:55
 * @LastEditors: YURI
 * @LastEditTime: 2022-01-05 22:23:05
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
static char usrdata[] = {"usr data!"};
int main(int argc ,char** argv)
{
    int fd,ret;
    char * filename;
    char readbuf[20],writebuf[20];
    if(argc!=3)printf("ERROR CMD \r\n");
    filename=argv[1];
    fd=open(filename,O_RDWR);
    if(fd<0){
        printf("OPEN ERROR !! \r\n");
        return -1;
    }
    if(atoi(argv[2])==1){//读数据
        ret=read(fd,readbuf,10);
        if(ret<0)printf("READ ERROR !! \r\n");else{
        printf("READ DATA :%s\r\n",readbuf);
        }
    }else if(atoi(argv[2])==2){
        ret=write(fd,usrdata,sizeof(usrdata));
        if(ret<0)printf("WRITE ERROR !! \r\n");else{
        printf("WRITE DATA :%s",readbuf);       
        }
    }else {
        printf("ERROR CMD \r\n");
    }
    ret=close(fd);
    if(ret<0){
        printf("CLOSE ERROR \r\n");
        return -1;
    }
    return 0;
}
