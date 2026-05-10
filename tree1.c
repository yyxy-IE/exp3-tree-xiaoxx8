/**
 * 实验：目录树查看器（仿 Linux tree 命令）
 * 学号：__________  姓名：__________
 * 说明：请补全所有标记为 TODO 的函数体，不要修改其他代码。
 * 目录树查看器（仿 Linux tree 命令）
 * 完整实现版本（C语言，左孩子右兄弟二叉树）
 * 编译：gcc -o tree tree.c -std=c99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// ================== 二叉树结点定义 ==================
typedef struct FileNode {
    char *name;                  // 文件/目录名
    int isDir;                   // 1:目录 0:文件
    struct FileNode *firstChild; // 左孩子：第一个子项
    struct FileNode *nextSibling;// 右兄弟：下一个同层项
} FileNode;

// ================== 函数声明 ==================
FileNode* createNode(const char *name, int isDir);
int cmpNode(const void *a, const void *b);
FileNode* buildTree(const char *path);
void printTree(FileNode *node, const char *prefix, int isLast);
int countNodes(FileNode *root);
int countLeaves(FileNode *root);
int treeHeight(FileNode *root);
void countDirFile(FileNode *root, int *dirs, int *files);
void freeTree(FileNode *root);
char* getBaseName(void);

// ================== 需要补全的函数 ==================

// 创建新结点（分配内存、复制字符串、初始化指针）
FileNode* createNode(const char *name, int isDir) {
    // TODO: 实现
    FileNode *node = (FileNode*)malloc(sizeof(FileNode));
    if (!node) return NULL;

    node->name = (char*)malloc(strlen(name) + 1);
    if (!node->name) {
        free(node);
        return NULL;
    }
    strcpy(node->name, name);

    node->isDir = isDir;
    node->firstChild = NULL;
    node->nextSibling = NULL;
    return node;
}

// 比较函数，用于 qsort 对子项按名称排序
int cmpNode(const void *a, const void *b) {
    // TODO: 实现
    FileNode *na = *(FileNode**)a;
    FileNode *nb = *(FileNode**)b;
    return strcmp(na->name, nb->name);
}

// 递归构建目录树（核心难点）
FileNode* buildTree(const char *path) {
    // TODO: 实现
    DIR *dir = opendir(path);
    if (!dir) return NULL;

    const char *p = strrchr(path, '/');
    char node_name[256];
    if (!p || (p == path && strlen(path) == 1)) {
        strcpy(node_name, path);
    } else {
        strcpy(node_name, p + 1);
    }

    FileNode *root = createNode(node_name, 1);
    FileNode **children = NULL;
    int cnt = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full[1024];
        snprintf(full, sizeof(full), "%s/%s", path, entry->d_name);
        struct stat st;
        if (stat(full, &st) != 0) continue;

        FileNode *child;
        if (S_ISDIR(st.st_mode)) {
            child = buildTree(full);
        } else {
            child = createNode(entry->d_name, 0);
        }
        if (!child) continue;

        children = realloc(children, (cnt + 1) * sizeof(FileNode*));
        children[cnt++] = child;
    }

    closedir(dir);
    qsort(children, cnt, sizeof(FileNode*), cmpNode);

    for (int i = 0; i < cnt; i++) {
        if (i == 0) {
            root->firstChild = children[i];
        } else {
            children[i-1]->nextSibling = children[i];
        }
    }

    free(children);
    return root;
}

// 树形输出（仿 tree 命令）
void printTree(FileNode *node, const char *prefix, int isLast) {
    // TODO: 实现
    if (!node) return;

    printf("%s%s%s", prefix, isLast ? "`-- " : "|-- ", node->name);
    if (node->isDir) printf("/");
    printf("\n");

    if (!node->firstChild) return;

    char new_pre[1024];
    snprintf(new_pre, sizeof(new_pre), "%s%s", prefix, isLast ? "    " : "|   ");

    FileNode *c = node->firstChild;
    int total = 0;
    FileNode *tmp = c;
    while (tmp) { total++; tmp = tmp->nextSibling; }

    int i = 0;
    tmp = c;
    while (tmp) {
        i++;
        printTree(tmp, new_pre, i == total);
        tmp = tmp->nextSibling;
    }
}

// 统计二叉树结点总数
int countNodes(FileNode *root) {
    // TODO: 实现（递归）
    if (!root) return 0;
    return 1 + countNodes(root->firstChild) + countNodes(root->nextSibling);
}

// 统计叶子结点数（firstChild == NULL 的结点）
int countLeaves(FileNode *root) {
    // TODO: 实现（递归）
    if (!root) return 0;
    if (!root->firstChild) {
        return 1 + countLeaves(root->nextSibling);
    }
    return countLeaves(root->firstChild) + countLeaves(root->nextSibling);
}

// 计算二叉树高度（根深度为1，空树高度为0）
int treeHeight(FileNode *root) {
    // TODO: 实现（递归）
    if (!root) return 0;
    int max_h = 0;
    FileNode *c = root->firstChild;
    while (c) {
        int h = treeHeight(c);
        if (h > max_h) max_h = h;
        c = c->nextSibling;
    }
    return 1 + max_h;
}

// 统计目录数和文件数（遍历整棵树）
void countDirFile(FileNode *root, int *dirs, int *files) {
    // TODO: 实现（递归）
    if (!root) return;
    if (root->isDir) (*dirs)++;
    else (*files)++;
    countDirFile(root->firstChild, dirs, files);
    countDirFile(root->nextSibling, dirs, files);
}

// 释放整棵树的内存
void freeTree(FileNode *root) {
    // TODO: 实现（递归释放左右子树，最后释放当前结点）
    if (!root) return;
    freeTree(root->firstChild);
    freeTree(root->nextSibling);
    free(root->name);
    free(root);
}

// 获取当前工作目录的“基本名称”（用于显示根结点名）
char* getBaseName(void) {
    // TODO: 实现
    char *cwd = getcwd(NULL, 0);
    if (!cwd) return NULL;

    char *last = strrchr(cwd, '/');
    char *ret;

    if (last) {
        ret = (char*)malloc(strlen(last + 1) + 1);
        strcpy(ret, last + 1);
    } else {
        ret = (char*)malloc(strlen(cwd) + 1);
        strcpy(ret, cwd);
    }

    free(cwd);
    return ret;
}

// ================== 主函数不变 ==================
int main(int argc, char *argv[]) {
    char targetPath[1024];
    if (argc >= 2) {
        strncpy(targetPath, argv[1], sizeof(targetPath)-1);
        targetPath[sizeof(targetPath)-1] = '\0';
    } else {
        if (getcwd(targetPath, sizeof(targetPath)) == NULL) {
            perror("getcwd");
            return 1;
        }
    }

    int len = strlen(targetPath);
    if (len > 0 && targetPath[len-1] == '/')
        targetPath[len-1] = '\0';

    struct stat st;
    if (stat(targetPath, &st) != 0) {
        perror("stat");
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "错误: %s 不是目录\n", targetPath);
        return 1;
    }

    FileNode *root = buildTree(targetPath);
    if (!root) {
        fprintf(stderr, "无法构建目录树\n");
        return 1;
    }

    // 输出根目录名
    char *displayName = NULL;
    if (argc >= 2) {
        displayName = root->name;
    } else {
        displayName = getBaseName();
    }
    printf("%s/\n", displayName);
    if (argc < 2) free(displayName);

    FileNode *child = root->firstChild;
    int childCount = 0;
    FileNode *tmp = child;
    while (tmp) { childCount++; tmp = tmp->nextSibling; }
    int idx = 0;
    while (child) {
        int isLast = (++idx == childCount);
        printTree(child, "", isLast);
        child = child->nextSibling;
    }

    int dirs = 0, files = 0;
    countDirFile(root, &dirs, &files);
    printf("\n%d 个目录, %d 个文件\n", dirs, files);
    printf("二叉树结点总数: %d\n", countNodes(root));
    printf("叶子结点数: %d\n", countLeaves(root));
    printf("树的高度: %d\n", treeHeight(root));

    freeTree(root);
    return 0;
}