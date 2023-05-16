# 文件IO

## 1、标准C库IO函数

![image-20220808095641399](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220808095641399.png)

​	标准C库IO函数的核心在于缓冲区，如果直接用Linux系统内核的read和write函数，每次读写都要重新访问一次磁盘，访问磁盘需要花费很多时间，IO的缓冲区很大程度减少了对磁盘的访问次数，提高了read和write函数的使用效率

## 2、标准C库IO函数和Linux系统IO函数对比

![image-20220808095707744](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220808095707744.png)



- 标准C库IO函数相较于Linux系统IO函数，具有跨平台的优势，它可以针对不同的系统调用相应的API来实现同样的操作。（JAVA通过虚拟机实现跨平台）
- 标准C库IO函数与Linux系统IO函数是调用与被调用的关系
- 标准C库IO函数比Linux系统IO函数效率高（缓冲区的功劳）
- C库与Linux系统IO函数的选择：
  - 磁盘通信时，调用C库IO比较合适，缓冲区能够提高读写效率。
  - 网络通信时，需要直接发送信息，调用Linux的IO通信比较合适。



## 3、虚拟地址空间

虚拟地址空间是不存在的，但是可以用来解释一些程序加载时的内存问题，以及程序当中堆和栈的一些概念

![image-20220808101548978](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220808101548978.png)

对于上面的情景，我们有4G内存和三个程序，三个程序分别需要内存1G,2G,2G。我们先把1G和2G的文件加载进去，第三个程序就加载不进来了。但是如果1G程序结束了，第三个文件能否加载呢？虚拟地址空间可以用来解决这些问题。

![image-20220808101838456](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220808101838456.png)





## 4、文件描述符

程序和进程：

- 进程是程序的运行过程。
- 程序占用磁盘空间不占用内存空间，进程占用内存空间不占用磁盘空间。
- 程序运行起来的时候操作系统会为其分配资源，创建一个进程



Linux哲学：一切皆文件

- 文件描述符可以帮助我们定位到文件
- 文件描述符位位于虚拟地址空间的内核区的PCB模块，内核利用文件描述符来访问文件。
- 文件描述符是非负整数。打开现存文件或新建文件时，内核会返回一个文件描述符。读写文件也需要使用文件描述符来指定待读写的文件
- 文件描述符表就是一个数组，存储了很多文件描述符，这样进程可以同时打开多个文件。这个数组的大小默认为1024,所以最多同时打开文件个数为1024。（32位系统，4G虚拟地址空间情况下）
- 不同的文件描述符可以对应同一个文件
  ![image-20220808131247682](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220808131247682.png)



## 5、Linux系统IO函数

```shell
int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
int stat(const char *pathname, struct stat *statbuf);
int lstat(const char *pathname, struct stat *statbuf);
```



`man page`对应的存储手册：

![](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBA6bG856u_6ZKT6bG85bmy,size_20,color_FFFFFF,t_70,g_se,x_16.png)







### open

open有两个函数，但是这并不是函数重载。`mode_t mode`是一个可选参数

前者主要用于打开已有文件

后者主要用于创建一个新的文件，因为是创建新文件，所以我们还要设置各个用户组对这个文件的权限

如果发生错误，我们可以使用`perro`函数打印发生的错误

```c
/*
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>

    // 打开一个已经存在的文件
    int open(const char *pathname, int flags);
        参数：
            - pathname：要打开的文件路径
            - flags：对文件的操作权限设置还有其他的设置
              O_RDONLY,  O_WRONLY,  O_RDWR  这三个设置是互斥的
        返回值：返回一个新的文件描述符，如果调用失败，返回-1

    errno：属于Linux系统函数库，库里面的一个全局变量，记录的是最近的错误号。

    #include <stdio.h>
    void perror(const char *s);		//作用：打印errno对应的错误描述
        
        //s参数：用户描述，比如hello,最终输出的内容是  hello:xxx(实际的错误描述)
    

    // 创建一个新的文件
    int open(const char *pathname, int flags, mode_t mode);
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {

    // 打开一个文件
    int fd = open("a.txt", O_RDONLY);

    if(fd == -1) {
        perror("open");
    }
    // 读写操作

    // 关闭
    close(fd);

    return 0;
}
```

