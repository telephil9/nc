#include "a.h"

int
mkdir(char *wd, char *name)
{
	char *p;
	int fd;
	
	p = abspath(wd, name);
	if(access(p, 0) >= 0){
		werrstr("directory already exists");
		free(p);
		return -1;
	}
	fd = create(p, OREAD, DMDIR|0755);
	if(fd < 0){
		free(p);
		return -1;
	}
	free(p);
	close(fd);
	return 0;
}

int
rmdir(char *path)
{
	Dir *dirs;
	int i, fd, ndirs;
	
	fd = open(path, OREAD);
	if(fd < 0)
		return -1;
	ndirs = dirreadall(fd, &dirs);
	close(fd);
	if(ndirs < 0)
		return -1;
	for(i = 0; i < ndirs; i++){
		if(rm(path, dirs[i]) < 0){
			free(dirs);
			return -1;
		}
	}
	free(dirs);
	if(remove(path) < 0)
		return -1;
	return 0;
}

int
rm(char *path, Dir d)
{
	char buf[1024] = {0};
	int rc;

	snprint(buf, sizeof buf, "%s/%s", path, d.name);
	if(d.qid.type&QTDIR){
		rc = rmdir(buf);
	}else{
		rc = remove(buf);
	}
	return rc;
}
