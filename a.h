#include <u.h>
#include <libc.h>
#include <draw.h>
#include <mouse.h>
#include <keyboard.h>
#include <thread.h>
#include <plumb.h>
#include <bio.h>
#include <ctype.h>

typedef struct Dirview Dirview;
typedef struct Dirpanel Dirpanel;
typedef struct Dirmodel Dirmodel;
typedef struct Text Text;
typedef struct Actionbar Actionbar;
typedef struct Binding Binding;

typedef void(*Action)(void);

struct Dirview
{
	Rectangle	r;
	Image*		b;
	Channel*	c;
	Rectangle	leftr;
	Dirpanel*	leftp;
	Rectangle	rightr;
	Dirpanel*	rightp;
};

struct Dirpanel
{
	Rectangle	r;
	Image*		b;
	Channel*	c;
	Dirmodel*	model;
	int			focused;
	int			nlines;
	int			offset;
	int			cursor;
	Rectangle	intr;
	Rectangle	titler;
	Rectangle	filesr;
	int			colw[3];
};

struct Dirmodel
{
	Channel*	c;
	char*		path;
	int			isroot;
	Dir*		dirs;
	long		ndirs;
	uchar*		sel;
	char*		filter;
	Dir*		fdirs;
	long		fndirs;
};

enum
{
	Maxlines = 65535,
};

struct Text
{
	Image*		b;
	Channel*	c;
	Rectangle	r;
	Rectangle	intr;
	Rectangle	titler;
	Rectangle	textr;
	int			vlines;
	int			offset;
	char*		title;
	char*		data;
	usize		ndata;
	usize		lines[Maxlines];
	int			nlines;
	int			s0;
	int			s1;
};

struct Actionbar
{
	Rectangle	r;
	Image*		b;
	char*		labels[10];
	Action		actions[10];
};

struct Binding
{
	Rune	k;
	Action	f;
};

enum{
	Dinfo,
	Derror,
	Dconfirm,
};

enum{
	Bno,
	Byes,
};

Dirview*	mkdirview(char*, char*);
void		dirviewsetrect(Dirview*, Rectangle);
void		dirviewredraw(Dirview*);
void		dirviewemouse(Dirview*, Mouse);
Dirpanel*	dirviewcurrentpanel(Dirview*);
Dirpanel*	dirviewotherpanel(Dirview*);

Dirmodel*	mkdirmodel(char*);
Dir			dirmodelgetdir(Dirmodel*, int);
long		dirmodelcount(Dirmodel*);
void		dirmodelreload(Dirmodel*);
void		dirmodelreloadifsame(Dirmodel*, Dirmodel*);
void		dirmodelcd(Dirmodel*, char*);
void		dirmodelfilter(Dirmodel*, char*);
long		dirmodelmarklist(Dirmodel*, Dir**);
int			dirmodeleq(Dirmodel*, Dirmodel*);

Dirpanel*	mkdirpanel(Dirmodel*);
void		dirpanelsetrect(Dirpanel*, Rectangle);
void		dirpanelredraw(Dirpanel*);
void		dirpanelredrawnotify(Dirpanel*);
void		dirpanelemouse(Dirpanel*, Mouse);
void		dirpanelresetcursor(Dirpanel*);
int			dirpanelselectedindex(Dirpanel*);

Text*		mktext(void);
void		textsetrect(Text*, Rectangle);
void		textredraw(Text*);
void		textemouse(Text*, Mouse);
void		textscroll(Text*, int);
void		textset(Text*, char*, char*, usize);

Actionbar*	mkactionbar(void);
void		actionbarsetrect(Actionbar*, Rectangle);
void		actionbarredraw(Actionbar*);
void		actionbaremouse(Actionbar*, Mouse);
void		actionbarclear(Actionbar*);
void		actionbarset(Actionbar*, int, char*, Action);

void		setmode(int);
void		setupdirviewbindings(void);
void		setupviewerbindings(void);

int			match(char*, char*);

int			message(int, const char*, Mousectl*, Keyboardctl*);
void		errormessage(char*, Mousectl*, Keyboardctl*);
int			input(char*, char*, int, Mousectl*, Keyboardctl*);

int			mkdir(char*, char*);
int			rm(char*, Dir);
int			rmdir(char*);
int			cp(char*, Dir, char*, char*);

int			wresize(int, int);
Rectangle	boundsrect(Rectangle);
Image*		ealloccolor(ulong);
void*		emalloc(ulong);
void*		erealloc(void*, ulong);
char*		slurp(char*);
char*		homedir(void);
char*		abspath(char*, char*);

enum
{
	Mdir,
	Mhelp,
	Mview,
};

enum
{
	Cbg,
	Cfg,
	Clfg,
	Ctitle,
	Cborder,
	Csel,
	Cerror,
	Cdialog,
	Ncols
};
extern Image*		cols[Ncols];
extern Image*		tick;
extern Mousectl*	mc;
extern Keyboardctl*	kc;
extern int			mode;
extern Dirview*		dview;
extern Text*		text;
extern Actionbar*	abar;
extern Binding*		bindings;
extern char			help[];
