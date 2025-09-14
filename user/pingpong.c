#include "kernel/types.h"
#include "user.h"

/*
/user/pingpong.c
使用管道实现父进程和子进程之间通信的展示
*/

int main() {
  // 使用该进程作为父进程，然后fork出来的作为子进程，持有两者同时持有两个pipe

  // 创建管道文件描述符
  int c2f_pipe_fd[2];  // c2f
  int f2c_pipe_fd[2];  // f2c

  // 创建两个管道
  pipe(c2f_pipe_fd);
  pipe(f2c_pipe_fd);

  if (fork() == 0) {
    // 子进程代码块
    // 发出自身pid使得父进程输出pong
    // 接收父进程pid并输出ping

    // 自身的pid
    int pid = getpid();
    int recv_pid_from_father;

    // f2c部分
    close(f2c_pipe_fd[1]);                           // 关闭写端
    read(f2c_pipe_fd[0], &recv_pid_from_father, 4);  // 阻塞等待父进程发送信息到管道
    printf("%d: received ping from pid %d\n", pid, recv_pid_from_father);
    close(f2c_pipe_fd[0]);  // 关闭读端

    // c2f部分
    close(c2f_pipe_fd[0]);  // 关闭读端
    write(c2f_pipe_fd[1], &pid, 4);
    close(c2f_pipe_fd[1]);  // 关闭写端

    exit(0);

  } else {
    // 父进程代码块
    // 发出自身pid给子进程使其输出ping

    int pid = getpid();       // 父进程pid
    int recv_pid_from_child;  // 收到的子进程pid

    // f2c部分
    close(f2c_pipe_fd[0]);  // 关闭读端
    write(f2c_pipe_fd[1], &pid, 4);
    close(f2c_pipe_fd[1]);  // 关闭写端

    // c2f部分
    close(c2f_pipe_fd[1]);  // 关闭写端
    read(c2f_pipe_fd[0], &recv_pid_from_child, 4);
    printf("%d: received pong from pid %d\n", pid, recv_pid_from_child);  // 输出
    close(c2f_pipe_fd[0]);

    exit(0);
  }
}