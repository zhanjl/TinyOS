#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "file.h"
#include "fcntl.h"
#include "monitor.h"
#include "pipe.h"

extern struct proc *curproc;
//获取第n个参数作为文件描述符，并返回对应的file结构体
static int argfd(int n, int *pfd, struct file **pf)
{
    int fd;
    struct file *f;

    if (argint(n, &fd) < 0)
        return -1;
    if (fd < 0 || fd >= NFILE || (f = curproc->ofile[fd]) == 0)
        return -1;
    if (pfd)
        *pfd = fd;
    if (pf)
        *pf = f;
    return 0;
}

//给一个文件分配文件描述符
static int fdalloc(struct file *f)
{
    int     fd;
    for (fd = 0; fd < NFILE; fd++)
    {
        if (curproc->ofile[fd] == 0)
        {
            curproc->ofile[fd] = f;
            return fd;
        }
    }
    return -1;
}

//dup系统调用
int sys_dup(void)
{
    struct file *f;
    int fd;
    if (argfd(0, 0, &f) < 0)
        return -1;
    if ((fd = fdalloc(f)) < 0)
        return -1;
    filedup(f);
    return fd;
}

int sys_read(void)
{
    struct file *f;
    int     n;
    char    *p;

    if (argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
        return -1;
    return fileread(f, p, n);
}


int sys_write(void)
{
    struct file *f;
    int     n;
    char    *p;

    if (argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
        return -1;
    return filewrite(f, p, n);
}


int sys_close(void)
{
    int     fd;
    struct file *f;

    if (argfd(0, &fd, &f) < 0)
        return -1;
    curproc->ofile[fd] = 0;
    fileclose(f);
    return 0;
}

int sys_fstat(void)
{
    struct file *f;
    struct stat *st;

    if (argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0)
        return -1;
    return filestat(f, st);
}

int sys_link(void)
{
    char name[DIRSIZ], *new, *old;
    struct inode *dp, *ip;

    if(argstr(0, &old) < 0 || argstr(1, &new) < 0)
      return -1;

    begin_op();
    if((ip = namei(old)) == 0){
      end_op();
      return -1;
    }

    if(ip->type == T_DIR){
      end_op();
      return -1;
    }

    ip->nlink++;
    iupdate(ip);

    if((dp = nameiparent(new, name)) == 0)
      goto bad;
    if(dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0){
      goto bad;
    }
    iput(ip);

    end_op();

    return 0;

bad:
    ip->nlink--;
    iupdate(ip);
    end_op();
    return -1;
}

//判断一个目录是否为空，(除了.和..外是否还有其他文件)
static int isdirempty(struct inode *dp)
{
    int     off;
    struct dirent   de;
    for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de))
    {
        if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
            PANIC("readi error");
        if (de.inum != 0)
            return 0;
    }
    return 1;
}

int sys_unlink(void)
{
    struct inode *ip, *dp;
    struct dirent de;
    char name[DIRSIZ], *path;
    uint off;

    if(argstr(0, &path) < 0)
      return -1;

    begin_op();
    if((dp = nameiparent(path, name)) == 0){
      end_op();
      return -1;
    }

    // Cannot unlink "." or "..".
    if(namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
      goto bad;

    if((ip = dirlookup(dp, name, &off)) == 0)
      goto bad;

    if(ip->nlink < 1)
      PANIC("unlink: nlink < 1");
    if(ip->type == T_DIR && !isdirempty(ip)){
      goto bad;
    }

    memset(&de, 0, sizeof(de));
    if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      PANIC("unlink: writei");
    if(ip->type == T_DIR){
      dp->nlink--;
      iupdate(dp);
    }
    ip->nlink--;
    iupdate(ip);

    end_op();

    return 0;

bad:
    end_op();
    return -1;
}

static struct inode*
create(char *path, short type, short major, short minor)
{
    uint off;
    struct inode *ip, *dp;
    char name[DIRSIZ];

    if((dp = nameiparent(path, name)) == 0)
      return 0;

    if((ip = dirlookup(dp, name, &off)) != 0){
      if(type == T_FILE && ip->type == T_FILE)
        return ip;
      return 0;
    }

    if((ip = ialloc(dp->dev, type)) == 0)
      PANIC("create: ialloc");

    ip->major = major;
    ip->minor = minor;
    ip->nlink = 1;
    iupdate(ip);

    if(type == T_DIR){  // Create . and .. entries.
      dp->nlink++;  // for ".."
      iupdate(dp);
      // No ip->nlink++ for ".": avoid cyclic ref count.
      if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
        PANIC("create dots");
    }

    if(dirlink(dp, name, ip->inum) < 0)
      PANIC("create: dirlink");
    return ip;
}


int sys_open(void)
{
    char *path;
    int fd, omode;
    struct file *f;
    struct inode *ip;

    if(argstr(0, &path) < 0 || argint(1, &omode) < 0)
      return -1;

    begin_op();

    if(omode & O_CREATE){
      ip = create(path, T_FILE, 0, 0);
      if(ip == 0){
        end_op();
        return -1;
      }
    } else {
      if((ip = namei(path)) == 0){
        end_op();
        return -1;
      }
      if(ip->type == T_DIR && omode != O_RDONLY){
        end_op();
        return -1;
      }
    }

    if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
      if(f)
        fileclose(f);
      end_op();
      return -1;
    }
    end_op();

    f->type = FD_INODE;
    f->ip = ip;
    f->off = 0;
    f->readable = !(omode & O_WRONLY);
    f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
    return fd;
}

int sys_mkdir(void)
{
    char *path;
    struct inode *ip;

    begin_op();
    if (argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0)
    {
        end_op();
        return -1;
    }
    end_op();
    return 0;
}

int sys_mknod(void)
{
    struct inode *ip;
    char *path;
    int len;
    int major, minor;

    begin_op();
    if ((len = argstr(0, &path)) < 0 ||
            argint(1, &major) < 0 ||
            argint(2, &minor) < 0 ||
            (ip = create(path, T_DEV, major, minor)) == 0)
    {
        end_op();
        return -1;
    }
    end_op();
    return 0;
}

int sys_chdir(void)
{
    char *path;
    struct inode *ip;

    begin_op();
    if(argstr(0, &path) < 0 || (ip = namei(path)) == 0){
      end_op();
      return -1;
    }
    if(ip->type != T_DIR){
      end_op();
      return -1;
    }
    iput(proc->cwd);
    end_op();
    proc->cwd = ip;
    return 0;
}

int sys_exec(void)
{
    char *path, *argv[MAXARG];
    int i;
    uint uargv, uarg;

    if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0){
      return -1;
    }
    memset(argv, 0, sizeof(argv));
    for(i=0;; i++){
      if(i >= NELEM(argv))
        return -1;
      if(fetchint(uargv+4*i, (int*)&uarg) < 0)
        return -1;
      if(uarg == 0){
        argv[i] = 0;
        break;
      }
      if(fetchstr(uarg, &argv[i]) < 0)
        return -1;
    }
    return exec(path, argv);
}

int sys_pipe(void)
{
    int *fd;
    struct file *rf, *wf;
    int fd0, fd1;

    if (argptr(0, (void*)&fd, 2*sizeof(fd[0])) < 0)
        return -1;
    if (pipealloc(&rf, &wf) < 0)
        return -1;
    fd0 = -1;
    if ((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0)
    {
        if (fd0 >= 0)
            curproc->ofile[fd0] = 0;
        fileclose(rf);
        fileclose(wf);
        return -1;
    }

    fd[0] = fd0;
    fd[1] = fd1;
    return 0;
}
