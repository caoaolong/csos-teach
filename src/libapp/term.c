#include <csos/term.h>
#include <csos/syscall.h>

void clear()
{
    syscall_arg_t args = {SYS_NR_CLEAR, 0, 0, 0, 0};
    _syscall(&args);
}

int tcgetattr(int fd, term_t *term)
{
    syscall_arg_t args = {SYS_NR_TCGETATTR, fd, (int)term, 0, 0};
    return _syscall(&args);
}

int tcsetattr(int fd, term_t *term)
{
    syscall_arg_t args = {SYS_NR_TCSETATTR, fd, (int)term, 0, 0};
    return _syscall(&args);
}