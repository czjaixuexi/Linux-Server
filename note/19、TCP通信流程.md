<<<<<<< HEAD
# TCP通信流程

- TCP 和 UDP -> 传输层的协议 
- UDP:用户数据报协议，面向无连接，可以单播，多播，广播， 面向数据报，不可靠 
- TCP:传输控制协议，面向连接的，可靠的，基于字节流，仅支持单播传输

|                | UDP                            | TCP                        |
| -------------- | :----------------------------- | :------------------------- |
| 是否创建连接   | 无连接                         | 面向连接                   |
| 是否可靠       | 不可靠                         | 可靠的                     |
| 连接的对象个数 | 一对一、一对多、多对一、多对多 | 支持一对一                 |
| 传输的方式     | 面向数据报                     | 面向字节流                 |
| 首部开销       | 8个字节                        | 最少20个字节               |
| 适用场景       | 实时应用（视频会议，直播）     | 可靠性高的应用（文件传输） |



<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220916151323692.png" alt="image-20220916151323692" style="zoom:50%;" />   

## **TCP 通信的流程** 

**服务器端 （被动接受连接的角色）** 

1、创建一个用于监听的套接字 

\- 监听：监听有客户端的连接 

\- 套接字：这个套接字其实就是一个文件描述符 

2、将这个监听文件描述符和本地的IP和端口绑定（IP和端口就是服务器的地址信息） 

\- 客户端连接服务器的时候使用的就是这个IP和端口 

3、设置监听，监听的fd开始工作 

4、阻塞等待，当有客户端发起连接，解除阻塞，接受客户端的连接，会得到一个和客户端通信的套接字（fd） 

5、通信 

\- 接收数据 

\- 发送数据 

6、通信结束，断开连接

**客户端** 

1、创建一个用于通信的套接字（fd） 

2、连接服务器，需要指定连接的服务器的 IP 和 端口 

3、连接成功了，客户端可以直接和服务器通信 

\- 接收数据 

\- 发送数据 

4、通信结束，断开连接 



### TCP通信服务器端

#### 1. 创建一个用于监听的socket

使用`socket`函数创建用于监听的套接字

```C
int socket(int domain, int type, int protocol)
	-domain:协议族
		AF_INET:IPV4
		AF_INET6:IPV6
		AF_UNIX,AF_LOCAL:本地套接字用于进程间通信
	-type：使用的协议类型
		SOCK_STREAM:流式协议
		SOCK_DGRAM:报式协议
	-protocol:具体协议，一般写0
		SOCK_STREAM:流式协议默认tcp
		SOCK_DGRAM:报式协议默认udp
	-返回值：
		成功返回文件描述符
		失败返回-1
```

```C
// 1.创建socket(用于监听的套接字)
    int lfd = socket(AF_INET, SOCK_STREAM, 0);

    if(lfd == -1) {
        perror("socket");
        exit(-1);
    }
```



#### 2. 将这个监听文件描述符和本地的IP和端口绑定

绑定之前需要先创建和初始化一个专用socket地址`struct sockaddr_in`存放本地IP和端口

**sockaddr_in结构体**

```C
#include <netinet/in.h>
struct sockaddr_in
{
    sa_family_t sin_family;     /* __SOCKADDR_COMMON(sin_) */
    in_port_t sin_port;         /* Port number.  */
    struct in_addr sin_addr;    /* Internet address.  */
    /* Pad to size of `struct sockaddr'. */
    unsigned char sin_zero[sizeof (struct sockaddr) - __SOCKADDR_COMMON_SIZE -
               sizeof (in_port_t) - sizeof (struct in_addr)];
};  
struct in_addr
{
    in_addr_t s_addr;
};
struct sockaddr_in6
{
    sa_family_t sin6_family;
    in_port_t sin6_port;    /* Transport layer port # */
    uint32_t sin6_flowinfo; /* IPv6 flow information */
    struct in6_addr sin6_addr;  /* IPv6 address */
    uint32_t sin6_scope_id; /* IPv6 scope-id */
 };
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
#define __SOCKADDR_COMMON_SIZE (sizeof (unsigned short int)
```

**创建和初始化**

```C
struct sockaddr_in saddr;
saddr.sin_family = AF_INET;
// inet_pton(AF_INET, "192.168.193.128", saddr.sin_addr.s_addr);
saddr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0
saddr.sin_port = htons(9999);
```

我们一般使用的IP地址表示方法为点分十进制表示法(一个字符串），而`sockaddr_in`中存储的IP地址为一般为整形的网络字节序形式，因此我们可以使用`inet_pton`函数来实现点分十进制IP地址字符床与网络字节序整数的转换。

当让我们可以直接给IP地址复制`INADDR_ANY`或者`0.0.0.0`二者的值是相同的，其意义是让服务器端计算机上的所有网卡的IP地址都可以作为服务器IP地址，也即监听外部客户端程序发送到服务器端所有网卡的网络请求

**绑定**
我们使用`bind`函数来实现绑定

```C
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen); // socket命
名
    - 功能：绑定，将fd 和本地的IP + 端口进行绑定
    - 参数：
            - sockfd : 通过socket函数得到的文件描述符
            - addr : 需要绑定的socket地址，这个地址封装了ip和端口号的信息
            - addrlen : 第二个参数结构体占的内存大小
```

```C
 // 2.绑定
    int ret = bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));

    if(ret == -1) {
        perror("bind");
        exit(-1);
    }
