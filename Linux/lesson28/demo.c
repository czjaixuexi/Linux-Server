#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

void work(int num){
    //捕捉到信号后,获取系统时间，写入磁盘文件
    time_t tm = time(NULL);
    struct tm *loc = localtime(&tm);
    char buf[1024];
    sprintf(buf,"%d-%d-%d %d:%d:%d",loc->tm_year,loc->tm_mon,loc->tm_mday,loc->tm_hour,loc->tm_min,loc->tm_sec);

    printf("%s\n",buf); 
}


int main()
{
    // 1.创建子进程，退出父进程
    pid_t pid = fork();
    if (pid > 0)
    {
        exit(0);
    }

    // 2.将子进程重建创建一个会话
    setsid();

    // 3.设置掩码
    umask(022);

    // 4.更改工作目录
    chdir("/");

    // 5.关闭、重定向文件描述符
    int fd = open("dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    // 6.业务逻辑
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = work;
    sigemptyset(&act.sa_mask);
    //捕捉定时信号
    sigaction(SIGALRM, &act, NULL);
    struct itimerval val;
    val.it_value.tv_sec = 2;
    val.it_value.tv_usec = 0;
    val.it_interval.tv_sec = 2;
    val.it_interval.tv_usec = 0;
    //创建定时器
    setitimer(ITIMER_REAL, &val, NULL);

    while(1){
        sleep(10);
    }
    return 0;
}
