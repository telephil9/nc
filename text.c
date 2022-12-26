#include "a.h"

enum
{
	Scrollwidth = 12,
	Padding = 4,
};

enum
{
	Msnarf,
	Mplumb,
};
char *menu2str[] = {
	"snarf",
	"plumb",
	nil,
};
Menu menu2 = { menu2str };

static void
computelines(Text *t)
{
	int i, x, w, l, c;
	Rune r;

	t->lines[0] = 0;
	t->nlines = 1;
	w = Dx(t->textr);
	x = 0;
	for(i = 0; i < t->ndata; ){
		c = chartorune(&r, &t->data[i]);
		if(r == '\n'){
			if(i + c == t->ndata)
				break;
			t->lines[t->nlines++] = i + c;
			x = 0;
		}else{
			l = 0;
			if(r == '\t'){
				x += stringwidth(font, "    ");
			}else{
				l = runestringnwidth(font, &r, 1);
				x += l;
			}
			if(x > w){
				t->lines[t->nlines++] = i;
				x = l;
			}
		}
		i += c;
	}
}

static int
indexat(Text *t, Point p)
{
	int line, i, s, e, x, c, l;
	Rune r;
	Rectangle textr;

	textr = rectaddpt(t->textr, t->r.min);
	if(!ptinrect(p, textr))
		return -1;
	line = t->offset + ((p.y - textr.min.y) / font->height);
	s = t->lines[line];
	if(line+1 >= t->nlines)
		e = t->ndata;
	else
		e = t->lines[line+1] - 2;
	x = textr.min.x;
	for(i = s; i < e; ){
		c = chartorune(&r, &t->data[i]);
		if(r == '\t')
			l = stringwidth(font, "    ");
		else
			l = runestringnwidth(font, &r, 1);
		if(x <= p.x && p.x <= x+l)
			break;
		i += c;
		x += l;
	}
	return i;
}

Text*
mktext(void)
{
	Text *t;
	
	t = emalloc(sizeof *t);
	t->c = chancreate(sizeof(ulong), 1);
	t->b = nil;
	t->s0 = -1;
	t->s1 = -1;
	t->offset = 0;
	return t;
}

void
textset(Text *t, char *title, char *data, usize ndata)
{
	t->s0 = -1;
	t->s1 = -1;
	t->offset = 0;
	t->title = title;
	t->data = data;
	t->ndata = ndata;
	computelines(t);
}

void
textsetrect(Text *t, Rectangle r)
{
	t->r = r;
	freeimage(t->b);
	t->b = nil;
	t->intr = insetrect(boundsrect(r), 2);
	t->titler = t->intr;
	t->titler.max.y = t->titler.min.y + 2 + font->height + 2;
	t->textr = 	t->intr;
	t->textr.min.x += 2;
	t->textr.min.y = t->titler.max.y;
	t->textr = insetrect(t->textr, 4);
	t->vlines = Dy(t->textr) / font->height;
	if(t->nlines > 0)
		computelines(t);
}

static int
selected(Text *t, int index)
{
	int s0, s1;

	if(t->s0 < 0 || t->s1 < 0)
		return 0;
	s0 = t->s0 < t->s1 ? t->s0 : t->s1;
	s1 = t->s0 > t->s1 ? t->s0 : t->s1;
	return s0 <= index && index <= s1;
}

static void
drawline(Text *t, int index)
{
	int i, s, e;
	Point p;
	Rune r;
	Image *fg, *bg;

	s = t->lines[t->offset+index];
	if(t->offset+index+1 >= t->nlines)
		e = t->ndata;
	else
		e = t->lines[t->offset+index+1];
	p = addpt(t->textr.min, Pt(0, index*font->height));
	for(i = s; i < e; ){
		fg = cols[Cfg];
		bg = selected(t, i) ? cols[Csel]  : cols[Cbg];
		i += chartorune(&r, &t->data[i]);
		if(r == '\n')
			if(s + 1 == e) /* empty line */
				r = L' ';
			else
				continue;
		if(r == '\t')
			p = stringbg(t->b, p, fg, ZP, font, "    ", bg, ZP);
		else
			p = runestringnbg(t->b, p, fg, ZP, font, &r, 1, bg, ZP);
	}
}

