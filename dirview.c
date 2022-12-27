#include "a.h"

void
dirviewsetrect(Dirview *d, Rectangle r)
{
	Rectangle lr, rr;

	d->r = r;
	freeimage(d->b);
	d->b = nil;
	lr = r;
	lr.max.x = r.min.x + Dx(r)/2;
	rr = r;
	rr.min.x = lr.max.x;
	dirpanelsetrect(d->leftp, lr);
	dirpanelsetrect(d->rightp, rr);
}

void
dirviewredraw(Dirview *d)
{
	Rectangle r, lr, rr;
	
	r = Rect(0, 0, Dx(d->r), Dy(d->r));
	lr = r;
	lr.max.x = r.min.x + Dx(r)/2;
	d->leftr = lr;
	rr = r;
	rr.min.x = lr.max.x + 1;
	d->rightr = rr;
	if(d->b == nil)
		d->b = allocimage(display, r, screen->chan, 0, DNofill);
	dirpanelredraw(d->leftp);
	dirpanelredraw(d->rightp);
	draw(d->b, r, cols[Cbg], nil, ZP);
	draw(d->b, lr, d->leftp->b, nil, ZP);
	draw(d->b, rr, d->rightp->b, nil, ZP);
}

void
switchfocus(Dirview *v)
{
	v->leftp->focused = !v->leftp->focused;
	v->rightp->focused = !v->rightp->focused;
	dirviewredraw(v);
	sendul(v->c, 1);
}

void
dirviewemouse(Dirview *v, Mouse m)
{
	if(m.buttons != 1)
		return;
	if((ptinrect(m.xy, v->leftp->r) && !v->leftp->focused) 
	|| (ptinrect(m.xy, v->rightp->r) && !v->rightp->focused))
		switchfocus(v);
}

Dirview*
mkdirview(char *lpath, char *rpath)
{
	Dirview *dv;
	Dirmodel *m;
	
	dv = emalloc(sizeof *dv);
	dv->c = chancreate(sizeof(ulong), 1);
	dv->b = nil;
	m = mkdirmodel(lpath);
	dv->leftp = mkdirpanel(m);
	dv->leftp->focused = 1;
	m = mkdirmodel(rpath);
	dv->rightp = mkdirpanel(m);
	return dv;
}

Dirpanel*
dirviewcurrentpanel(Dirview *v)
{
	if(v->leftp->focused)
		return v->leftp;
	return v->rightp;
}

Dirpanel*
dirviewotherpanel(Dirview *v)
{
	if(v->leftp->focused)
		return v->rightp;
	return v->leftp;
}
