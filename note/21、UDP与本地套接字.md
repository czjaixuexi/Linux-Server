# UDP与本地套接字



## UDP通信

### 通信流程

![image-20211127210835952](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211127210835952.png)

### 消息收发函数

- `ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);`
  - 功能：udp发送消息函数
  - 参数
    - `sockfd`：通信的套接字(文件描述符)
    - `buf`：要发送的数据 
    - `len`：发送数据的长度 
    - `flags`：设置为0即可
    - `dest_addr`：通信的另外一端的地址信息 
    - `addrlen`：地址的内存大小，即`sizeof(dest_addr)`
  - 返回值：失败-1，否则返回发送数据大小
- `ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen); `
  - 功能：udp接收消息函数
  - 参数
    - `sockfd`：通信的套接字(文件描述符)
    - `buf`：接收的数据 
    - `len`：接收数据的长度 
    - `flags`：设置为0即可
    - `dest_addr`：通信的另外一端的地址信息，不需要设置为NULL即可
    - `addrlen`：地址的内存大小，即`sizeof(dest_addr)`
  - 返回值：失败-1，否则返回发送数据大小

### 实例：UDP通信

#### 说明

- 服务端不需要设置监听文件描述符=>因为不需要三次握手
- 不需要多进程/多线程，或者IO多路复用即可实现多并发

#### 服务端

```c
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#define SERVERIP "127.0.0.1"
#define PORT 6789

int main()
{
    // 1. 创建通信套接字
    int connfd = socket(PF_INET, SOCK_DGRAM, 0);
    // 2. 绑定本机地址(服务端)
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    int ret = bind(connfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret == -1) {
        perror("bind");
        exit(-1);
    }
    // 3. 通信
    while (1) {
        char recvbuf[128];
        char ipbuf[16];

        struct sockaddr_in cliaddr;
        int len = sizeof(cliaddr);

        // 接收数据
        int num = recvfrom(connfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&cliaddr, &len);

        printf("client IP : %s, Port : %d\n", 
            inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ipbuf, sizeof(ipbuf)),
            ntohs(cliaddr.sin_port));

        printf("client say : %s\n", recvbuf);

        // 发送数据
        sendto(connfd, recvbuf, strlen(recvbuf) + 1, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
    }
    return 0;
}
```

#### 客户端

```c
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVERIP "127.0.0.1"
#define PORT 6789

int main()
{
    // 1. 创建通信套接字
    int connfd = socket(PF_INET, SOCK_DGRAM, 0);
    // 2. 通信
    // 设置服务器信息
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    int num = 0;
    while (1) {
        // 发送数据
        char sendBuf[128];
        sprintf(sendBuf, "hello , i am client %d \n", num++);
        sendto(connfd, sendBuf, strlen(sendBuf) + 1, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        // 接收数据
        int num = recvfrom(connfd, sendBuf, sizeof(sendBuf), 0, NULL, NULL);
        printf("server say : %s\n", sendBuf);

        sleep(1);
    }
    return 0;
}
```

## 广播

### 简介

- 只能在局域网中使用
- 客户端需要绑定服务器广播使用的端口，才可以接收到广播消息

> 向子网中多台计算机发送消息，并且子网中所有的计算机都可以接收到发送方发送的消息，每个广播消息都包含一个特殊的IP地址，这个IP中子网内主机标志部分的二进制全部为1

![image-20211204205113069](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211204205113069.png)

### 方法

- 通过设置`setsockopt`函数，服务端进行设置（发送广播端）
- `int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen); `
  - `sockfd`：通信套接字
  - `level`：设置为`SOL_SOCKET`
  - `optname`：设置为`SO_BROADCAST`
  - `optval`：int类型的值，为1表示允许广播 
  - `optlen`：optval的大小

### 注意事项

- 此时客户端和服务端界限模糊，按理来说，需要`bind`端为服务端，而在广播时，需要`bind`的一端为接收消息端

- `发送广播端`需要通过`setsockopt`设置相关信息，广播地址需要根据本地IP进行配置，即`xxx.xxx.xxx.255`

- `接收广播端`需要绑定广播地址或设置为接收任意地址消息

- 接收端在连入时，已经过去的消息将不被接收

  ![image-20211204212615592](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211204212615592.png)

### 实例：广播

#### 服务端（发送广播端）

```c
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BROADCASTIP "192.168.213.255"
#define PORT 6789

int main()
{
    // 1. 创建通信套接字
    int connfd = socket(PF_INET, SOCK_DGRAM, 0);

    // 2.设置广播属性
    int op = 1;
    setsockopt(connfd, SOL_SOCKET, SO_BROADCAST, &op, sizeof(op));

    // 3.创建一个广播的地址
    struct sockaddr_in broad_addr;
    broad_addr.sin_family = AF_INET;
    broad_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, BROADCASTIP, &broad_addr.sin_addr.s_addr);

    // 4. 通信
    int num = 0;
    while (1) {
        char sendBuf[128];
        sprintf(sendBuf, "hello, client....%d", num++);
        // 发送数据
        sendto(connfd, sendBuf, strlen(sendBuf) + 1, 0, (struct sockaddr *)&broad_addr, sizeof(broad_addr));
        printf("广播的数据：%s\n", sendBuf);
        sleep(1);
    }
    close(connfd);
    return 0;
}
```

