# IO多路复用(IO多路转接)

## 阻塞等待(BIO模型)

### 简介

- 遇到`read`/`recv`/`accept`时，阻塞等待，直接有数据或者连接时才继续往下执行

### 单任务

- 好处：不占用CPU宝贵的时间片
- 缺点：同一时刻只能处理一个操作, 效率低
- 克服缺点：多线程或者多进程解决，一个线程/进程对应一个任务

![image-20211124122737594](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211124122737594.png)

### 多任务

- 优点：能够同时处理多个任务，一个线程/进程对应一个任务
- 缺点：
  - 线程或者进程会消耗资源
  - 线程或进程调度消耗CPU资源
- 根本问题：阻塞(`blocking`)

![image-20211124122820504](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211124122820504.png)

## 非阻塞，忙轮询(NIO模型)

- 优点：提高了程序的执行效率
- 缺点：需要占用更多的CPU和系统资源，每循环都需要 O(n) 系统调用（用来查找哪个任务可执行）
- 克服缺点：使用IO多路转接技术select/poll/epoll

![image-20211124123055701](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211124123055701.png)

## IO多路转接技术(简介)

### select/poll

- 委托内核进行操作
- 只会通知有几个任务可用，但不知道具体哪几个任务，还需遍历（与NIO模型略有不同）

![image-20211124125216066](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211124125216066.png)

### epoll

- 委托内核进行操作
- 会通知具体有哪几个任务可用

![image-20211124125254136](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211124125254136.png)

## select

### 主旨思想

1. 首先要构造一个关于文件描述符的列表，将要监听的文件描述符添加到该列表中
2. 调用一个系统函数(`select`)，监听该列表中的文件描述符，直到这些描述符中的一个或者多个进行I/O操作时，该函数才返回
   - 这个函数是阻塞
   - 函数对文件描述符的检测的操作是由内核完成的
3. 在返回时，它会告诉进程有多少（哪些）描述符要进行I/O操作

### 函数说明

- 概览

  ```c++
  #include <sys/time.h> 
  #include <sys/types.h> 
  #include <unistd.h> 
  #include <sys/select.h> 
  
  int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
  
  // 将参数文件描述符fd对应的标志位设置为0 
  void FD_CLR(int fd, fd_set *set); 
  // 判断fd对应的标志位是0还是1， 返回值 ： fd对应的标志位的值，0，返回0， 1，返回1 
  int FD_ISSET(int fd, fd_set *set); 
  // 将参数文件描述符fd 对应的标志位，设置为1 
  void FD_SET(int fd, fd_set *set);
  // fd_set一共有1024 bit, 全部初始化为0 
  void FD_ZERO(fd_set *set);
  ```

- `int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout); `

  - 通过`man select`查看帮助
  - 参数
    - `nfds`：委托内核检测的最大文件描述符的值 + 1（+1是因为遍历是下标从0开始，for循环＜设定）
    - `readfds`：要检测的文件描述符的读的集合，委托内核检测哪些文件描述符的读的属性 
      - 一般检测读操作 
      - 对应的是对方发送过来的数据，因为读是被动的接收数据，检测的就是读缓冲区
      - 是一个传入传出参数
    - `writefds`：要检测的文件描述符的写的集合，委托内核检测哪些文件描述符的写的属性 
      - 委托内核检测写缓冲区是不是还可以写数据（不满的就可以写）
    - `exceptfds`：检测发生异常的文件描述符的集合，一般不用
    - `timeout`：设置的超时时间，含义见**`select`参数列表说明**
      - `NULL`：永久阻塞，直到检测到了文件描述符有变化 
      - `tv_sec = tv_usec = 0`， 不阻塞
      - ` tv_sec > 0,tv_usec > 0`：阻塞对应的时间 

  - 返回值
    - -1：失败
    - \>0(n)：检测的集合中有n个文件描述符发生了变化  

