#ifndef CSOS_OS_H
#define CSOS_OS_H

#define KERNEL_CODE_SEG         (1 * 8)
#define KERNEL_DATA_SEG         (2 * 8)
#define USER_CODE_SEG           (3 * 8)
#define USER_DATA_SEG           (4 * 8)
#define SYSCALL_GATE_SEG        (5 * 8)

#define KERNEL_VERSION          "1.0.0"
#define OP_SYS_VERSION          "1.0.0"

#define OS_TZ                   (+8)

#define OS_TASK_MAX_SIZE        128
#define SECTOR_SIZE             512

#define ARP_MAP_SECTOR          4000

#endif