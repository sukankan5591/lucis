# lucis
Reverse Control

# Introduction
这是一个远程shell应用,与ssh类似,但不同.在某些场景中,比如您的目标主机在私有网络(例如家庭或公司内部网络)
之内或于防火墙之后,您无法使用传统的ssh服务,那么本应用或许能提供些帮助.传统的ssh服务是客户端输入字符命令,
然后服务器端负责解释执行并将结果返回,而本应用是在服务器端输入命令,客户端执行并返回结果.

# Install 
```
    $ git clone https://github.com/sukankan5591/lucis.git
    $ cd lucis
    $ make
```

# Usage
    运行每个程序使用-h选项，获取帮助信息.

# Example
以多IP主机为例:  
服务器IP:  
&emsp;&emsp; A.A.A.A  
&emsp;&emsp; B.B.B.B  
客户端IP:  
&emsp;&emsp; C.C.C.C  
&emsp;&emsp; D.D.D.D  

首先运行服务器端(本端要能够被客户端连接,例如和客户端属同一局域网或拥有公网IP)并指定监听一个网络端口:

**land -p 8000**  (如果端口号小于1024需要有相应权限)  
此时land程序监听主机的 **8000** 端口,从任意IP访问的连接都会接受，如果只想要监听一个IP:  
**land -l A.A.A.A -p 8000**  (或指定另一个IP)  

然后运行客户端:  
**halo -s A.A.A.A -p 8000**  
如果客户端主机有多个IP,可指定使用的本机IP或绑定的端口,  
也可同时都指定(默认使用主机默认路由IP):  
**halo -l C.C.C.C -s A.A.A.A -p 8000**    (指定使用本机的C.C.C.C地址)  
**halo -t 49152   -s A.A.A.A -p 8000**    (指定使用本机的49152端口)  
**halo -l C.C.C.C -t 49152 -s A.A.A.A -p 8000**   (同时指定使用本机的C.C.C.C地址和49152端口)  

# BUG
+ 子进程运行的shell以有效用户ID从用户信息中获取(**see getpwuid(3)**)，如果用户不支持shell( **/usr/sbin/nologin** or **/bin/false** )，运行不会成功  
+ 使用本程序登录的系统使用***who***或***w***命令无法被查看到  

# Change Log
1.0.0  (2020-06-23)  
第一个可用版本发布   
1.1.0  (2020-06-28)  
添加子Shell若干环境变量  
1.2.0  (2020-07-03)  
支持域名解析 (随机选取,使用可连接的第一个IP，且不保证顺序)   