```

tip:注意对&saddr进行强制类型转换`(struct sockaddr *)&saddr`

早期网络编程函数使用通用socket地址`struct sockaddr`结构体，为了向前兼容，现在`sockaddr`退化成了`void *`，也就是说这片内存区域的使用和解释方法取决于谁去用和怎么用

专用socket地址类型变量在实际使用时都需要转化为通用socket地址类型socketaddr，因此在具体使用这些接口的时候必须指定具体的指针类型

#### 3.设置监听，监听的fd开始工作

我们使用`listen`函数来进行监听，`listen`创建一个监听队列来接收客户端的连接，backlog参数就是设置监听队列的长度一般为5,如果设置为`SOMAXCONN`那么就由系统来决定，一般是比较大的。
另外，如果监听队列满了那么客户端会收到 `ECONNREFUSED` 错误

```C
int listen(int sockfd, int backlog);    // /proc/sys/net/core/somaxconn
    - 功能：监听这个socket上的连接
    - 参数：
        - sockfd : 通过socket()函数得到的文件描述符
        - backlog : 未连接的和已经连接的和的最大值， 5
```

```C
 // 3.监听
    ret = listen(lfd, 8);
    if(ret == -1) {
        perror("listen");
        exit(-1);
    }
```

#### 4.阻塞等待客户端发起链接

我们使用`accept`函数接收客户端发起的连接，并记录下来
因为需要记录因此，我们还需要一个`struct socketaddr`来存储客户端的信息

```C
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    - 功能：接收客户端连接，默认是一个阻塞的函数，阻塞等待客户端连接 
    - 参数：
            - sockfd : 用于监听的文件描述符
            - addr : 传出参数，记录了连接成功后客户端的地址信息（ip，port）
            - addrlen : 指定第二个参数的对应的内存大小
    - 返回值：
            - 成功 ：用于通信的文件描述符 
            - -1 ： 失败		
```

```C
 struct sockaddr_in clientaddr;
 int len = sizeof(clientaddr);
 int cfd = accept(lfd, (struct sockaddr *)&clientaddr, &len);
 
 if(cfd == -1) {
     perror("accept");
     exit(-1);
 }
```

如果我们要查看客户端信息，那么最好使用`inet_ntop`将存储的网络字节序转化为点分十进制表示法，用`ntohs`将端口从网络字节序转化为主机字节序

```C
    char clientIP[16];
    inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, clientIP, sizeof(clientIP));
    unsigned short clientPort = ntohs(clientaddr.sin_port);
    printf("client ip is %s, port is %d\n", clientIP, clientPort);
```

#### 5.通信收发数据

还是那句话，Linux系统中万物皆是文件，socket也不例外。
在文件描述符后就可以使用`read`和`write`直接进行读写操作了

```C
// 5.通信
    char recvBuf[1024] = {0};
    while(1) {
        
        // 获取客户端的数据
        int num = read(cfd, recvBuf, sizeof(recvBuf));
        if(num == -1) {
            perror("read");
            exit(-1);
        } else if(num > 0) {
            printf("recv client data : %s\n", recvBuf);
        } else if(num == 0) {
            // 表示客户端断开连接
            printf("clinet closed...");
            break;
        }

        char * data = "hello,i am server";
        // 给客户端发送数据
        write(cfd, data, strlen(data));
    }
```

#### 6.通信结束断开链接

当通信结束断开连接的时候，我们需要关闭监听的文件描述符和客户端的文件描述符

```C
    // 关闭文件描述符
    close(cfd);
    close(lfd);
```

#### 最终代码

```C
// TCP 通信的服务器端

