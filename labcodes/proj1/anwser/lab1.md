[toc]

# Lab1

## 背景知识

## BIOS

内存分为两部分：RAM，ROM，其中ROM断电之后数据不会消失，BIOS就存储在这里，起始地址为0xFFFFFFF0。

![image-20221111105522530](https://raw.githubusercontent.com/MakerFace/images/main/image-20221111105522530.png)

BIOS系统调用：（只能在实模式下使用）

- `int 10h`：显示字符
- `int 13h`：磁盘扇区读写
- `int 15h`：检测内存大小
- `int 16h`：键盘输入

## bootloader

绝大多数计算机加电启动过程：会到一个特殊地址开始执行，这个地址存放了**系统初始化软件**，通过这个软件的基本初始化部分，可以初始化硬件、建立系统的**内存空间映射图**，最后软件的引导部分把OS内核映像加载到RAM中，并转移**系统控制权**给OS。

i386的系统初始化软件由固件中的BIOS和磁盘中的Boot Loader组成(ucore中的bootasm.S和bootmain.c)。

i386加电后，处理器处于实模式，从物理地址`0xFFFFFFF0`开始执行。初始化状态的CS和EIP确定了处理器的初始执行地址，CS可见部分(16bits)地址为`0xF000`，不可见部分-基地址为`0xFFFF0000`;EIP=`0xFFF0`，这样实际的[线性地址](#逻辑地址转换为线性地址)为CS.base+EIP=`0xFFFFFFF0`。这个地址只存放了一条ljmp指令，用于跳转到**BIOS程序起始点**[0xFFFF0000, 0xE05B]（固件不需要load到ram中吗？不调入RAM就可以直接被CPU执行）。

生成镜像

```makefile
ucore.img: $(kernel) $(bootblock)
	dd if=/dev/zero of=$@ count=10000
	dd if=$(bootblock) of=$@ conv=notrunc
	dd if=$(kernel) of=$@ seek=1 conv=notrunc
```

### 练习一、理解makefile执行过程

> 列出本实验各练习中对应的OS原理的知识点，并说明本实验中的实现部分如何对应和体现了原理中的基本概念和关键知识点。
>
> 在此练习中，大家需要通过静态分析代码来了解：
>
> 1. 操作系统镜像文件ucore.img是如何一步一步生成的？(需要比较详细地解释Makefile中每一条相关命令和命令参数的含义，以及说明命令导致的结果)
> 2. 一个被系统认为是符合规范的硬盘主引导扇区的特征是什么？
>
> 补充材料：
>
> 如何调试Makefile
>
> 当执行make时，一般只会显示输出，不会显示make到底执行了哪些命令。
>
> 如想了解make执行了哪些命令，可以执行：
>
> ```
> $ make "V="
> ```
>
> 要获取更多有关make的信息，可上网查询，并请执行
>
> ```
> $ man make
> ```

1. ucore.img生成过程

   

2. MBR特征

   sign为什么要给bootloader最后两个字节赋值为`0x55`和`0xAA`。[MBR](https://zh.wikipedia.org/wiki/%E4%B8%BB%E5%BC%95%E5%AF%BC%E8%AE%B0%E5%BD%95)

   ![image-20221109115537224](https://raw.githubusercontent.com/MakerFace/images/main/image-20221109115537224.png)

   一个bootloader最大可用字节数是446，并且尾部有0x55AA标志。

### 练习二、使用qemu执行并调试lab1中的软件

>为了熟悉使用qemu和gdb进行的调试工作，我们进行如下的小练习：
>
>1. 从CPU加电后执行的第一条指令开始，单步跟踪BIOS的执行。
>2. 在初始化位置0x7c00设置实地址断点,测试断点正常。
>3. 从0x7c00开始跟踪代码运行,将单步跟踪反汇编得到的代码与bootasm.S和 bootblock.asm进行比较。
>4. 自己找一个bootloader或内核中的代码位置，设置断点并进行测试。
>
>> 提示：参考附录“启动后第一条执行的指令”，可了解更详细的解释，以及如何单步调试和查看BIOS代码。
>
>> 提示：查看 labcodes_answer/lab1_result/tools/lab1init 文件，用如下命令试试如何调试bootloader第一条指令：
>
>> ```
>> $ cd labcodes_answer/lab1_result/
>> $ make lab1-mon
>> ```
>
>补充材料： 我们主要通过硬件模拟器qemu来进行各种实验。在实验的过程中我们可能会遇上各种各样的问题，调试是必要的。qemu支持使用gdb进行的强大而方便的调试。所以用好qemu和gdb是完成各种实验的基本要素。
>
>默认的gdb需要进行一些额外的配置才进行qemu的调试任务。qemu和gdb之间使用**网络端口1234**进行通讯。在打开qemu进行模拟之后，执行gdb并输入
>
>```
>target remote localhost:1234
>```
>
>即可连接qemu，此时qemu会进入停止状态，听从gdb的命令。
>
>另外，我们可能需要qemu在一开始便进入等待模式，则我们不再使用make qemu开始系统的运行，而使用make debug来完成这项工作。这样qemu便不会在gdb尚未连接的时候擅自运行了。
>
>***gdb的地址断点***
>
>在gdb命令行中，使用b *[地址]便可以在指定内存地址设置断点，当qemu中的cpu执行到指定地址时，便会将控制权交给gdb。
>
>***关于代码的反汇编***
>
>有可能gdb无法正确获取当前qemu执行的汇编指令，通过如下配置可以在每次gdb命令行前强制反汇编当前的指令，在gdb命令行或配置文件中添加：
>
>```
>define hook-stop
>x/i $pc
>end
>```
>
>即可
>
>***gdb的单步命令***
>
>在gdb中，有next, nexti, step, stepi等指令来单步调试程序，他们功能各不相同，区别在于单步的“跨度”上。
>
>```
>next 单步到程序源代码的下一行，不进入函数。
>nexti 单步一条机器指令，不进入函数。
>step 单步到下一个不同的源代码行（包括进入函数）。
>stepi 单步一条机器指令。
>```

## qemu-monitor

qemu启动

使用`qemu -S`选项freeze处理器，不执行任何指令，获得i386启动后执行第一条指令前寄存器的内容，同时进入monitor执行qemu的指令，使用shell的标准输入输出。

```bash
qemu -S -vnc :1 -monitor stdio
```

> QEMU 7.1.0 monitor - type 'help' for more information
> (qemu) info registers
>
> EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000663
> ESI=00000000 EDI=00000000 EBP=00000000 ESP=00000000
> EIP=0000fff0 EFL=00000002 [-------] CPL=0 II=0 A20=1 SMM=0 HLT=0
> ES =0000 00000000 0000ffff 00009300
> CS =f000 ffff0000 0000ffff 00009b00

BIOS做完硬件自检和初始化后，会选择一个启动设备，读取该设备的第一扇区（主引导分区MBR）到内存特定的地址**0x7C00**（为什么是0x7C00见后面的编译错误）处，然后CPU转移到这个地址继续执行bootloader的指令；bootloader将CPU从实模式切换到保护模式，从硬盘上读取ucore。

![image-20221101172737915](https://raw.githubusercontent.com/MakerFace/images/main/image-20221101172737915.png)

---

调试bootloader

```bash
# 载入os映像
qemu -S -s -d in_asm -D bin/q.log -monitor stdio -hda bin/ucore.img -serial file:qemu.out
# 开启另一个终端
gdb -q -tui file bin/kernel
target remote :1234
set architecture i8086  # bios阶段是i8086模式
break *0x7C00 #在0x7C00处停止
continue
x /2i $pc # /2i显示两条指令，$pc在pc处
# or
x 0x7C00 # 执行cli屏蔽终端，cld清除方向标志
nexti/ni # 步进下一条机器指令
p/x *(struct elfhdr *)0x10000
$1 = {e_magic = 0x0, e_elf = {0x0 <repeats 12 times>}, e_type = 0x0, e_machine = 0x0,
  e_version = 0x0, e_entry = 0x0, e_phoff = 0x0, e_shoff = 0x0, e_flags = 0x0,
  e_ehsize = 0x0, e_phentsize = 0x0, e_phnum = 0x0, e_shentsize = 0x0, e_shnum = 0x0,
  e_shstrndx = 0x0}
# 如果出现被优化的值，加volatile解决。
```

---

### 练习三、分析bootloader进入保护模式的过程

#### 启用A20：16位模式，ax,bx等可用

> 未启用A20，只有20根地址总线，CPU数据总线16位，4位偏移，可以表示2^20^，即1M内存。
>
> 在i80386中，一共有32位总线，通过添加A20（向下兼容），控制实模式不可以访问超过1M的地址空间（超过则回卷地址），**进入保护模式前**解除A20就可以访问全部地址空间。
>
> 实际上A20就是第21根线，用来控制是否允许对0x10FFEF以上的实际内存寻址，也叫A20 Gate。
>
> A20 Gate实现方式是与键盘控制器的一个输出做AND操作，来控制A20线打开和关闭，通过系统软件的IO操作打开它。

> **[8042控制器](https://chyyuu.gitbooks.io/ucore_os_docs/content/lab1/lab1_appendix_a20.html)**
>
> 键盘控制器8042的逻辑结构图
>
> P2端口用于特殊用途，位0(P20)用于实现CPU复位，位1(P21)用于控制A20开启关闭。系统向输出缓存0x64写入一个字节（发送一个键盘控制器命令），可以带一个参数，通过0x60发送，命令的返回值也从0x60端口读取。
>
> 读0x60端口，读输出缓存，只读
>
> 写0x60端口，写输入缓存，只写
>
> 读0x64端口，读状态寄存器，只读
>
> 操作控制寄存器，先要向0x64端口写一个命令（0x20为读命令，0x60为写命令），然后向0x60端口读取控制寄存器的数据或者写入数据。

1. seta20.1段

   读取8042端口状态：空闲则向0x64写入`0xd1`命令，表示要向P2端口写数据。

2. seta20.2段

   读取8042端口状态：空闲则向0x60写入`0xdf`，要写入P2端口的数据为`11011111`，表示把A20的第1个bit置1。

---

#### [加载GDT表](https://www.ics.uci.edu/~aburtsev/143A/2017fall/lectures/lecture07-system-boot/lecture07-system-boot.pdf)

![image-20221110191902847](https://raw.githubusercontent.com/MakerFace/images/main/image-20221110191902847.png)

![image-20221110192050047](https://raw.githubusercontent.com/MakerFace/images/main/image-20221110192050047.png)

> 使用引导GDT表，使虚拟地址直接映射到物理地址，保证在转换期间有效内存映射不会改变。
>
> 为什么要直接使用虚拟地址呢？
>
> 加载后的GDT表存放在内存中，其地址在GDTR寄存器中。

```assembly
# GDT表的定义
.p2align 2 # force 4 byte alignment
gdt:
  SEG_NULLASM # null seg
  SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff) # code seg：STAX|STAR是访问权限，可读可执行，起止地址0~4GB
  SEG_ASM(STA_W, 0x0, 0xffffffff) # data seg：STAW可写权限，起始地址0~终止地址4GB

gdtdesc:
  .word (gdtdesc - gdt -1) # sizeof(gdt)-1
  .long gdt
```

![image-20221110214538333](https://raw.githubusercontent.com/MakerFace/images/main/image-20221110214538333.png)

---

#### 切换保护模式

i08386有一组控制寄存器：CR0~CR3共4个，用来保存全局性和任务无关的机器状态。与分页机制有关。

CR0包含6个预定义标识：0位是保护允许位PE，用于启动保护模式；CR0第31位是分页启用位。

```assembly
movl %cr0, %eax
orl $CR0_PE_ON, %eax
movl %eax, %cr0
```

就完成了保护模式的切换。

---

**使用ljmp执行长跳转**到32位代码模式中。[虽然已经把CPU切换成保护模式了，但是当前执行的代码仍然处于16位实模式下，当前代码的段地址和段偏移地址都是16位的，通过ljmp做一个跳转，会切换段地址，代码才会真正进入保护模式。](https://www.zhihu.com/question/378576453/answer/1071731242)

用ljmp改变代码段（CS代码段寄存器）

```assembly
ljmp $(SEC_KCODE<<3), $start32 # 实际跳转到 ljmp 0xb866, 0x87c32？
```

> CS代码段为什么是0x8？！！！！所有的段寄存器都是和段选择子一样的格式，因为每个段的构成是一样的，基址+特权级别，不只是CS是0x8，其它DS(data segment)、SS(stack segment)、ES, FS, GS(extra data segment)都是一样的。此外0x80=1000~(2)~，表示后面全是0，即此段位于GDT中，特权级别为最高。
>
> 段选择子可以显示指定也可以隐式指定，最常用的方式是把它加载都段寄存器中然后由CPU隐式选择寄存器，隐式选择依赖于将要执行的操作，如[下表所示](https://cse.unl.edu/~goddard/Courses/CSCE351/IntelArchitecture/IntelDataType.pdf)：
>
> ![image-20221122205053248](https://raw.githubusercontent.com/MakerFace/images/main/image-20221122205053248.png)
>
> [保护模式下的段寄存器由：16位(0-15)的段选择器和64位的段描述符寄存器组成](https://doctording.github.io/os/content/03_protected_mode/)。
>
> ![image-20221110202843609](https://raw.githubusercontent.com/MakerFace/images/main/image-20221110202843609.png)
>
> - 描述符索引：用来在描述符表中选择一个段描述符（即GDT中的偏移），2^13^=8192个段。
> - 描述符表指示器TI：为0表示在GDT表中，1表示在LDT中。
> - 请求特权级别：4个级别
>
> 段访问提供保护机制，在描述符中规定了访问权限，在保护模式下，一个段由[访问权限，基址，限长]组成，它们加在一起放在一个64位的数据结构中，即**段描述符**：
>
> ![image-20221110204710435](https://raw.githubusercontent.com/MakerFace/images/main/image-20221110204710435.png)
>
> 1. 基址分散在64位中，整体是32位的，保证可以访问4GB内存
> 2. 段限长分散在64位中，整体是20位的，当颗粒度G=0时，单位是B，即limit范围从1B到1MB；当G=1时，单位是4KB，limit范围从4KB到4GB，确保可以访问4GB内存。
> 3. GDTR寄存器48位，32位描述GDT在内存的起始地址；**16位限长**，每个段描述符都是8字节，2^13^=8192个段。
>
> 段机制：利用页机制实现分段（lab2）。
>
> ![image-20221116114645129](https://raw.githubusercontent.com/MakerFace/images/main/image-20221116114645129.png)
>
> <span name="逻辑地址转换为线性地址">逻辑地址转换为线性地址</span>
>
> ![image-20221116114946411](https://raw.githubusercontent.com/MakerFace/images/main/image-20221116114946411.png)
>
> 段选择子就是Index，用来查找段描述符表中的表项，即段描述符，存放了段的起始地址和大小，通过段起始地址与偏移量(==是EIP吗?==)之和可以得到线性地址。
>
> > 如果段选择器初始化为0x8，则：
> >
> > 段选择器为：0000 0000 0000 1000
> >
> > 描述符索引：0000 0000 0000 1
> >
> > TI: 0
> >
> > RPL: 00
> >
> > 假设第一个段描述符的基址为`0x1234`，要访问`ds:0x56`的内存，最后访问的就是`0x128A`。
>
> ==注意：如果段选择器忘记初始化，那么将访问第0个描述符，但是第0个描述符在段定义中定义为空段，若选择了第0段，处理器会报错。==

---

### 练习四、分析bootloader加载ELF格式的OS过程

调用C程序bootmain.c，读取kernel。

> 补充：`endbr32`, `endbr64`是[intel为控制流实施技术新加的指令](https://segmentfault.com/q/1010000021706812###)，该指令不执行任何操作，只是验证目标地址是期望的跳转地址。要求相对跳转地址的目标地址一定是一条`endbr32`或`endbr64`指令，否则就会异常。

重置DS、ES、FS、GS、SS寄存器为0x10。

> 为什么DS是0x10？
>
> 32位数据段寄存器，从GDT的0x10偏移开始。

调用bootmain函数：

> 当执行push ebp指令后：
>
> ![image-20221110221011372](https://raw.githubusercontent.com/MakerFace/images/main/image-20221110221011372.png)
>
> 变化的是ESP栈顶指针寄存器，减少4。

1. 从磁盘读第一页(ELF header)到内存va(0x10000)中，读取大小为4KB(8*sector_size)。

2. [读一个扇区](https://www.zengl.com/a/201407/165.html)（IDE/ATA磁盘，两种读取方式：CHS(cylinder,head,sector)，LBA(logical block addressing)。[wiki](https://wiki.osdev.org/ATA_read/write_sectors)）

   CHS转LBA：
   $$
   lba=(c*H+h)*S+s-1
   $$
   LBA转CHS：
   $$
   c=lba/(S*H)\\
   h=(lba/S)\mod H\\
   s=(lba\mod s)+1
   $$
   其中c是柱面号，h是磁头号，s是扇区号。H是每个柱面的磁头数，S是每个磁道的扇区数。

   > [命令/状态返回值](https://wiki.osdev.org/PCI_IDE_Controller#Status)

   ```c
   /* waitdisk - wait for disk ready */
   static void waitdisk(void) {
     //? why read 0x1f7 port? why not 0x1f0
     //* 0x1F7=BAR0基地址0x1f0+STATUS偏移0x07
     //? why and 1100 0000? sr_ready is 0100 0000
     //* 返回8个状态，每个状态用1位**比特掩码**表示。
     //* 用0xC0与，就是取前两位，不可能同时出现两个1。
     //? 那么为什么不直接与0x20？判等或与
     // while ((inb(0x1F7) & 0xC0) != ATA_SR_DRDY) /* do nothing */;
     while (!(inb(ATA_BAR0 + ATA_REG_STATUS) & ATA_SR_DRDY))
       ; /* wait ready */
   }
   ```

   ```c
   /* readsect - read a single sector at @secno into @dst */
   static void readsect(void *dst, uint32_t secno) {
     // wait for disk to be ready
     waitdisk();
   
     /*Bit 0: When set, the primary channel is in PCI native mode. When clear, the
      * primary channel is in compatibility mode (ports 0x1F0-0x1F7, 0x3F6,
      * IRQ14).*/
     //? https://ceunican.github.io/aos/36.IO_Devices.pdf 第26页
     //* outb(0x3F6, 0); //产生中断，为什么不执行这个？
     outb(ATA_BAR0 + ATA_REG_SECCOUNT0, 1);                // count = 1
     outb(ATA_BAR0 + ATA_REG_LBA0, secno & 0xFF);          // LBA低位
     outb(ATA_BAR0 + ATA_REG_LBA1, (secno >> 8) & 0xFF);   // LBA中位
     outb(ATA_BAR0 + ATA_REG_LBA2, (secno >> 16) & 0xFF);  // LBA高位
     //* 读写，用于选择通道中的驱动器
     outb(ATA_BAR0 + ATA_REG_HDDEVSEL,
          //?  23~26位，为什么只取低8位，或有用吗？
          ((secno >> 24) & 0xF) | 0xE0);  //?
     outb(ATA_BAR0 + ATA_REG_COMMAND,
          ATA_CMD_READ_PIO);  // cmd 0x20 - read sectors
   
     // wait for disk to be ready
     waitdisk();
   
     // read a sector
     //* 这里除以4是因为insl以双字节为单位进行IO
     insl(ATA_BAR0, dst, SECTSIZE / 4);
   }
   ```

   内嵌汇编：insl指令：

   ```c
   /**
    * @brief 从EDX指定的IO端口读入双字，输入ES:EDI寄存器指定的内存地址
    * @param  port             放入EDX中，数据读取端口
    * @param  addr             放入EDI中，数据目的地址
    * @param  cnt              放入ECX中，重复cnt次
    */
   static inline void
   insl(uint32_t port, void* addr, int cnt){
       "cld;"
       "repne; insl;"
       : "=D" (addr), "=c" (cnt)
       : "d" (port), "0" (addr), "1" (cnt)
       : "memory", "cc");
   }
   ```

   1. cld清空标志寄存器DF的值，使SI、DI寄存器自动增加（SI存放数据源地址，DI存放数据目的地址）
   2. repne是重复前一条的指令，重复次数是ecx次。
   3. `”0“(addr)`表示使用之前的EDI寄存器，cnt使用ecx寄存器。

   内联汇编样例2：把CR0寄存器的高位置1

   ```c
   static inline void encro () {
       uint32_t cr0;
       asm volatile ("movl %%cr0, %0\n" : "=r"(cr0)); // 读取cr0寄存器的值到cr0变量中
       cr0 |=0x80000000;
       asm volatile ("movl %0, %%cr0\n" : : "r"(cr0)); // 把cr0变量的值存入cr0寄存器
   }
   ```

   1. r表示可以使用任意寄存器。

   > 寄存器约束：
   >
   > a = eax, b = ebx, c = ecx, d = edx, S = esi, D = edi, 0 = 和之前用的寄存器一样。

3. 检查是否是有效的ELF文件(具体值可以用`readelf -h`查看)

   只需要检查前4个字节是否为`{0x7f, 'E', 'L', 'F'}`。

4. 从磁盘中读取每个程序段放到对应的虚拟地址中

   > p/x *0x10010
   >
   > $8 = 0x30002 # type=0x02，即可执行文件，machine=0x03，即intel 80386
   >
   > p/x *0x10018
   >
   > $11 = 0x100000  # 函数入口地址，通过x/10i 0x100000可查看对应指令
   >
   > p/x *0x1001C
   >
   > $12 = 0x34 # 程序头表在ELF头中的偏移位置？
   >
   > p/x *0x10028
   >
   > $13 = 0x200034 # 低16位表示elf头的总大小，高16位表示入口程序头的总大小0x20，即32字节
   >
   > p/x *0x1002C
   >
   > $14 = 0x280003 # 低16位表示程序头数量，高16位表示段头的数量

   entry为什么取低24位？

5. 调用kernel的入口函数(e_entry)

### 练习五、实现函数调用堆栈跟踪函数

> 我们需要在lab1中完成kdebug.c中函数print_stackframe的实现，可以通过函数print_stackframe来跟踪函数调用堆栈中记录的返回地址。在如果能够正确实现此函数，可在lab1中执行 “make qemu”后，在qemu模拟器中得到类似如下的输出：
>
> ```
> ……
> ebp:0x00007b28 eip:0x00100992 args:0x00010094 0x00010094 0x00007b58 0x00100096
>  kern/debug/kdebug.c:305: print_stackframe+22
> ebp:0x00007b38 eip:0x00100c79 args:0x00000000 0x00000000 0x00000000 0x00007ba8
>  kern/debug/kmonitor.c:125: mon_backtrace+10
> ebp:0x00007b58 eip:0x00100096 args:0x00000000 0x00007b80 0xffff0000 0x00007b84
>  kern/init/init.c:48: grade_backtrace2+33
> ebp:0x00007b78 eip:0x001000bf args:0x00000000 0xffff0000 0x00007ba4 0x00000029
>  kern/init/init.c:53: grade_backtrace1+38
> ebp:0x00007b98 eip:0x001000dd args:0x00000000 0x00100000 0xffff0000 0x0000001d
>  kern/init/init.c:58: grade_backtrace0+23
> ebp:0x00007bb8 eip:0x00100102 args:0x0010353c 0x00103520 0x00001308 0x00000000
>  kern/init/init.c:63: grade_backtrace+34
> ebp:0x00007be8 eip:0x00100059 args:0x00000000 0x00000000 0x00000000 0x00007c53
>  kern/init/init.c:28: kern_init+88
> ebp:0x00007bf8 eip:0x00007d73 args:0xc031fcfa 0xc08ed88e 0x64e4d08e 0xfa7502a8
> <unknow>: -- 0x00007d72 –
> ……
> ```
>
> 请完成实验，看看输出是否与上述显示大致一致，并解释最后一行各个数值的含义。
>
> 提示：可阅读小节“函数堆栈”，了解编译器如何建立函数调用关系的。在完成lab1编译后，查看lab1/obj/bootblock.asm，了解bootloader源码与机器码的语句和地址等的对应关系；查看lab1/obj/kernel.asm，了解 ucore OS源码与机器码的语句和地址等的对应关系。
>
> 要求完成函数kern/debug/kdebug.c::print_stackframe的实现，提交改进后源代码包（可以编译执行），并在实验报告中简要说明实现过程，并写出对上述问题的回答。
>
> 补充材料：
>
> 由于显示完整的栈结构需要解析内核文件中的调试符号，较为复杂和繁琐。代码中有一些辅助函数可以使用。例如可以通过调用print_debuginfo函数完成查找对应函数名并打印至屏幕的功能。具体可以参见kdebug.c代码中的注释。

#### 问题一、如何退栈

> 如何使用当前栈帧保存的旧ebp退栈？即如何恢复ebp、eip？
>
> > eip=*(ebp+4)
> >
> > ebp=*ebp
>
> print_debuginfo(eip-1)不能正确打印函数栈帧。
>
> > 重新编译后可以用了？**是Clang的问题**，使用GCC可以打印函数名和行号。
>
> 何时停止打印？即如何判断栈帧已经为空？
>
> > 遇到输出unknown停止，那么0x00007d72是什么意思呢？猜测应该是bootloader调用kern_init函数的指令。unknown是指不在源码中的指令。

```c
 /* LAB1 YOUR CODE : STEP 1 */
 /* (1) call read_ebp() to get the value of ebp. the type is (uint32_t);
  * (2) call read_eip() to get the value of eip. the type is (uint32_t);
  * (3) from 0 .. STACKFRAME_DEPTH
  *    (3.1) printf value of ebp, eip
  *    (3.2) (uint32_t)calling arguments [0..4] = the contents in address (unit32_t)ebp +2 [0..4]
  *    (3.3) cprintf("\n");
  *    (3.4) call print_debuginfo(eip-1) to print the C calling function name and line number, etc.
  *    (3.5) popup a calling stackframe
  *           NOTICE: the calling funciton's return addr eip  = ss:[ebp+4]
  *                   the calling funciton's ebp = ss:[ebp]
  */
 uint32_t ebp = read_ebp();
 uint32_t eip = read_eip();
 uint32_t argc = 4;
 for (int i = 0; i < STACKFRAME_DEPTH; ++i) {
   cprintf("ebp:0x%08x eip:0x%08x args:", ebp, eip);
   for (int j = 0; j < argc; ++j) { // TODO 怎么确定函数变量个数？
     // cprintf("0x%08x ", *(uint32_t *)(ebp + 2 * j)); // ERROR 参数获取有问题，应该是((ebp+2)+j*4)
        cprintf("0x%08x ", *(uint32_t *)(ebp + 2 + 4 * j));
   }
   cprintf("\n");
   if (print_debuginfo(eip - 1) != 0) break;
   eip = *(uintptr_t *)(ebp + 4);
   ebp = *(uintptr_t *)(ebp);
 }
```

![image-20221115163249997](https://raw.githubusercontent.com/MakerFace/images/main/image-20221115163249997.png)

> 最后一行各个数值的意思：
>
> ebp:0x7bf8是bootloader的bootmain.c:bootmain的栈基址寄存器的值，在bootmain.c:129行执行了call kern_init，实际是函数调用`((void (*)(void))(ELFHDR->e_entry & 0xFFFFFF))();`导致的ljmp。
>
> 函数调用时，要把ebp压栈，同时还要把epc+1压栈，在退栈时继续执行下一条指令。

#### 问题二、如何确定参数个数

> 可以用ebp-esp的差值确定参数个数吗？
>
> > 不可以，调用函数分三步，第一步是caller，调用者；第二步是callee，被调用者；第三步caller复原寄存器，获取结果：
> >
> > ![image-20221115172503785](https://raw.githubusercontent.com/MakerFace/images/main/image-20221115172503785.png)
> >
> > 1. caller
> >
> >    - 将通用寄存器（需要的，不是所有寄存器都需要保存）压栈
> >    - 将所有参数逆序<span style="color:blue">压栈</span>：argn, ..., arg2, arg1
> >    - 返回值地址（调用者的局部变量）<span style="color:purple">压栈</span>：ret addr
> >    - `call`指令将`pc+1`压栈，并执行jmp/ljmp
> >
> >    需要注意：局部变量在任何一个函数中都是**逆序存放的**，<span style="color:green">数组内的元素</span>是正的，即低地址存放低索引元素。当参数比较多时，会利用ecx, esi寄存器交替存放参数，然后压栈。
> >
> >    ![caller](https://raw.githubusercontent.com/MakerFace/images/main/caller.png)
> >
> > 2. callee
> >
> >    - 将ebp压栈，把esp赋值给ebp
> >    - *将esp减少N个数，用来存放局部变量*
> >    - 执行函数功能
> >    - 把结果放入ret指向的内存中。
> >    - pop ebp
> >
> >    ![callee](https://raw.githubusercontent.com/MakerFace/images/main/callee.png)
> >
> > 3. caller
> >
> >    - 将esp复原，即减去参数个数和返回值个数
> >
> >    - 获取返回值
> >
> > ```c
> > #include <stdio.h>
> > typedef struct ret{
> >     volatile int ret0,ret1,ret2,ret3, ret4,ret5,ret6,ret7;
> > }ret_t;
> > 
> > ret_t foo(int a, int b, int c, int d, int e, int f, int g, int h, int i) {
> >     ret_t ret;
> >     ret.ret0 = a+b;
> >     ret.ret1 = ret.ret0+c;
> >     ret.ret2 = ret.ret1+d;
> >     ret.ret3 = ret.ret2+e;
> >     ret.ret4 = ret.ret3+f;
> >     ret.ret5 = ret.ret4+g;
> >     ret.ret6 = ret.ret5+h;
> >     ret.ret7 = ret.ret6+i;
> >   return ret;
> > }
> > 
> > int main(int argc, char* argv[]) {
> >   int nums[9];
> >   ret_t ret = foo(nums[0], nums[1], nums[2], nums[3], nums[4], nums[5], nums[6], nums[7],
> >       nums[8]);
> >   printf("%d\n", ret.ret7);
> > }
> > ```
> >
> > 对应汇编源码可以在[compiler explorer](https://godbolt.org/)查看。
> >
> > esp的复原是汇编源码实现的，通过函数调用结束后，减去开辟的临时空间值实现。
> >
> > 关于函数调用约定，详见[wiki_x86 calling conventions](https://en.wikipedia.org/wiki/X86_calling_conventions)。
>
> backtrace实现了，可以参考这个。但是很难读。实际上也不需要打印参数个数或者参数值，只需要知道运行到哪里出错了，以及当前函数调用栈是什么就行。

### 设备管理

bootloader对I/O设备进行访问，通过串口、并口、CGA(提供文字和图形显示模式，最高16色)显示字符串，读取硬盘数据，处理时钟中断等。

I/O控制器包含三个层次：I/O地址空间、I/O接口、设备控制器。每个连接到I/O总线的设备都有唯一的I/O地址空间（即I/O端口），CPU可以直接访问。支持基于I/O的I/O地址空间（IN/OUT等I/O访问指令），也支持基于内存的I/O访问（通过MOV等访存指令），访问请求通过I/O总线传递给I/O接口。

I/O接口是介于I/O端口(等同于地址空间)和设备控制器之间的硬件电路，将I/O总线上的**访问请求转换为设备可以识别的指令**；并监测设备状态，将状态和数据写回I/O地址空间。I/O接口包括：键盘、图形、磁盘、总线鼠标、网络接口、并口、串口、通用串行总线(USB)、PCMCIA和SCSI。

设备控制器负责解释从I/O接口收到的高级命令，并发送给I/O设备；对I/O设备发送的消息进行解释并修改状态寄存器。例如磁盘控制器，将读写数据指令转换为磁盘操作。

### 串口(Serial Port)访问控制

串口是一个字符设备，通过串口输出，第一步执行`inb`指令读取串口的I/O地址(COM1+COM_LSR?DSR)的值，如果读取的值表示串口正忙，则空转轮询(0x84?)；第二步如果发现读出的值表示串口处于空闲状态，则执行`outb`指令把字符写到串口I/O地址(COM1+COM_TxD)；就实现了串口输出。具体实现在`bootmain.c::serial_putc`中。

### 并口(Parallel Port)访问控制

并口也是一个字符设备，通过并口输出，第一步执行`inb`指令读取并口I/O地址(LPTPORT+1)的值，如果并口忙则空转等待；如果空闲则执行`outb`指令把字符写到并口的I/O地址(LPTPORT)，就完成了一个字符的并口输出。具体实现在`bootmain.c::lpt_putc`中。

### CGA字符显示控制

彩色图形适配器（Color Graphics Adapter, CGA）支持7种彩色和文本/图形显示，在80col*25row文本模式下又单色和16色两种显示方式。CGA标配16KB显存（地址范围`OxB8000`~`0xBC000`，共2000字），可以将之视为内存块设备，即bootloader和操作系统可以直接对显存进行写操作，完成显示。每个字符需要用两个字节表示：第一个字节表示要显示的字符，第二个字节确定前景和背景，前景用低4位，背景用4-6位，最高为用来表示这个字符是否闪烁(1=flash,0=no-flash)。

光标设置，必须通过CGA显示控制器的I/O端口，**显示控制索引寄存器**的I/O端口地址为`0x3D4`；**数据寄存器**I/O端口地址为`0x3D5`。CGA显示控制器内存有一系列寄存器可以用来访问显示器状态，通过两个I/O端口读写其内部寄存器。方法为先向`0x3D4`写入要访问的寄存器编号，再通过`0x3D5`读写寄存器数据。存放光标位置的寄存器编号为14,15，两个寄存器组合一起为一个16位整数，就是光标的位置。比如0在第0行第0列，81在第一行第一列（屏幕共有80列）。

通过CGA显示控制器进行输出：通过`in/out`指令获取当前光标位置；然后计算显存地址，直接通过访存指令完成字符输出；最后通过`in/out`指令更新光标位置。具体实现在`bootmain.c::cga_putc`中。

## Interrupt

> 中断、异常、系统调用
>
> 源头：
>
> - 中断：外设
> - 异常：程序出错
> - 系统调用：应用程序请求操作系统提供服务
>
> 软硬件中断处理：
>
> 1. 中断服务例程(ISR routine)，用中断号去标识。
>
>    ![image-20221116132910326](https://raw.githubusercontent.com/MakerFace/images/main/image-20221116132910326.png)
>
>    最主要的是段选择器和偏移量，**段选择器如何确定呢？**是GDT表的地址，也就是说中断处理例程的入口地址存放在GDT表中。
>
> 2. 中断描述符表(IDR)，起始地址和大小在IDTR寄存器中。
>
>    ![image-20221116132645103](https://raw.githubusercontent.com/MakerFace/images/main/image-20221116132645103.png)
>
>    IDT中每一项存放中断描述符（也叫中断门、陷阱门）
>
> 3. 中断过程
>
>    ![image-20221116133124256](https://raw.githubusercontent.com/MakerFace/images/main/image-20221116133124256.png)
>
>    首先根据中断号去IDT中查中断门，根据中断描述符表给出的段选择器查询GDT，找到中断服务例程段的起始地址，与偏移相加得到中断服务例程的线性地址。ucore需要提供IDT和GDT。
>
> 4. 不同特权级的中断切换对堆栈的影响
>
>    - 同一个特权级，在同一个堆栈上操作
>    - 不同特权级，还需要在内核栈中压入ESP和SS
>    - 中断服务例程要使用的堆栈段选择器和栈指针从当前进程的TSS中得到。
>
>    ![intel-sdm-vol-3](https://raw.githubusercontent.com/MakerFace/images/main/image-20221116133550337.png)
>
> 5. 中断返回：`iret`
>
>    iret弹出`EFLAGS`、`CS`和`ESP`寄存器。
>
>    ret只弹出EIP
>
>    retf只弹出CS和EIP
>
>    此外，中断服务例程还需要保存被打断进程的寄存器，并在iret前恢复这些寄存器。
>
> 初始化中断向量：
>
> IDT和GDT表。
>
> ![image-20221111112544827](https://raw.githubusercontent.com/MakerFace/images/main/image-20221111112544827.png)

### 练习六、完善中断初始化和处理

> 请完成编码工作和回答如下问题：
>
> 1. 中断描述符表（也可简称为保护模式下的中断向量表）中一个表项占多少字节？其中哪几位代表中断处理代码的入口？
> 2. 请编程完善kern/trap/trap.c中对中断向量表进行初始化的函数idt_init。在idt_init函数中，依次对所有中断入口进行初始化。使用mmu.h中的SETGATE宏，填充idt数组内容。每个中断的入口由tools/vectors.c生成，使用trap.c中声明的vectors数组即可。
> 3. 请编程完善trap.c中的中断处理函数trap，在对时钟中断进行处理的部分填写trap函数中处理时钟中断的部分，使操作系统每遇到100次时钟中断后，调用print_ticks子程序，向屏幕上打印一行文字”100 ticks”。
>
> > 【注意】除了系统调用中断(T_SYSCALL)使用陷阱门描述符且权限为用户态权限以外，其它中断均使用特权级(DPL)为０的中断门描述符，权限为内核态权限；而ucore的应用程序处于特权级３，需要采用｀int 0x80`指令操作（这种方式称为软中断，软件中断，Tra中断，在lab5会碰到）来发出系统调用请求，并要能实现从特权级３到特权级０的转换，所以系统调用中断(T_SYSCALL)所对应的中断门描述符中的特权级（DPL）需要设置为３。
>
> 要求完成问题2和问题3 提出的相关函数实现，提交改进后的源代码包（可以编译执行），并在实验报告中简要说明实现过程，并写出对问题1的回答。完成这问题2和3要求的部分代码后，运行整个系统，可以看到大约每1秒会输出一次”100 ticks”，而*按下的键也会在屏幕上显示*（==没有显示是为什么？==）。
>
> 提示：可阅读小节“中断与异常”。

1. 中断描述符表

   一个表项8B，段选择器+偏移量+其它一些标志位共64bits。

   lidt指令把一个线性地址基址和限长放入IDTR寄存器中，属于特权指令。

2. 中断描述符

   ![image-20221116200001544](https://raw.githubusercontent.com/MakerFace/images/main/image-20221116200001544.png)

   其中第二个字中的第13位D表示：0则gate的size为16bits，1则gate的size为32bits。

3. 填充IDT

   1. 获取中断向量入口地址数组

      `tools/vector.c`生成了0~255共266个中断向量，他们的地址就是中断向量在gdt表中的偏移量

   2. kernel的全局GDT表与bootloader的GDT表不一样，需要重新加载，由pmm实现。加载后的GDT基址在GD_KTEXT中存放。

   3. 对于内核状态切换

      - 从用户态切换为内核态，权限提升
      - 从内核态切换为用户态，权限降低

      具体切换在`trap_dispatch`中实现。

#### 问题一、为什么pmm_init调用`movw eax, gs`之后会异常

> 调用前gs=0x10, fs=0x10，调用后gs=0x0,fs=0x0,eax=0x0。
>
> 原因是没有加`-fno-PIC`编译指令，kernel需要生成于位置无关代码。

#### 问题二、用户态和内核态的切换硬件做了什么，需要OS做什么？

> 处理器转去执行一个中断服务例程，会将EFLAGS，CS，EIP寄存器保存到当前栈，如果异常产生错误码也会压入栈中。
>
> [cpl、rpl、dpl](https://www.cnblogs.com/pang123hui/archive/2010/11/27/2309924.html)操作系统特权等级：
>
> - CPL：当前特权级保存在代码段寄存器CS的最低两位，CPL是当前活动代码段的特权级，并且定义了当前执行程序的特权级别。
> - DPL：描述符特权级存储在段描述符中，用于描述对应段所属的特权等级，即段本身的特权级。
> - RPL：请求特权级别保存在选择子最低两位，RPL说明的是进程对应段访问的请求特权，意思是当前进程想要的请求权限。RPL由程序员自己设置，并不一定RPL>=CPL，但是当RPL<CPL时，实际起作用的就是CPL，因为访问时特权检查是判断：max(RPL,CPL)<=DPL是否成立，所以RPL可以看成访问时的附加限制，RPL=0时，附加限制最小，RPL=3时附加限制最大。
>
> 前13位是基址，中间一位为GDT/LDT指示位，最后两位为特权级别。
>
> 软中断处理通用流程：
>
> 1. 硬件保存中断现场：**CPU**将寄存器压栈，在GDT表中找到中断向量（入口地址），将错误码（没有默认为0，有的错误码需要程序生成，因此不压入）和中断号压入栈内，然后使用jmp指令跳转到__alltraps所有陷入的入口；
> 2. 软件保存中断现场并处理中断：**OS**将代码段、数据段、通用寄存器等压入栈中形成陷入帧(trapframe)，执行中断入口函数trap，继而执行trap_dispatch，调用实际中断处理程序；
> 3. 软件恢复中断现场：处理完中断后，ret返回\_\_alltraps恢复中断环境，执行iret交给硬件恢复；
> 4. 硬件恢复中断现场：复原段寄存器和指令寄存器等
>
> ![trap](https://raw.githubusercontent.com/MakerFace/images/main/image-20221117202045172.png)
>
> 其中上半部分是<span style="color:purple">硬件做的</span>，下半部分是<span style="color:green">OS做的</span>，中断的具体实现也是OS做的。
>
> CPU根据CPL和中断服务例程的段描述符的DPL信息确认是否发生了特权级的转换，如果发生用户态到内核态的转换，CPU就从当前程序的**TSS信息**中获取该程序的**内核栈地址**(内核态的ss、esp地址)，并立即将系统当前使用的栈切换成新内核的栈（硬件实现）。
>
> 切换流程：
>
> 1. [内核态切换到用户态](https://chyyuu.gitbooks.io/simple_os_book/content/zh/chapter-2/kernel_to_user.html)
>
>    > 不存在权限切换，因为切换前就是内核态，中断处理例程仍旧是内核态！需要OS实现切换堆栈。硬件只会在内核栈中压入ErrorCode、EIP、CS、EFLAGS，然后找到中断号，执行中断服务例程。
>
>    首先内核态发出0x78(120)中断；然后经过<span style="color:purple">硬件</span>和<span style="color:green">OS</span>协助，进入用户态切换的中断处理例程。
>
>    如果切换前就是用户态，则什么都不做；否则进行堆栈的切换，具体流程如下：
>
>    现将当前trapframe**复制**到<span style="color:fuchsia">全局变量switchk2u</span>(数据段中，应该在低地址处)中，改变段寄存器的值为用户空间，eflags去掉OS特权（设置IO特权位，保证用户可以使用in/out指令），设置临时栈(``tf-1`，就是最后push的esp的值，在pop esp时指向<span style="color:fuchsia">全局变量switchk2u</span>)，**指向switchk2u**，这样当执行`iret`指令后，CPU会从switchk2u恢复数据，而不是从现有栈恢复数据。
>
>    ![image-20221117205818756](https://raw.githubusercontent.com/MakerFace/images/main/image-20221117205818756.png)![kernel-to-user](https://raw.githubusercontent.com/MakerFace/images/main/kernel2user2.png)
>
>    有个疑问就是，为什么要把esp指向距离tf一个trapframe大小的地方？esp、oesp都有什么用？不是通过ebp+esp确定栈么？ebp可以小于esp吗？
>
>    1. esp增加则退栈，减少则入栈；`tf_esp=tf+sizeof(trapframe)-sizeof(tf_trapno)-sizeof(tf_err)`，tf_esp增加，则表现为退栈；这个部分退栈主要原因是CPU从switchk2u中恢复，不会从内核栈中退栈，因此必须由OS执行退栈操作。
>    2. 通过esp可以确定一个栈，因为栈中有ebp可以随时恢复。
>    3. 可以，esp可以在任何地方，前提是esp指向的地方必须保存有原来的ebp和esp的值，才能回到未中断时的栈。
>
>    调试结果分析：
>
>    ![image-20221123104254934](https://raw.githubusercontent.com/MakerFace/images/main/image-20221123104254934.png)
>
>    1. tf和(用户态的)switchk2u
>    2. 在中断例程的ebp,esp
>    3. 执行pop esp的状态，esp指向switchk2u
>    4. CPU从switchk2u中恢复
>    5. iret后的状态，已经回到中断执行前的地方，此时系统处于用户态
>
>    **CPU在返回后，以用户态模式继续执行`int 0x78`后一条指令**。
>
> 2. [用户态切换到核心态](https://chyyuu.gitbooks.io/simple_os_book/content/zh/chapter-2/user_to_kernel.html)
>
>    CPU执行到中断指令时，正处于用户态，所以存在特权级转换。硬件会在内核栈压入Error Code、EIP、CS、EFLAGS以及**用户态的ESP和SS**，然后执行中断服务例程。
>
>    ![image-20221123114837051](https://raw.githubusercontent.com/MakerFace/images/main/image-20221123114837051.png)
>
>    1. 执行中断前的状态，用户态；栈为什么不是用户态的栈呢？那么哪里才是内核栈和用户栈呢？
>
>       stack0的首址和尾址：
>
>       ![image-20221123145229046](https://raw.githubusercontent.com/MakerFace/images/main/image-20221123145229046.png)
>
>    2. 执行中断例程服务，改变tf的段寄存器和和IOPL
>
>    3. 中断例程服务中的ebp和esp的值，指向stack0（一个全局变量，在pmm中声明，在gdt_init保存到TSS寄存器中）
>
>       switchu2k的地址：
>
>       ![image-20221123145333960](https://raw.githubusercontent.com/MakerFace/images/main/image-20221123145333960.png)
>
>    4. 执行完trap函数，回退到trapEntry.S中，执行pop esp
>
>    5. 执行iret从内核栈恢复CPU
>
> 通过用户态和核心态的相互切换，可以实现系统调用吗？

#### 补充知识：TSS

> https://chyyuu.gitbooks.io/simple_os_book/content/zh/chapter-2/task_switch.html
>
> tss中保存有ts_esp~i~, ts_ss~i~(i=0,1,2；为什么没有3呢？)
>
> 其中比较重要的是**内层栈指针**区域，为了有效地实现保护，**同一个任务**在**不同的特权级**下**使用不同的栈**。例如，当从外层特权级3变换到内层特权级0时，任务使用的栈也同时从3级变换到0级栈；当从内层特权级0变换到外层特权级3时，任务使用的栈也同时从0级栈变换到3级栈。
>
> 当发生从3级向0级转移时，**把0级栈指针装入0级的SS及ESP寄存器**以变换到0级栈。没有指向3级栈的指针，因为3级是**最外层**，所以任何一个向内层的转移都不可能转移到3级。但是，当特权级由0级向3级变换时，并不把0级栈的指针保存到TSS的栈指针区域。这表明向3级向0级转移时，总是**把0级栈认为是一个空栈**。
>
> 当发生任务切换时，80386中各寄存器的当前值被**自动保存**到TR所指定的TSS中，然后下一任务的TSS的选择子被装入TR；最后，从TR所指定的TSS中取出各寄存器的值送到处理器的各寄存器中。由此可见，通过在TSS中保存任务现场各寄存器状态的完整映象，实现任务的切换。
>
> [任务切换](https://wizardforcel.gitbooks.io/intel-80386-ref-manual/content/29.html)：
>
> 1. 执行JMP或CALL，操作数中指定TSS描述符
> 2. 执行JMP或CALL，操作数中指定任务门
> 3. IDT的中断向量或异常向量导致一个新的任务切换
> 4. 当前任务执行了一条IRET指令，而且NT置位时。

#### 问题三、数据结构与栈的关系

> 指令`pusha`（`pushal`暂时没找到出处）将通用寄存器全部压栈：
>
> ![pusha指令-intel-sdm-vol1](https://raw.githubusercontent.com/MakerFace/images/main/image-20221117152453386.png)
>
> 可以看到压栈顺序为：eax,ecx,edx,ebx,old_esp(在eax压入前esp的值),ebp,esi,edi。
>
> 指令`popa`以相反顺序恢复寄存器。
>
> 因此数据结构中越接近栈顶的元素，在数据结构中越靠前。

---

## 系统调用

依据系统调用号提供/使用服务。系统调用使用`int`和`iret`，堆栈切换和特权指令切换；函数调用使用`call`和`ret`，没有堆栈切换（有特权切换？没有）

系统调用开销：比函数调用开销大

- 引导机制
- 建立内核堆栈
- 验证参数
- 内核态映射到用户态地址空间：更新页面映射权限
- 内核态独立地址空间：TLB会失效

实例：文件复制系统调用cp实现

1. read读取文件

   定义在在`/user/libs/file.h`中，`read(int fd, void* des, size_t len)`。内部调用`syscall(SYS_READ, fd, des, len)`。

   `syscall`定义在`user/libs/syscall.h`中，实现在对应的c文件。汇编部分是具体的系统调用。此处转为内核态，调用trap。

   `trap`定义于`kern/trap/trap.h`，实现在对应c文件中。

### 练习七、增加syscall功能

> 扩展proj4,增加syscall功能，即增加一用户态函数（可执行一特定系统调用：获得时钟计数值），当内核初始完毕后，可从内核态返回到用户态的函数，而用户态的函数又通过系统调用得到内核态的服务（通过网络查询所需信息，可找老师咨询。如果完成，且有兴趣做代替考试的实验，可找老师商量）。需写出详细的设计和分析报告。完成出色的可获得适当加分。
>
> 提示： 规范一下 challenge 的流程。
>
> kern_init 调用 switch_test，该函数如下：
>
> ```c
> static void
> switch_test(void) {
>   print_cur_status();          // print 当前 cs/ss/ds 等寄存器状态
>   cprintf("+++ switch to  user  mode +++\n");
>   switch_to_user();            // switch to user mode
>   print_cur_status();
>   cprintf("+++ switch to kernel mode +++\n");
>   switch_to_kernel();         // switch to kernel mode
>   print_cur_status();
> }
> ```
>
> switch*to** 函数建议通过 中断处理的方式实现。主要要完成的代码是在 trap 里面处理 T_SWITCH_TO* 中断，并设置好返回的状态。
>
> 在 lab1 里面完成代码以后，执行 make grade 应该能够评测结果是否正确。

### 练习八、实现目态管态切换

> 用键盘实现用户模式内核模式切换。具体目标是：“键盘输入3时切换到用户模式，键盘输入0时切换到内核模式”。 基本思路是借鉴软中断(syscall功能)的代码，并且把trap.c中软中断处理的设置语句拿过来。
>
> 注意：
>
> 　1.关于调试工具，不建议用lab1_print_cur_status()来显示，要注意到寄存器的值要在中断完成后tranentry.S里面iret结束的时候才写回，所以再trap.c里面不好观察，建议用print_trapframe(tf)
>
> 　2.关于内联汇编，最开始调试的时候，参数容易出现错误，可能的错误代码如下
>
> ```
> asm volatile ( "sub $0x8, %%esp \n"
> "int %0 \n"
> "movl %%ebp, %%esp"
> : )
> ```
>
> 要去掉参数int %0 \n这一行
>
> 3.软中断是利用了临时栈来处理的，所以有压栈和出栈的汇编语句。硬件中断本身就在内核态了，直接处理就可以了。
>
> 1. 参考答案在mooc_os_lab中的mooc_os_2014 branch中的labcodes_answer/lab1_result目录下



## 设备驱动



---

# 编译时问题

1. bootloader编译失败

   `obj/bootblock.out` size: 604 bytes（反正会超过512bytes）。

   解决方法：

   1. 把变量改为常量，或使用#define，或者使用`-fno-pie`禁止生成与位置无关代码

      ![6548e70ea6e416bc9e6908c682dc660](https://raw.githubusercontent.com/MakerFace/images/main/6548e70ea6e416bc9e6908c682dc660.png)

   2. `-Ttext 0x7C00`改为`-T tools/boot.ld`。

      `tools/boot.ld`内容为：

      ```ld
      OUTPUT_FORMAT("elf32-i386")
      OUTPUT_ARCH(i386)
       
      SECTIONS {
          . = 0x7C00;
       
          .startup : {
              *bootasm.o(.text)
          }
       
          .text : { *(.text) }
          .data : { *(.data .rodata) }
       
          /DISCARD/ : { *(.eh_*) }
      }
      ```

      即使用自定义的链接脚本替换ld链接：

      1. section

         定义生成可执行文件的布局

         - 0x7C00是section块的起始地址。

           同时0x7C00也是[主引导的内存地址](https://www.ruanyifeng.com/blog/2015/09/0x7c00.html)，最开始的OS需要的最小内存是32KB(0x0000\~0x7FFF)，使用0x7FFF-0x200+0x1=0x7C00，其中0x200=512字节。从0x0000\~0x03FF保存中断向量（80h等），所以有效内存地址为0x0400\~0x7BFF。

           [放在后面的原因](https://www.glamenv-septzen.net/en/view/6)是一旦操作系统加载并启动，引导扇区在重新boot前永远不会使用，因此操作系统和应用程序可以自由使用。

         - startup

   ---

   原因分析：

   全局变量为什么会增加bootloader大小？

2. bootloader编译失败

   > make: \*\*\* No rule to make target 'obj/boot/', needed by 'obj/boot/bootasm.d'.  Stop.

   没有`obj/boot/`，也不应该有这个东西。

   解决：

   1. 手动创建`obj/boot`文件夹，因为这是编译生成中间文件的目标文件夹。

   2. 使用makefile创建

      ```makefile
      define do_finish_all
      ALLDEPS = $$(ALLOBJS:.o=.d)
      $$(sort $$(dir $$(ALLOJBS)) $(BINDIR)$(SLASH) $(OBJDIR)$(SLASH))
          @$(MKDIR) $$@
      endef
      ```

      > `$(dir $ALLOBS)`用于提取所有中间文件的目录
      >
      > `$(sort list)`按字典序排序

3. bootloader编译失败

   >/bin/sh: 1: bin/sign: not found
   >make: *** [Makefile:85: debug] Error 127

   没有这个文件，或者这个文件损坏。bin/sign用于写bootloader到bootblock中，并校验大小，加MBR结尾。

   