- `select`参数列表说明

  - `fd_set`：是一块固定大小的缓冲区(结构体)，`sizeof(fd_set)=128`，即对应1024个比特位

  - `timeval `：结构体类型

    ```c++
    struct timeval { 
        long tv_sec; /* seconds */ 
        long tv_usec; /* microseconds */ 
    };
    ```

### 工作过程分析

1. 初始设定

   ![image-20211124232543733](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211124232543733.png)

2. 设置监听文件描述符，将`fd_set`集合相应位置为1

   ![image-20211124232723210](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211124232723210.png)

3. 调用`select`委托内核检测

   ![image-20211124233009618](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211124233009618.png)

4. 内核检测完毕后，返回给用户态结果

   ![image-20211124233108458](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211124233108458.png)

### 代码实现

#### 注意事项

- `select`中需要的监听集合需要两个
  - 一个是用户态真正需要监听的集合`rSet`
  - 一个是内核态返回给用户态的修改集合`tmpSet`
- 需要先判断监听文件描述符是否发生改变
  - 如果改变了，说明有客户端连接，此时需要将**新的连接文件描述符加入到`rSet`**，并更新最大文件描述符
  - 如果没有改变，说明没有客户端连接
- 由于`select`无法确切知道哪些文件描述符发生了改变，所以需要执行遍历操作，使用`FD_ISSET`判断是否发生了改变
- 如果客户端断开了连接，需要从`rSet`中清除需要监听的文件描述符
- <font color='red'>程序存在的问题：中间的一些断开连接后，最大文件描述符怎么更新？</font>=>==估计不更新，每次都会遍历到之前的最大值处==，解决方案见[高并发优化思考](###高并发优化思考)

#### 服务端

```c++
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>

#define SERVERIP "127.0.0.1"
#define PORT 6789


int main()
{
    // 1. 创建socket（用于监听的套接字）
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        exit(-1);
    }
    // 2. 绑定
    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    // 点分十进制转换为网络字节序
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    // 服务端也可以绑定0.0.0.0即任意地址
    // server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    int ret = bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("bind");
        exit(-1);
    }
    // 3. 监听
    ret = listen(listenfd, 8);
        if (ret == -1) {
        perror("listen");
        exit(-1);
    }
    // 创建读检测集合
    // rSet用于记录正在的监听集合，tmpSet用于记录在轮训过程中由内核态返回到用户态的集合
    fd_set rSet, tmpSet;
    // 清空
    FD_ZERO(&rSet);
    // 将监听文件描述符加入
    FD_SET(listenfd, &rSet);
    // 此时最大的文件描述符为监听描述符
    int maxfd = listenfd;
    // 不断循环等待客户端连接
    while (1) {
        tmpSet = rSet;
        // 使用select，设置为永久阻塞，有文件描述符变化才返回
        int num = select(maxfd + 1, &tmpSet, NULL, NULL, NULL);
        if (num == -1) {
            perror("select");
            exit(-1);
        } else if (num == 0) {
            // 当前无文件描述符有变化，执行下一次遍历
            // 在本次设置中无效（因为select被设置为永久阻塞）
            continue;
        } else {
            // 首先判断监听文件描述符是否发生改变（即是否有客户端连接）
            if (FD_ISSET(listenfd, &tmpSet)) {
                // 4. 接收客户端连接
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
                if (connfd == -1) {
                    perror("accept");
                    exit(-1);
                }
                // 输出客户端信息，IP组成至少16个字符（包含结束符）
                char client_ip[16] = {0};
                inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
                unsigned short client_port = ntohs(client_addr.sin_port);
                printf("ip:%s, port:%d\n", client_ip, client_port);

                FD_SET(connfd, &rSet);
                // 更新最大文件符
                maxfd = maxfd > connfd ? maxfd : connfd;
            }

            // 遍历集合判断是否有变动，如果有变动，那么通信
            char recv_buf[1024] = {0};
            for (int i = listenfd + 1; i <= maxfd; i++) {
                if (FD_ISSET(i, &tmpSet)) {
                    ret = read(i, recv_buf, sizeof(recv_buf));
                    if (ret == -1) {
                        perror("read");
                        exit(-1);
                    } else if (ret > 0) {
                        printf("recv server data : %s\n", recv_buf);
                        write(i, recv_buf, strlen(recv_buf));
                    } else {
                        // 表示客户端断开连接
                        printf("client closed...\n");
                        close(i);
                        FD_CLR(i, &rSet);
                        break;
                    }
                }
            }
        }
    }

    close(listenfd);
    return 0;
}
```

#### 客户端

```c++
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SERVERIP "127.0.0.1"
#define PORT 6789

int main()
{
    // 1. 创建socket（用于通信的套接字）
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connfd == -1) {
        perror("socket");
        exit(-1);
    }
    // 2. 连接服务器端
    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    server_addr.sin_port = htons(PORT);
    int ret = connect(connfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("connect");
        exit(-1);
    }
    // 3. 通信
    char recv_buf[1024] = {0};
    while (1) {
        // 发送数据
        char *send_buf = "client message";
        write(connfd, send_buf, strlen(send_buf));
        // 接收数据
        ret = read(connfd, recv_buf, sizeof(recv_buf));
        if (ret == -1) {
            perror("read");
            exit(-1);
        } else if (ret > 0) {
            printf("recv server data : %s\n", recv_buf);
        } else {
            // 表示客户端断开连接
            printf("client closed...\n");
        }
        // 休眠的目的是为了更好的观察，放在此处可以解决read: Connection reset by peer问题
        sleep(1);
    }
    // 关闭连接
    close(connfd);
    return 0;
}
```

### 高并发优化思考

#### 问题

- 每次都需要利用`FD_ISSET`轮训`[0, maxfd]`之间的连接状态，<font color='red'>如果位于中间的某一个客户端断开了连接</font>，此时不应该再去利用`FD_ISSET`轮训，造成资源浪费
- 如果在处理客户端数据时，某一次read没有对数据读完，那么造成重新进行下一次时select，获取上一次未处理完的文件描述符，从0开始遍历到maxfd，对上一次的进行再一次操作，效率十分低下

#### 解决

- 考虑到`select`只有`1024`个最大可监听数量，可以<font color='red'>申请等量客户端数组</font>
  - 初始置为-1，当有状态改变时，置为相应文件描述符
  - 此时再用`FD_ISSET`轮训时，跳过标记为-1的客户端，加快遍历速度
- 对于问题二：对读缓存区循环读，直到返回`EAGAIN`再处理数据

#### 参考

- [多路复用IO模型之select与并发问题进一步优化](https://blog.csdn.net/weixin_42889383/article/details/102367621)

### 存在问题(缺点)

- 每次调用select，都需要把fd集合从用户态拷贝到内核态，这个开销在fd很多时会很大
- 同时每次调用select都需要在内核遍历传递进来的所有fd，这个开销在fd很多时也很大
- select支持的文件描述符数量太小了，默认是1024
- fds集合不能重用，每次都需要重置

![image-20211126224641170](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211126224641170.png)

## poll

### 主旨思想

- 用一个结构体记录文件描述符集合，并记录用户态状态和内核态状态

### 函数说明

- 概览

  ```c++
  #include <poll.h> 
  struct pollfd { 
      int fd; /* 委托内核检测的文件描述符 */ 
      short events; /* 委托内核检测文件描述符的什么事件 */ 
      short revents; /* 文件描述符实际发生的事件 */ 
  };
  
  int poll(struct pollfd *fds, nfds_t nfds, int timeout);
  ```

- `int poll(struct pollfd *fds, nfds_t nfds, int timeout); `

  - 通过`man poll`查看帮助
  - 参数
    - `fds`：是一个`struct pollfd` 结构体数组，这是一个需要检测的文件描述符的集合
    - `nfds`：这个是第一个参数数组中最后一个有效元素的下标 + 1 
    - `timeout`：阻塞时长 
      - 0：不阻塞
      - -1：阻塞，当检测到需要检测的文件描述符有变化，解除阻塞
      - \>0：具体的阻塞时长(ms)
  - 返回值
    - -1：失败
    - \>0(n)：检测的集合中有n个文件描述符发生了变化

- `events`及`revents`取值，如果有多个事件需要检测，用`|`即可，如同时检测读和写：`POLLIN | POLLOUT`

  ![image-20211126233707281](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211126233707281.png)

### 代码实现

#### 注意事项

- `nfds`表示的监听文件描述符的下标，所以在遍历时，需要使用`fds[i].fd`取得相应的文件描述符
- <font color='red'>如何优雅的更新nfds?代码中使用连接的文件描述符作为替代更新</font>

#### 服务端

```c++
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

#define SERVERIP "127.0.0.1"
#define PORT 6789


int main()
{
    // 1. 创建socket（用于监听的套接字）
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        exit(-1);
    }
    // 2. 绑定
    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    // 点分十进制转换为网络字节序
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    // 服务端也可以绑定0.0.0.0即任意地址
    // server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    int ret = bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("bind");
        exit(-1);
    }
    // 3. 监听
    ret = listen(listenfd, 8);
        if (ret == -1) {
        perror("listen");
        exit(-1);
    }
    
    struct pollfd fds[1024];
    // 初始化
    for (int i = 0; i < 1024; i++) {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }
    // 将监听文件描述符加入
    fds[0].fd = listenfd;
    int nfds = 0;
    // 不断循环等待客户端连接
    while (1) {
        // 使用poll，设置为永久阻塞，有文件描述符变化才返回
        int num = poll(fds, nfds + 1, -1);
        if (num == -1) {
            perror("poll");
            exit(-1);
        } else if (num == 0) {
            // 当前无文件描述符有变化，执行下一次遍历
            // 在本次设置中无效（因为select被设置为永久阻塞）
            continue;
        } else {
            // 首先判断监听文件描述符是否发生改变（即是否有客户端连接）
            if (fds[0].revents & POLLIN) {
                // 4. 接收客户端连接
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
                if (connfd == -1) {
                    perror("accept");
                    exit(-1);
                }
                // 输出客户端信息，IP组成至少16个字符（包含结束符）
                char client_ip[16] = {0};
                inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
                unsigned short client_port = ntohs(client_addr.sin_port);
                printf("ip:%s, port:%d\n", client_ip, client_port);
                // 遍历集合, 将新的需要监听的文件描述符加入集合
                for (int i = 1; i < 1024; i++) {
                    if (fds[i].fd == -1) {
                        fds[i].fd = connfd;
                        fds[i].events = POLLIN;
                        break;
                    }
                }
                // 更新最大的监听文件描述符集合下标
                // 存在问题：使用文件描述符替代最大对应下标
                nfds = nfds > connfd ? nfds : connfd;
            }

            // 遍历集合判断是否有变动，如果有变动，那么通信
            char recv_buf[1024] = {0};
            for (int i = 1; i <= nfds; i++) {
                if (fds[i].fd != -1 && fds[i].revents & POLLIN) {
                    ret = read(fds[i].fd, recv_buf, sizeof(recv_buf));
                    if (ret == -1) {
                        perror("read");
                        exit(-1);
                    } else if (ret > 0) {
                        printf("recv server data : %s\n", recv_buf);
                        write(fds[i].fd, recv_buf, strlen(recv_buf));
                    } else {
                        // 表示客户端断开连接
                        printf("client closed...\n");
                        close(fds[i].fd);
                        fds[i].fd = -1;
                        break;
                    }
                }
            }
        }
    }

    close(listenfd);
    return 0;
}
```

#### 客户端

```c++
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SERVERIP "127.0.0.1"
#define PORT 6789

int main()
{
    // 1. 创建socket（用于通信的套接字）
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connfd == -1) {
        perror("socket");
        exit(-1);
    }
    // 2. 连接服务器端
    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    server_addr.sin_port = htons(PORT);
    int ret = connect(connfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("connect");
        exit(-1);
    }
    // 3. 通信
    char recv_buf[1024] = {0};
    while (1) {
        // 发送数据
        char *send_buf = "client message";
        write(connfd, send_buf, strlen(send_buf));
        // 接收数据
        ret = read(connfd, recv_buf, sizeof(recv_buf));
        if (ret == -1) {
            perror("read");
            exit(-1);
        } else if (ret > 0) {
            printf("recv server data : %s\n", recv_buf);
        } else {
            // 表示客户端断开连接
            printf("client closed...\n");
        }
        // 休眠的目的是为了更好的观察，放在此处可以解决read: Connection reset by peer问题
        sleep(1);
    }
    // 关闭连接
    close(connfd);
    return 0;
}
```

### 存在问题(缺点)

- 缺点同`select`第一点和第二点(如下)，即解决了第三点和第四点
- 每次调用select，都需要把fd集合从用户态拷贝到内核态，这个开销在fd很多时会很大
- 同时每次调用select都需要在内核遍历传递进来的所有fd，这个开销在fd很多时也很大

## epoll

### 主旨思想

- 直接在**内核态**创建` eventpoll实例`(结构体)，通过`epoll`提供的API操作该实例
- 结构体中有`红黑树`和`双链表`，分别用来**存储需要检测的文件描述符**和**存储已经发生改变的文件描述符**

![image-20211127170241104](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211127170241104.png)

### 函数说明

- 概览

  ```c
  #include <sys/epoll.h>
  
  // 创建一个新的epoll实例
  // 在内核中创建了一个数据，这个数据中有两个比较重要的数据，一个是需要检测的文件描述符的信息（红黑树），还有一个是就绪列表，存放检测到数据发送改变的文件描述符信息（双向链表）
  int epoll_create(int size);
  
  // 对epoll实例进行管理：添加文件描述符信息，删除信息，修改信息
  int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
  struct epoll_event { 
      uint32_t events; /* Epoll events */ 
      epoll_data_t data; /* User data variable */ 
  };
  typedef union epoll_data { 
      void *ptr; 
      int fd; 
      uint32_t u32; 
      uint64_t u64; 
  } epoll_data_t;
  
  // 检测函数
  int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
  ```

- `int epoll_create(int size);`

  - 功能：创建一个新的epoll实例
  - 参数：`size`，目前没有意义了(之前底层实现是哈希表，现在是红黑树)，随便写一个数，必须大于0
  - 返回值
    - -1：失败
    - \>0：操作`epoll实例`的文件描述符

- `int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);`

  - 功能：对epoll实例进行管理：添加文件描述符信息，删除信息，修改信息
  - 参数：
    - `epfd`：epoll实例对应的文件描述符
    - `op`：要进行什么操作
      - 添加：`EPOLL_CTL_ADD`
      - 删除：`EPOLL_CTL_DEL`
      - 修改：`EPOLL_CTL_MOD`
    - `fd`：要检测的文件描述符 
    - `event`：检测文件描述符什么事情，通过设置`epoll_event.events`，常见操作
      - 读事件：`EPOLLIN`
      - 写事件：`EPOLLOUT `
      - 错误事件：`EPOLLERR`
      - 设置边沿触发：`EPOLLET`（默认水平触发）
  - 返回值：成功0，失败-1

- `int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);`

  - 功能：检测哪些文件描述符发生了改变
  - 参数：
    - `epfd`：epoll实例对应的文件描述符
    - `events`：传出参数，保存了发生了变化的文件描述符的信息
    - `maxevents`：第二个参数结构体数组的大小 
    - `timeout`：阻塞时长 
      - 0：不阻塞
      - -1：阻塞，当检测到需要检测的文件描述符有变化，解除阻塞
      - \>0：具体的阻塞时长(ms)
  - 返回值：
    -  \> 0：成功，返回发送变化的文件描述符的个数
    -  -1：失败

### 代码实现

#### 注意事项

- `events`是封装了监听描述符信息的结构体，每一个新增文件都需要这个(可重用)

- 需要注意可能同时发生了多个监听（如监听读事件和写事件），那么代码逻辑需要做相应判断

  > 如本例中只检测读事件，排除了写事件

#### 服务端

```c++
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>

#define SERVERIP "127.0.0.1"
#define PORT 6789


int main()
{
    // 1. 创建socket（用于监听的套接字）
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        exit(-1);
    }
    // 2. 绑定
    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    // 点分十进制转换为网络字节序
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    // 服务端也可以绑定0.0.0.0即任意地址
    // server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    int ret = bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("bind");
        exit(-1);
    }
    // 3. 监听
    ret = listen(listenfd, 8);
        if (ret == -1) {
        perror("listen");
        exit(-1);
    }
    
    // 创建epoll实例
    int epfd = epoll_create(100);
    // 将监听文件描述符加入实例
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = listenfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);
    if (ret == -1) {
        perror("epoll_ctl");
        exit(-1);
    }
    // 此结构体用来保存内核态返回给用户态发生改变的文件描述符信息
    struct epoll_event events[1024];
    // 不断循环等待客户端连接
    while (1) {
        // 使用epoll，设置为永久阻塞，有文件描述符变化才返回
        int num = epoll_wait(epfd, events, 1024, -1);
        if (num == -1) {
            perror("poll");
            exit(-1);
        } else if (num == 0) {
            // 当前无文件描述符有变化，执行下一次遍历
            // 在本次设置中无效（因为select被设置为永久阻塞）
            continue;
        } else {
            // 遍历发生改变的文件描述符集合
            for (int i = 0; i < num; i++) {
                // 判断监听文件描述符是否发生改变（即是否有客户端连接）
                int curfd = events[i].data.fd;
                if (curfd == listenfd) {
                    // 4. 接收客户端连接
                    struct sockaddr_in client_addr;
                    socklen_t client_addr_len = sizeof(client_addr);
                    int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
                    if (connfd == -1) {
                        perror("accept");
                        exit(-1);
                    }
                    // 输出客户端信息，IP组成至少16个字符（包含结束符）
                    char client_ip[16] = {0};
                    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
                    unsigned short client_port = ntohs(client_addr.sin_port);
                    printf("ip:%s, port:%d\n", client_ip, client_port);
                    // 将信息加入监听集合
                    event.events = EPOLLIN;
                    event.data.fd = connfd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event);
                } else {
                    // 只检测读事件
                    if (events[i].events & EPOLLOUT) {
                        continue;
                    }
                    // 接收消息
                    char recv_buf[1024] = {0};
                    ret = read(curfd, recv_buf, sizeof(recv_buf));
                    if (ret == -1) {
                        perror("read");
                        exit(-1);
                    } else if (ret > 0) {
                        printf("recv server data : %s\n", recv_buf);
                        write(curfd, recv_buf, strlen(recv_buf));
                    } else {
                        // 表示客户端断开连接
                        printf("client closed...\n");
                        close(curfd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, NULL);
                        break;
                    }
                }
            }
        }
    }

    close(listenfd);
    close(epfd);
    return 0;
}
```

#### 客户端

```c++
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SERVERIP "127.0.0.1"
#define PORT 6789

int main()
{
    // 1. 创建socket（用于通信的套接字）
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connfd == -1) {
        perror("socket");
        exit(-1);
    }
    // 2. 连接服务器端
    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    server_addr.sin_port = htons(PORT);
    int ret = connect(connfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("connect");
        exit(-1);
    }
    // 3. 通信
    char recv_buf[1024] = {0};
    while (1) {
        // 发送数据
        char *send_buf = "client message";
        write(connfd, send_buf, strlen(send_buf));
        // 接收数据
        ret = read(connfd, recv_buf, sizeof(recv_buf));
        if (ret == -1) {
            perror("read");
            exit(-1);
        } else if (ret > 0) {
            printf("recv server data : %s\n", recv_buf);
        } else {
            // 表示客户端断开连接
            printf("client closed...\n");
        }
        // 休眠的目的是为了更好的观察，放在此处可以解决read: Connection reset by peer问题
        sleep(1);
    }
    // 关闭连接
    close(connfd);
    return 0;
}
```

### 工作模式(LT与ET)

#### 水平触发(level triggered, LT)

- epoll的缺省的工作方式，并且同时支持 block 和 non-block socket
- 在这种做法中，内核告诉你一个文件描述符是否就绪了，然后你可以对这个就绪的 fd 进行 IO 操作。如果你不作任何操作，内核还是会继续通知你的

#### 边沿触发(edge triggered, ET)

- 是高速工作方式，只支持 non-block socket，需要对监听文件描述符设置才能实现
- 在这种模式下，当描述符从未就绪变为就绪时，内核通过epoll告诉你。然后它会假设你知道文件描述符已经就绪，并且不会再为那个文件描述符发送更多的就绪通知，直到你做了某些操作导致那个文件描述符不再为就绪状态了。但是请注意，如果一直不对这个 fd 作 IO 操作（从而导致它再次变成未就绪），内核不会发送更多的通知（only once）

#### 区别与说明

- ET 模式在很大程度上减少了 epoll 事件被重复触发的次数，因此效率要比 LT 模式高
- epoll工作在 ET 模式的时候，必须使用非阻塞套接口，以避免由于一个文件句柄的阻塞读/阻塞写操作把处理多个文件描述符的任务饿死
- 所以如果使用ET且缓冲区内容不能一次性读完，**需要写一个循环将内容全部读取，且需要将套接字设置为非阻塞**

- 说明：假设委托内核检测读事件，即检测fd的读缓冲区，那么如果读缓冲区有数据 ，epoll检测到了会给用户通知
  - LT
    - 用户不读数据，数据一直在缓冲区，epoll 会一直通知
    - 用户只读了一部分数据，epoll会通知
    - 缓冲区的数据读完了，不通知
  - ET
    - 用户不读数据，数据一致在缓冲区中，epoll下次检测的时候就不通知了
    - 用户只读了一部分数据，epoll不通知
    - 缓冲区的数据读完了，不通知

#### 代码(ET)

##### 服务端

```c++
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define SERVERIP "127.0.0.1"
#define PORT 6789


int main()
{
    // 1. 创建socket（用于监听的套接字）
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        exit(-1);
    }
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    // 2. 绑定
    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    // 点分十进制转换为网络字节序
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    // 服务端也可以绑定0.0.0.0即任意地址
    // server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    int ret = bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("bind");
        exit(-1);
    }
    // 3. 监听
    ret = listen(listenfd, 8);
        if (ret == -1) {
        perror("listen");
        exit(-1);
    }
    
    // 创建epoll实例
    int epfd = epoll_create(100);
    // 将监听文件描述符加入实例
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = listenfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);
    if (ret == -1) {
        perror("epoll_ctl");
        exit(-1);
    }
    // 此结构体用来保存内核态返回给用户态发生改变的文件描述符信息
    struct epoll_event events[1024];
    // 不断循环等待客户端连接
    while (1) {
        // 使用epoll，设置为永久阻塞，有文件描述符变化才返回
        int num = epoll_wait(epfd, events, 1024, -1);
        // 方便观察epoll通知了几次
        printf("num = %d\n", num);
        if (num == -1) {
            perror("poll");
            exit(-1);
        } else if (num == 0) {
            // 当前无文件描述符有变化，执行下一次遍历
            // 在本次设置中无效（因为select被设置为永久阻塞）
            continue;
        } else {
            // 遍历发生改变的文件描述符集合
            for (int i = 0; i < num; i++) {
                // 判断监听文件描述符是否发生改变（即是否有客户端连接）
                int curfd = events[i].data.fd;
                if (curfd == listenfd) {
                    // 4. 接收客户端连接
                    struct sockaddr_in client_addr;
                    socklen_t client_addr_len = sizeof(client_addr);
                    int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
                    if (connfd == -1) {
                        perror("accept");
                        exit(-1);
                    }
                    // 输出客户端信息，IP组成至少16个字符（包含结束符）
                    char client_ip[16] = {0};
                    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
                    unsigned short client_port = ntohs(client_addr.sin_port);
                    printf("ip:%s, port:%d\n", client_ip, client_port);
                    // 将通信套接字设置为非阻塞
                    int flag = fcntl(connfd, F_GETFL);
                    flag |= O_NONBLOCK;
                    fcntl(connfd, F_SETFL, flag);

                    // 将信息加入监听集合，设置为ET模式
                    event.events = EPOLLIN | EPOLLET;
                    event.data.fd = connfd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event);
                } else {
                    // 只检测读事件
                    if (events[i].events & EPOLLOUT) {
                        continue;
                    }
                    // 接收消息，将缓冲区减少，这样能更好说明一次性无法读取数据时，epoll的操作
                    // 需要循环读取数据
                    char recv_buf[5] = {0};
                    while ((ret = read(curfd, recv_buf, sizeof(recv_buf))) > 0) {
                        // 应该是打印的时候最后没有结束符
                        char test_buf[6] = {0};
                        strcpy(test_buf, recv_buf);
                        printf("recv server data : %s\n", test_buf);
                        // write(STDOUT_FILENO, recv_buf, ret);
                        // write(curfd, recv_buf, strlen(recv_buf));
                        write(curfd, recv_buf, sizeof(recv_buf));
                        memset(recv_buf, 0, sizeof(recv_buf));
                    }
                    if (ret == -1) {
                        if(errno == EAGAIN) {
                            printf("data over...\n");
                        }else {
                            perror("read");
                            exit(-1);
                        }
                    } else {
                        // 表示客户端断开连接
                        printf("client closed...\n");
                        close(curfd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, NULL);
                        break;
                    }
                }
            }
        }
    }

    close(listenfd);
    close(epfd);
    return 0;
}
```

##### 客户端

```c++
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SERVERIP "127.0.0.1"
#define PORT 6789

int main()
{
    // 1. 创建socket（用于通信的套接字）
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connfd == -1) {
        perror("socket");
        exit(-1);
    }
    // 2. 连接服务器端
    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    server_addr.sin_port = htons(PORT);
    int ret = connect(connfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("connect");
        exit(-1);
    }
    // 3. 通信
    char recv_buf[1024] = {0};
    while (1) {
        // 发送数据，修改为从键盘获取内容
        fgets(recv_buf, sizeof(recv_buf), stdin);
        write(connfd, recv_buf, strlen(recv_buf));
        // 因为用的时同一个数组，不清空就会有残留数据
        memset(recv_buf, 0, sizeof(recv_buf));
        // 接收数据
        ret = read(connfd, recv_buf, sizeof(recv_buf));
        if (ret == -1) {
            perror("read");
            exit(-1);
        } else if (ret > 0) {
            printf("recv server data : %s\n", recv_buf);
        } else {
            // 表示客户端断开连接
            printf("client closed...\n");
        }
        // 休眠的目的是为了更好的观察，放在此处可以解决read: Connection reset by peer问题
        sleep(1);
    }
    // 关闭连接
    close(connfd);
    return 0;
}
```