#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {

    // 1.创建socket(用于监听的套接字)
    int lfd = socket(AF_INET, SOCK_STREAM, 0);

    if(lfd == -1) {
        perror("socket");
        exit(-1);
    }

    // 2.绑定
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    // inet_pton(AF_INET, "192.168.193.128", saddr.sin_addr.s_addr);
    saddr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0
    saddr.sin_port = htons(9999);
    int ret = bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));

    if(ret == -1) {
        perror("bind");
        exit(-1);
    }

    // 3.监听
    ret = listen(lfd, 8);
    if(ret == -1) {
        perror("listen");
        exit(-1);
    }

    // 4.接收客户端连接
    struct sockaddr_in clientaddr;
    int len = sizeof(clientaddr);
    int cfd = accept(lfd, (struct sockaddr *)&clientaddr, &len);
    
    if(cfd == -1) {
        perror("accept");
        exit(-1);
    }

    // 输出客户端的信息
    char clientIP[16];
    inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, clientIP, sizeof(clientIP));
    unsigned short clientPort = ntohs(clientaddr.sin_port);
    printf("client ip is %s, port is %d\n", clientIP, clientPort);

    // 5.通信
    char recvBuf[1024] = {0};
    while(1) {
        
        // 获取客户端的数据
        int num = read(cfd, recvBuf, sizeof(recvBuf));
        if(num == -1) {
            perror("read");
            exit(-1);
        } else if(num > 0) {
            printf("recv client data : %s\n", recvBuf);
        } else if(num == 0) {
            // 表示客户端断开连接
            printf("clinet closed...");
            break;
        }

        char * data = "hello,i am server";
        // 给客户端发送数据
        write(cfd, data, strlen(data));
    }
   
    // 关闭文件描述符
    close(cfd);
    close(lfd);

    return 0;
}
```



### TCP通信客户端

客户端比服务端就简单许多了

#### 1.创建一个用于通信的套接字

```C
// 1.创建套接字
int fd = socket(AF_INET, SOCK_STREAM, 0);
if(fd == -1) {
    perror("socket");
    exit(-1);
}
```

#### 2.连接服务器，指定连接服务器的IP和端口

```C
// 2.连接服务器端
 struct sockaddr_in serveraddr;
 serveraddr.sin_family = AF_INET;
 inet_pton(AF_INET, "192.168.193.128", &serveraddr.sin_addr.s_addr);
 serveraddr.sin_port = htons(9999);
 int ret = connect(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

 if(ret == -1) {
     perror("connect");
     exit(-1);
 }
```

#### 3.连接成功收发数据

```C
// 3. 通信
char recvBuf[1024] = {0};
while(1) {

    char * data = "hello,i am client";
    // 给客户端发送数据
    write(fd, data , strlen(data));

    sleep(1);
    
    int len = read(fd, recvBuf, sizeof(recvBuf));
    if(len == -1) {
        perror("read");
        exit(-1);
    } else if(len > 0) {
        printf("recv server data : %s\n", recvBuf);
    } else if(len == 0) {
        // 表示服务器端断开连接
        printf("server closed...");
        break;
    }

}
```

#### 4.通信结束，断开连接

```C
close(fd);
```

#### 5.最终代码

```C
// TCP通信的客户端

#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {

    // 1.创建套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        perror("socket");
        exit(-1);
    }

    // 2.连接服务器端
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.193.128", &serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(9999);
    int ret = connect(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    if(ret == -1) {
        perror("connect");
        exit(-1);
    }

    
    // 3. 通信
    char recvBuf[1024] = {0};
    while(1) {

        char * data = "hello,i am client";
        // 给客户端发送数据
        write(fd, data , strlen(data));

        sleep(1);
        
        int len = read(fd, recvBuf, sizeof(recvBuf));
        if(len == -1) {
            perror("read");
            exit(-1);
        } else if(len > 0) {
            printf("recv server data : %s\n", recvBuf);
        } else if(len == 0) {
            // 表示服务器端断开连接
            printf("server closed...");
            break;
        }

    }

    // 关闭连接
    close(fd);

    return 0;
}
```

### 运行结果

**服务端：**

![image-20220918210758536](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220918210758536.png)

**客户端：**

![image-20220918210821676](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220918210821676.png)

## TCP三次握手——建立连接

- 此节需要结合`网络基础->协议->TCP协议`一起看

### 简易图示

![image-20211121144550714](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211121144550714.png)

### 握手流程

![image-20211121145128707](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211121145128707.png)

#### 第一次握手

- 客户端将SYN标志位置为1 
- 生成一个随机的32位的序号seq=J ， 这个序号后边是可以携带数据（数据的大小）

#### 第二次握手

- 服务器端接收客户端的连接： ACK=1
- 服务器会回发一个确认序号： ack=客户端的序号 + 数据长度 + SYN/FIN(按一个字节算)
- 服务器端会向客户端发起连接请求： SYN=1 
- 服务器会生成一个随机序号：seq = K 

#### 第三次握手

- 客户端应答服务器的连接请求：ACK=1
- 客户端回复收到了服务器端的数据：ack=服务端的序号 + 数据长度 + SYN/FIN(按一个字节算)

### 示例：携带数据通信流程

- 括号内数字代表携带数据大小

![image-20211121151016137](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211121151016137.png)

## TCP滑动窗口——流量控制

### 简介

- `滑动窗口`是 TCP 中实现诸如 ACK 确认、流量控制、拥塞控制的承载结构
- TCP 中采用滑动窗口来进行传输控制，滑动窗口的大小意味着**接收方还有多大的缓冲区可以用于接收数据**。**发送方可以通过滑动窗口的大小来确定应该发送多少字节的数据**。当滑动窗口为 0时，发送方一般不能再发送数据报

> 滑动窗口（Sliding window）是一种流量控制技术。早期的网络通信中，通信双方不会考虑网络的拥挤情况直接发送数据。由于大家不知道网络拥塞状况，同时发送数据，导致中间节点阻塞掉包，谁也发不了数据，所以就有了滑动窗口机制来解决此问题
>
> 滑动窗口协议是用来改善吞吐量的一种技术，即容许发送方在接收任何应答之前传送附加的包。接收方告诉发送方在某一时刻能送多少包（称窗口尺寸）

### 滑动窗口与缓冲区

- **滑动窗口可以理解为缓冲区的大小**

- 滑动窗口的大小会随着发送数据和接收数据而变化，通信的双方都有发送缓冲区和接收数据的缓冲区

- 图示说明：单向发送数据（发送端->接收端）

  ![image-20211121153104245](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211121153104245.png)

  - 发送方的缓冲区
    - 白色格子：空闲的空间
    - 灰色格子：数据已经被发送出去了，但是还没有被接收
    - 紫色格子：还没有发送出去的数据 
  - 接收方的缓冲区
    - 白色格子：空闲的空间 
    - 紫色格子：已经接收到的数据

## TCP四次挥手——断开连接

### 简易图示

![image-20211121154555977](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211121154555977.png)

### 挥手流程

- 四次挥手发生在断开连接的时候，在程序中当调用了`close()`会使用TCP协议进行四次挥手
- 客户端和服务器端都可以主动发起断开连接，谁先调用`close()`谁就是发起方
- 因为在TCP连接的时候，采用三次握手建立的的连接是双向的，在断开的时候需要双向断开

![image-20211121154857445](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211121154857445.png)

## 实例：完整的TCP通信

![image-20211121155042845](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211121155042845.png)

### 注解

- 图中`MSS`表示Maximum Segment Size(一条数据的最大的数据量) 

- `win`表示滑动窗口大小

- 图中部分`ACK`应为确认号`ack`，而非标志位`ACK`

### 流程说明

1. 第1次，**第一次握手**，客户端向服务器发起连接，客户端的滑动窗口大小是4096，一次发送的最大数据量是1460 

2. 第2次，**第二次握手**，服务器接收连接情况，告诉客户端服务器的窗口大小是6144，一次发送的最大数据量是1024 

3. 第3次，**第三次握手**

4. 第4-9次，客户端连续给服务器发送了6k的数据，每次发送1k 

5. 第10次，服务器告诉客户端：发送的6k数据以及接收到，存储在缓冲区中，缓冲区数据已经处理了2k，窗口大小是2k(还剩4k未处理，后面同理，不再做单独说明)

6. 第11次，服务器告诉客户端：发送的6k数据以及接收到，存储在缓冲区中，缓冲区数据已经处理了4k，窗口大小是4k 

7. 第12次，客户端给服务器发送了1k的数据 

8. 第13次，**第一次挥手**，客户端主动请求和服务器断开连接，并且给服务器发送了1k的数据 

9. 第14-16次，**第二次挥手**，服务器回复ACK 8194(包含FIN标记，所以结果上多加了1)，表示**同意断开连接的请求**，并通知客户端依次已经处理了2k，4k，6k数据，滑动窗口大小依次为2k，4k，6k
10. 第17次，**第三次挥手**，服务器端给客户端发送FIN，请求断开连接 
11. 第18次，**第四次回收**，客户端同意了服务器端的断开请求

## TCP通信并发

### 注解

- 要实现TCP通信服务器处理并发的任务，使用多进程或者多线程来解决

### 实例：多进程实现TCP并发通信

#### 思路

- 服务端使用一个父进程，多个子进程 

  - 父进程负责等待并接受客户端的连接 

  - 子进程：完成通信，接受一个客户端连接，就创建一个子进程用于通信

- 客户端不需要改变（同一对一通信）

#### 遇到问题及解决*

- 断开连接后，服务器端如何处理子进程，回收资源？
  - 使用信号处理
- 使用信号捕捉回收子进程资源后，出现服务端`accept: Interrupted system call`，且不能有新客户端连接，如何解决？
  - 产生`EINTR`信号，具体说明通过`man 2 accept`查看
  - 在`accept`返回值处进行判断处理，不输出错误即可
- 当停止所有的客户端连接后，出现`read: Connection reset by peer`，如何解决？
  - 产生的原因：连接断开后的读和写操作引起的
  - 简单修改：将客户端中休眠语句的位置进行更改
  - 方法：[[261]Connection reset by peer的常见原因及解决办法](https://blog.csdn.net/xc_zhou/article/details/80950753)
- 解决上一个问题后，服务端出现两次`client closed...`，如何解决？
  - 是因为在关闭连接后，应该退出循环，所以在该`printf`语句后，添加`break`即可

#### 服务端

```c
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define SERVERIP "127.0.0.1"
#define PORT 6789

