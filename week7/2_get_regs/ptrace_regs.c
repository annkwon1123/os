#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
  pid_t child;
  long orig_rax, rax;
  long params[3];
  int status;
  int insyscall = 0;
  struct user_regs_struct regs;

  child = fork();

  // Child Proess
  if (child == 0)
  {
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    execl("/bin/ls", "ls", NULL);
  }
  // Parent Proess
  else
  {
    while (1)
    {
      wait(&status);
      if (WIFEXITED(status))
        break;
      orig_rax = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
      if (orig_rax == SYS_write)
      {
        if (insyscall == 0)
        {
          insyscall = 1;
          /* 레지스터 값 모두 가져오기 (system call의 인자 값 저장) */
          ptrace(PTRACE_GETREGS, child, NULL, &regs);
          // 레지스터 값 출력 (system call의 인자로 들어가는 값 출력)
          printf("Write called with "
                 "%lld %lld %lld\n",
                 regs.rdi, regs.rsi, regs.rdx);
        }
        else
        {
          rax = ptrace(PTRACE_PEEKUSER, child, 8 * RAX, NULL);
          printf("Write returned "
                 "with %ld\n",
                 rax);
          insyscall = 0;
        }
      }
      ptrace(PTRACE_SYSCALL, child, NULL, NULL); // proceed to next system call and stop the process with the pid
    }
  }
  return 0;
}