对比前者，这个open多了个`mode_t mode`参数用来设置文件权限

我们采用3位8进制来表示不同用户/用户组对这个文件的权限

![在这里插入图片描述](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/3814de87d97a4b34b58e37de56b8f42b.png)

我们使用`ll`查看目录下方文件信息的时候，前面有一串字母，我们以截图中第一行为例

`l rwx rwx rwx`中第一个字母表示文件类型，后面每三个一组表示不同用户/用户组对该文件的权限。

依次为：当前用户，当前用户组，其他组

- `r`读权限
- `w`写权限
- `x`执行权限

对于单个用户组，我们可以用三位二进制表示权限

例如：rwx对应二进制111,我们可以采用8进制表示，也就是7

那么对于三个用户组:rwxrwxrwx,对应的8进制数就是777

但是我们直接设置权限可能发生不合理的情况，因此再传入mode参数后，还需要进一步处理，最终的值为`mode & ~umask`

umask用于抹去一些不合理的权限设置，我们可以直接输入umask查看当前用户的umask

![在这里插入图片描述](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/8e34223f6bca416c98eae5c3756f98ac.png)

具体例子可以看下面的代码

```C
/*
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>

    int open(const char *pathname, int flags, mode_t mode);
        参数：
            - pathname：要创建的文件的路径
            - flags：对文件的操作权限和其他的设置
                - 必选项：O_RDONLY,  O_WRONLY, O_RDWR  这三个之间是互斥的
                - 可选项：O_CREAT 文件不存在，创建新文件
            - mode：八进制的数，表示创建出的新的文件的操作权限，比如：0775
            最终的权限是：mode & ~umask
            0777   ->   111111111
        &   0775   ->   111111101
        ----------------------------
                        111111101
        按位与：0和任何数都为0
        umask的作用就是抹去某些权限。

        flags参数是一个int类型的数据，占4个字节，32位。
        flags 32个位，每一位就是一个标志位。

*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {

    // 创建一个新的文件
    int fd = open("create.txt", O_RDWR | O_CREAT, 0777);

    if(fd == -1) {
        perror("open");
    }

    // 关闭
    close(fd);

    return 0;
}
```

另外对于`flag`参数，多了一个可选选项用于处理没有文件的情况

通过上方代码，我们可以发现`flag`参数我们使用按位或设置多个权限，其原理如下。flag参数是一个32位整数，每一位为一个标记位置。

我们原来有一个读的标记，现在加入一个创建的标记，我们只需要按位或过去，那么最终权限的创建标记就变成1了。

![image-20220808193436464](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220808193436464.png)





### read & write

使用read和write函数实现文件拷贝

```C
/*  
    #include <unistd.h>
    ssize_t read(int fd, void *buf, size_t count);
        参数：
            - fd：文件描述符，open得到的，通过这个文件描述符操作某个文件
            - buf：需要读取数据存放的地方，数组的地址（传出参数）
            - count：指定的数组的大小
        返回值：
            - 成功：
                >0: 返回实际的读取到的字节数
                =0：文件已经读取完了
            - 失败：-1 ，并且设置errno

    #include <unistd.h>
    ssize_t write(int fd, const void *buf, size_t count);
        参数：
            - fd：文件描述符，open得到的，通过这个文件描述符操作某个文件
            - buf：要往磁盘写入的数据，数据
            - count：要写的数据的实际的大小
        返回值：
            成功：实际写入的字节数
            失败：返回-1，并设置errno
*/
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {

    // 1.通过open打开english.txt文件
    int srcfd = open("english.txt", O_RDONLY);
    if(srcfd == -1) {
        perror("open");
        return -1;
    }

    // 2.创建一个新的文件（拷贝文件）
    int destfd = open("cpy.txt", O_WRONLY | O_CREAT, 0664);
    if(destfd == -1) {
        perror("open");
        return -1;
    }

    // 3.频繁的读写操作
    char buf[1024] = {0};
    int len = 0;
    while((len = read(srcfd, buf, sizeof(buf))) > 0) {
        write(destfd, buf, len);
    }

    // 4.关闭文件
    close(destfd);
    close(srcfd);


    return 0;
}
```



### lseek

控制文件指针，实现众多功能

