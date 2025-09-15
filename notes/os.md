## 1. 文件描述符 (File Descriptor, fd) 🔢

**核心思想:** 在Linux/UNIX里，一切皆文件。操作系统给你一个数字"号码牌"（就是fd）来代表你打开的文件，你用这个号码牌去操作文件。

- `0`: 标准输入 (键盘 ⌨️)
- `1`: 标准输出 (屏幕 🖥️)
- `2`: 标准错误 (也是屏幕 🖥️)

**代码速览 (C):**

```c
#include <fcntl.h>  // for open()
#include <unistd.h> // for write(), close()
#include <string.h>

int main() {
    // open() 返回一个文件描述符 fd
    int fd = open("log.txt", O_WRONLY | O_CREAT, 0644); // 得到号码牌, 比如 fd = 3，可选字段

    // 用这个号码牌去写文件
    write(fd, "Hello FD! 👋\n", 13);

    // 用完归还号码牌
    close(fd);
    return 0;
}
```

简单说：`open()` 拿号 -> `write()`/`read()` 用号 -> `close()` 还号。

## 2. 重定向 (Redirection) 👉📜

**核心思想:** 就是给文件描述符"换绑"一个新文件。最常见的就是把标准输出 `1` (屏幕) 换成一个文件。

**代码速览 (C):**

`dup2(old_fd, new_fd)` 是关键！意思是“让 `new_fd` 做 `old_fd` 的事”。

```c
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    // 1. 打开一个文件
    int file_fd = open("output.log", O_WRONLY | O_CREAT, 0644);

    // 2. 关键一步！让 fd=1 (标准输出) 指向 file_fd 指向的文件
    dup2(file_fd, 1); // 施展“换绑”魔法 ✨

    // 3. 关闭原来的文件fd，因为 fd=1 已经接管了它的工作
    close(file_fd);

    // 这句话不会显示在屏幕上，而是跑进了 output.log 文件里！
    printf("Hello, Redirection! 🎯\n");

    return 0;
}
```

Shell里的 `ls > files.list` 就是这么干的！

> `dup(fd)`用来复制一个`fd`然后选择当前最小的没有打开的`new_fd`用来指向同一个底层文件
>
> `dup2(new_fd,exist_fd)`用来将已有的复制到一个指定的被操作的是可控的

## 3. 管道 (Pipe) 📤➡️📥

**核心思想:** 管道是内存里的一个"管子"，用来连接两个进程。一个进程往管子这头写 (`write`)，另一个进程从那头读 (`read`)。

**代码速览 (C):**

这通常和 `fork()` (创建子进程) 一起使用，实现父子“隔空传话”。

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipe_fd[2]; // pipe_fd[0]是读端, pipe_fd[1]是写端
    pipe(pipe_fd);  // 创建管子 ⚙️

    if (fork() == 0) { // --- 子进程 ---
        close(pipe_fd[1]); // 子进程只读，关掉写端
        char buf[100];
        read(pipe_fd[0], buf, 100); // 从管子里读信 📥
        printf("子进程收到: %s\n", buf);
        close(pipe_fd[0]);

    } else { // --- 父进程 ---
        close(pipe_fd[0]); // 父进程只写，关掉读端
        write(pipe_fd[1], "Hi Son! 👨‍👦", 11); // 往管子里写信 📤
        close(pipe_fd[1]);
        wait(NULL); // 等儿子读完信
    }
    return 0;
}
```

Shell里的 `cat file.txt | grep "hello"` 就是用管道连接了 `cat` 和 `grep` 两个进程。