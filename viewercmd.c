#include "a.h"

static void
cmdquit(void)
{
	if(mode == Mview){
		free(text->title);
		free(text->data);
	}
	setmode(Mdir);
}

static void
cmdup(void)
{
	textscroll(text, -1);
}

static void
cmddown(void)
{
	textscroll(text, 1);
}

static void
cmdhome(void)
{
	textscroll(text, -text->nlines);
}

static void
cmdend(void)
{
	textscroll(text, text->nlines);
}

static void
cmdpageup(void)
{
	textscroll(text, -text->vlines);
}

static void
cmdpagedown(void)
{
	textscroll(text, text->vlines);
}

Binding	viewerbindings[] = {
	{ KF|10,	cmdquit },
	{ Kup,		cmdup },
	{ Kdown,	cmddown },
	{ Khome,	cmdhome },
	{ Kend,		cmdend },
	{ Kpgup,	cmdpageup },
	{ Kpgdown,	cmdpagedown },
	nil
};

void
setupviewerbindings(void)
{
	bindings = viewerbindings;
	actionbarclear(abar);
	actionbarset(abar, 10, "Quit", cmdquit);
}