1. 移动文件指针到文件头
   lseek(fd, 0, SEEK_SET);

1. 获取当前文件指针的位置
   lseek(fd, 0, SEEK_CUR);
2. 获取文件长度
   lseek(fd, 0, SEEK_END);
3. 拓展文件的长度，当前文件10b -> 110b, 增加了100个字节
   lseek(fd, 100, SEEK_END)
   **注意：需要写一次数据**

下面的代码是以拓展文件的长度为例子。

```C
/*  
    标准C库的函数
    #include <stdio.h>
    int fseek(FILE *stream, long offset, int whence);

    Linux系统函数
    #include <sys/types.h>
    #include <unistd.h>
    off_t lseek(int fd, off_t offset, int whence);
        参数：
            - fd：文件描述符，通过open得到的，通过这个fd操作某个文件
            - offset：偏移量
            - whence:
                SEEK_SET
                    设置文件指针的偏移量
                SEEK_CUR
                    设置偏移量：当前位置 + 第二个参数offset的值
                SEEK_END
                    设置偏移量：文件大小 + 第二个参数offset的值
        返回值：返回文件指针的位置


    作用：
        1.移动文件指针到文件头
        lseek(fd, 0, SEEK_SET);

        2.获取当前文件指针的位置
        lseek(fd, 0, SEEK_CUR);

        3.获取文件长度
        lseek(fd, 0, SEEK_END);

        4.拓展文件的长度，当前文件10b, 110b, 增加了100个字节
        lseek(fd, 100, SEEK_END)
        注意：需要写一次数据

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {

    int fd = open("hello.txt", O_RDWR);

    if(fd == -1) {
        perror("open");
        return -1;
    }

    // 扩展文件的长度
    int ret = lseek(fd, 100, SEEK_END);
    if(ret == -1) {
        perror("lseek");
        return -1;
    }

    // 写入一个空数据
    write(fd, " ", 1);

    // 关闭文件
    close(fd);

    return 0;
}
```

那么扩展文件长度有什么用呢
假设我们需要下载5G的资料，但是同时我们还要使用磁盘，这有可能造成下到一半磁盘不够了。这时候我们可以`lseek`实现扩展出5G的文件，然后往扩展出来的文件当中写入数据



### stat & lstat

这两个函数用于返回文件的一些相关信息。实际上Linux自己也有stat命令，案例如下。

![image-20220808195504203](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220808195504203.png)

#### 两者的区别:

- 当我们关注的是符号链接指向的文件时，就应该使用`stat`；
- 当我们想要识别符号链接文件的时候，只能使用`lstat`。



如果b.txt链接到a.txt:	`b.txt -> a.txt` 

- 使用 `stat` 函数获取 `b.txt` 软链接的信息，获取的是其指向的 `a.txt` 文件的信息
- `lstat b.txt`获取的是`b.txt`的信息，也就是链接的信息

```C
/*
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>

    int stat(const char *pathname, struct stat *statbuf);
        作用：获取一个文件相关的一些信息
        参数:
            - pathname：操作的文件的路径
            - statbuf：结构体变量，传出参数，用于保存获取到的文件的信息
        返回值：
            成功：返回0
            失败：返回-1 设置errno
    int lstat(const char *pathname, struct stat *statbuf);
        参数:
            - pathname：操作的文件的路径
            - statbuf：结构体变量，传出参数，用于保存获取到的文件的信息
        返回值：
            成功：返回0
            失败：返回-1 设置errno

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

int main() {

    struct stat statbuf;

    int ret = stat("a.txt", &statbuf);

    if(ret == -1) {
        perror("stat");
        return -1;
    }

    printf("size: %ld\n", statbuf.st_size);


    return 0;
}
```



#### stat结构体：

![image-20220808195923596](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220808195923596.png)

其中`st_mode`的信息存储方式如下：

![image-20220808200015345](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220808200015345.png)

如果要判断某个权限，我们使用按位与的操作看01即可

如果要判断某个文件类型，我们需要先和掩码按位与

```C
(st_mode & S_IFMT)==S_IFREG
```



## 6、文件属性操作函数

```C
int access(const char *pathname, int mode);
int chmod(const char *filename, int mode);
int chown(const char *path, uid_t owner, gid_t group);
int truncate(const char *path, off_t length);
```



