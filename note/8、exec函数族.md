<<<<<<< HEAD
# exec函数族



## exec函数族介绍

- exec 函数族的作用是根据指定的文件名找到可执行文件，并用它来取代调用进程的内容，换句话说，**就是在调用进程内部执行一个可执行文件。**
- exec 函数族的函数执行成功后不会返回，因为调用进程的实体，包括代码段，数据段和堆栈等都已经被新的内容取代，只留下进程 ID 等一些表面上的信息仍保持原样，只有调用失败了，它们才会返回 -1，从原程序的调用点接着往下执行。

一般我们创建一个子进程，然后在子进程中使用exec函数族。

## 虚拟地址空间视角

执行exec函数族后a.out的程序替换掉用户区数据

![image-20220810133412503](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810133412503.png)

## exec 函数族的七个函数

```C
int execl(const char *path, const char *arg, .../* (char *) NULL */);
int execlp(const char *file, const char *arg, ... /* (char *) NULL */);
int execle(const char *path, const char *arg, .../*, (char *) NULL, char * const envp[] */);
int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
int execvpe(const char *file, char *const argv[], char *const envp[]);
int execve(const char *filename, char *const argv[], char *const envp[]);

//前6个都是C库的函数，最后一个是Linux的函数,前6个基于最后一个进行封装，前两个比较常用。
l(list) 参数地址列表，以空指针结尾
v(vector) 存有各参数地址的指针数组的地址
p(path) 按 PATH 环境变量指定的目录搜索可执行文件
e(environment) 存有环境变量字符串地址的指针数组的地址
```



### execl

```C
/*  
    #include <unistd.h>
    int execl(const char *path, const char *arg, ...);
        - 参数：
            - path:需要指定的执行的文件的路径或者名称
                a.out /home/nowcoder/a.out 推荐使用绝对路径
                ./a.out hello world

            - arg:是执行可执行文件所需要的参数列表
                第一个参数一般没有什么作用，为了方便，一般写的是执行的程序的名称
                从第二个参数开始往后，就是程序执行所需要的的参数列表。
                参数最后需要以NULL结束（哨兵）

        - 返回值：
            只有当调用失败，才会有返回值，返回-1，并且设置errno
            如果调用成功，没有返回值。

*/
#include <unistd.h>
#include <stdio.h>

int main() {


    // 创建一个子进程，在子进程中执行exec函数族中的函数
    pid_t pid = fork();

    if(pid > 0) {
        // 父进程
        printf("i am parent process, pid : %d\n",getpid());
        sleep(1);
    }else if(pid == 0) {
        // 子进程
        // execl("hello","hello",NULL);

        execl("/bin/ps", "ps", "aux", NULL);
        perror("execl");
        printf("i am child process, pid : %d\n", getpid());

    }

    for(int i = 0; i < 3; i++) {
        printf("i = %d, pid = %d\n", i, getpid());
    }
    return 0;
}
```



执行结果如下:

![image-20220810203514153](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810203514153.png)



因为子进程调用了`execl`函数，所以用户区被可执行文件的数据替换掉，原来的代码也就没有了，所以子进程**不会**执行下面这些代码

```C
 printf("i am child process, pid : %d\n", getpid());
 
    for(int i = 0; i < 3; i++) {
        printf("i = %d, pid = %d\n", i, getpid());
    }
```

### execlp

根据函数名从环境变量中找可执行程序

```C
/*  
    #include <unistd.h>
    int execlp(const char *file, const char *arg, ... );
        - 会到环境变量中查找指定的可执行文件，如果找到了就执行，找不到就执行不成功。
        - 参数：
            - file:需要执行的可执行文件的文件名
                a.out
                ps

            - arg:是执行可执行文件所需要的参数列表
                第一个参数一般没有什么作用，为了方便，一般写的是执行的程序的名称
                从第二个参数开始往后，就是程序执行所需要的的参数列表。
                参数最后需要以NULL结束（哨兵）

        - 返回值：
            只有当调用失败，才会有返回值，返回-1，并且设置errno
            如果调用成功，没有返回值。


        int execv(const char *path, char *const argv[]);
        argv是需要的参数的一个字符串数组
        char * argv[] = {"ps", "aux", NULL};
        execv("/bin/ps", argv);

        int execve(const char *filename, char *const argv[], char *const envp[]);
        char * envp[] = {"/home/nowcoder", "/home/bbb", "/home/aaa"};


*/
#include <unistd.h>
#include <stdio.h>

int main() {


    // 创建一个子进程，在子进程中执行exec函数族中的函数
    pid_t pid = fork();

    if(pid > 0) {
        // 父进程
        printf("i am parent process, pid : %d\n",getpid());
        sleep(1);
    }else if(pid == 0) {
        // 子进程
        execlp("ps", "ps", "aux", NULL);

        printf("i am child process, pid : %d\n", getpid());

    }

    for(int i = 0; i < 3; i++) {
        printf("i = %d, pid = %d\n", i, getpid());
    }


    return 0;
}
```

其他几个函数可以根据末尾字母含义或者查询man手册了解使用方法
=======
# exec函数族