void
textredraw(Text *t)
{
	int i;
	Rectangle r;

	r = boundsrect(t->r);
	if(t->b == nil)
		t->b = allocimage(display, r, screen->chan, 0, DNofill);
	draw(t->b, r, cols[Cbg], nil, ZP);
	border(t->b, t->intr, 2, cols[Ctitle], ZP);
	string(t->b, addpt(t->intr.min, Pt(4, 2)), cols[Cfg], ZP, font, t->title);
	line(t->b, Pt(t->intr.min.x, t->titler.max.y + 1), Pt(t->intr.max.x, t->titler.max.y + 1), 0, 0, 0, cols[Ctitle], ZP);
	for(i = 0; i < t->vlines; i++){
		if(t->offset+i >= t->nlines)
			break;
		drawline(t, i);
	}
}

void
textscroll(Text *t, int lines)
{
	if(t->nlines <= t->vlines)
		return;
	if(lines < 0 && t->offset == 0)
		return;
	if(lines > 0 && t->offset + t->vlines >= t->nlines)
		return;
	t->offset += lines;
	if(t->offset < 0)
		t->offset = 0;
	if(t->offset + t->nlines%t->vlines >= t->nlines)
		t->offset = t->nlines - t->nlines%t->vlines;
	textredraw(t);
	sendul(t->c, 1);
}

static void
snarfsel(Text *t)
{
	int fd, s0, s1;

	if(t->s0 < 0 || t->s1 < 0)
		return;
	fd = open("/dev/snarf", OWRITE);
	if(fd < 0)
		return;
	s0 = t->s0 < t->s1 ? t->s0 : t->s1;
	s1 = t->s0 > t->s1 ? t->s0 : t->s1;
	write(fd, &t->data[s0], s1 - s0 + 1);
	close(fd);
}

static void
plumbsel(Text *t)
{
	int fd, s0, s1;
	char *s;

	if(t->s0 < 0 || t->s1 < 0)
		return;
	fd = plumbopen("send", OWRITE);
	if(fd < 0)
		return;
	s0 = t->s0 < t->s1 ? t->s0 : t->s1;
	s1 = t->s0 > t->s1 ? t->s0 : t->s1;
	s = smprint("%.*s", s1 - s0 + 1, &t->data[s0]);
	plumbsendtext(fd, argv0, nil, nil, s);
	free(s);
	close(fd);
}

static void
menu2hit(Text *t, Mousectl *mc)
{
	int n;

	n = menuhit(2, mc, &menu2, nil);
	switch(n){
		case Msnarf:
			snarfsel(t);
			break;
		case Mplumb:
			plumbsel(t);
			break;
	}
}

void
textemouse(Text *t, Mouse)
{
	static selecting = 0;
	Point p;
	int n;
	Rectangle textr;

	textr = rectaddpt(t->textr, t->r.min);
	if(ptinrect(mc->xy, textr)){
		if(mc->buttons == 0)
			selecting = 0;
		if(mc->buttons == 1){
			if(!selecting){
				selecting = 1;
				t->s0 = t->s1 = -1;
				n = indexat(t, mc->xy);
				if(n < 0)
					return;
				t->s0 = n;
				t->s1 = -1;
				textredraw(t);
				nbsendul(t->c, 1);
			}else{
				n = indexat(t, mc->xy);
				if(n < 0)
					return;
				t->s1 = n;
			}
			for(readmouse(mc); mc->buttons == 1; readmouse(mc)){
				p = mc->xy;
				if(p.y <= textr.min.y){
					textscroll(t, -1);
					p.y = textr.min.y + 1;
				}else if(p.y >= textr.max.y){
					textscroll(t, 1);
					p.y = textr.max.y - 1;
				}
				n = indexat(t, p);
				if(n < 0)
					break;
				t->s1 = n;
				textredraw(t);
				nbsendul(t->c, 1);
			}
		}else if(mc->buttons == 2){
			menu2hit(t, mc);
		}else if(mc->buttons == 8){
			n = mousescrollsize(t->vlines);
			textscroll(t, -n);
		}else if(mc->buttons == 16){
			n = mousescrollsize(t->vlines);
			textscroll(t, n);
		}
	}
}