### access

作用：判断某个文件是否有某个权限，或者判断文件是否存在

```C
/*
    #include <unistd.h>
    int access(const char *pathname, int mode);
        作用：判断某个文件是否有某个权限，或者判断文件是否存在
        参数：
            - pathname: 判断的文件路径
            - mode:
                R_OK: 判断是否有读权限
                W_OK: 判断是否有写权限
                X_OK: 判断是否有执行权限
                F_OK: 判断文件是否存在
        返回值：成功返回0， 失败返回-1
*/

#include <unistd.h>
#include <stdio.h>

int main() {

    int ret = access("a.txt", F_OK);
    if(ret == -1) {
        perror("access");
    }

    printf("文件存在！！!\n");

    return 0;
}
```



### chmod

作用：修改文件的权限

```C
/*
    #include <sys/stat.h>
    int chmod(const char *pathname, mode_t mode);
        修改文件的权限
        参数：
            - pathname: 需要修改的文件的路径
            - mode:需要修改的权限值，八进制的数
        返回值：成功返回0，失败返回-1

*/
#include <sys/stat.h>
#include <stdio.h>
int main() {

    int ret = chmod("a.txt", 0777);

    if(ret == -1) {
        perror("chmod");
        return -1;
    }

    return 0;
}
```



### chown

使用下面两个命令可以查询用户和组的id

```shell
vim /etc/passwd
vim /etc/group
```



`chown`也就是`change owner`改变拥有者和用户组

```C
int chown(const char *pathname, uid_t owner, gid_t group);
```



### truncate

作用：缩减或者扩展文件的尺寸至指定的大小

```C
/*
    #include <unistd.h>
    #include <sys/types.h>
    int truncate(const char *path, off_t length);
        作用：缩减或者扩展文件的尺寸至指定的大小
        参数：
            - path: 需要修改的文件的路径
            - length: 需要最终文件变成的大小
        返回值：
            成功返回0， 失败返回-1
*/

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main() {

    int ret = truncate("b.txt", 5);

    if(ret == -1) {
        perror("truncate");
        return -1;
    }

    return 0;
}
```



## 7、目录操作函数

```C
int rename(const char *oldpath, const char *newpath);
int chdir(const char *path);
char *getcwd(char *buf, size_t size);
int mkdir(const char *pathname, mode_t mode);
int rmdir(const char *pathname);
```



### mkdir

作用：创建一个目录

```C
/*
    #include <sys/stat.h>
    #include <sys/types.h>
    int mkdir(const char *pathname, mode_t mode);
        作用：创建一个目录
        参数：
            pathname: 创建的目录的路径
            mode: 权限，八进制的数
        返回值：
            成功返回0， 失败返回-1
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

int main() {

    int ret = mkdir("aaa", 0777);//777前面要加0才能表示8进制不然会有问题

    if(ret == -1) {
        perror("mkdir");
        return -1;
    }

    return 0;
}
```

- 权限0777,不要漏掉0,不然默认10进制
- 一个目录一定要有可执行权限才能进入目录
- 最终权限为`mode & ~umask`



### rmdir

作用：删除空目录

```C
int rmdir(const char *pathname);
```



### rename

作用：更改目录名

```C
/*
    #include <stdio.h>
    int rename(const char *oldpath, const char *newpath);

*/
#include <stdio.h>

int main() {

    int ret = rename("aaa", "bbb");

    if(ret == -1) {
        perror("rename");
        return -1;
    }

    return 0;
}
```



### chdir & getcwd

`chdir`   	作用:修改进程的工作目录，类似shell当中的`cd` 
`getcwd` 	作用:类似shell中的`pwd`，获取当前的工作目录