#### 客户端（接收广播端）

```c
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BROADCASTIP "192.168.213.255"
#define PORT 6789

int main()
{
    // 1. 创建通信套接字
    int connfd = socket(PF_INET, SOCK_DGRAM, 0);

    // 2.客户端绑定通信的IP和端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    // 设置为接收任意网址信息或指定多播地址
    // addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, BROADCASTIP, &addr.sin_addr.s_addr);

    // 3. 将信息进行绑定
    int ret = bind(connfd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret == -1) {
        perror("bind");
        exit(-1);
    }
    // 4. 通信
    while (1) {
        char buf[128];
        // 接收数据
        int num = recvfrom(connfd, buf, sizeof(buf), 0, NULL, NULL);
        printf("server say : %s\n", buf);
    }
    close(connfd);
    return 0;
}
```

## 组播(多播）

### 简介

- 组播既可以用于局域网，也可以用于广域网
- 客户端需要加入多播组，才能接收到多播的数据

> 单播地址标识单个 IP 接口，广播地址标识某个子网的所有 IP 接口，多播地址标识一组 IP 接口
>
> 单播和广播是寻址方案的两个极端（要么单个要么全部），多播则意在两者之间提供一种折中方案
>
> 多播数据报只应该由对它感兴趣的接口接收，也就是说由运行相应多播会话应用系统的主机上的接口接收。另外，广播一般局限于局域网内使用，而多播则既可以用于局域网，也可以跨广域网使用

![image-20211204212840626](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211204212840626.png)

### 方法

- 通过设置`setsockopt`函数，服务器和客户端都需要进行设置
- `int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen); `
- 服务端：设置多播的信息，外出接口
  - `sockfd`：通信套接字
  - `level`：设置为`IPPROTO_IP`
  - `optname`：设置为`IP_MULTICAST_IF`
  - `optval`：`struct in_addr`类型
  - `optlen`：optval的大小
- 客户端：加入多播组
  - `sockfd`：通信套接字
  - `level`：设置为`IPPROTO_IP`
  - `optname`：设置为`IP_ADD_MEMBERSHIP`
  - `optval`：`struct ip_mreq`类型
  - `optlen`：optval的大小

```c
typedef uint32_t in_addr_t; 
struct in_addr { 
    in_addr_t s_addr; 
};

struct ip_mreq { 
    /* IP multicast address of group. */ 
    struct in_addr imr_multiaddr; // 组播的IP地址 
    /* Local IP address of interface. */ 
    struct in_addr imr_interface; // 本地的IP地址 
};
```

### 注意事项

- 服务端通过`setsockopt`设置`optval`时，需要指定多播地址，即`239.0.0.0~239.255.255.255`其中一个即可

![image-20211204220128304](https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20211204220128304.png)

### 实例：组播

#### 服务端

```c
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MULTIIP "239.0.0.10"
#define PORT 6789

int main()
{
    // 1. 创建通信套接字
    int connfd = socket(PF_INET, SOCK_DGRAM, 0);

    // 2.设置多播属性
    struct in_addr op;
    // 初始化多播地址
    inet_pton(AF_INET, MULTIIP, &op.s_addr);
    setsockopt(connfd, IPPROTO_IP, IP_MULTICAST_IF, &op, sizeof(op));

    // 3.初始化客户端的地址信息
    struct sockaddr_in cliaddr;
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, MULTIIP, &cliaddr.sin_addr.s_addr);

    // 4. 通信
    int num = 0;
    while (1) {
        char sendBuf[128];
        sprintf(sendBuf, "hello, client....%d", num++);
        // 发送数据
        sendto(connfd, sendBuf, strlen(sendBuf) + 1, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
        printf("多播的数据：%s\n", sendBuf);
        sleep(1);
    }
    close(connfd);
    return 0;
}
```

#### 客户端

```c
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MULTIIP "239.0.0.10"
#define PORT 6789

int main()
{
    // 1. 创建通信套接字
    int connfd = socket(PF_INET, SOCK_DGRAM, 0);

    // 2.客户端绑定通信的IP和端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    // 设置为接收任意网址信息或指定多播地址
    // addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, MULTIIP, &addr.sin_addr.s_addr);

    // 3. 将信息进行绑定
    int ret = bind(connfd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret == -1) {
        perror("bind");
        exit(-1);
    }

    // 4. 加入多播组
    // 设置多播属性
    struct ip_mreq op;
    inet_pton(AF_INET, MULTIIP, &op.imr_multiaddr.s_addr);
    op.imr_interface.s_addr = INADDR_ANY;
    // 加入多播组
    setsockopt(connfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &op, sizeof(op));

    // 5. 通信
    while (1) {
        char buf[128];
        // 接收数据
        int num = recvfrom(connfd, buf, sizeof(buf), 0, NULL, NULL);
        printf("server say : %s\n", buf);
    }
    close(connfd);
    return 0;
}
```

