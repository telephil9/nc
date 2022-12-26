#include "a.h"

#define	GLOB	((char)0x01)
/*
 * Is c the first character of a utf sequence?
 */
#define	onebyte(c)	(((c)&0x80)==0x00)
#define twobyte(c)	(((c)&0xe0)==0xc0)
#define threebyte(c)	(((c)&0xf0)==0xe0)
#define fourbyte(c)	(((c)&0xf8)==0xf0)
#define xbyte(c)	(((c)&0xc0)==0x80)

/*
 * Return a pointer to the next utf code in the string,
 * not jumping past nuls in broken utf codes!
 */
static char*
nextutf(char *p)
{
	int i, n, c = *p;

	if(onebyte(c))
		return p+1;
	if(twobyte(c))
		n = 2;
	else if(threebyte(c))
		n = 3;
	else
		n = 4;
	for(i = 1; i < n; i++)
		if(!xbyte(p[i]))
			break;
	return p+i;
}

/*
 * Convert the utf code at *p to a unicode value
 */
static int
unicode(char *p)
{
	int c = *p;

	if(onebyte(c))
		return c&0xFF;
	if(twobyte(c)){
		if(xbyte(p[1]))
			return ((c&0x1F)<<6) | (p[1]&0x3F);
	} else if(threebyte(c)){
		if(xbyte(p[1]) && xbyte(p[2]))
			return ((c&0x0F)<<12) | ((p[1]&0x3F)<<6) | (p[2]&0x3F);
	} else if(fourbyte(c)){
		if(xbyte(p[1]) && xbyte(p[2]) && xbyte(p[3]))
			return ((c&0x07)<<18) | ((p[1]&0x3F)<<12) | ((p[2]&0x3F)<<6) | (p[3]&0x3F);
	}
	return -1;
}

/*
 * Do p and q point at equal utf codes
 */
static int
equtf(char *p, char *q)
{
	if(*p!=*q)
 		return 0;
	return unicode(p) == unicode(q);
}

int
domatch(char *s, char *p, int stop)
{
	int compl, hit, lo, hi, t, c;

	for(; *p!=stop && *p!='\0'; s = nextutf(s), p = nextutf(p)){
		if(*p!=GLOB){
			if(!equtf(p, s)) return 0;
		}
		else switch(*++p){
		case GLOB:
			if(*s!=GLOB)
				return 0;
			break;
		case '*':
			for(;;){
				if(domatch(s, nextutf(p), stop)) return 1;
				if(!*s)
					break;
				s = nextutf(s);
			}
			return 0;
		case '?':
			if(*s=='\0')
				return 0;
			break;
		case '[':
			if(*s=='\0')
				return 0;
			c = unicode(s);
			p++;
			compl=*p=='~';
			if(compl)
				p++;
			hit = 0;
			while(*p!=']'){
				if(*p=='\0')
					return 0;		/* syntax error */
				lo = unicode(p);
				p = nextutf(p);
				if(*p!='-')
					hi = lo;
				else{
					p++;
					if(*p=='\0')
						return 0;	/* syntax error */
					hi = unicode(p);
					p = nextutf(p);
					if(hi<lo){ t = lo; lo = hi; hi = t; }
				}
				if(lo<=c && c<=hi)
					hit = 1;
			}
			if(compl)
				hit=!hit;
			if(!hit)
				return 0;
			break;
		}
	}
	return *s=='\0';
}

/*
 * Does the string s match the pattern p
 * . and .. are only matched by patterns starting with .
 * * matches any sequence of characters
 * ? matches any single character
 * [...] matches the enclosed list of characters
 */

int
match(char *s, char *p)
{
	char pat[512] = {0};
	int i, j;

	if(s[0]=='.' && (s[1]=='\0' || s[1]=='.' && s[2]=='\0') && p[0]!='.')
		return 0;
	for(i = 0, j = 0; p[i] != '\0'; i++, j++){
		if(i == 512) sysfatal("OVERFLOW IN GLOB PATTERN");
		if(p[i] == '*' || p[i] == '[' || p[i] == '?')
			pat[j++] = GLOB;
		pat[j] = p[i];
	}
	return domatch(s, pat, '/');
}