```C
/*

    #include <unistd.h>
    int chdir(const char *path);
        作用：修改进程的工作目录
            比如在/home/nowcoder 启动了一个可执行程序a.out, 进程的工作目录 /home/nowcoder
        参数：
            path : 需要修改的工作目录

    #include <unistd.h>
    char *getcwd(char *buf, size_t size);
        作用：获取当前工作目录
        参数：
            - buf : 存储的路径，指向的是一个数组（传出参数）
            - size: 数组的大小
        返回值：
            返回的指向的一块内存，这个数据就是第一个参数

*/
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main() {

    // 获取当前的工作目录
    char buf[128];
    getcwd(buf, sizeof(buf));
    printf("当前的工作目录是：%s\n", buf);

    // 修改工作目录
    int ret = chdir("/home/nowcoder/Linux/lesson13");
    if(ret == -1) {
        perror("chdir");
        return -1;
    } 

    // 创建一个新的文件
    int fd = open("chdir.txt", O_CREAT | O_RDWR, 0664);
    if(fd == -1) {
        perror("open");
        return -1;
    }

    close(fd);

    // 获取当前的工作目录
    char buf1[128];
    getcwd(buf1, sizeof(buf1));
    printf("当前的工作目录是：%s\n", buf1);
    
    return 0;
}
```



## 8、目录遍历函数

万物皆文件，目录也可以看作是一个文件
对于一个目录我们也有打开，读取，关闭的相关函数，他们分别为

```C
DIR *opendir(const char *name); //打开目录
struct dirent *readdir(DIR *dirp);//读取目录
int closedir(DIR *dirp);		//关闭目录
```



### 统计目录下文件数量

首先，我们需要判断输入参数是否合法，如果合法进入功能函数。
因为一个目录下还有目录，所以我们需要有个递归。

然后我们要一个个统计文件，所以我们函数里要有循环，那么这个函数的基本结构如下：

1. 首先打开目录

2. 进入循环，循环出去条件为读取到末尾
   循环内部一个判断分支：

   - 如果是目录，就递归这个函数

   - 如果是文件，计数器加

3. 最后关闭目录

4. 返回计数器

```C
/*
    // 打开一个目录
    #include <sys/types.h>
    #include <dirent.h>
    DIR *opendir(const char *name);
        参数：
            - name: 需要打开的目录的名称
        返回值：
            DIR * 类型，理解为目录流
            错误返回NULL


    // 读取目录中的数据
    #include <dirent.h>
    struct dirent *readdir(DIR *dirp);
        - 参数：dirp是opendir返回的结果
        - 返回值：
            struct dirent，代表读取到的文件的信息
            读取到了末尾或者失败了，返回NULL

    // 关闭目录
    #include <sys/types.h>
    #include <dirent.h>
    int closedir(DIR *dirp);

*/
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int getFileNum(const char * path);

// 读取某个目录下所有的普通文件的个数
int main(int argc, char * argv[]) {

    if(argc < 2) {
        printf("%s path\n", argv[0]);
        return -1;
    }

    int num = getFileNum(argv[1]);

    printf("普通文件的个数为：%d\n", num);

    return 0;
}

// 用于获取目录下所有普通文件的个数
int getFileNum(const char * path) {

    // 1.打开目录
    DIR * dir = opendir(path);

    if(dir == NULL) {
        perror("opendir");
        exit(0);
    }

    struct dirent *ptr;

    // 记录普通文件的个数
    int total = 0;

    while((ptr = readdir(dir)) != NULL) {

        // 获取名称
        char * dname = ptr->d_name;

        // 忽略掉. 和..
        if(strcmp(dname, ".") == 0 || strcmp(dname, "..") == 0) {
            continue;
        }

        // 判断是否是普通文件还是目录
        if(ptr->d_type == DT_DIR) {
            // 目录,需要继续读取这个目录
            char newpath[256];
            sprintf(newpath, "%s/%s", path, dname);
            total += getFileNum(newpath);
        }

        if(ptr->d_type == DT_REG) {
            // 普通文件
            total++;
        }


    }

    // 关闭目录
    closedir(dir);

    return total;
}
```



### struct dirent 结构体和d_type

![image-20220808202443490](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220808202443490.png)



## 9、文件描述符相关函数



### dup

作用：复制文件描述符

```C
/*
    #include <unistd.h>
    int dup(int oldfd);
        作用：复制一个新的文件描述符
        fd=3, int fd1 = dup(fd),
        fd指向的是a.txt, fd1也是指向a.txt
        从空闲的文件描述符表中找一个最小的，作为新的拷贝的文件描述符


*/

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int main() {

    int fd = open("a.txt", O_RDWR | O_CREAT, 0664);

    int fd1 = dup(fd);

    if(fd1 == -1) {
        perror("dup");
        return -1;
    }

    printf("fd : %d , fd1 : %d\n", fd, fd1);

    close(fd);

    char * str = "hello,world";
    int ret = write(fd1, str, strlen(str));
    if(ret == -1) {
        perror("write");
        return -1;
    }

    close(fd1);

    return 0;
}
```



