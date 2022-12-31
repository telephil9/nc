#include "a.h"

void
datestr(char *buf, ulong bufsz, long dt)
{
	char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
			   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	Tm *tm;

	tm = localtime(dt);
	snprint(buf, bufsz, "%s %02d %02d:%02d", months[tm->mon], tm->mday, tm->hour, tm->min);
}

Dirpanel*
mkdirpanel(Dirmodel *dm)
{
	Dirpanel *dp;
	
	dp = emalloc(sizeof *dp);
	dp->c = chancreate(sizeof(ulong), 1);
	dp->b = nil;
	dp->model = dm;
	dp->focused = 0;
	dp->offset = 0;
	dp->cursor = 0;
	return dp;
}

void
dirpanelsetrect(Dirpanel *p, Rectangle r)
{
	p->r = r;
	freeimage(p->b);
	p->b = nil;
	p->intr = insetrect(boundsrect(r), 2);
	p->titler = p->intr;
	p->titler.max.y = p->titler.min.y + 2 + font->height + 2;
	p->filesr = p->intr;
	p->filesr.min.y = p->titler.max.y;
	p->filesr.max.y -= 1;
	p->nlines = Dy(p->filesr) / (font->height + 2);
}

void
drawfname(Dirpanel *p, Point pt, Image *f, char *t, int xmax)
{
	char *s;
	Rune r;
	int i;
	
	s = t;
	for(i = 0; *s && i < 2; i++){
		s += chartorune(&r, s);
		pt = runestringn(p->b, pt, f, ZP, font, &r, 1);
	}
	if(*s && (pt.x + stringwidth(font, s) > xmax)){
		chartorune(&r, "~");
		pt = runestringn(p->b, pt, f, ZP, font, &r, 1);
		while(*s && (pt.x + stringwidth(font, s) > xmax))
			++s;
	}
	while(*s){
		s += chartorune(&r, s);
		pt = runestringn(p->b, pt, f, ZP, font, &r, 1);
	}
}

void
drawline(Dirpanel *p, int index) 
{
	Rectangle r;
	Image *b, *f;
	Point pr, pt;
	char buf[32];
	Dir d;

	d = dirmodelgetdir(p->model, p->offset + index);
	r = p->filesr;
	r.min.x += 2;
	r.min.y += index * (1 + font->height + 1);
	r.max.x -= 2;
	r.max.y = r.min.y + 1 + font->height + 1;
	f = cols[Cfg];
	b = cols[Cbg];
	if(index == p->cursor && p->focused){
		f = cols[Cselfg];
		b = cols[Csel];
	}
	if(p->model->sel[p->offset + index])
		f = cols[Ctitle];
	else if(!p->focused)
		f = cols[Clfg];
	draw(p->b, r, b, nil, ZP);
	pt = addpt(r.min, Pt(4, 1));
	pt = string(p->b, pt, f, ZP, font, (d.qid.type&QTDIR) ? "/" : " ");
	drawfname(p, pt, f, d.name, p->colw[0] - 4);
	pt.x = p->filesr.min.x + p->colw[0] + 4;
	snprint(buf, sizeof buf, "%*lld", 6, d.length);
	string(p->b, pt, f, ZP, font, buf);
	pt.x = p->filesr.max.x - p->colw[2] + 4;
	datestr(buf, sizeof buf, d.mtime);
	string(p->b, pt, f, ZP, font, buf);
	pr = addpt(r.min, Pt(p->colw[0] - 2, 0));
	pt = addpt(r.min, Pt(p->colw[0] - 2, Dy(r) + 1));
	line(p->b, pr, pt, 0, 0, 0, cols[Cborder], ZP);
	pr = addpt(pr, Pt(p->colw[1], 0));
	pt = addpt(pt, Pt(p->colw[1], 0));
	line(p->b, pr, pt, 0, 0, 0, cols[Cborder], ZP);
}

