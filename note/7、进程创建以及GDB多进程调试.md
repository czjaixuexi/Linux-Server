<<<<<<< HEAD
# 进程创建以及GDB多进程调试

## 进程创建

系统允许一个进程创建新进程，新进程就是子进程，子进程还可以创建子进程，形成树结构模型

我们可以使用`fork`函数创建子进程

```C
/*
    #include <sys/types.h>
    #include <unistd.h>

    pid_t fork(void);
        函数的作用：用于创建子进程。
        返回值：
            fork()的返回值会返回两次。一次是在父进程中，一次是在子进程中。
            在父进程中返回创建的子进程的ID,
            在子进程中返回0
            如何区分父进程和子进程：通过fork的返回值。
            在父进程中返回-1，表示创建子进程失败，并且设置errno
			失败的两个主要原因：
				1. 当前系统的进程数已经达到了系统规定的上限，这时 errno 的值被设置为 EAGAIN
				2. 系统内存不足，这时 errno 的值被设置为 ENOMEM
        
        父子进程之间的关系：
        区别：
            1.fork()函数的返回值不同
                父进程中: >0 返回的子进程的ID
                子进程中: =0
            2.pcb中的一些数据
                当前的进程的id pid
                当前的进程的父进程的id ppid
                信号集

        共同点：
            某些状态下：子进程刚被创建出来，还没有执行任何的写数据的操作
                - 用户区的数据
                - 文件描述符表
        
        父子进程对变量是不是共享的？
            - 刚开始的时候，是一样的，共享的。如果修改了数据，不共享了。
            - 读时共享（子进程被创建，两个进程没有做任何的写的操作），写时拷贝。
        
*/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main() {

    int num = 10;

    // 创建子进程
    pid_t pid = fork();

    // 判断是父进程还是子进程
    if(pid > 0) {
        // printf("pid : %d\n", pid);
        // 如果大于0，返回的是创建的子进程的进程号，当前是父进程
        printf("i am parent process, pid : %d, ppid : %d\n", getpid(), getppid());

        printf("parent num : %d\n", num);
        num += 10;
        printf("parent num += 10 : %d\n", num);


    } else if(pid == 0) {
        // 当前是子进程
        printf("i am child process, pid : %d, ppid : %d\n", getpid(),getppid());
       
        printf("child num : %d\n", num);
        num += 100;
        printf("child num += 100 : %d\n", num);
    }

    // for循环
    for(int i = 0; i < 3; i++) {
        printf("i : %d , pid : %d\n", i , getpid());
        sleep(1);
    }

    return 0;
}

/*
实际上，更准确来说，Linux 的 fork() 使用是通过写时拷贝 (copy- on-write) 实现。
写时拷贝是一种可以推迟甚至避免拷贝数据的技术。
内核此时并不复制整个进程的地址空间，而是让父子进程共享同一个地址空间。
只用在需要写入的时候才会复制地址空间，从而使各个进行拥有各自的地址空间。
也就是说，资源的复制是在需要写入的时候才会进行，在此之前，只有以只读方式共享。
注意：fork之后父子进程共享文件，
fork产生的子进程与父进程相同的文件文件描述符指向相同的文件表，引用计数增加，共享文件偏移指针。
*/
```

## 父子进程虚拟地址空间情况

下面给出父子进程分别执行的代码，注意这里是执行的代码，实际上所有代码都被包含进去的

父进程执行的代码

```C
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(){
        int num = 10;
        pid_t pid = fork();
        if(pid>0){
                printf("i am parent process,pid: %d ,ppid: %d\n",getpid(),getppid());
                printf("parent num: %d\n",num);
                num += 10;
                printf("parent num += 10 :%d\n",num);
        }

        for(int i = 0; i < 3; i++){
                printf("i : %d , pid : %d\n", i , getpid());
                sleep(1);
        }
        return 0;
}
```

子进程执行的代码

