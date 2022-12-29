#include "a.h"

static long
dirlist(Dirpanel *p, Dir **d)
{
	long n;

	n = dirmodelmarklist(p->model, d);
	if(n == 0){
		n = dirpanelselectedindex(p);
		if(n == 0 && !p->model->isroot) /* up dir */
			return 0;
		*d = emalloc(sizeof(Dir));
		**d = dirmodelgetdir(p->model, n);
		n = 1;
	}
	return n;
}

static void
cmdhelp(void)
{
	textset(text, "Help", help, strlen(help));
	setmode(Mhelp);
}

static void
cmdview(void)
{
	Dirpanel *p;
	Dir d;
	char *s, *t;

	p = dirviewcurrentpanel(dview);
	d = dirmodelgetdir(p->model, dirpanelselectedindex(p));
	if(d.qid.type&QTDIR){
		p->cursor = 0;
		p->offset = 0;
		dirmodelcd(p->model, d.name);
	}else{
		t = smprint("%s/%s", p->model->path, d.name);
		s = slurp(t);
		textset(text, t, s, strlen(s));
		setmode(Mview);
	}
}

static void
cmdplumb(void)
{
	Dirpanel *p;
	Dir d;
	int fd;
	char buf[1024];
	
	p = dirviewcurrentpanel(dview);
	d = dirmodelgetdir(p->model, dirpanelselectedindex(p));
	fd = plumbopen("send", OWRITE|OCEXEC);
	snprint(buf, sizeof buf, "%s/%s", p->model->path, d.name);
	if(d.qid.type&QTDIR)
		plumbsendtext(fd, "nc", nil, nil, buf);
	else
		plumbsendtext(fd, "nc", nil, p->model->path, buf);
	close(fd);
}

static void
cmdcopy(void)
{
	Dirpanel *p, *o;
	Dir *d;
	int nd, n;
	char buf[1024] = {0};
	
	p = dirviewcurrentpanel(dview);
	o = dirviewotherpanel(dview);
	nd = dirlist(p, &d);
	if(nd == 1)
		snprint(buf, sizeof buf, "copy %s '%s' to '%s' ?", 
			(d[0].qid.type&QTDIR) ? "directory" : "file", d[0].name, o->model->path);
	else
		snprint(buf, sizeof buf, "copy %d files/directories ?", nd);
	if(message(Dconfirm, buf, mc, kc) == Bno)
		return;
	for(n = 0; n < nd; n++){
		if(cp(p->model->path, d[n], o->model->path, nil) < 0){
			errormessage("copy failed: %r", mc, kc);
			return;
		}
	}
	dirmodelreload(o->model);
}

static void
cmdrenmov(void)
{
	Dirpanel *p, *o;
	Dir *d, null;
	char opath[1024] = {0}, buf[255] = {0};
	int i, n;

	p = dirviewcurrentpanel(dview);
	o = dirviewotherpanel(dview);
	n = dirlist(p, &d);
	if(dirmodeleq(p->model, o->model)){
		if(n > 1){
			errormessage("cannot rename multiple files/directories.", mc, kc);
			return;
		}
		snprint(buf, sizeof buf, d[0].name);
		if((n = input("rename to:", buf, sizeof buf, mc, kc)) <= 0)
			return;
		if(strncmp(buf, d[0].name, n) == 0){
			errormessage("target file name should be different.", mc, kc);
			return;
		}
		if(d[0].qid.type&QTDIR){
			if(cp(p->model->path, d[0], o->model->path, buf) < 0){
				errormessage("rename failed: %r", mc, kc);
				return;
			}
			if(rm(p->model->path, d[0]) < 0){
				errormessage("rename failed: %r", mc, kc);
				return;
			}
		}else{
			snprint(opath, sizeof opath, "%s/%s", p->model->path, d[0].name);
			nulldir(&null);
			null.name = buf;
			if(dirwstat(opath, &null) < 0){
				errormessage("rename failed: %r", mc, kc);
				return;
			}
		}
	}else{
		if(n == 1)
			snprint(buf, sizeof buf, "move %s '%s' to '%s' ?", 
				(d[0].qid.type&QTDIR) ? "directory" : "file", d[0].name, o->model->path);
		else
			snprint(buf, sizeof buf, "move %d files/directories ?", n);
		if(message(Dconfirm, buf, mc, kc) == Bno)
			return;
		for(i = 0; i < n; i++){
			if(cp(p->model->path, d[i], o->model->path, nil) < 0){
				errormessage("move failed: %r", mc, kc);
				return;
			}
			if(rm(p->model->path, d[i]) < 0){
				errormessage("move failed: %r", mc, kc);
				return;
			}
		}
	}
	dirmodelreload(p->model);
	dirmodelreload(o->model);		
	return;	
}