void recycleChild(int arg) {
    // 写while是为了处理多个信号
    while (1) {
        int ret = waitpid(-1, NULL, WNOHANG);
        if (ret == -1) {
            // 所有子进程都回收了
            break;
        } else if (ret == 0) {
            // 还有子进程活着
            break;
        } else {
            // 回收子进程
            printf("子进程 %d 被回收了\n", ret);
        }
    }
}

int main()
{
    // 注册信号捕捉
    struct sigaction act;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = recycleChild;
    sigaction(SIGCHLD, &act, NULL);

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
    // 不断循环等待客户端连接
    while (1) {
        // 4. 接收客户端连接
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (connfd == -1) {
            // 用于处理信号捕捉导致的accept: Interrupted system call
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            exit(-1);
        }
        pid_t pid = fork();
        if (pid == 0) {
            // 子进程
            // 输出客户端信息，IP组成至少16个字符（包含结束符）
            char client_ip[16] = {0};
            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
            unsigned short client_port = ntohs(client_addr.sin_port);
            printf("ip:%s, port:%d\n", client_ip, client_port);

            // 5. 开始通信
            // 服务端先接收客户端信息，再向客户端发送数据
            // 接收数据
            char recv_buf[1024] = {0};
            while (1) {
                ret = read(connfd, recv_buf, sizeof(recv_buf));
                if (ret == -1) {
                    perror("read");
                    exit(-1);
                } else if (ret > 0) {
                    printf("recv client data : %s\n", recv_buf);
                } else {
                    // 表示客户端断开连接
                    printf("client closed...\n");
                    // 退出循环，用来解决出现两次client closed...
                    break;
                }
                // 发送数据
                char *send_buf = "hello, i am server";
                // 粗心写成sizeof，那么就会导致遇到空格终止
                write(connfd, send_buf, strlen(send_buf));
            }
            // 关闭文件描述符
            close(connfd);
        }
    }

    close(listenfd);
    return 0;
}
```

#### 客户端

```c
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
        // 粗心写成sizeof，那么就会导致遇到空格终止
        write(connfd, send_buf, strlen(send_buf));
        // 休眠的目的是为了更好的观察，此处使用sleep语句会导致read: Connection reset by peer
        // sleep(1);
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

