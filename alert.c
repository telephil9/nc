#include "a.h"

enum { Padding = 8, };

static
int
max(int a, int b)
{
	return a>b ? a : b;
}

void
alert(const char *title, const char *message, const char *err, Mousectl *mctl, Keyboardctl *kctl)
{
	Alt alts[3];
	Rectangle r, sc;
	Point o, p;
	Image *b, *save, *bg, *fg;
	int done, h, w, tw, mw, ew;
	Mouse m;
	Rune k;

	alts[0].op = CHANRCV;
	alts[0].c  = mctl->c;
	alts[0].v  = &m;
	alts[1].op = CHANRCV;
	alts[1].c  = kctl->c;
	alts[1].v  = &k;
	alts[2].op = CHANEND;
	alts[2].c  = nil;
	alts[2].v  = nil;
	while(nbrecv(kctl->c, nil)==1)
		;
	bg = cols[Cdialog];
	fg = cols[Cfg];
	done = 0;
	save = nil;
	h = Padding+2*font->height+Padding;
	if(err != nil)
		h += font->height;
	tw = stringwidth(font, title);
	mw = stringwidth(font, message);
	ew = err != nil ? stringwidth(font, err) : 0;
	w = Padding+max(tw, max(mw, ew))+Padding;
	b = screen;
	sc = b->clipr;
	replclipr(b, 0, b->r);
	while(!done){
		o = addpt(screen->r.min, Pt((Dx(screen->r)-w)/2, (Dy(screen->r)-h)/2));
		r = Rect(o.x, o.y, o.x+w, o.y+h);
		if(save==nil){
			save = allocimage(display, r, b->chan, 0, DNofill);
			if(save==nil)
				break;
			draw(save, r, b, nil, r.min);
		}
		draw(b, r, bg, nil, ZP);
		border(b, r, 3, fg, ZP);
		p = addpt(o, Pt(Padding, Padding));
		string(b, p, fg, ZP, font, title);
		p.y += font->height;
		string(b, p, fg, ZP, font, message);
		if(err != nil){
			p.x = o.x + Padding;
			p.y += font->height;
			string(b, p, fg, ZP, font, err);
		}
		flushimage(display, 1);
		if(b!=screen || !eqrect(screen->clipr, sc)){
			freeimage(save);
			save = nil;
		}
		b = screen;
		sc = b->clipr;
		replclipr(b, 0, b->r);
		switch(alt(alts)){
		default:
			continue;
			break;
		case 1:
			done = (k=='\n' || k==Kesc);
			break;
		case 0:
			done = m.buttons&1 && ptinrect(m.xy, r);
			break;
		}
		if(save){
			draw(b, save->r, save, nil, save->r.min);
			freeimage(save);
			save = nil;
		}
			
	}
	replclipr(b, 0, sc);
	flushimage(display, 1);
}