int
sizecolwidth(Dirpanel *p)
{
	vlong m;
	int i, n;
	Dir d;
	
	m = 1;
	for(i = 0; i < dirmodelcount(p->model); i++){
		d = dirmodelgetdir(p->model, i);
		if(d.length > m)
			m = d.length;
	}
	n = 6;
	if(m != 0) n = 1 + log(m)/log(10);
	if(n < 6)  n = 6;
	return n * stringwidth(font, "0");
}

void
dirpanelredraw(Dirpanel *p)
{
	Rectangle r, clipr, ir;
	Point pr, pt;
	Image *b;
	int i;

	p->colw[2] = 4 + stringwidth(font, "XXX 99 99:99") + 4;
	p->colw[1] = 4 + sizecolwidth(p) + 4;
	p->colw[0] = Dx(p->filesr) - (p->colw[1] + p->colw[2]);
	r = boundsrect(p->r);
	if(p->b == nil)
		p->b = allocimage(display, r, screen->chan, 0, DNofill);
	draw(p->b, r, cols[Cbg], nil, ZP);
	b = p->focused ? cols[Ctitle] : cols[Cborder];
	ir = insetrect(r, 2);
	border(p->b, ir, 2, b, ZP);
	pt = string(p->b, addpt(ir.min, Pt(4, 2)), p->focused ? cols[Cfg] : cols[Clfg], ZP, font, p->model->path);
	if(p->model->filter != nil){
		pt = string(p->b, pt, cols[Clfg], ZP, font, " [");
		pt = string(p->b, pt, cols[Cfg], ZP, font, p->model->filter);
		pt = string(p->b, pt, cols[Clfg], ZP, font, "]");
	}
	pr = Pt(0, ir.min.y + 2 + font->height + 2);
	line(p->b, addpt(pr, Pt(ir.min.x, 0)), addpt(pr, Pt(ir.max.x, 0)), 0, 0, 1, b, ZP);
	pt = addpt(p->filesr.min, Pt(4, 1));
	clipr = p->b->clipr;
	replclipr(p->b, 0, p->filesr);
	for(i = 0; ; i++){
		if(i >= p->nlines || (p->offset + i) >= dirmodelcount(p->model))
			break;
		drawline(p, i);
	}
	replclipr(p->b, 0, clipr);
	pr = addpt(p->filesr.min, Pt(p->colw[0], 1));
	pt = addpt(p->filesr.min, Pt(p->colw[0], Dy(p->filesr) - 1));
	line(p->b, pr, pt, 0, 0, 0, cols[Cborder], ZP);
	pr = addpt(pr, Pt(p->colw[1], 0));
	pt = addpt(pt, Pt(p->colw[1], 0));
	line(p->b, pr, pt, 0, 0, 0, cols[Cborder], ZP);
}

void
dirpanelredrawnotify(Dirpanel *p)
{
	dirpanelredraw(p);
	sendul(p->c, 1);
}

void
dirpanelemouse(Dirpanel *p, Mouse m)
{
	Point pt;
	int n;

	pt = subpt(m.xy, p->r.min);
	if(!ptinrect(pt, p->filesr))
		return;
	if(m.buttons == 1){
		n = (pt.y - p->filesr.min.y) / (font->height+2);
		if(n != p->cursor && n < p->nlines){
			p->cursor = n;
			dirpanelredrawnotify(p);
		}
	}else if(m.buttons == 4){
		n = (pt.y - p->filesr.min.y) / (font->height+2);
		if(n < p->nlines){
			p->cursor = n;
			sendul(kc->c, L'\n');
		}
	}else if(m.buttons == 8)
		sendul(kc->c, Kup);
	else if(m.buttons == 16)
		sendul(kc->c, Kdown);
}

void
dirpanelresetcursor(Dirpanel *p)
{
	p->cursor = 0;
	p->offset = 0;
}

int
dirpanelselectedindex(Dirpanel *p)
{
	return p->offset + p->cursor;
}