#### 通信效果

![image-20211121162229827](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211121162229827.png)

### 实例：多线程实现TCP并发通信

#### 思路

- 服务端使用一个主线程，多个子线程 

  - 主线程负责等待并接受客户端的连接 

  - 子线程：完成通信，接受一个客户端连接，就创建一个子进程用于通信

- 客户端不需要改变（同一对一通信）

#### 服务端

```c
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define SERVERIP "127.0.0.1"
#define PORT 6789

struct sockInfo{
    int fd;                             // 通信文件描述符
    pthread_t tid;                      // 线程号
    struct sockaddr_in addr;            // 客户端信息
};

struct sockInfo sockinfos[128];     // 表示最大有128个客户端连接

void* working(void *arg) {
    // 子线程与客户端通信
    struct sockInfo *pinfo = (struct sockInfo*)arg;

    // 输出客户端信息，IP组成至少16个字符（包含结束符）
    char client_ip[16] = {0};
    inet_ntop(AF_INET, &pinfo->addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
    unsigned short client_port = ntohs(pinfo->addr.sin_port);
    printf("ip:%s, port:%d\n", client_ip, client_port);

    // 5. 开始通信
    // 服务端先接收客户端信息，再向客户端发送数据
    // 接收数据
    char recv_buf[1024] = {0};
    while (1) {
        int ret = read(pinfo->fd, recv_buf, sizeof(recv_buf));
        if (ret == -1) {
            perror("read");
            exit(-1);
        } else if (ret > 0) {
            printf("recv client data : %s\n", recv_buf);
        } else {
            // 表示客户端断开连接
            printf("client closed...\n");
            break;
        }
        // 发送数据
        char *send_buf = "hello, i am server";
        // 粗心写成sizeof，那么就会导致遇到空格终止
        write(pinfo->fd, send_buf, strlen(send_buf));
    }
    // 关闭文件描述符
    close(pinfo->fd);
}

int main()
{
    // 初始化线程结构体数据
    int sockinfo_maxLen = sizeof(sockinfos) / sizeof(sockinfos[0]);
    for (int i = 0; i < sockinfo_maxLen; i++) {
        bzero(&sockinfos[i], sizeof(sockinfos[i]));
        sockinfos[i].fd = -1;
        sockinfos[i].tid = -1;
    }

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
    // 不断循环等待客户端连接
    while (1) {
        // 4. 接收客户端连接
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (connfd == -1) {
            perror("accept");
            exit(-1);
        }
        // 创建子线程
        struct sockInfo *pinfo;
        // 从线程数组中找到一个可用的元素进行赋值
        for (int i = 0; i < sockinfo_maxLen; i++) {
            if (sockinfos[i].tid == -1) {
                pinfo = &sockinfos[i];
                break;
            }
            // 当遍历到最后还没有找到，那么休眠一秒后，从头开始找
            if (i == sockinfo_maxLen - 1) {
                sleep(1);
                i = -1;
            }
        }
        // 结构体赋值
        pinfo->fd = connfd;
        memcpy(&pinfo->addr, &client_addr, client_addr_len);
        pthread_create(&pinfo->tid, NULL, working, pinfo);
        // 释放资源
        pthread_detach(pinfo->tid);
    }

    close(listenfd);
    return 0;
}
```

#### 客户端