## exec函数族介绍

- exec 函数族的作用是根据指定的文件名找到可执行文件，并用它来取代调用进程的内容，换句话说，**就是在调用进程内部执行一个可执行文件。**
- exec 函数族的函数执行成功后不会返回，因为调用进程的实体，包括代码段，数据段和堆栈等都已经被新的内容取代，只留下进程 ID 等一些表面上的信息仍保持原样，只有调用失败了，它们才会返回 -1，从原程序的调用点接着往下执行。

一般我们创建一个子进程，然后在子进程中使用exec函数族。

## 虚拟地址空间视角

执行exec函数族后a.out的程序替换掉用户区数据

![image-20220810133412503](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810133412503.png)

## exec 函数族的七个函数

```C
int execl(const char *path, const char *arg, .../* (char *) NULL */);
int execlp(const char *file, const char *arg, ... /* (char *) NULL */);
int execle(const char *path, const char *arg, .../*, (char *) NULL, char * const envp[] */);
int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
int execvpe(const char *file, char *const argv[], char *const envp[]);
int execve(const char *filename, char *const argv[], char *const envp[]);

//前6个都是C库的函数，最后一个是Linux的函数,前6个基于最后一个进行封装，前两个比较常用。
l(list) 参数地址列表，以空指针结尾
v(vector) 存有各参数地址的指针数组的地址
p(path) 按 PATH 环境变量指定的目录搜索可执行文件
e(environment) 存有环境变量字符串地址的指针数组的地址
```



### execl

```C
/*  
    #include <unistd.h>
    int execl(const char *path, const char *arg, ...);
        - 参数：
            - path:需要指定的执行的文件的路径或者名称
                a.out /home/nowcoder/a.out 推荐使用绝对路径
                ./a.out hello world

            - arg:是执行可执行文件所需要的参数列表
                第一个参数一般没有什么作用，为了方便，一般写的是执行的程序的名称
                从第二个参数开始往后，就是程序执行所需要的的参数列表。
                参数最后需要以NULL结束（哨兵）

        - 返回值：
            只有当调用失败，才会有返回值，返回-1，并且设置errno
            如果调用成功，没有返回值。

*/
#include <unistd.h>
#include <stdio.h>

int main() {


    // 创建一个子进程，在子进程中执行exec函数族中的函数
    pid_t pid = fork();

    if(pid > 0) {
        // 父进程
        printf("i am parent process, pid : %d\n",getpid());
        sleep(1);
    }else if(pid == 0) {
        // 子进程
        // execl("hello","hello",NULL);

        execl("/bin/ps", "ps", "aux", NULL);
        perror("execl");
        printf("i am child process, pid : %d\n", getpid());

    }

    for(int i = 0; i < 3; i++) {
        printf("i = %d, pid = %d\n", i, getpid());
    }
    return 0;
}
```



执行结果如下:

![image-20220810203514153](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220810203514153.png)



因为子进程调用了`execl`函数，所以用户区被可执行文件的数据替换掉，原来的代码也就没有了，所以子进程**不会**执行下面这些代码

```C
 printf("i am child process, pid : %d\n", getpid());
 
    for(int i = 0; i < 3; i++) {
        printf("i = %d, pid = %d\n", i, getpid());
    }
```

### execlp

根据函数名从环境变量中找可执行程序

```C
/*  
    #include <unistd.h>
    int execlp(const char *file, const char *arg, ... );
        - 会到环境变量中查找指定的可执行文件，如果找到了就执行，找不到就执行不成功。
        - 参数：
            - file:需要执行的可执行文件的文件名
                a.out
                ps

            - arg:是执行可执行文件所需要的参数列表
                第一个参数一般没有什么作用，为了方便，一般写的是执行的程序的名称
                从第二个参数开始往后，就是程序执行所需要的的参数列表。
                参数最后需要以NULL结束（哨兵）

        - 返回值：
            只有当调用失败，才会有返回值，返回-1，并且设置errno
            如果调用成功，没有返回值。


        int execv(const char *path, char *const argv[]);
        argv是需要的参数的一个字符串数组
        char * argv[] = {"ps", "aux", NULL};
        execv("/bin/ps", argv);

        int execve(const char *filename, char *const argv[], char *const envp[]);
        char * envp[] = {"/home/nowcoder", "/home/bbb", "/home/aaa"};


*/
#include <unistd.h>
#include <stdio.h>

int main() {


    // 创建一个子进程，在子进程中执行exec函数族中的函数
    pid_t pid = fork();

    if(pid > 0) {
        // 父进程
        printf("i am parent process, pid : %d\n",getpid());
        sleep(1);
    }else if(pid == 0) {
        // 子进程
        execlp("ps", "ps", "aux", NULL);

        printf("i am child process, pid : %d\n", getpid());

    }

    for(int i = 0; i < 3; i++) {
        printf("i = %d, pid = %d\n", i, getpid());
    }


    return 0;
}
```

其他几个函数可以根据末尾字母含义或者查询man手册了解使用方法
>>>>>>> f5b7f421229ec4a664efbc33954cf3fd58c9b115