```C
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(){
        else if(pid == 0){
                printf("i am child process,pid: %d ,ppid: %d\n",getpid(),getppid());
                 printf("child num : %d\n", num);
                 num += 100;
                 printf("child num += 100 : %d\n", num);
        }

        for(int i = 0; i < 3; i++){
                printf("i : %d , pid : %d\n", i , getpid());
                sleep(1);
        }
        return 0;
}
```



在虚拟地址空间的视角下，fork()函数相当于把父进程的虚拟地址空间clone给子进程。

fork()以后，子进程用户区数据和父进程一样。内核区也会拷贝过来，但是pid不一样。但是两个虚拟地址空间是相互独立的，可以看到num的计算是没有任何影响的分开计算的。

![image-20220809132722712](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220809132722712.png)

**读时共享，写时拷贝**

实际上，更准确来说，Linux 的 fork() 使用是通过写时拷贝 (copy- on-write) 实现。

写时拷贝是一种可以推迟甚至避免拷贝数据的技术，从而提高效率。

内核此时并不复制整个进程的地址空间，而是让父子进程共享同一个地址空间。

**只用在需要写入的时候才会复制地址空间，从而使各个进行拥有各自的地址空间。**

也就是说，资源的复制是在需要写入的时候才会进行，在此之前，只有以只读方式共享。

注意：fork之后父子进程共享文件，

fork产生的子进程与父进程相同的文件文件描述符指向相同的文件表，引用计数增加，共享文件偏移指针。

## 父子进程之间的关系

### 区别

**1、fork()函数返回值不同**

- 父进程中: >0 返回的子进程的ID    
- 子进程中: =0

**2、pcb中的一些数据**

- 当前进程的id（PID）

- 当前进程的父进程的id（PPID）信号集

### 共同点

在某些状态下：子进程刚被创建出来，还没有被执行任何的写数据操作。

用户区数据、文件描述符表是一样的（共享的）

### 父子进程对变量是不是共享的？

刚开始的时候是一样的，共享的；如果修改了数据，不共享了。**读时共享，写时拷贝**。



## GDB多进程调试

对下面的代码进行gdb调试

```C
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(){
        printf("begin\n");
        if(fork()>0){
                printf("我是父进程 pid = %d , ppid = %d \n",getpid(),getppid());
                int i=0;
                for(i = 0; i < 10 ; i ++){
                        printf("i = %d\n",i);
                        sleep(1);
                }
        }
        else{

                printf("我是子进程 pid = %d , ppid = %d \n",getpid(),getppid());
                int j=0;
                for(j = 0; j < 10 ; j ++){
                        printf("j = %d\n",j);
                        sleep(1);

                }
        }
        return 0;
}
```



父进程和子进程分别打上断点后run,发现GDB默认情况下追踪父进程，子进程直接运行完:

![image-20220810123630690](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810123630690.png)



### 设置调试父进程或者子进程

```shell
set follow-fork-mode [parent（默认）| child]
```

![image-20220810124217636](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810124217636.png)



设置调试子进程后打上同样断点run，结果如下:

![image-20220810124343087](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810124343087.png)



### 设置调试模式

```shell
set detach-on-fork [on | off]
```

默认为 on，表示调试当前进程的时候，其它的进程继续运行，如果为 off，调试当前进程的时候，其它进程被 GDB 挂起。(8.0版本以上的GDB在调试多进程时候会有BUG，需要安装一个8.0版本以下的Ubuntu 例如Ubunutu16的虚拟机)

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810125224169.png" alt="image-20220810125224169" style="zoom: 80%;" />



### 查看调试的进程

```shell
info inferiors #查看调试的进程
inferior id    #切换当前调试的进程为Num号为id的进程
```

切到子进程后先按一下c （continue）转到调试子进程

![image-20220810130154380](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810130154380.png)



### 使进程脱离 GDB 调试

```shell
detach inferiors id #使Num为id的进程脱离 GDB 调试
```