```c
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
        // 粗心写成sizeof，那么就会导致遇到空格终止
        write(connfd, send_buf, strlen(send_buf));
        // 休眠的目的是为了更好的观察，此处使用sleep语句会导致read: Connection reset by peer
        // sleep(1);
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

#### 通信效果

![image-20211121172701543](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211121172701543.png)

## TCP状态转换

### 通信过程状态转换图1

![image-20211123124438914](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211123124438914.png)

### 通信过程状态转换图2

- 红色实线代表客户端（主动发起连接）
- 绿色虚线代表服务端（被动接收连接）
- 黑色实现代表特殊情况
- 数字代表三次握手流程

![image-20211123124611335](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211123124611335.png)

### MSL与半关闭

- 主动断开连接的一方，最后会进入一个 `TIME_WAIT`状态，这个状态会持续`2msl`

- `msl`：官方建议2分钟，实际是30s，**主要是为了防止挥手信息丢失**

  > 当 TCP 连接主动关闭方接收到被动关闭方发送的 FIN 和最终的 ACK 后，连接的主动关闭方必须处于TIME_WAIT 状态并持续 2MSL 时间
  >
  > 这样就能够让 TCP 连接的主动关闭方在它发送的 ACK 丢失的情况下重新发送最终的 ACK
  >
  > 主动关闭方重新发送的最终 ACK 并不是因为被动关闭方重传了 ACK（它们并不消耗序列号，被动关闭方也不会重传），而是因为被动关闭方重传了它的 FIN。事实上，被动关闭方总是重传 FIN 直到它收到一个最终的 ACK

- `半关闭`：当 TCP 连接中 A 向 B 发送 FIN 请求关闭，另一端 B 回应 ACK 之后（A 端进入 FIN_WAIT_2状态），并没有立即发送 FIN 给 A，A 方处于半连接状态（半开关），此时 **A 可以接收 B 发送的数据，但是 A 已经不能再向 B 发送数据**

- API 来控制实现半连接状态的方法：` shutdown函数`

  - `int shutdown(int sockfd, int how); `
    - 功能：实现半连接状态
    - 参数
      - `sockfd`：需要关闭的socket的描述符 
      - `how`：允许为shutdown操作选择以下几种方式
        - `SHUT_RD(0)`：关闭sockfd上的读功能，此选项将不允许sockfd进行读操作，该套接字不再接收数据，任何当前在套接字接受缓冲区的数据将被无声的丢弃掉
        - `SHUT_WR(1)`：关闭sockfd的写功能，此选项将不允许sockfd进行写操作。进程不能在对此套接字发 出写操作
        - `SHUT_RDWR(2)`：关闭sockfd的读写功能。相当于调用shutdown两次：首先调用`SHUT_RD`,然后调用 `SHUT_WR`

### shutdown与close

- 使用 `close` 中止一个连接，但它只是**减少描述符的引用计数，并不直接关闭连接**，只有当描述符的引用计数为 0 时才关闭连接
- `shutdown` 不考虑描述符的引用计数，**直接关闭描述符**。也可选择中止一个方向的连接，只中止读或只中止写
- 如果有多个进程共享一个套接字，close 每被调用一次，计数减 1 ，直到计数为 0 时，也就是所用进程都调用了 close，套接字将被释放
- **在多进程中如果一个进程调用了 `shutdown(sfd, SHUT_RDWR)` 后，其它的进程将无法进行通信**。但如果一个进程 `close(sfd)` 将不会影响到其它进程=>==难怪800那个项目调shutdown之后其他线程就不能用了==

### 端口复用

#### 用途

- 防止服务器重启时之前绑定的端口还未释放
- 程序突然退出而系统没有释放端口

#### 方法——`setsockopt`

- `int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen); `
  - 功能：设置套接字的属性（不仅仅能设置端口复用），以下说明仅针对端口复用，其他详细内容可查看`slide/04Linux网络编程/02 socket通信/UNP（Unix网络编程）.pdf`第七章相关内容 
  - 参数
    - `sockfd`：要操作的文件描述符 
    - `level`：级别，`SOL_SOCKET` (端口复用的级别)
    - `optname`：选项的名称，使用`SO_REUSEADDR`或`SO_REUSEPORT`
    - `optval`：端口复用的值（整形） ，1表示可复用，0表示不可复用
    - `optlen`：optval参数的大小

#### 注意

- 端口复用的设置时机是**在服务器绑定端口之前**
- 如果不设置端口复用，那么在程序异常终止后，再次启动服务会出现`Bind error:Address already in use`

### 查看看网络相关信息命令——netstat

- 格式：`netstat -参数名`
- 常用参数
  - `a`：所有的socket
  - `p`：显示正在使用socket的程序的名称
  - `n`：直接使用IP地址，而不通过域名服务器
=======
# TCP通信流程

- TCP 和 UDP -> 传输层的协议 
- UDP:用户数据报协议，面向无连接，可以单播，多播，广播， 面向数据报，不可靠 
- TCP:传输控制协议，面向连接的，可靠的，基于字节流，仅支持单播传输

|                | UDP                            | TCP                        |
| -------------- | :----------------------------- | :------------------------- |
| 是否创建连接   | 无连接                         | 面向连接                   |
| 是否可靠       | 不可靠                         | 可靠的                     |
| 连接的对象个数 | 一对一、一对多、多对一、多对多 | 支持一对一                 |
| 传输的方式     | 面向数据报                     | 面向字节流                 |
| 首部开销       | 8个字节                        | 最少20个字节               |
| 适用场景       | 实时应用（视频会议，直播）     | 可靠性高的应用（文件传输） |



<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220916151323692.png" alt="image-20220916151323692" style="zoom:50%;" />   

## **TCP 通信的流程** 

**服务器端 （被动接受连接的角色）** 

1、创建一个用于监听的套接字 

\- 监听：监听有客户端的连接 

\- 套接字：这个套接字其实就是一个文件描述符 

2、将这个监听文件描述符和本地的IP和端口绑定（IP和端口就是服务器的地址信息） 

\- 客户端连接服务器的时候使用的就是这个IP和端口 

3、设置监听，监听的fd开始工作 

4、阻塞等待，当有客户端发起连接，解除阻塞，接受客户端的连接，会得到一个和客户端通信的套接字（fd） 

5、通信 

\- 接收数据 

\- 发送数据 

6、通信结束，断开连接

**客户端** 

1、创建一个用于通信的套接字（fd） 

2、连接服务器，需要指定连接的服务器的 IP 和 端口 

3、连接成功了，客户端可以直接和服务器通信 

\- 接收数据 

\- 发送数据 

4、通信结束，断开连接 



### TCP通信服务器端

#### 1. 创建一个用于监听的socket

使用`socket`函数创建用于监听的套接字

```C
int socket(int domain, int type, int protocol)
	-domain:协议族
		AF_INET:IPV4
		AF_INET6:IPV6
		AF_UNIX,AF_LOCAL:本地套接字用于进程间通信
	-type：使用的协议类型
		SOCK_STREAM:流式协议
		SOCK_DGRAM:报式协议
	-protocol:具体协议，一般写0
		SOCK_STREAM:流式协议默认tcp
		SOCK_DGRAM:报式协议默认udp
	-返回值：
		成功返回文件描述符
		失败返回-1
