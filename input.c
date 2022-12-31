#include "a.h"

enum{ 
	Border = 1, 
	Padding = 8, 
};

static
int
max(int a, int b)
{
	return a>b ? a : b;
}

static void
button(Image *b, Rectangle br, char *label, int focus)
{
	Point p;
	int dx;

	dx = (Dx(br) - stringwidth(font, label)) / 2;
	border(b, br, 2, focus ? cols[Ctitle] : cols[Cdlgbord], ZP);
	p = addpt(br.min, Pt(dx, 0.25*font->height));
	p = stringn(b, p, cols[Ctitle], ZP, font, label, 1);
	string(b, p, cols[Cfg], ZP, font, label+1);
}

static void
entry(Image *b, Rectangle r, char *text, int focus)
{
	Point p;

	border(b, r, 2, focus ? cols[Ctitle] : cols[Csel], ZP);
	p = addpt(r.min, Pt(Padding, Padding/2));
	p = string(b, p, cols[Cfg], ZP, font, text);
	if(focus)
		draw(b, rectaddpt(tick->r, p), tick, nil, ZP);
}

int
input(char *ask, char *buf, int nbuf, Mousectl *mctl, Keyboardctl *kctl)
{
	Alt alts[3];
	Rectangle r, er, br, brn, sc;
	Point o, p;
	Image *b, *save, *bg, *fg, *hi;
	int rc, done, len, h, w, ew, eh, bw, bh, mw, dy, focus;
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
	rc = -1;
	len = strlen(buf);
	bg = cols[Cdlgbg];
	fg = cols[Cfg];
	hi = cols[Ctitle];
	focus = 0;
	done = 0;
	save = nil;
	bw = 2*stringwidth(font, "Cancel");
	bh = 1.5*font->height;
	mw = stringwidth(font, ask);
	w = Border+Padding+3*mw+Padding+Border;
	ew = w - 2*(Border+Padding);
	eh = Padding+font->height;
	h = Border+Padding+2*font->height+Padding+eh+Padding+bh+Padding+Border;
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
		border(b, r, Border, cols[Cdlgbord], ZP);
		p = addpt(o, Pt(0, 2));
		line(b, p, Pt(r.max.x, p.y), 0, 0, 2, hi, ZP);
		p = addpt(o, Pt(Border+Padding, Border+Padding+0.5*font->height));
		string(b, p, fg, ZP, font, ask);
		p.y += Padding+1.5*font->height;
		dy = Border+Padding+1.5*font->height+Padding;
		er = rectaddpt(Rect(0, 0, ew, eh), addpt(o, Pt(Border+Padding, dy)));
		entry(b, er, buf, focus == 0);
		dy += eh+0.5*font->height+Padding;
		br = rectaddpt(Rect(0, 0, bw, bh), addpt(o, Pt(Border+Padding, dy)));
		button(b, br, "Ok", focus == 1);
		brn = rectaddpt(br, Pt(bw+Padding, 0));
		button(b, brn, "Cancel", focus == 2);
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
			if(k == '\t'){
				focus = (focus + 1) % 3;
			}else if(focus != 0 && (k == 'o' || k == 'O')){
				done = 1;
				rc = len;
			}else if(k == '\n'){
				done = 1;
				if(focus == 0 || focus == 1)
					rc = len;
				else
					rc = -1;
			}else if(focus != 0 && (k == 'c' || k == 'C')){
				done = 1;
				rc = -1;
			}else if(k == Kesc){
				done = 1;
				rc = -1;
			}else{
				if(isprint(k)){
					if(len >= nbuf){
						done = 1;
						rc = -1;
					}else
						buf[len++] = k;
				}else if(k == Knack){
					len = 0;
				}else if(k == Kbs){
					if(len > 0)
						buf[--len] = '\0';
				}
				buf[len] = 0;
			}
			break;
		case 0:
			if(m.buttons&4){
				if(ptinrect(m.xy, br)){
					done = 1;
					rc = len;
				}else if(ptinrect(m.xy, brn)){
					done = 1;
					rc = -1;
				}
			}
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
	return rc;
}