=======
# 进程创建以及GDB多进程调试

## 进程创建

系统允许一个进程创建新进程，新进程就是子进程，子进程还可以创建子进程，形成树结构模型

我们可以使用`fork`函数创建子进程

```C
/*
    #include <sys/types.h>
    #include <unistd.h>

    pid_t fork(void);
        函数的作用：用于创建子进程。
        返回值：
            fork()的返回值会返回两次。一次是在父进程中，一次是在子进程中。
            在父进程中返回创建的子进程的ID,
            在子进程中返回0
            如何区分父进程和子进程：通过fork的返回值。
            在父进程中返回-1，表示创建子进程失败，并且设置errno
			失败的两个主要原因：
				1. 当前系统的进程数已经达到了系统规定的上限，这时 errno 的值被设置为 EAGAIN
				2. 系统内存不足，这时 errno 的值被设置为 ENOMEM
        
        父子进程之间的关系：
        区别：
            1.fork()函数的返回值不同
                父进程中: >0 返回的子进程的ID
                子进程中: =0
            2.pcb中的一些数据
                当前的进程的id pid
                当前的进程的父进程的id ppid
                信号集

        共同点：
            某些状态下：子进程刚被创建出来，还没有执行任何的写数据的操作
                - 用户区的数据
                - 文件描述符表
        
        父子进程对变量是不是共享的？
            - 刚开始的时候，是一样的，共享的。如果修改了数据，不共享了。
            - 读时共享（子进程被创建，两个进程没有做任何的写的操作），写时拷贝。
        
*/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main() {

    int num = 10;

    // 创建子进程
    pid_t pid = fork();

    // 判断是父进程还是子进程
    if(pid > 0) {
        // printf("pid : %d\n", pid);
        // 如果大于0，返回的是创建的子进程的进程号，当前是父进程
        printf("i am parent process, pid : %d, ppid : %d\n", getpid(), getppid());

        printf("parent num : %d\n", num);
        num += 10;
        printf("parent num += 10 : %d\n", num);


    } else if(pid == 0) {
        // 当前是子进程
        printf("i am child process, pid : %d, ppid : %d\n", getpid(),getppid());
       
        printf("child num : %d\n", num);
        num += 100;
        printf("child num += 100 : %d\n", num);
    }

    // for循环
    for(int i = 0; i < 3; i++) {
        printf("i : %d , pid : %d\n", i , getpid());
        sleep(1);
    }

    return 0;
}

/*
实际上，更准确来说，Linux 的 fork() 使用是通过写时拷贝 (copy- on-write) 实现。
写时拷贝是一种可以推迟甚至避免拷贝数据的技术。
内核此时并不复制整个进程的地址空间，而是让父子进程共享同一个地址空间。
只用在需要写入的时候才会复制地址空间，从而使各个进行拥有各自的地址空间。
也就是说，资源的复制是在需要写入的时候才会进行，在此之前，只有以只读方式共享。
注意：fork之后父子进程共享文件，
fork产生的子进程与父进程相同的文件文件描述符指向相同的文件表，引用计数增加，共享文件偏移指针。
*/
```

## 父子进程虚拟地址空间情况

下面给出父子进程分别执行的代码，注意这里是执行的代码，实际上所有代码都被包含进去的

父进程执行的代码

```C
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(){
        int num = 10;
        pid_t pid = fork();
        if(pid>0){
                printf("i am parent process,pid: %d ,ppid: %d\n",getpid(),getppid());
                printf("parent num: %d\n",num);
                num += 10;
                printf("parent num += 10 :%d\n",num);
        }

        for(int i = 0; i < 3; i++){
                printf("i : %d , pid : %d\n", i , getpid());
                sleep(1);
        }
        return 0;
}
```

子进程执行的代码