```

```C
// 1.创建socket(用于监听的套接字)
    int lfd = socket(AF_INET, SOCK_STREAM, 0);

    if(lfd == -1) {
        perror("socket");
        exit(-1);
    }
```



#### 2. 将这个监听文件描述符和本地的IP和端口绑定

绑定之前需要先创建和初始化一个专用socket地址`struct sockaddr_in`存放本地IP和端口

**sockaddr_in结构体**

```C
#include <netinet/in.h>
struct sockaddr_in
{
    sa_family_t sin_family;     /* __SOCKADDR_COMMON(sin_) */
    in_port_t sin_port;         /* Port number.  */
    struct in_addr sin_addr;    /* Internet address.  */
    /* Pad to size of `struct sockaddr'. */
    unsigned char sin_zero[sizeof (struct sockaddr) - __SOCKADDR_COMMON_SIZE -
               sizeof (in_port_t) - sizeof (struct in_addr)];
};  
struct in_addr
{
    in_addr_t s_addr;
};
struct sockaddr_in6
{
    sa_family_t sin6_family;
    in_port_t sin6_port;    /* Transport layer port # */
    uint32_t sin6_flowinfo; /* IPv6 flow information */
    struct in6_addr sin6_addr;  /* IPv6 address */
    uint32_t sin6_scope_id; /* IPv6 scope-id */
 };
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
#define __SOCKADDR_COMMON_SIZE (sizeof (unsigned short int)
```

**创建和初始化**

```C
struct sockaddr_in saddr;
saddr.sin_family = AF_INET;
// inet_pton(AF_INET, "192.168.193.128", saddr.sin_addr.s_addr);
saddr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0
saddr.sin_port = htons(9999);
```

我们一般使用的IP地址表示方法为点分十进制表示法(一个字符串），而`sockaddr_in`中存储的IP地址为一般为整形的网络字节序形式，因此我们可以使用`inet_pton`函数来实现点分十进制IP地址字符床与网络字节序整数的转换。

当让我们可以直接给IP地址复制`INADDR_ANY`或者`0.0.0.0`二者的值是相同的，其意义是让服务器端计算机上的所有网卡的IP地址都可以作为服务器IP地址，也即监听外部客户端程序发送到服务器端所有网卡的网络请求

**绑定**
我们使用`bind`函数来实现绑定

```C
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen); // socket命
名
    - 功能：绑定，将fd 和本地的IP + 端口进行绑定
    - 参数：
            - sockfd : 通过socket函数得到的文件描述符
            - addr : 需要绑定的socket地址，这个地址封装了ip和端口号的信息
            - addrlen : 第二个参数结构体占的内存大小
```

```C
 // 2.绑定
    int ret = bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));

    if(ret == -1) {
        perror("bind");
        exit(-1);
    }
```

tip:注意对&saddr进行强制类型转换`(struct sockaddr *)&saddr`

早期网络编程函数使用通用socket地址`struct sockaddr`结构体，为了向前兼容，现在`sockaddr`退化成了`void *`，也就是说这片内存区域的使用和解释方法取决于谁去用和怎么用

专用socket地址类型变量在实际使用时都需要转化为通用socket地址类型socketaddr，因此在具体使用这些接口的时候必须指定具体的指针类型

#### 3.设置监听，监听的fd开始工作

我们使用`listen`函数来进行监听，`listen`创建一个监听队列来接收客户端的连接，backlog参数就是设置监听队列的长度一般为5,如果设置为`SOMAXCONN`那么就由系统来决定，一般是比较大的。
另外，如果监听队列满了那么客户端会收到 `ECONNREFUSED` 错误

```C
int listen(int sockfd, int backlog);    // /proc/sys/net/core/somaxconn
    - 功能：监听这个socket上的连接
    - 参数：
        - sockfd : 通过socket()函数得到的文件描述符
        - backlog : 未连接的和已经连接的和的最大值， 5
```

```C
 // 3.监听
    ret = listen(lfd, 8);
    if(ret == -1) {
        perror("listen");
        exit(-1);
    }
```

#### 4.阻塞等待客户端发起链接

我们使用`accept`函数接收客户端发起的连接，并记录下来
因为需要记录因此，我们还需要一个`struct socketaddr`来存储客户端的信息

```C
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    - 功能：接收客户端连接，默认是一个阻塞的函数，阻塞等待客户端连接 
    - 参数：
            - sockfd : 用于监听的文件描述符
            - addr : 传出参数，记录了连接成功后客户端的地址信息（ip，port）
            - addrlen : 指定第二个参数的对应的内存大小
    - 返回值：
            - 成功 ：用于通信的文件描述符 
            - -1 ： 失败		
```

```C
 struct sockaddr_in clientaddr;
 int len = sizeof(clientaddr);
 int cfd = accept(lfd, (struct sockaddr *)&clientaddr, &len);
 
 if(cfd == -1) {
     perror("accept");
     exit(-1);
 }
```

如果我们要查看客户端信息，那么最好使用`inet_ntop`将存储的网络字节序转化为点分十进制表示法，用`ntohs`将端口从网络字节序转化为主机字节序

```C
    char clientIP[16];
    inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, clientIP, sizeof(clientIP));
    unsigned short clientPort = ntohs(clientaddr.sin_port);
    printf("client ip is %s, port is %d\n", clientIP, clientPort);
