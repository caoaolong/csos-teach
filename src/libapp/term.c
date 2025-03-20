#include <csos/term.h>
#include <csos/syscall.h>

void clear()
{
    syscall_arg_t args = {SYS_NR_CLEAR, 0, 0, 0, 0};
    _syscall(&args);
}