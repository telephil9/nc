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

static int
fcopy(char *ipath, char *in, char *opath, char *out, int mode)
{
	int rc, ifd, ofd, r, w;
	char buf[8192];
	
	snprint(buf, sizeof buf, "%s/%s", ipath, in);
	ifd = open(buf, OREAD);
	if(ifd < 0)
		return -1;
	snprint(buf, sizeof buf, "%s/%s", opath, out);
	ofd = create(buf, OWRITE, mode);
	if(ofd < 0){
		close(ifd);
		return -1;
	}
	rc = 0;
	for(;;){
		r = read(ifd, buf, sizeof buf);
		if(r < 0){
			rc = -1;
			break;
		}else if(r == 0)
			break;
		w = write(ofd, buf, r);
		if(w != r){
			rc = -1;
			break;
		}
	}
	close(ifd);
	close(ofd);
	return rc;
}

static int
dircp(char *ipath, Dir d, char *opath, char *oname)
{
	char outp[1024], inp[1024];
	Dir *dirs;
	int i, n, fd;
	
	snprint(outp, sizeof outp, "%s/%s", opath, oname ? oname : d.name);
	if(access(outp, 0) < 0){
		fd = create(outp, OREAD, DMDIR|0755);
		if(fd < 0)
			return -1;
		close(fd);
	}
	snprint(inp, sizeof inp, "%s/%s", ipath, d.name);
	fd = open(inp, OREAD);
	if(fd < 0)
		return -1;
	n = dirreadall(fd, &dirs);
	if(n < 0){
		close(fd);
		return -1;
	}
	close(fd);
	for(i = 0; i < n; i++){
		if(cp(inp, dirs[i], outp, dirs[i].name) < 0){
			free(dirs);
			return -1;
		}
	}
	free(dirs);
	return 0;
}

int
cp(char *ipath, Dir d, char *opath, char *oname)
{
	int rc;

	if(d.qid.type&QTDIR){
		rc = dircp(ipath, d, opath, oname);
	}else
		rc = fcopy(ipath, d.name, opath, oname ? oname : d.name, d.mode);
	return rc;
}