## 本地套接字通信

### 简介

- 本地套接字的作用：本地的进程间通信，包括`有关系的进程通信(父子进程)`和`没有关系的进程间通信`
- 本地套接字实现流程和网络套接字类似，一般采用`TCP的通信流程`

### 通信流程

- 服务端
  1. 创建监听的套接字：`int lfd = socket(AF_UNIX/AF_LOCAL, SOCK_STREAM, 0);`
  2. 监听的套接字绑定本地的套接字文件：`bind(lfd, addr, len); `，绑定成功之后，指定的`sun_path`中的套接字文件会自动生成
  3. 监听：`listen(lfd, 100);`
  4. 等待并接受连接请求：`int cfd = accept(lfd, &cliaddr, len);`
  5. 通信
     - 接收数据：`read/recv`
     - 发送数据：`write/send`
  6. 关闭连接：`close()`
- 客户端
  1. 创建通信的套接字：`int cfd = socket(AF_UNIX/AF_LOCAL, SOCK_STREAM, 0); `
  2. 监听的套接字绑定本地的IP端口：`bind(cfd, &addr, len); `，绑定成功之后，指定的sun_path中的套接字文件会自动生成
  3. 连接服务器：`connect(fd, &serveraddr, sizeof(serveraddr));`
  4. 通信
     - 接收数据：`read/recv`
     - 发送数据：`write/send`
  5. 关闭连接：`close()`

### 注意事项

- 地址结构体为：`struct sockaddr_un`类型

  ```c
  // 头文件: sys/un.h 
  #define UNIX_PATH_MAX 108 
  struct sockaddr_un { 
      sa_family_t sun_family; // 地址族协议 af_local 
      char sun_path[UNIX_PATH_MAX]; // 套接字文件的路径, 这是一个伪文件, 大小永远=0 
  };
  ```

- 使用`unlink`解除占用：本地套接字通信通过文件，如果不用unlink解除占用，则会出现"bind: Address already in use"

### 实例：本地进程间通信

#### 服务端

```c
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
    // 本地套接字通信通过文件，如果不用unlink解除占用，则会出现"bind: Address already in use"
    unlink("server.sock");
    // 1. 创建监听套接字
    int listenfd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        exit(-1);
    }
    // 2. 绑定本地信息
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_LOCAL;
    strcpy(server_addr.sun_path, "server.sock"); 
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
    // 4. 接收连接
    struct sockaddr_un client_addr;
    int client_addr_len = sizeof(client_addr);
    client_addr.sun_family = AF_LOCAL;
    strcpy(server_addr.sun_path, "client.sock");
    int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (connfd == -1) {
        perror("connect");
        exit(-1);
    }
    // 5. 通信
    while (1) {
        // 接收信息
        char buf[1024];
        int buf_len = recv(connfd, buf, sizeof(buf), 0);
        if (buf_len == -1) {
            perror("recv");
            exit(-1);
        } else if (buf_len == 0) {
            printf("client close...\n");
            break;
        } else {
            printf("client say : %s\n", buf);
            // 发送信息
            send(connfd, buf, strlen(buf) + 1, 0);
        }
    }
    // 6. 关闭套接字
    close(connfd);
    close(listenfd);
    return 0;
}
```

#### 客户端

```c
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
    // 本地套接字通信通过文件，如果不用unlink解除占用，则会出现"bind: Address already in use"
    unlink("client.sock");
    // 1. 创建通信套接字
    int connfd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (connfd == -1) {
        perror("socket");
        exit(-1);
    }
    // 2. 绑定
    struct sockaddr_un client_addr;
    client_addr.sun_family = AF_LOCAL;
    strcpy(client_addr.sun_path, "client.sock");
    int ret = bind(connfd, (struct sockaddr*)&client_addr, sizeof(client_addr));
    if (ret == -1) {
        perror("bind");
        exit(-1);
    }
    // 3. 建立连接
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_LOCAL;
    strcpy(server_addr.sun_path, "server.sock");
    ret = connect(connfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("connect");
        exit(-1);
    }
    int num = 0;
    // 5. 通信
    while (1) {
        // 发送信息
        char buf[1024];
        sprintf(buf, "the data is %d", num++);
        send(connfd, buf, strlen(buf) + 1, 0);
        // 接收信息
        int buf_len = recv(connfd, buf, sizeof(buf), 0);
        if (buf_len == -1) {
            perror("recv");
            exit(-1);
        } else if (buf_len == 0) {
            printf("server close...\n");
            break;
        } else {
            printf("server say : %s\n", buf);
        }
        sleep(1);
    }

    // 6. 关闭套接字
    close(connfd);
    return 0;
}
```

