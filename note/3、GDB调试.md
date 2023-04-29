# GDB调试

## GDB是什么

GDB是由GNU软件系统社区提供的调试工具，同[GCC](https://so.csdn.net/so/search?q=GCC&spm=1001.2101.3001.7020)配套组成了一套完整的开发环境

GDB可以帮助完成下面四个功能：

1. 启动程序，可以按照自定义要求运行程序
2. 可以让调试的程序在指定断点位置停住
3. 当程序停住时可以检查程序中发生的事情
4. 可以改变程序，将一个bug产生的影响修正从而测试其他bug

## 预先准备

如果为了调试而编译我们通常会

- 关掉优化选项（-o）
- 打开调试选项（-g）
- -Wall尽量全开

```makefile
gcc -g -Wall program.c -o program
```

-g选项的作用是在可执行文件中加入源代码信息，但是不把整个源文件嵌入进去，所以调试时要保证gdb能找到源文件

## 基本命令

##### 启动

gdb + 可执行程序

```shell
gdb test
```

##### 退出

输入`quit`或`q`

给程序设置参数/获取设置参数

```shell
set args 10 20
show args
```

##### GDB 使用帮助

```shell
help
```

##### 查看当前文件代码

```shell
list/l （从默认位置显示）
list/l 行号 （从指定的行显示）
list/l 函数名（从指定的函数显示）
```

##### 查看非当前文件代码

```shell
list/l 文件名:行号
list/l 文件名:函数名
```

##### 设置显示的行数

```shell
show list/listsize
set list/listsize 行数
```

##### 设置断点

- ```shell
  b/break 行号
  ```

  <img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807171752156.png" alt="image-20220807171752156" style="zoom:67%;" />

- ```shell
  b/break 函数名
  ```

  <img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807172334118.png" alt="image-20220807172334118" style="zoom: 67%;" />

- ```shell
  b/break 文件名:行号
  ```

  <img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807172426515.png" alt="image-20220807172426515" style="zoom:67%;" />

- ```shell
  b/break 文件名:函数
  ```

  <img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807172604546.png" alt="image-20220807172604546" style="zoom:67%;" />

##### 查看断点

```shell
i/info b/break
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807172613450.png" alt="image-20220807172613450" style="zoom:50%;" />

##### 删除断点

```shell
d/del/delete 断点编号
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807172820455.png" alt="image-20220807172820455" style="zoom: 67%;" />

##### 设置断点无效

```shell
dis/disable 断点编号
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807173211535.png" alt="image-20220807173211535" style="zoom: 67%;" />

##### 设置断点生效

```shell
ena/enable 断点编号
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807173241266.png" alt="image-20220807173241266" style="zoom:67%;" />

##### 设置条件断点（一般用在循环的位置）

```shell
b/break 21 if i=3	
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807181559788.png" alt="image-20220807181559788" style="zoom:50%;" />

##### 运行GDB程序

```shell
start   #（程序停在第一行）
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807174931575.png" alt="image-20220807174931575" style="zoom:67%;" />

```shell
run  #遇到断点才停
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807175212932.png" alt="image-20220807175212932" style="zoom:50%;" />

##### 继续运行，到下一个断点停

```shell
c/continue
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807175226084.png" alt="image-20220807175226084" style="zoom:67%;" />

##### 向下执行一行代码（不会进入函数体）

```shell
n/next
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807180432407.png" alt="image-20220807180432407" style="zoom:67%;" />

##### 变量操作

```shell
p/print  #变量名（打印变量值）
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807175610797.png" alt="image-20220807175610797" style="zoom:67%;" />

```shell
ptype 	#变量名（打印变量类型）
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807175632695.png" alt="image-20220807175632695" style="zoom:67%;" />

##### 向下单步调试（遇到函数进入函数体）

```shell
s/step
finish  #跳出函数体,函数体程序下面的步骤不能有断点
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807180611250.png" alt="image-20220807180611250" style="zoom:67%;" />

##### 自动变量操作

```shell
display 变量名			#（自动打印指定变量的值）
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807181017118.png" alt="image-20220807181017118" style="zoom:67%;" />

```shell
i/info display		#查看设置的自动变量
undisplay 编号		#删除自动变量
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807181214619.png" alt="image-20220807181214619" style="zoom:67%;" />

##### 其它操作

```shell
set var 变量名=变量值 	#（循环中用的较多）
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807181656235.png" alt="image-20220807181656235" style="zoom:67%;" />

```shell
until 		#（跳出循环,循环内部不能有断点，当前循环代码要执行完））
```

<img src="https://gitee.com/czjaixuexi/typora_pictures/raw/master/img/image-20220807180155796.png" alt="image-20220807180155796" style="zoom: 50%;" />
