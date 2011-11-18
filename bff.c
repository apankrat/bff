/*
 *	Copyright (c) 2004 Alex Pankratov. All rights reserved.
 *
 *	Slightly-optimizing (tm) Brainfuck interpreter, 1.0.4
 *	http://swapped.cc/bf
 */

/*
 *	The program is distributed under terms of BSD license. 
 *	You can obtain the copy of the license by visiting:
 *	
 *	http://www.opensource.org/licenses/bsd-license.php
 */

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 */
int    chew(char * p, int j, int * v_off);
void * xalloc(void * ptr, size_t sz);
void * grow(int * v, int * j, int * jmax);
int    getc_ext(FILE * fh);

void usage();
void die(const char * msg, ...);

/*
 *
 */
struct bf_op
{
	char op;
	int  op_arg;
	int  p_off, p_ofx;
	int  v_off, v_ofx;
};
 
int main(int argc, char ** argv)
{
	FILE * fh = stdin;
	char buf[1024], * p;
	int  i, k, l, b, n, jmax;
	int (* input)(FILE * fh);
	struct bf_op * x;
		
	/* main loop variables - keep 'em in registers */
	register struct bf_op * z;
	register int j, v_off;
	register int * v;

	if ( (argc > 1 && argv[1][0] == '-') ||
	     (argc > 2 && argv[2][0] == '-') ||
	     (argc > 3) )
		usage();

	/* open program file */
	if (argc > 1 && ! (fh = fopen(argv[1], "r")))
		die("cannot open program file (%s)\n", argv[1]);

	/* read proggy */
	n = 0;
	p = 0;
	while (! feof(fh))
	{
		b = fread(buf, 1, sizeof buf, fh);
		p = xalloc(p, (n += b) + 1);
		memcpy(p + n - b, buf, b);
	}
	fclose(fh);

	/* open data file */
	if (argc > 2)
	{
		if (! (fh = fopen(argv[2], "r")))
			die("cannot open input data file (%s)\n", argv[2]);
		input = getc_ext;
	}
	else
	{
		fh = stdin;
		input = getc;
	}

	/* strip comments */
	for (i=j=0; i<n; i++)
		if (strchr("[]<>+-,.", p[i]))
			p[j++] = p[i];
			
	n = j++;
	p[n] = 0; /* 'end of program' */

	/* skip leading < > */
	for (i=0; i<n; i++)
		if (! strchr("<>", p[i]))
			break;
	
	n -= i;
	memmove(p, p+i, n);

	/* preprocess */
	x = xalloc(NULL, n * sizeof(*x));

	for (i=0; i<n; i++)
	{
		x[i].op = p[i];
		
		switch (p[i])
		{
		case '[':
			for (j=i+1,l=1; j<n; j++)
				if (p[j] == '[')
					l++;  /* recurse */
				else
				if (p[j] == ']')
					if (! --l)
						break;
			if (l)
				die("no matching ]\n");
	
			x[i].op = 'c';
			x[i].p_ofx = chew(p, j+1, &x[i].v_ofx) - i;
			x[i].p_off = chew(p, i+1, &x[i].v_off) - i;
			break;

		case ']':
			for (j=i-1,l=1; 0<=j; j--)
				if (p[j] == ']')
					l++;
				else
				if (p[j] == '[')
					if (! --l)
						break;
			if (j < 0)
				die("no matching [\n");

			x[i].op = 'c';
			x[i].p_off = chew(p, j+1, &x[i].v_off) - i;
			x[i].p_ofx = chew(p, i+1, &x[i].v_ofx) - i;
			break;
		
		case '<':
		case '>':
			/* ignore */
			break;
		
		default:
			/* + - . , */
			for (j=i; j<n && p[j]==x[i].op; j++);
			x[i].op_arg = j-i;
			x[i].p_off = chew(p, j, &x[i].v_off) - i;
			break;
		}
	}

	/* allocate cell array - start with the moderate size */
	jmax = 256;
	v = xalloc(NULL, jmax * sizeof(int));
	
	/* get busy */
	j = jmax / 2;

	z = x;
	v_off = 0;
	for (;;)
	{
		if (v_off)
		{
			j += v_off;
			if (j < 0 || jmax <= j)
			{
				int _j = j, _m = jmax;
				v = grow(v, &_j, &_m);
				j = _j; jmax = _m;
			}
		}

		v_off = z->v_off;

		switch (z->op)
		{
		case '+':
			v[j] += z->op_arg;
			break;			
		case '-':
			v[j] -= z->op_arg;
			break;
		case 'c': 
			if (! v[j])
			{
				v_off = z->v_ofx;
				z += z->p_ofx;
				continue;
			}
			break;
		case '.':
			for (k=0; k<z->op_arg; k++)
				putchar(v[j]);
			break;
		case ',':
			for (k=0; k<z->op_arg; k++)
				v[j] = input(fh);
			break;
		default: 
			/* 0, ie eop */
			goto _break;
		}

		z += z->p_off;
	}
_break:	
	return 0;
}

/*
 *	miscellania
 */
int chew(char * p, int j, int * v_off)
{
	for ( ; p[j]; j++)
		switch (p[j])
		{
		case '<': (*v_off)--; break;
		case '>': (*v_off)++; break;
		default:
			goto _break;
		}
_break:
	return j;
}

void * xalloc(void * ptr, size_t sz)
{
	ptr = ptr ? realloc(ptr, sz) : calloc(1, sz);
	if (! ptr)
		die("out of memory (%ld)\n", sz);
	return ptr;
}

void * grow(int * v, int * j, int * jmax)
{
	int   n = *jmax;
	int inc = n/2; /* grow at least by half */
	int pos = *j;

	if (pos + inc < 0) /* underflow */
		inc = 3 * (0 - pos) / 2;
	if (n + inc < pos) /* overflow */
		inc = 3 * (pos - n) / 2;

	v = xalloc(v, (n+inc) * sizeof(int));
	memset(v + n, 0, inc * sizeof(int));

	if (pos < 0)
	{
		/* underflow */
		memmove(v + inc, v, n * sizeof(int));
		*j += inc;
	}

	*jmax += inc;
	return v;
}

int getc_ext(FILE * fh)
{
	return feof(fh) ? 0 : getc(fh);
}

/*
 *	boring stuff
 */
void usage()
{
	fprintf(stderr, 
		"bff: slightly-opimizing Brainfuck interpreter, 1.0.3.1, "
		"http://swapped.cc/bf\n"
		"Usage: bff [<program file> [<input data file]]\n");
		
	exit(-1);
}

void die(const char * msg, ...)
{
	va_list m;
	va_start(m, msg);
	fprintf(stderr, "bff: ");
	vfprintf(stderr, msg, m);
	va_end(m);
	exit(-1);
}


