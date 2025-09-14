#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 格式化文件名，返回固定长度的文件名字符串
char *fmtname(char *path) {
  static char buf[DIRSIZ + 1];  // 静态缓冲区，用于存储格式化后的文件名
  char *p;

  // 找到路径中最后一个'/'之后的字符（即文件名）
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // 返回空格填充的文件名
  if (strlen(p) >= DIRSIZ) return p;  // 如果文件名长度超过DIRSIZ，直接返回
  memmove(buf, p, strlen(p));  // 复制文件名到缓冲区
  memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));  // 用空格填充剩余位置
  return buf;
}

// 列出指定路径的文件或目录内容
void ls(char *path) {
  char buf[512], *p;  // 缓冲区用于构建完整路径
  int fd;  // 文件描述符
  struct dirent de;  // 目录项结构
  struct stat st;  // 文件状态结构

  // 打开文件或目录
  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  // 获取文件状态信息
  if (fstat(fd, &st) < 0) {
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  // 根据文件类型进行不同处理
  switch (st.type) {
    case T_FILE:  // 如果是普通文件
      // 打印文件信息：文件名 类型 inode号 大小
      printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
      break;

    case T_DIR:  // 如果是目录
      // 检查路径长度是否超出缓冲区大小
      if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
        printf("ls: path too long\n");
        break;
      }
      strcpy(buf, path);  // 复制路径到缓冲区
      p = buf + strlen(buf);  // 指向路径末尾
      *p++ = '/';  // 添加路径分隔符
      
      // 读取目录中的每个条目
      while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0) continue;  // 跳过空的目录项
        memmove(p, de.name, DIRSIZ);  // 复制文件名到路径末尾
        p[DIRSIZ] = 0;  // 添加字符串结束符
        
        // 获取该文件的状态信息
        if (stat(buf, &st) < 0) {
          printf("ls: cannot stat %s\n", buf);
          continue;
        }
        // 打印文件信息：文件名 类型 inode号 大小
        printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
      }
      break;
  }
  close(fd);  // 关闭文件描述符
}

// 主函数
int main(int argc, char *argv[]) {
  int i;

  // 如果没有参数，列出当前目录
  if (argc < 2) {
    ls(".");
    exit(0);
  }
  
  // 遍历所有参数，对每个路径执行ls操作
  for (i = 1; i < argc; i++) ls(argv[i]);
  exit(0);
}
