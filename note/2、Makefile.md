## Makefile

#### 什么是Makefile

Makefile 文件定义了一系列的规则来指定那些文件需要先编译，那些文件需要后编译，哪些文件需要重新编译，甚至于更复杂的操作。
Makefile带来的好处就是自动化编译，一旦写好，只需要一个make命令，整个工程就完全自动编译。make是一个命令工具，解释Makefile文件中指令的命令工具
大多数IDE自带make,Linux下的GNU就也有

#### 文件命名和规则

**命名：**

makefile或者Makefile

**规则:**

一个Makefile文件可以有一个或多个规则

1. ```makefile
   目标...:依赖...
   	命令（shell命令）
   	...
   目标：最终要生成的文件（伪目标除外）
   依赖：生成目标所需要的文件或是目标
   命令:通过执行命令对依赖操作生成目标（命令前必须TAB缩进）
       
   例如：
       app:sub.c add.c main.c
   		gcc sub.c add.c main.c -o app
   ```

2. Makefile 中的其他规则一般都是为第一条规则服务的

    

3. 命令在执行之前，需要先检查规则中的依赖是否存在

   - 如果存在，执行命令

   - 如果不存在，向下检查其它的规则，检查有没有一个规则是用来生成这个依赖的，

   - 如果找到了，则执行该规则中的命令

     ```makefile
     app:sub.o add.o main.o
     	gcc sub.o add.o main.o -o app
     sub.o:sub.c
     	gcc -c sub.c -o sub.o
     add.o:add.c
     	gcc -c add.c -o add.o
     main.o:main.c
     	gcc -c main.c -o main.o
     #注意：这么写比上一个写法要好，否则单个更新了就要全部重新编译
     ```

     

4. 检测更新，在执行规则中的命令时，会比较目标和依赖文件的时间

   - 如果依赖的时间比目标的时间晚，需要重新生成目标

   - 如果依赖的时间比目标的时间早，目标不需要更新，对应规则中的命令不需要被执行

#### 变量

##### 自定义变量

变量名=变量值

```makefile
var=hello    				
```

##### 预定义变量

`AR:`归档维护程序的名称，默认值为ar
`CC:`C编译器的名称，默认值为cc
`CXX:`C++编译器的名称，默认值为g++

下面三个自动变量，**只能在规则的命令中使用**

`$@:`目标的完整名称
`$<:`第一个依赖文件的名称
`$^:`所有的依赖文件

```makefile
app:main.c a.c b.c
	$(CC) -c $^ -o $@
```

##### 获取变量的值

```makefile
$(变量名）
```

通过上述知识对Makefile文件进行简化

```makefile
src=sub.o add.o main.o
target=app
$(target):$(src)
        $(CC) $(src) -o $(target)
sub.o:sub.c
        gcc -c sub.c -o sub.o
add.o:add.c
        gcc -c add.c -o add.o
main.o:main.c
        gcc -c main.c -o main.o
```

#### 模式匹配

虽然Makefile通过自定义变量的方式简化了，但是下面的部分还是显得复杂

```makefile
sub.o:sub.c
        gcc -c sub.c -o sub.o
add.o:add.c
        gcc -c add.c -o add.o
main.o:main.c
        gcc -c main.c -o main.o
```

我们可以采用模式匹配的方式继续简化Makefile文件

```makefile
%.o:%.c
	$(CC) -c $< -o $@
```

`%`通配符匹配字符串，两个`%`匹配的是同一个字符串

```makefile
src=sub.o add.o main.o
target=app
$(target):$(src)
        $(CC) $(src) -o $(target)
%.o:%.c
        $(CC) -c $< -o $@
```

如果第一个规则的依赖没有被生成，那么会通过下一个规则的模式匹配找到生成依赖的规则

#### 函数

Makefile文件通过通配符得到了进一步优化，但是有个问题，如果`.o`文件太多了，那么src就会很长，有没有办法解决这点的呢？

我们可以使用函数来对他进行优化



##### 函数1

```makefile
$(wildcard *.c ./sub/*.c)
```

`功能：`获取指定目录下指定类型的文件列表
`参数：`PATTERN指的是某个或多个目录下的对应的某种类型的文件，如果有多个目录，一般使用空格间隔
`返回:`得到若干个文件的文件列表，文件名之间使用空格间隔

```makefile
$(wildcard *.c ./sub/*.c)   #返回值:a.c b.c c.c d.c e.c f.c
```

优化后的Makefile文件:

```makefile
src=$(wildcard ./*.c)
target=app
$(target):$(src)
        $(CC) $(src) -o $(target)
%.o:%.c
        $(CC) -c $< -o $@
```







##### 函数2

```makefile
$(patsubst <pattern>,<replacement>,<text>)
```

`功能`查找`<text>`中的单词(单词以“空格”、“TAB”、“回音“、“换行”分隔）是否符合模式`<pattern>`，如果匹配就用`<replacement>`替换

`<pattern>`可以包括通配符%表示任意长度字符串,可以\来转化转义字符

`返回`函数返回替换后的字符串

```makefile
$(patsubst %.c,%.o，x.c bar.c)  #返回值x.o bar.o
```

```makefile
src=$(wildcard ./*.c)
objs=$(patsubst %.c, %.o, $(src))
target=app
$(target):$(objs)
        $(CC) $(objs) -o $(target)
%.o:%.c
        $(CC) -c $< -o $@
.PHONY:clean
clean:
        rm $(objs) -f
```

使用`make clean`命令执行删除.o文件的命令, 会提示make clean已最新无法执行清除指令.

**原因:**

​	clean后没有依赖文件，会默认clean一直是最新的。

**解决方法：**

​	在clean上一行添加`.PHONY:clean` 保证make clean指令能够执行。