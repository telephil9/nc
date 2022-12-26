#include "a.h"

Image *bg;

void
actionbarsetrect(Actionbar *abar, Rectangle r)
{
	abar->r = r;
	freeimage(abar->b);
	abar->b = nil;
}

void
actionbarredraw(Actionbar *abar)
{
	Rectangle r, wr;
	Point p;
	int i, w;
	char buf[16];
	
	r = Rect(0, 0, Dx(abar->r), Dy(abar->r));
	if(abar->b == nil)
		abar->b = allocimage(display, r, screen->chan, 0, DNofill);
	draw(abar->b, r, cols[Cbg], nil, ZP);
	w = Dx(r)/10;
	p = Pt(0, 2);
	for(i = 0; i < 10; i++){
		p.x = i*w;
		snprint(buf, sizeof buf, "%2d", i+1);
		p = string(abar->b, p, cols[Cfg], ZP, font, buf);
		wr = Rect(p.x, 1, (i+1)*w - 1, r.max.y - 1);
		draw(abar->b, wr, cols[Ctitle], nil, ZP);
		p.x += 2;
		if(abar->labels[i] != nil)
			string(abar->b, p, cols[Cfg], ZP, font, abar->labels[i]);
	}
}

void
actionbaremouse(Actionbar *abar, Mouse m)
{
	int w, i;

	if(!(ptinrect(m.xy, abar->r) && m.buttons == 4))
		return;
	w = Dx(abar->r) / 10;
	i = (m.xy.x - screen->r.min.x) / w;
	if(abar->actions[i] != nil)
		abar->actions[i]();
}

Actionbar*
mkactionbar(void)
{
	Actionbar *abar;

	abar = emalloc(sizeof *abar);
	abar->b = nil;
	return abar;
}

void
actionbarclear(Actionbar *abar)
{
	int i;

	for(i = 0; i < 10; i++){
		abar->labels[i] = nil;
		abar->actions[i] = nil;
	}
}

void
actionbarset(Actionbar *abar, int index, char *label, Action action)
{
	if(index < 1 || index > 10)
		return;
	abar->labels[index-1] = label;
	abar->actions[index-1] = action;
}
