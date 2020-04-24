#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <sys/xattr.h>
#include <sys/wait.h>
#include <pthread.h>

static const char *dirpath = "/home/afiahana/Documents";
char cipher[100];

void enkrip_1(char *nama){
	if(!strcmp(nama,".") || !strcmp(nama,"..")) return;
	for(int i = 0; i < (strlen(nama)); i++){
		if(nama[i]!='/'){
			for(int j = 0; j < (strlen(cipher)); j++){
				if(cipher[j] == nama[i]){
					int c_idx = (j + 10) % 87;
					nama[i] = cipher[c_idx];
					break;
				}
			}
		}	
	}
}

void dekrip_1(char *nama){
	if(!strcmp(nama,".") || !strcmp(nama,"..")) return;
	for(int i = 0; i < (strlen(nama)); i++){
		if(nama[i]!='/'){
			for(int j = 0; j < (strlen(cipher)); j++){
				if(cipher[j] == nama[i]){
					int c_idx = (j + 87-10) % 87;
					nama[i] = cipher[c_idx];
					break;
				}
			}
		}	
	}
}

static int xmp_getattr(const char *path, struct stat *stbuf){
    int res;
	char fpath[1000], new_name[1000];
    
    sprintf(new_name, "%s", path);
    dekrip_1(new_name);

	sprintf(fpath,"%s%s",dirpath,new_name);
	res = lstat(fpath, stbuf);

	if (res == -1) return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
    int res;
    DIR *dp;
    struct dirent *de;
 
    (void) offset;
    (void) fi;
    char fpath[1000];
    char name[1000];
    if (strcmp(path, "/") == 0){
        sprintf(fpath, "%s", dirpath);
    }
    else{
        sprintf(name,"%s",path);
        dekrip_1(name);
        sprintf(fpath, "%s%s",dirpath,name);
    }
    dp = opendir(fpath);
    if (dp == NULL)
        return -errno;
 
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        char fullpathname[1000];
        sprintf(fullpathname, "%s/%s", fpath, de->d_name);
        char temp[1000];
        strcpy(temp,de->d_name);
        enkrip_1(temp);
    res = (filler(buf, temp, &st, 0));
    if(res!=0) break;
    }
 
    closedir(dp);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    char fpath[1000];
    char name[1000];
    if (strcmp(path, "/") == 0){
        sprintf(fpath, "%s", dirpath);
    }
    else{
        sprintf(name,"%s",path);
        dekrip_1(name);
        sprintf(fpath, "%s%s",dirpath,name);
    }
    int res = 0;
    int fd = 0 ;
 
    (void) fi;
    fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;
 
    res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;
 
    close(fd);
    return res;
}

static int xmp_mkdir(const char *path, mode_t mode){
	int res;
    char fpath[1000];
    char spath[1000];
	sprintf(spath,"%s",path);
	dekrip_1(spath);
	
	sprintf(fpath,"%s%s",dirpath,spath);
	res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	int fd;
	int res;
    char fpath[1000];
    char name[1000];

	sprintf(name,"%s",path);
    dekrip_1(name);
	sprintf(fpath,"%s%s",dirpath,name);

	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;
	printf("%s\n",buf);
	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);	
	return res;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi){
    (void) fi;

    int res;
    char fpath[1000];
	char spath[1000];
	sprintf(spath,"%s",path);

	dekrip_1(spath);
    
	sprintf(fpath,"%s%s",dirpath,spath);

    res = creat(fpath, mode);
    if(res == -1)
	return -errno;

    close(res);

    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi){
	int res;
    char fpath[1000];
    char name[1000];
	sprintf(name,"%s",path);
    dekrip_1(name);
	sprintf(fpath, "%s%s",dirpath,name);
	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_access(const char *path, int mask){
	int res;

	char fpath[1000];
    char name[1000];
	sprintf(name,"%s",path);
    dekrip_1(name);
	sprintf(fpath, "%s%s",dirpath,name);

	res = access(fpath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2]){
	int res;
	struct timeval tv[2];
	char fpath[1000];
    char spath[1000];
	sprintf(spath,"%s",path);
	dekrip_1(spath);
	sprintf(fpath,"%s%s",dirpath,spath);

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(fpath, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size){
	int res;
    char fpath[1000];
    char name[1000];
	sprintf(name,"%s",path);
    dekrip_1(name);
	sprintf(fpath, "%s%s",dirpath,name);
	res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}


static struct fuse_operations xmp_oper = {
    .getattr    = xmp_getattr,
    .readdir    = xmp_readdir,
    .read       = xmp_read,
    .mkdir      = xmp_mkdir,
    .write      = xmp_write,
	.create     = xmp_create,
    .open       = xmp_open,
	.access     = xmp_access,
	.utimens    = xmp_utimens,
    .truncate   = xmp_truncate,
};

int main(int argc, char *argv[]){
    umask(0);
    sprintf(cipher, "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%crI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO", '%');
    return fuse_main(argc, argv, &xmp_oper, NULL); 
}