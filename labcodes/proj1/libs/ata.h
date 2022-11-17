#ifndef ATA_H
#define ATA_H

#define ATA_SR_BSY 0x80   // Busy
#define ATA_SR_DRDY 0x40  // Drive ready
#define ATA_SR_DF 0x20    // Drive write fault
#define ATA_SR_DSC 0x10   // Drive seek complete
#define ATA_SR_DRQ 0x08   // Data request ready
#define ATA_SR_CORR 0x04  // Corrected data
#define ATA_SR_IDX 0x02   // Index
#define ATA_SR_ERR 0x01   // Error

#define ATA_ER_BBK 0x80    // Bad block
#define ATA_ER_UNC 0x40    // Uncorrectable data
#define ATA_ER_MC 0x20     // Media changed
#define ATA_ER_IDNF 0x10   // ID mark not found
#define ATA_ER_MCR 0x08    // Media change request
#define ATA_ER_ABRT 0x04   // Command aborted
#define ATA_ER_TK0NF 0x02  // Track 0 not found
#define ATA_ER_AMNF 0x01   // No address mark

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_READ_PIO_EXT 0x24
#define ATA_CMD_READ_DMA 0xC8
#define ATA_CMD_READ_DMA_EXT 0x25
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_WRITE_PIO_EXT 0x34
#define ATA_CMD_WRITE_DMA 0xCA
#define ATA_CMD_WRITE_DMA_EXT 0x35
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET 0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY 0xEC

#define ATAPI_CMD_READ 0xA8
#define ATAPI_CMD_EJECT 0x1B

#define ATA_IDENT_DEVICETYPE 0
#define ATA_IDENT_CYLINDERS 2
#define ATA_IDENT_HEADS 6
#define ATA_IDENT_SECTORS 12
#define ATA_IDENT_SERIAL 20
#define ATA_IDENT_MODEL 54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID 106
#define ATA_IDENT_MAX_LBA 120
#define ATA_IDENT_COMMANDSETS 164
#define ATA_IDENT_MAX_LBA_EXT 200

#define IDE_ATA 0x00
#define IDE_ATAPI 0x01

#define ATA_MASTER 0x00
#define ATA_SLAVE 0x01

// BAR0 PCI原生模式下主通道基地址，8个端口
#define ATA_BAR0 0x1F0
// BAR1 PCI原生模式下主通道控制端口基地址，4个端口
#define ATA_BAR1 0x3F6
// BAR2 PCI原生模式下辅助通道基地址，8个端口
#define ATA_BAR2 0x170
// BAR3 PCI原生模式下辅助通道控制端口基地址，4个端口
#define ATA_BAR3 0x376
// BAR4 总线主控IDE（16个端口）
// #define ATA_BAR4    定义为多少？
//! NOTE：
//! BAR1和BAR3指定了4个端口，但仅偏移为2的端口可用，0,1,3不可用。
//? 初始化IDE控制器的工作是BIOS来完成吗？

// 任务文件是一个由 8 个端口组成的范围
// 这些端口与 BAR0（主通道）和/或 BAR2（辅助通道）有偏移
//* 数据寄存器rw
#define ATA_REG_DATA 0x00
//* 错误寄存器r
#define ATA_REG_ERROR 0x01
//* 特征寄存器w
#define ATA_REG_FEATURES 0x01
//* 要读写的扇区数rw
#define ATA_REG_SECCOUNT0 0x02
//* lba=lba2:lba1:lba0
#define ATA_REG_LBA0 0x03
#define ATA_REG_LBA1 0x04
#define ATA_REG_LBA2 0x05
//* 选择通道中的驱动器rw
#define ATA_REG_HDDEVSEL 0x06
//* 命令寄存器w
#define ATA_REG_COMMAND 0x07
//* 状态寄存器r
#define ATA_REG_STATUS 0x07

#define ATA_REG_SECCOUNT1 0x08
#define ATA_REG_LBA3 0x09
#define ATA_REG_LBA4 0x0A
#define ATA_REG_LBA5 0x0B
#define ATA_REG_CONTROL 0x0C
#define ATA_REG_ALTSTATUS 0x0C
#define ATA_REG_DEVADDRESS 0x0D

// 通道:
#define ATA_PRIMARY 0x00
#define ATA_SECONDARY 0x01

// 方向:
#define ATA_READ 0x00
#define ATA_WRITE 0x01

#endif  // ATA_h