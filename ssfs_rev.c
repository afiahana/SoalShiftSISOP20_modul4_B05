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

char *en1 = "encv1_";
char *en2 = "encv2_";
char *syncs = "sync_";
int x = 0;

static const char *dirpath = "/home/afiahana/Documents";
static const char *log_path = "/home/afiahana/fs.log";
char cipher[100];

int ext_id(char *path){
    for(int i = strlen(path) - 1; i >= 0; i--){
        if(path[i] == '.') return i;
    }
    return strlen(path);
}

int slash_id(char *path, int akhir){
    for(int i = 0; i < strlen(path); i++){
        if(path[i] == '/') return i + 1;
    }
    return akhir;
}

void enkrip_1(char *nama){
	if(!strcmp(nama,".") || !strcmp(nama,"..")) return;
    int awalid = slash_id(nama, 0);
    int akhirid = ext_id(nama);
	for(int i = awalid; i < akhirid; i++){
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
    int awalid = slash_id(nama, 0);
    int akhirid = ext_id(nama);
	for(int i = awalid; i < akhirid; i++){
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

void writeLog(char *level, char *cmd_desc){
  FILE * fp;
  fp = fopen (log_path, "a+");

  time_t rawtime = time(NULL);
  
  struct tm *info = localtime(&rawtime);
  
  char time[100];
  strftime(time, 100, "%y%m%d-%H:%M:%S", info);

  char log[100];
  sprintf(log, "%s::%s::%s\n", level, time, cmd_desc);
  fputs(log, fp);

  fclose(fp);
}

static int xmp_getattr(const char *path, struct stat *stbuf){
    int res;
	char fpath[1000];

    char *newname = strstr(path, en1);
    if(newname != NULL) dekrip_1(newname);

    if(strcmp(path, "/") == 0){
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

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
    char *newname = strstr(path, en1);

    if(newname != NULL) dekrip_1(newname);
    if(strcmp(path, "/") == 0){
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

    dp = opendir(fpath);
    if (dp == NULL)
        return -errno;
 
    while ((de = readdir(dp)) != NULL) {
        if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        if(newname != NULL) enkrip_1(de->d_name);
        
        res = (filler(buf, de->d_name, &st, 0));
        if(res!=0) break;
    }
 
    closedir(dp);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    char fpath[1000];
    char *newname = strstr(path, en1);

    if(newname != NULL) dekrip_1(newname);
    if(strcmp(path, "/") == 0){
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

    int res = 0;
    int fd = 0 ;
 
    (void) fi;
    fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;
 
    res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;
 
    close(fd);
    return res;
}

static int xmp_mkdir(const char *path, mode_t mode){
	int res;
    char fpath[1000];
    char *newname = strstr(path, en1);

    if(newname != NULL) dekrip_1(newname);
    if(strcmp(path, "/") == 0){
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

	res = mkdir(fpath, mode);
	if (res == -1) return -errno;

    char desc[100];
    sprintf(desc, "MKDIR::%s", fpath);
    writeLog("INFO", desc);
	return 0;
}

static int xmp_rmdir(const char *path, mode_t mode){
	int res;
    char fpath[1000];
    char *newname = strstr(path, en1);

    if(newname != NULL) dekrip_1(newname);
    if(strcmp(path, "/") == 0){
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);
    
	res = rmdir(fpath);
	if (res == -1) return -errno;

    char desc[100];
    sprintf(desc, "RMDIR::%s", fpath);
    writeLog("WARNING", desc);
	return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	int fd;
	int res;
    char fpath[1000];
    char *newname = strstr(path, en1);

    if(newname != NULL) dekrip_1(newname);
    if(strcmp(path, "/") == 0){
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;
    
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
	char *newname = strstr(path, en1);

    if(newname != NULL) dekrip_1(newname);
    if(strcmp(path, "/") == 0){
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

    res = creat(fpath, mode);
    if(res == -1)
	return -errno;

    close(res);

    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi){
	int res;
    char fpath[1000];
    char *newname = strstr(path, en1);

    if(newname != NULL) dekrip_1(newname);
    if(strcmp(path, "/") == 0){
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_access(const char *path, int mask){
	int res;
	char fpath[1000];
    char *newname = strstr(path, en1);

    if(newname != NULL) dekrip_1(newname);
    if(strcmp(path, "/") == 0){
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

	res = access(fpath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2]){
	int res;
	struct timeval tv[2];
	char fpath[1000];
    char *newname = strstr(path, en1);

    if(newname != NULL) dekrip_1(newname);
    if(strcmp(path, "/") == 0){
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

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
    char *newname = strstr(path, en1);

    if(newname != NULL) dekrip_1(newname);
    if(strcmp(path, "/") == 0){
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);
	
    res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to){
	int res;
	char frompath[1000], topath[1000];

	char *newname = strstr(to, en1);

	if (newname != NULL)
		dekrip_1(newname);

	sprintf(frompath, "%s%s", dirpath, from);
	sprintf(topath, "%s%s", dirpath, to);

	res = rename(frompath, topath);
	if (res == -1) return -errno;

	char desc[100];
    sprintf(desc, "RENAME::%s::%s", from, to);
    writeLog("INFO", desc);
	return 0;
}

static int xmp_unlink(const char *path){
	int res;
	char fpath[1000];
	char *newname = strstr(path, en1);
	if (newname != NULL)
		dekrip_1(newname);

	if (strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	}
    else{
		sprintf(fpath, "%s%s", dirpath, path);
	}

	res = unlink(fpath);
	if (res == -1) return -errno;

    char desc[100];
    sprintf(desc, "REMOVE::%s", fpath);
    writeLog("WARNING", desc);
	return 0;
}


static struct fuse_operations xmp_oper = {
    .getattr    = xmp_getattr,
    .readdir    = xmp_readdir,
    .read       = xmp_read,
    .mkdir      = xmp_mkdir,
    .rmdir      = xmp_rmdir,
    .write      = xmp_write,
	.create     = xmp_create,
    .open       = xmp_open,
	.access     = xmp_access,
	.utimens    = xmp_utimens,
    .truncate   = xmp_truncate,
    .rename     = xmp_rename,
    .unlink     = xmp_unlink,
};

int main(int argc, char *argv[]){
    umask(0);
    sprintf(cipher, "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%crI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO", '%');
    return fuse_main(argc, argv, &xmp_oper, NULL); 
}