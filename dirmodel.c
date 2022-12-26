#include "a.h"

static int
dircmpname(Dir *a, Dir *b)
{
	if(a->qid.type == b->qid.type)
		return strcmp(a->name, b->name);
	if(a->qid.type&QTDIR)
		return -1;
	return 1;
} 

static void
loadpath(Dirmodel *m)
{
	char buf[1024];
	int fd;
	Dir *t, *d;
	
	m->isroot = strcmp(m->path, "/") == 0;
	fd = open(m->path, OREAD);
	/* FIXME: error handling */
	m->ndirs = dirreadall(fd, &m->dirs);
	if(m->ndirs > 0)
		qsort(m->dirs, m->ndirs, sizeof *m->dirs, (int(*)(void*,void*))dircmpname);
	close(fd);
	if(!m->isroot){
		t = emalloc((m->ndirs + 1) * sizeof(Dir));
		memmove(&t[1], m->dirs, m->ndirs*sizeof(Dir));
		m->dirs = t;
		m->ndirs++;
		snprint(buf, sizeof buf, "%s/..", m->path);
		d = dirstat(buf);
		memmove(&m->dirs[0], &d[0], sizeof(Dir));
		m->dirs[0].name = "..";
	}
	m->sel = emalloc(m->ndirs * sizeof(uchar));
	memset(m->sel, 0, m->ndirs * sizeof(uchar));
}

void
dirmodelreload(Dirmodel *m)
{
	free(m->dirs);
	free(m->sel);
	loadpath(m);
	sendul(m->c, 1);
}

void
dirmodelcd(Dirmodel *m, char *p)
{
	char newpath[1024] = {0};

	if(p[0] == '/')
		snprint(newpath, sizeof newpath, "%s", p);
	else
		snprint(newpath, sizeof newpath, "%s/%s", m->path, p);
	if(access(newpath, 0)<0)
		sysfatal("directory does not exist: %r");
	free(m->path);
	m->path = abspath(m->path, newpath);
	free(m->filter);
	m->filter = nil;
	free(m->fdirs);
	m->fndirs = 0;
	dirmodelreload(m);
}	

Dirmodel*
mkdirmodel(char *path)
{
	Dirmodel *dm;
	
	dm = emalloc(sizeof *dm);
	dm->c = chancreate(sizeof(ulong), 1);
	dm->path = strdup(path);
	dm->filter = nil;
	dm->fdirs = nil;
	loadpath(dm);
	return dm;
}

Dir
dirmodelgetdir(Dirmodel *m, int i)
{
	if(m->filter != nil)
		return m->fdirs[i];
	return m->dirs[i];
}

long
dirmodelcount(Dirmodel *m)
{
	if(m->filter != nil)
		return m->fndirs;
	return m->ndirs;
}

void
dirmodelfilter(Dirmodel *m, char *p)
{
	char buf[1024];
	int fd, i;
	Dir *d, *u;
	long n;

	if(p == nil){
		free(m->filter);
		m->filter = nil;
		free(m->fdirs);
		m->fndirs = 0;
		sendul(m->c, 1);
		return;
	}
	fd = open(m->path, OREAD);
	/* FIXME: error handling */
	n = dirreadall(fd, &d);
	if(n > 0)
		qsort(d, n, sizeof *d, (int(*)(void*,void*))dircmpname);
	close(fd);
	m->fdirs = emalloc((n+1) * sizeof(Dir));
	if(!m->isroot){
		snprint(buf, sizeof buf, "%s/..", m->path);
		u = dirstat(buf);
		memmove(&m->fdirs[0], &u[0], sizeof(Dir));
		m->fdirs[0].name = "..";
		m->fndirs++;
	}
	for(i = 0; i < n; i++){
		if((d[i].qid.type&QTDIR) || match(d[i].name, p))
			memmove(&m->fdirs[m->fndirs++], &d[i], sizeof(Dir));
	}
	memset(m->sel, 0, m->ndirs * sizeof(uchar));
	m->filter = strdup(p);
	sendul(m->c, 1);
}
