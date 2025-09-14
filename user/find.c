#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 提取文件名（去掉路径部分）
char *basename(char *path) {
  char *p;

  // 从字符串末尾开始向前查找，直到找到 '/' 或到达字符串开头
  for (p = path + strlen(path); p >= path && *p != '/'; p--);

  // 返回 '/' 之后的部分（即文件名）
  return p + 1;
}

// 递归查询
void find(char *path, char *target) {
  char buf[512], *p;  // 缓冲区用于构建完整路径
  int fd;             // 文件描述符用于打开当前路径文件
  struct dirent de;   // 目录entry结构体
  struct stat st;     // 文件状态

  // 打开当前路径
  if ((fd = open(path, 0)) < 0) {
    // 打开失败
    printf("%s 打开失败\n", path);
    return;
  }

  // 使用fd获取文件状态
  if (fstat(fd, &st) < 0) {
    // 获取文件状态失败
    printf("%s 获取文件状态失败\n", path);
    close(fd);
    return;
  }

  // 检查文件类型
  switch (st.type) {
    case T_FILE:
      // 文件类型
      if (strcmp(basename(path), target) == 0) {
        // 当前文件与target匹配成功
        printf("%s\n", path);
      }
      close(fd);
      break;
    case T_DIR:
      // 目录类型

      // 检查路径是不是太大了
      if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
        printf("find: path too long\n");
        close(fd);
        return;
      }

      // 将当前路径path复制到buf中
      strcpy(buf, path);
      p = buf + strlen(buf);
      *p++ = '/';  // 现在的path是目录，为等下找到的文件or目录添加分隔符

      // 开始遍历当前目录的所有de
      // 目录类型的文件里面装的都是一个个的de，所以遍历的去读取
      while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        // 可以读取de大小的数据，说明这是一个de
        if (de.inum == 0) {
          // 这是无效目录，跳过
          continue;
        }

        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
          // 不要递归查找自身目录和上级目录
          continue;
        }

        // 将当前de找出来的name用来构建完整路径
        memmove(p, de.name, DIRSIZ);  // 从p之前添加完/后来添加本次de.name的数据
        p[DIRSIZ] = 0;                // 结束符号

        // 匹配当前目录目录名
        if (strcmp(de.name, target) == 0) {
          printf("%s\n", buf);
        }

        // 如果当前是路径是一个目录的话继续递归查找
        if (stat(buf, &st) == 0 && st.type == T_DIR) {
          find(buf, target);
        }
      }
      close(fd);
      break;
  }
}

int main(int argc, char *argv[]) {
  // 检查参数数量
  if (argc != 3) {
    fprintf(2, "Usage: find <path> <name>\n");
    exit(1);
  }

  // 调用查找函数
  find(argv[1], argv[2]);
  exit(0);
}