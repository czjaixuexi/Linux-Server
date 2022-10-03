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