### dup2

重定向文件描述符

作用：

两个文件a,b,两个文件描述符fd,fd1
原来fd指向a,fd1指向b
使用dup2之后可以把fd1指向a,以后fd1读写操作就是对a文件的操作

![image-20220808202609216](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220808202609216.png)

```C
/*
    #include <unistd.h>
    int dup2(int oldfd, int newfd);
        作用：重定向文件描述符
        oldfd 指向 a.txt, newfd 指向 b.txt
        调用函数成功后：newfd 和 b.txt 做close, newfd 指向了 a.txt
        oldfd 必须是一个有效的文件描述符
        若oldfd和newfd值相同，相当于什么都没有做
*/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main() {

    int fd = open("1.txt", O_RDWR | O_CREAT, 0664);
    if(fd == -1) {
        perror("open");
        return -1;
    }

    int fd1 = open("2.txt", O_RDWR | O_CREAT, 0664);
    if(fd1 == -1) {
        perror("open");
        return -1;
    }

    printf("fd : %d, fd1 : %d\n", fd, fd1);

    int fd2 = dup2(fd, fd1);
    if(fd2 == -1) {
        perror("dup2");
        return -1;
    }

    // 通过fd1去写数据，实际操作的是1.txt，而不是2.txt
    char * str = "hello, dup2";
    int len = write(fd1, str, strlen(str));

    if(len == -1) {
        perror("write");
        return -1;
    }

    printf("fd : %d, fd1 : %d, fd2 : %d\n", fd, fd1, fd2);

    close(fd);
    close(fd1);

    return 0;
}
```



### fcntl

作用：

- 复制文件描述符
- 设置/获取文件的状态

```C
/*

    #include <unistd.h>
    #include <fcntl.h>

    int fcntl(int fd, int cmd, ...);
    参数：
        fd : 表示需要操作的文件描述符
        cmd: 表示对文件描述符进行如何操作
            - F_DUPFD : 复制文件描述符,复制的是第一个参数fd，得到一个新的文件描述符（返回值）
                int ret = fcntl(fd, F_DUPFD);

            - F_GETFL : 获取指定的文件描述符文件状态flag
              获取的flag和我们通过open函数传递的flag是一个东西。

            - F_SETFL : 设置文件描述符文件状态flag
              必选项：O_RDONLY, O_WRONLY, O_RDWR 不可以被修改
              可选性：O_APPEND, O_NONBLOCK
                O_APPEND 表示追加数据
                NONBLOK 设置成非阻塞
        
        阻塞和非阻塞：描述的是函数调用的行为。
*/

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main() {

    // 1.复制文件描述符
    // int fd = open("1.txt", O_RDONLY);
    // int ret = fcntl(fd, F_DUPFD);

    // 2.修改或者获取文件状态flag
    int fd = open("1.txt", O_RDWR);
    if(fd == -1) {
        perror("open");
        return -1;
    }

    // 获取文件描述符状态flag
    int flag = fcntl(fd, F_GETFL);
    if(flag == -1) {
        perror("fcntl");
        return -1;
    }
    flag |= O_APPEND;   // flag = flag | O_APPEND

    // 修改文件描述符状态的flag，给flag加入O_APPEND这个标记
    int ret = fcntl(fd, F_SETFL, flag);
    if(ret == -1) {
        perror("fcntl");
        return -1;
    }

    char * str = "nihao";
    write(fd, str, strlen(str));

    close(fd);

    return 0;
}
```

### 阻塞和非阻塞

两者描述的是函数调用的行为。

- 阻塞： 阻塞调用是指调用结果返回之前，当前线程会被挂起。函数只有在得到结果之后才会返回（干不完不准回来）。
- 非阻塞：非阻塞和阻塞的概念相对应，指在不能立刻得到结果之前，该函数不会阻塞当前线程，而会立刻返回。
