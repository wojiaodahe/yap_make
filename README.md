# yap_make
## 说明: yap_make 为 yap 的makefile版本,内容与yap保持一至,使用make命令编译
## 编译,清除及运行方法:
### 1 make 编译代码并生成yap, yap.bin, yap.dis及各个.c的依赖文件(xxx/xxx.d)
### 2 make clean清除工程
### 3 执行--暂时没有引导程序,需要gdb调试和执行,按以下步骤执行
#### a 开发板通过jlink链接电脑
#### b 打开j-link gdb server, 去掉localhost only前的勾,并重启j-link gdb server
#### c 执行gdb命令: arm-linux-gdb -x run.init yap 
