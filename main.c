#include "a.h"

int			mainstacksize = 32768;

int			bmode;
Image		*cols[Ncols];
Image		*tick;
Mousectl	*mc;
Keyboardctl	*kc;
int			mode;
Dirview		*dview;
Text		*text;
Actionbar	*abar;
Binding		*bindings;
Point		minbounds;

void
colsinit(void)
{
	if(!bmode){
		cols[Cbg] = display->white;
		cols[Cfg] = display->black;
		cols[Clfg] = ealloccolor(0x666666FF);
		cols[Ctitle] = ealloccolor(DGreygreen);
		cols[Cborder] = ealloccolor(0xAAAAAAFF);
		cols[Csel] = ealloccolor(0xCCCCCCFF);
		cols[Cselfg] = display->black;
		cols[Cerror] = ealloccolor(0x721c24ff);
		cols[Cdlgbg] = ealloccolor(0xFAFAFAFF);
		cols[Cdlgbord] = ealloccolor(0xCCCCCCFF);
	}else{
		cols[Cbg] = display->black;
		cols[Cfg] = display->white;
		cols[Clfg] = ealloccolor(0x666666FF);
		cols[Ctitle] = ealloccolor(DPurpleblue);
		cols[Cborder] = ealloccolor(0x333333FF);
		cols[Csel] = ealloccolor(DPurpleblue);
		cols[Cselfg] = display->black;
		cols[Cerror] = ealloccolor(DRed);
		cols[Cdlgbg] = ealloccolor(0x1A1A1AFF);
		cols[Cdlgbord] = ealloccolor(0x666666FF);
	}
}

void
tickinit(void)
{
	enum { Tickw = 3 };

	tick = allocimage(display, Rect(0, 0, Tickw, font->height), screen->chan, 0, DWhite);
	if(tick == nil)
		sysfatal("allocimage: %r");
	draw(tick, tick->r, cols[Cbg], nil, ZP);
	draw(tick, Rect(Tickw/2, 0, Tickw/2+1, font->height), cols[Cfg], nil, ZP);
	draw(tick, Rect(0, 0, Tickw, Tickw), cols[Cfg], nil, ZP);
	draw(tick, Rect(0, font->height-Tickw, Tickw, font->height), cols[Cfg], nil, ZP);
}

void
redraw(void)
{
	draw(screen, screen->r, cols[Cbg], nil, ZP);
	if(mode == Mdir){
		dirviewredraw(dview);
		draw(screen, dview->r, dview->b, nil, ZP);
	}else if(mode == Mview || mode == Mhelp){
		textredraw(text);
		draw(screen, text->r, text->b, nil, ZP);
	}
	actionbarredraw(abar);
	draw(screen, abar->r, abar->b, nil, ZP);
	flushimage(display, 1);
}

int
max(int x, int y)
{
	if(x > y)
		return x;
	return y;
}

void
resize(void)
{
	Rectangle dr, ar;
	int ah, sx, sy;
	
	sx = Dx(screen->r) + 2*4; /* 4 is rio window border width */
	sy = Dy(screen->r) + 2*4;
	if((sx < minbounds.x) || (sy < minbounds.y))
		wresize(max(sx, minbounds.x), max(sy, minbounds.y));
	ah = 2+font->height+2;
	dr = screen->r;
	dr.max.y -= ah;
	dirviewsetrect(dview, dr);
	textsetrect(text, dr);
	ar = screen->r;
	ar.min.y = dr.max.y;
	actionbarsetrect(abar, ar);
}

void
emouse(Mouse m)
{
	if(mode == Mdir)
		dirviewemouse(dview, m);
	else if(mode == Mview || mode == Mhelp)
		textemouse(text, m);
	actionbaremouse(abar, m);
}

void
ekbd(Rune k)
{
	int i;
	
	for(i = 0; bindings[i].k != 0; i++){
		if(bindings[i].k == k && bindings[i].f != nil){
			bindings[i].f();
			return;
		}
	}
}

void
setmode(int m)
{
	mode = m;
	switch(mode){
	case Mdir:
		setupdirviewbindings();
		break;
	case Mhelp:
	case Mview:
		setupviewerbindings();
		break;
	}
	redraw();
}

void
usage(void)
{
	fprint(2, "%s [-b]\n", argv0);
	exits("usage");
}

enum
{
	Emouse,
	Eresize,
	Ekbd,
	Edirview,
	Eleftmodel,
	Eleftpanel,
	Erightmodel,
	Erightpanel,
	Etext,
};

void
threadmain(int argc, char **argv)
{
	char *home, pwd[1024] = {0};
	Mouse m;
	Rune k;
	ulong l;
	Alt alts[] = 
	{
		{ nil, &m,   CHANRCV },
		{ nil, nil,  CHANRCV },
		{ nil, &k,   CHANRCV },
		{ nil, &l,   CHANRCV },
		{ nil, &l,   CHANRCV },
		{ nil, &l,   CHANRCV },
		{ nil, &l,   CHANRCV },
		{ nil, &l,   CHANRCV },
		{ nil, &l,   CHANRCV },
		{ nil, nil,  CHANEND },
	};

	bmode = 0;
	ARGBEGIN{
	case 'b':
		bmode = 1;
		break;
	default:
		usage();
	}ARGEND
	
	if(initdraw(nil, nil, argv0) < 0)
		sysfatal("initdraw: %r");
	if((mc = initmouse(nil, screen)) == nil)
		sysfatal("initmouse: %r");
	if((kc = initkeyboard(nil)) == nil)
		sysfatal("initkdb: %r");
	display->locking = 0;
	home = homedir();
	if(getwd(pwd, sizeof pwd) == nil)
		strncpy(pwd, home, strlen(home));
	dview = mkdirview(pwd, home);
	text = mktext();
	abar = mkactionbar();
	colsinit();
	tickinit();
	minbounds = Pt(10*9*stringwidth(font, "X"), 10*font->height); /* 10*9 is 10 action buttons with max 9 chars */
	resize();
	setmode(Mdir);
	alts[Emouse].c = mc->c;
	alts[Eresize].c = mc->resizec;
	alts[Ekbd].c = kc->c;
	alts[Edirview].c = dview->c;
	alts[Eleftmodel].c = dview->leftp->model->c;
	alts[Eleftpanel].c = dview->leftp->c;
	alts[Erightmodel].c = dview->rightp->model->c;
	alts[Erightpanel].c = dview->rightp->c;
	alts[Etext].c = text->c;
	for(;;){
		switch(alt(alts)){
		case Emouse:
			emouse(m);
			break;
		case Eresize:
			if(getwindow(display, Refnone) < 0)
				sysfatal("getwindow: %r");
			resize();
			redraw();
			break;
		case Ekbd:
			ekbd(k);
			break;
		case Edirview:
			draw(screen, dview->r, dview->b, nil, ZP);
			flushimage(display, 1);
			break;
		case Eleftmodel:
			dirpanelredraw(dview->leftp);
		case Eleftpanel:
			draw(dview->b, dview->leftr, dview->leftp->b, nil, ZP);
			draw(screen, dview->r, dview->b, nil, ZP);
			flushimage(display, 1);
			break;		
		case Erightmodel:
			dirpanelredraw(dview->rightp);
		case Erightpanel:
			draw(dview->b, dview->rightr, dview->rightp->b, nil, ZP);
			draw(screen, dview->r, dview->b, nil, ZP);
			flushimage(display, 1);
			break;
		case Etext:
			draw(screen, text->r, text->b, nil, ZP);
			flushimage(display, 1);
			break;
		}
	}
}
