#include<linux/module.h>
#include<linux/fs.h>
#include<linux/pagemap.h>
#include<linux/mount.h>
#include<linux/init.h>
#include<linux/namei.h>


#define AUFS_MAGIC 0x65668735
#define PAGE_CACHE_SIZE 4096

static struct super_block *aufs_get_sb(struct file_system_type *fs_type,
                                        int flag,const char *dev_name,
                                        void *data);

static struct vfsmount *aufs_mount;
static int aufs_mount_conut;

static struct inode *aufs_get_inode(struct super_block *sb,int mode,dev_t dev)
{
    struct inode *inode=new_inode(sb);
    if(inode){
        inode->i_mode=mode;
        //inode->i_uid=fs_high2lowuid(i_uid_read(inode));
        //inode->i_gid=fs_high2lowgid(i_gid_read(inode));
        //inode->i_blksize=PAGE_CACHE_SIZE;
        inode->i_blocks=0;
        //inode->i_atime=inode->i_mtime=inode->i_ctime=CURRENT_TIME;
        switch (mode&S_IFMT)
        {
        case S_IFREG:
            printk("creat a file\n");
            break;
        case S_IFDIR:
            inode->i_op=&simple_dir_inode_operations;
            inode->i_fop=&simple_dir_operations;
            printk("creat a dir file\n");
            inode->i_nlink++;
            break;
        default:
            init_spcial_inode(inode,mode,dev);
            break;
        }
    }
    return inode;
}

static int aufs_mknod(struct inode *dir,struct dentry *dentry,int mode,dev_t dev)
{
    struct inode *inode;
    int error=-EPERM;
    if(dentry->d_inode)
    {
        return -EEXIST;
    }
    inode=aufs_get_inode(dir->i_sb,mode,dev);
    if(inode)
    {
        d_instantiate(dentry,inode);
        dget(dentry);
        error=0;
    }
    return error;
}

static int aufs_mkdir(struct inode *dir,struct dentry *dentry,int mode)
{
    int res;
    res=aufs_mkond(dir,dentry,mode|S_IFDIR,0);
    if(!res)
        dir->i_nlink++;
    return res;
}

static int aufs_create(struct inode *dir,struct dentry *dentry,int mode)
{
    return aufs_mknod(dir,dentry,mode|S_IFREG,0);
}

static int aufs_fill_super(struct super_block *sb,void *data,int silent)
{
    static struct tree_descr debug_files[]={{""}};
    return simple_fill_super(sb,AUFS_MAGIC,debug_files);
}

static struct super_block *aufs_get_sb(struct file_system_type *fs_type,
                                        int flag,const char *dev_name,
                                        void *data)
{
    return get_sb_single(fs_type,flag,data,aufs_fill_super);
}

static struct file_system_type au_fs_type={
    .owner=THIS_MODULE,
    .name="aufs",
    .get_sb=aufs_get_sb,
    .kill_sb=kill_litter_super,
};

static int aufs_create_by_name(const char *name,mode_t mode,
                                struct dentry *parent,
                                struct dentry **dentry)
{
    int error=0;
    if(!parent){
        printk("can not find parent!\n");
        return -EFAULT;
    }

    *dentry=NULL;
    mutex_lock(&parent->d_inode->i_mutex);
    *dentry=lookup_one_len(name,parent,strlen(name));
    if(!IS_ERR(dentry)){
        if((mode&S_IFMT)==S_IFDIR)
        {
            error=aufs_mkdir(parent->d_inode,*dentry,mode);
        }else
        {
            error=aufs_create(parent->d_inode,*dentry,mode);
        }
        
    }else
    {
        error=PTR_ERR(dentry);
    }
    mutex_unlock(&parent->d_inode->i_mutex);
    return error;
}

struct dentry *aufs_create_file(const char *name,mode_t mode,
                                struct dentry *parent,void *data,
                                struct file_operations *fops)
{
    struct dentry *dentry=NULL;
    int error;
    printk("aufs:creating file '%s'\n",name);
    error=aufs_create_by_name(name,mode,parent,&dentry);
    if(error){
        dentry=NULL;
        goto exit;
    }
    if(dentry->d_inode){
        if(data)
            dentry->d_inode->u.generic_ip=data;
        if(fops)
            dentry->d_inode->i_fop=fops;
    }
    exit:
        return dentry;
}

struct ddentry *aufs_create_dir(const char *name,struct dentry *parent)
{
    return aufs_create_file(name,S_IFDIR|S_IRWXU|S_IRUGO|S_IXUGO,parent,NULL,NULL);
}

static int __init aufs_init(void)
{
    int retval;
    struct dentry *pslot;
    retval=register_filesystem(&au_fs_type);

    if(!retval){
        aufs_mount=kern_mount(&au_fs_type);
        if(IS_ERR(aufs_mount)){
            printk(KERN_ERR "aufs: could not mount!\n");
            unregister_filesystem(&au_fs_type);
            return retval;
        }
    }

    pslot=aufs_create_dir("woman star",NULL);
    aufs_create_file("llb",S_IFREG|S_IRUGO,pslot,NULL,NULL);
    aufs_create_file("fbb",S_IFREG|S_IRUGO,pslot,NULL,NULL);
    aufs_create_file("ljl",S_IFREG|S_IRUGO,pslot,NULL,NULL);

    pslot=aufs_create_dir("man star",NULL);
    aufs_create_file("ldh",S_IFREG|S_IRUGO,pslot,NULL,NULL);
    aufs_create_file("lcw",S_IFREG|S_IRUGO,pslot,NULL,NULL);
    aufs_create_file("jw",S_IFREG|S_IRUGO,pslot,NULL,NULL);

    return retval;
}

static void __exit aufs_exit(void)
{
    simple_release_fs(&aufs_mount,&aufs_mount_conut);
    unregister_filesystem(&au_fs_type);
}

module_init(aufs_init);
module_exit(aufs_exit);
MODULE_LICENSE("GPL");