static void
cmdmkdir(void)
{
	Dirpanel *p;
	char buf[1024] = {0};

	p = dirviewcurrentpanel(dview);
	if(input("create directory:", buf, sizeof buf, mc, kc) <= 0)
		return;
	if(mkdir(p->model->path, buf) < 0){
		errormessage("directory creation failed: %r", mc, kc);
		return;
	}
	dirmodelreload(p->model);
	dirmodelreloadifsame(p->model, dirviewotherpanel(dview)->model);
}

static void
cmddelete(void)
{
	Dirpanel *p, *o;
	Dir *d;
	char buf[1024] = {0};
	int i, n;

	p = dirviewcurrentpanel(dview);
	o = dirviewotherpanel(dview);
	n = dirlist(p, &d);
	if(n == 1)
		snprint(buf, sizeof buf, "delete %s '%s' ?", (d[0].qid.type&QTDIR) ? "directory" : "file", d[0].name);
	else
		snprint(buf, sizeof buf, "delete %d files/directories ?", n);
	if(message(Dconfirm, buf, mc, kc) == Bno)
		return;
	for(i = 0; i < n; i++){
		if(rm(p->model->path, d[i]) < 0){
			errormessage("delete failed: %r", mc, kc);
			return;
		}
	}
	dirmodelreload(p->model);
	dirmodelreloadifsame(p->model, o->model);
}

static void
cmdquit(void)
{
	threadexitsall(nil);
}

static void
cmdswitchfocus(void)
{
	dview->leftp->focused = !dview->leftp->focused;
	dview->rightp->focused = !dview->rightp->focused;
	dirviewredraw(dview);
	sendul(dview->c, 1);
}

static void
cmdreload(void)
{
	Dirpanel *p;
	
	p = dirviewcurrentpanel(dview);
	dirmodelreload(p->model);
}

static void
cmdcd(void)
{
	Dirpanel *p;
	char buf[1024] = {0};

	p = dirviewcurrentpanel(dview);
	if(input("go to directory:", buf, sizeof buf, mc, kc) <= 0)
		return;
	dirpanelresetcursor(p);
	dirmodelcd(p->model, buf);
}

static void
cmdcdo(void)
{
	Dirpanel *p;
	char buf[1024] = {0};

	p = dirviewotherpanel(dview);
	if(input("go to directory:", buf, sizeof buf, mc, kc) <= 0)
		return;
	dirpanelresetcursor(p);
	dirmodelcd(p->model, buf);
}

static void
cmdcdmatch(void)
{
	Dirpanel *o;
	char *path;

	path = dirviewcurrentpanel(dview)->model->path;
	o = dirviewotherpanel(dview);
	dirpanelresetcursor(o);
	dirmodelcd(o->model, path);
}

static void
cmdselectgroup(void)
{
	Dirpanel *p;
	Dir d;
	int i;
	char buf[256] = {0};

	p = dirviewcurrentpanel(dview);
	if(input("select pattern:", buf, sizeof(buf), mc, kc) <= 0)
		return;
	for(i = !p->model->isroot; i < dirmodelcount(p->model); i++){
		d = dirmodelgetdir(p->model, i);
		p->model->sel[i] = match(d.name, buf);
	}
	dirpanelredrawnotify(p);
}

static void
cmdunselectgroup(void)
{
	Dirpanel *p;
	Dir d;
	int i, r;
	char buf[256] = {0};

	p = dirviewcurrentpanel(dview);
	if(input("unselect pattern:", buf, sizeof(buf), mc, kc) <= 0)
		return;
	r = 0;
	for(i = !p->model->isroot; i < dirmodelcount(p->model); i++){
		d = dirmodelgetdir(p->model, i);
		if(match(d.name, buf) && p->model->sel[i]){
			p->model->sel[i] = 0;
			r = 1;
		}
	}
	if(r)
		dirpanelredrawnotify(p);
}

static void
cmdinvertselection(void)
{
	Dirpanel *p;
	int i;

	p = dirviewcurrentpanel(dview);
	for(i = !p->model->isroot; i < dirmodelcount(p->model); i++)
		p->model->sel[i] = !p->model->sel[i];
	dirpanelredrawnotify(p);
}

static void
cmdselect(void)
{
	Dirpanel *p;
	Dir d;
	int i;

	p = dirviewcurrentpanel(dview);
	i = p->offset + p->cursor;
	d = dirmodelgetdir(p->model, i);
	if(i == 0 && !p->model->isroot)
		return;
	p->model->sel[i] = !p->model->sel[i];
	if(p->cursor == p->nlines - 1 && (p->offset + p->nlines >= dirmodelcount(p->model)))
		goto Draw;
	if(p->offset + p->cursor + 1 >= dirmodelcount(p->model))
		goto Draw;
	if(p->cursor == p->nlines - 1){
		p->offset += p->nlines;
		p->cursor = 0;
	}else{
		p->cursor += 1;
	}
Draw:
	dirpanelredrawnotify(p);
}

