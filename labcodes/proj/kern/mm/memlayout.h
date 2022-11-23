#ifndef __KERN_MM_MEMLAYOUT_H__
#define __KERN_MM_MEMLAYOUT_H__

/* This file contains the definitions for memory management in our OS. */

/* global segment number */
//? 段选择器怎么回事？为什么用户代码段是3？
//* 1~5是段的index，需要左移3位，再加上TI+RPL
#define SEG_KTEXT    1
#define SEG_KDATA    2
#define SEG_UTEXT    3
#define SEG_UDATA    4
#define SEG_TSS        5

/* global descriptor numbers */
#define GD_KTEXT    ((SEG_KTEXT) << 3)        // kernel text
#define GD_KDATA    ((SEG_KDATA) << 3)        // kernel data
#define GD_UTEXT    ((SEG_UTEXT) << 3)        // user text
#define GD_UDATA    ((SEG_UDATA) << 3)        // user data
#define GD_TSS        ((SEG_TSS) << 3)        // task segment selector

#define DPL_KERNEL    (0)
#define DPL_USER    (3)

#define KERNEL_CS    ((GD_KTEXT) | DPL_KERNEL) //* 0x8, index=1,TI=0,RPL=0
#define KERNEL_DS    ((GD_KDATA) | DPL_KERNEL) //* 0x10, index=2,TI=0,RPL=0
#define USER_CS        ((GD_UTEXT) | DPL_USER) //* 0x1b, index=3,TI=0,RPL=3
#define USER_DS        ((GD_UDATA) | DPL_USER) //* 0x23, index=4,TI=0,RPL=3

#endif /* !__KERN_MM_MEMLAYOUT_H__ */