```

#### 5.通信收发数据

还是那句话，Linux系统中万物皆是文件，socket也不例外。
在文件描述符后就可以使用`read`和`write`直接进行读写操作了

```C
// 5.通信
    char recvBuf[1024] = {0};
    while(1) {
        
        // 获取客户端的数据
        int num = read(cfd, recvBuf, sizeof(recvBuf));
        if(num == -1) {
            perror("read");
            exit(-1);
        } else if(num > 0) {
            printf("recv client data : %s\n", recvBuf);
        } else if(num == 0) {
            // 表示客户端断开连接
            printf("clinet closed...");
            break;
        }

        char * data = "hello,i am server";
        // 给客户端发送数据
        write(cfd, data, strlen(data));
    }
```

#### 6.通信结束断开链接

当通信结束断开连接的时候，我们需要关闭监听的文件描述符和客户端的文件描述符

```C
    // 关闭文件描述符
    close(cfd);
    close(lfd);
```

#### 最终代码

```C
// TCP 通信的服务器端

#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {

    // 1.创建socket(用于监听的套接字)
    int lfd = socket(AF_INET, SOCK_STREAM, 0);

    if(lfd == -1) {
        perror("socket");
        exit(-1);
    }

    // 2.绑定
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    // inet_pton(AF_INET, "192.168.193.128", saddr.sin_addr.s_addr);
    saddr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0
    saddr.sin_port = htons(9999);
    int ret = bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));

    if(ret == -1) {
        perror("bind");
        exit(-1);
    }

    // 3.监听
    ret = listen(lfd, 8);
    if(ret == -1) {
        perror("listen");
        exit(-1);
    }

    // 4.接收客户端连接
    struct sockaddr_in clientaddr;
    int len = sizeof(clientaddr);
    int cfd = accept(lfd, (struct sockaddr *)&clientaddr, &len);
    
    if(cfd == -1) {
        perror("accept");
        exit(-1);
    }

    // 输出客户端的信息
    char clientIP[16];
    inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, clientIP, sizeof(clientIP));
    unsigned short clientPort = ntohs(clientaddr.sin_port);
    printf("client ip is %s, port is %d\n", clientIP, clientPort);

    // 5.通信
    char recvBuf[1024] = {0};
    while(1) {
        
        // 获取客户端的数据
        int num = read(cfd, recvBuf, sizeof(recvBuf));
        if(num == -1) {
            perror("read");
            exit(-1);
        } else if(num > 0) {
            printf("recv client data : %s\n", recvBuf);
        } else if(num == 0) {
            // 表示客户端断开连接
            printf("clinet closed...");
            break;
        }

        char * data = "hello,i am server";
        // 给客户端发送数据
        write(cfd, data, strlen(data));
    }
   
    // 关闭文件描述符
    close(cfd);
    close(lfd);

    return 0;
}
```



### TCP通信客户端

客户端比服务端就简单许多了

#### 1.创建一个用于通信的套接字

```C
// 1.创建套接字
int fd = socket(AF_INET, SOCK_STREAM, 0);
if(fd == -1) {
    perror("socket");
    exit(-1);
}
```

#### 2.连接服务器，指定连接服务器的IP和端口

```C
// 2.连接服务器端
 struct sockaddr_in serveraddr;
 serveraddr.sin_family = AF_INET;
 inet_pton(AF_INET, "192.168.193.128", &serveraddr.sin_addr.s_addr);
 serveraddr.sin_port = htons(9999);
 int ret = connect(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

 if(ret == -1) {
     perror("connect");
     exit(-1);
 }
```

#### 3.连接成功收发数据

```C
// 3. 通信
char recvBuf[1024] = {0};
while(1) {

    char * data = "hello,i am client";
    // 给客户端发送数据
    write(fd, data , strlen(data));

    sleep(1);
    
    int len = read(fd, recvBuf, sizeof(recvBuf));
    if(len == -1) {
        perror("read");
        exit(-1);
    } else if(len > 0) {
        printf("recv server data : %s\n", recvBuf);
    } else if(len == 0) {
        // 表示服务器端断开连接
        printf("server closed...");
        break;
    }

}
```

#### 4.通信结束，断开连接

```C
close(fd);
```

#### 5.最终代码

```C
// TCP通信的客户端

#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {

    // 1.创建套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        perror("socket");
        exit(-1);
    }

    // 2.连接服务器端
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.193.128", &serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(9999);
    int ret = connect(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    if(ret == -1) {
        perror("connect");
        exit(-1);
    }

    
    // 3. 通信
    char recvBuf[1024] = {0};
    while(1) {

        char * data = "hello,i am client";
        // 给客户端发送数据
        write(fd, data , strlen(data));

        sleep(1);
        
        int len = read(fd, recvBuf, sizeof(recvBuf));
        if(len == -1) {
            perror("read");
            exit(-1);
        } else if(len > 0) {
            printf("recv server data : %s\n", recvBuf);
        } else if(len == 0) {
            // 表示服务器端断开连接
            printf("server closed...");
            break;
        }

    }

    // 关闭连接
    close(fd);

    return 0;
}
```

### 运行结果

**服务端：**

![image-20220918210758536](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220918210758536.png)

**客户端：**

![image-20220918210821676](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220918210821676.png)
>>>>>>> f5b7f421229ec4a664efbc33954cf3fd58c9b115