static void
cmdfilter(void)
{
	Dirpanel *p;
	char buf[256] = {0};

	p = dirviewcurrentpanel(dview);
	if(input("filter pattern:", buf, sizeof buf, mc, kc) <= 0){
		if(p->model->filter != nil){
			dirpanelresetcursor(p);
			dirmodelfilter(p->model, nil);
		}
	}else{
		dirpanelresetcursor(p);
		dirmodelfilter(p->model, buf);
	}
}

static void
cmdup(void)
{
	Dirpanel *p;

	p = dirviewcurrentpanel(dview);
	if(p->cursor == 0 && p->offset == 0)
		return;
	if(p->cursor == 0){
		p->offset -= p->nlines;
		if(p->offset < 0)
			p->offset = 0;
		p->cursor = p->nlines -1;
	}else{
		p->cursor -= 1;
	}
	dirpanelredrawnotify(p);
}

static void
cmddown(void)
{
	Dirpanel *p;

	p = dirviewcurrentpanel(dview);
	if(p->cursor == p->nlines - 1 && (p->offset + p->nlines >= dirmodelcount(p->model)))
		return;
	if(p->offset + p->cursor + 1 >= dirmodelcount(p->model))
		return;
	if(p->cursor == p->nlines - 1){
		p->offset += p->nlines;
		p->cursor = 0;
		if(dirmodelcount(p->model)-p->offset < p->nlines)
			p->offset = dirmodelcount(p->model) - p->nlines;
	}else{
		p->cursor += 1;
	}
	dirpanelredrawnotify(p);
}

static void
cmdhome(void)
{
	Dirpanel *p;

	p = dirviewcurrentpanel(dview);
	dirpanelresetcursor(p);
	dirpanelredrawnotify(p);
}

static void
cmdend(void)
{
	Dirpanel *p;

	p = dirviewcurrentpanel(dview);
	p->offset = p->nlines * (dirmodelcount(p->model) / p->nlines);
	if(dirmodelcount(p->model)-p->offset < p->nlines)
		p->offset = dirmodelcount(p->model) - p->nlines;
	if(p->offset < 0)
		p->offset = 0;
	p->cursor = dirmodelcount(p->model) - p->offset - 1;
	dirpanelredrawnotify(p);
}

static void
cmdpageup(void)
{
	Dirpanel *p;

	p = dirviewcurrentpanel(dview);
	if(p->offset == 0 && p->cursor == 0)
		return;
	if(p->offset == 0)
		p->cursor = 0;
	else
		p->offset -= p->nlines;
	if(p->offset < 0)
		p->offset = 0;
	dirpanelredrawnotify(p);
}

static void
cmdpagedown(void)
{
	Dirpanel *p;
	int end;

	p = dirviewcurrentpanel(dview);
	end = dirmodelcount(p->model) - p->offset - 1;
	if(p->offset + p->nlines >= dirmodelcount(p->model) && p->cursor == end)
		return;
	if(p->offset + p->nlines < dirmodelcount(p->model)){
		p->offset += p->nlines;
		if(dirmodelcount(p->model)-p->offset < p->nlines)
			p->offset = dirmodelcount(p->model) - p->nlines;		
	}else
		p->cursor = end;
	if(p->cursor > end)
		p->cursor = end;
	dirpanelredrawnotify(p);
}

Binding	dirviewbindings[] = {
	{ KF|1,		cmdhelp },
	{ KF|3,		cmdview },
	{ KF|4,		cmdplumb },
	{ KF|5,		cmdcopy },
	{ KF|6,		cmdrenmov },
	{ KF|7,		cmdmkdir },
	{ KF|8,		cmddelete },
	{ KF|10,	cmdquit },
	{ '\t',		cmdswitchfocus },
	{ 'r',		cmdreload },
	{ 'c',		cmdcd },
	{ 'C',		cmdcdo },
	{ '=',		cmdcdmatch },
	{ '\n',		cmdview },
	{ '+',		cmdselectgroup },
	{ '-',		cmdunselectgroup },
	{ '*',		cmdinvertselection },
	{ Kins,		cmdselect },
	{ 'f',		cmdfilter },
	{ Kup,		cmdup },
	{ Kdown,	cmddown },
	{ Khome,	cmdhome },
	{ Kend,		cmdend },
	{ Kpgup,	cmdpageup },
	{ Kpgdown,	cmdpagedown },
	nil
};

void
setupdirviewbindings(void)
{
	bindings = dirviewbindings;
	actionbarclear(abar);
	actionbarset(abar, 1, "Help",   cmdhelp);
	actionbarset(abar, 3, "View",   cmdview);
	actionbarset(abar, 4, "Plumb",  cmdplumb);
	actionbarset(abar, 5, "Copy",   cmdcopy);
	actionbarset(abar, 6, "RenMov", cmdrenmov);
	actionbarset(abar, 7, "Mkdir",  cmdmkdir);
	actionbarset(abar, 8, "Delete", cmddelete);
	actionbarset(abar, 10, "Quit",  cmdquit);
}
