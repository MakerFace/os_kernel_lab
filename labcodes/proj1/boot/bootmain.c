#include <ata.h>
#include <defs.h>
#include <elf.h>
#include <x86.h>

/* *********************************************************************
 * This a dirt simple boot loader, whose sole job is to boot
 * an ELF kernel image from the first IDE hard disk.
 *
 * DISK LAYOUT
 *  * This program(bootasm.S and bootmain.c) is the bootloader.
 *    It should be stored in the first sector of the disk.
 *
 *  * The 2nd sector onward holds the kernel image.
 *
 *  * The kernel image must be in ELF format.
 *
 * BOOT UP STEPS
 *  * when the CPU boots it loads the BIOS into memory and executes it
 *
 *  * the BIOS intializes devices, sets of the interrupt routines, and
 *    reads the first sector of the boot device(e.g., hard-drive)
 *    into memory and jumps to it.
 *
 *  * Assuming this boot loader is stored in the first sector of the
 *    hard-drive, this code takes over...
 *
 *  * control starts in bootasm.S -- which sets up protected mode,
 *    and a stack so C code then run, then calls bootmain()
 *
 *  * bootmain() in this file takes over, reads in the kernel and jumps to it.
 * */

#define SECTSIZE 512
#define ELFHDR ((struct elfhdr *)0x10000)  // scratch space暂存空间

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
  insl(ATA_BAR0, dst, SECTSIZE / 4);
}

/* *
 * readseg - read @count bytes at @offset from kernel into virtual address @va,
 * might copy more than asked.
 * */
static void readseg(uintptr_t va, uint32_t count, uint32_t offset) {
  uintptr_t end_va = va + count;  // 终止虚拟地址，0x10000+0x1000=0x11000

  // round down to sector boundary
  va -= offset % SECTSIZE;  // 如果有偏移的话，不满一个扇区要读整个扇区

  // translate from bytes to sectors; kernel starts at sector 1
  uint32_t secno =
      (offset / SECTSIZE) + 1;  // 把读取的字节数转换为扇区数，从第1扇区开始读

  // If this is too slow, we could read lots of sectors at a time.
  // We'd write more to memory than asked, but it doesn't matter --
  // we load in increasing order.
  for (; va < end_va; va += SECTSIZE, ++secno) {
    readsect((void *)va, secno);
  }
}

/* bootmain - the entry of bootloader */
void bootmain(void) {
  // read the 1st page off disk
  //* 读第一页4K数据，也就是8个扇区
  readseg((uintptr_t)ELFHDR, SECTSIZE * 8, 0);

  // is this a valid ELF?
  if (ELFHDR->e_magic != ELF_MAGIC) {
    goto bad;
  }

  struct proghdr *ph, *eph;

  // load each program segment (ignores ph flags)
  ph = (struct proghdr *)((uintptr_t)ELFHDR + ELFHDR->e_phoff);
  eph = ph + ELFHDR->e_phnum;
  for (; ph < eph; ph++) {
    readseg(ph->p_va & 0xFFFFFF, ph->p_memsz, ph->p_offset);
  }

  // call the entry point from the ELF header
  // note: does not return
  ((void (*)(void))(ELFHDR->e_entry & 0xFFFFFF))();

bad:
  outw(0x8A00, 0x8A00);
  outw(0x8A00, 0x8E00);

  /* do nothing */
  while (1)
    ;
}