```C
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(){
        else if(pid == 0){
                printf("i am child process,pid: %d ,ppid: %d\n",getpid(),getppid());
                 printf("child num : %d\n", num);
                 num += 100;
                 printf("child num += 100 : %d\n", num);
        }

        for(int i = 0; i < 3; i++){
                printf("i : %d , pid : %d\n", i , getpid());
                sleep(1);
        }
        return 0;
}
```



在虚拟地址空间的视角下，fork()函数相当于把父进程的虚拟地址空间clone给子进程。

fork()以后，子进程用户区数据和父进程一样。内核区也会拷贝过来，但是pid不一样。但是两个虚拟地址空间是相互独立的，可以看到num的计算是没有任何影响的分开计算的。

![image-20220809132722712](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220809132722712.png)

**读时共享，写时拷贝**

实际上，更准确来说，Linux 的 fork() 使用是通过写时拷贝 (copy- on-write) 实现。

写时拷贝是一种可以推迟甚至避免拷贝数据的技术，从而提高效率。

内核此时并不复制整个进程的地址空间，而是让父子进程共享同一个地址空间。

**只用在需要写入的时候才会复制地址空间，从而使各个进行拥有各自的地址空间。**

也就是说，资源的复制是在需要写入的时候才会进行，在此之前，只有以只读方式共享。

注意：fork之后父子进程共享文件，

fork产生的子进程与父进程相同的文件文件描述符指向相同的文件表，引用计数增加，共享文件偏移指针。

## 父子进程之间的关系

### 区别

**1、fork()函数返回值不同**

- 父进程中: >0 返回的子进程的ID    
- 子进程中: =0

**2、pcb中的一些数据**

- 当前进程的id（PID）

- 当前进程的父进程的id（PPID）信号集

### 共同点

在某些状态下：子进程刚被创建出来，还没有被执行任何的写数据操作。

用户区数据、文件描述符表是一样的（共享的）

### 父子进程对变量是不是共享的？

刚开始的时候是一样的，共享的；如果修改了数据，不共享了。**读时共享，写时拷贝**。



## GDB多进程调试

对下面的代码进行gdb调试

```C
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(){
        printf("begin\n");
        if(fork()>0){
                printf("我是父进程 pid = %d , ppid = %d \n",getpid(),getppid());
                int i=0;
                for(i = 0; i < 10 ; i ++){
                        printf("i = %d\n",i);
                        sleep(1);
                }
        }
        else{

                printf("我是子进程 pid = %d , ppid = %d \n",getpid(),getppid());
                int j=0;
                for(j = 0; j < 10 ; j ++){
                        printf("j = %d\n",j);
                        sleep(1);

                }
        }
        return 0;
}
```



父进程和子进程分别打上断点后run,发现GDB默认情况下追踪父进程，子进程直接运行完:

![image-20220810123630690](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810123630690.png)



### 设置调试父进程或者子进程

```shell
set follow-fork-mode [parent（默认）| child]
```

![image-20220810124217636](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810124217636.png)



设置调试子进程后打上同样断点run，结果如下:

![image-20220810124343087](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810124343087.png)



### 设置调试模式

```shell
set detach-on-fork [on | off]
```

默认为 on，表示调试当前进程的时候，其它的进程继续运行，如果为 off，调试当前进程的时候，其它进程被 GDB 挂起。(8.0版本以上的GDB在调试多进程时候会有BUG，需要安装一个8.0版本以下的Ubuntu 例如Ubunutu16的虚拟机)

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810125224169.png" alt="image-20220810125224169" style="zoom: 80%;" />



### 查看调试的进程

```shell
info inferiors #查看调试的进程
inferior id    #切换当前调试的进程为Num号为id的进程
```

切到子进程后先按一下c （continue）转到调试子进程

![image-20220810130154380](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810130154380.png)



### 使进程脱离 GDB 调试

```shell
detach inferiors id #使Num为id的进程脱离 GDB 调试
```

>>>>>>> f5b7f421229ec4a664efbc33954cf3fd58c9b115
