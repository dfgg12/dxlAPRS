/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Hannes Schmelzer <oe5hpm@oevsv.at>
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define _LARGEFILE64_SOURCE

#include <fcntl.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#include "Lib.h"
#include "osic.h"

#define DEBUG
#ifdef DEBUG
# define DBG(...)	printf(__VA_ARGS__)
#else
# define DBG(...)	do { } while (0)
#endif

#define FNLENCHECK	1

void osic_WrLn(void)
{
	fprintf(stdout, "\n");
	fflush(stdout);
}

void osic_WrStr(char s[], unsigned long s_len)
{
	size_t len;

	len = strnlen(s, s_len);
	fprintf(stdout, "%.*s", len, s);
}

void osic_WrStrLn(char s[], unsigned long s_len)
{
	size_t len;

	len = strnlen(s, s_len);
	fprintf(stdout, "%.*s\n", len, s);
	osic_WrLn();
}

void osic_WrUINT32(uint32_t x, unsigned long witdh)
{
	fprintf(stdout, "%*d", (int)witdh, x);
}

void osic_WrFixed(float x, long place, unsigned long witdh)
{
	fprintf(stdout, "%*.*f", (int)witdh, (int)place, x);
}

void osic_WrHex(unsigned long n, unsigned long f)
{
	fprintf(stdout, "%*X", (int)f, n);
	fflush(stdout);
}

long osic_OpenAppendLong(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return open(fn, O_WRONLY | O_APPEND | O_LARGEFILE);
}


long osic_OpenAppend(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return open(fn, O_WRONLY | O_APPEND);
}

long osic_OpenWrite(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return creat(fn, 0664);
}

long osic_OpenReadLong(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return open(fn, O_RDONLY | O_LARGEFILE);
}

long osic_OpenRead(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return open(fn, O_RDONLY);
}

long osic_OpenRW(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return open(fn, O_RDWR);
}
long osic_OpenNONBLOCK(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return open(fn, O_RDWR | O_NONBLOCK);
}
char osic_FdValid(long fd)
{
	return (fd >= 0);
}

void osic_Close(long fd)
{
	close(fd);
}

void osic_CloseSock(long fd)
{
	close(fd);
}

long osic_RdBin(long fd,
		      char buf[], unsigned long buf_len,
		      unsigned long size)
{
	if (size > (buf_len-1) + 1)
		size = (buf_len-1)+1;
	return read(fd, (char *)buf, size);
}

void osic_WrBin(long fd, char buf[],
		      unsigned long buf_len,
		      unsigned long size)
{
	if (size > (buf_len-1)+1)
		size = (buf_len-1)+1;
	write(fd, (char *)buf, size);
}

void osic_Rename(char fname[], unsigned long fname_len,
		       char newname[], unsigned long newname_len)
{
	if (FNLENCHECK && strnlen(fname, fname_len) >= fname_len)
		return;
	if (FNLENCHECK && strnlen(newname, newname_len) >= newname_len)
		return;
	rename(fname, newname);
}

int osic_Size(int fd)
{
	struct stat st = { };

	fstat(fd, &st);
	return st.st_size;
}

void osic_Seek(long fd, unsigned long pos)
{
	lseek(fd, (long)pos, SEEK_SET);
}


void osic_Seekcur(long fd, long rel)
{
	if (lseek64(fd, rel, (unsigned long)SEEK_CUR) < 0)
		lseek(fd, 0, SEEK_SET);
}

void osic_BEGIN(void)
{
}

void X2C_PCOPY(void **ppcpy, size_t size)
{
	void *pnew;

	pnew = malloc(size);
	if (pnew == NULL)
		return;
	memcpy(pnew, *ppcpy, size);
	*ppcpy = pnew;
}

void X2C_PFREE(void *p)
{
	if (p == NULL)
		return;
	free(p);
}

X2C_INT32 X2C_TRUNCI(X2C_LONGREAL x, X2C_INT32 min0, X2C_INT32 max0)
{
	X2C_INT32 i;

	if (x < (X2C_LONGREAL)min0 || x > (X2C_LONGREAL)max0)
		assert(0);
	i = (long)x;
	if (x > 0.0) {
		if ((X2C_LONGREAL)i > x)
			--i;
	} else if ((X2C_LONGREAL)i < x) {
		++i;
	}
	return i;
}
X2C_CARD32 X2C_TRUNCC(X2C_LONGREAL x, X2C_CARD32 min0, X2C_CARD32 max0)
{
	X2C_CARD32 i;

	if (x < (X2C_LONGREAL)min0)
		i = (unsigned long)min0;
	if (x > (X2C_LONGREAL)max0)
		i = (unsigned long)max0;

	i = (unsigned long)x;
	if ((X2C_LONGREAL)i > x)
		--i;
	return i;
}

X2C_REAL X2C_DIVR_F(X2C_REAL a, X2C_REAL b)
{
	if (b == 0.0f)
		return a;
	return (a/b);
}

X2C_LONGREAL X2C_DIVL_F(X2C_LONGREAL a, X2C_LONGREAL b)
{
	assert(b);
	return (a/b);
}

X2C_INT32 X2C_DIV_F(X2C_INT32 a, X2C_INT32 b)
{
	X2C_INT32 c;

	if (b <= 0)
		assert(0);
	c = (a/b);
	if (a < 0 && c*b > a)
		--c;
	return c;
}


X2C_pVOID X2C_COPY(void *src, size_t src_len, void *dst, size_t dst_len)
{
	return memcpy(dst, src, src_len <= dst_len ? src_len : dst_len);
}

void X2C_ABORT(void)
{
	exit(-1);
}

X2C_REAL RealMath_cos(X2C_REAL x)
{
	return cos(x);
}

X2C_REAL RealMath_ln(X2C_REAL x)
{
	if (x <= 0.0)
		assert(0);
	return log(x);
}

X2C_REAL RealMath_sqrt(X2C_REAL x)
{
	if (x < 0.0)
		assert(0);
	return sqrt(x);
}

X2C_REAL RealMath_sin(X2C_REAL x)
{
	return sin(x);
}
X2C_REAL RealMath_arctan(X2C_REAL x)
{
	return atan(x);
}

X2C_REAL RealMath_tan(X2C_REAL x)
{
	return tan(x);
}

X2C_REAL RealMath_exp(X2C_REAL x)
{
	return exp(x);
}

X2C_REAL RealMath_power(X2C_REAL base, X2C_REAL exponent)
{
	if (base <= 0.0)
		assert(0);
	return pow(base, exponent);
}


void X2C_EXIT(void)
{
	exit(0);
}

void X2C_HALT(X2C_INT32 x)
{
	exit(x);
}

void Storage_ALLOCATE(X2C_ADDRESS *p, X2C_CARD32 size)
{
	void *pmem;

	pmem = malloc(size);
	if (pmem == NULL)
		return;
	*p = pmem;
}

void Storage_DEALLOCATE(X2C_ADDRESS *a, X2C_CARD32 size)
{
	assert(*a);
	free(*a);
}

X2C_CARD32 TimeConv_time(void)
{
	return time(NULL);
}

X2C_CARD32 X2C_LSH(X2C_CARD32 a, X2C_INT16 length, X2C_INT32 n)
{
	X2C_CARD32 m;

	m = 0;
	m = (length == 32) ? 0xFFFFFFFFl : (1 << length) - 1;
	if (n > 0) {
		if (n >= (X2C_INT32)length)
			return 0;
		return (a << n) & m;
	}

	if (n <= (X2C_INT32)-length)
		return 0;
	return (a >> -n) & m;
}

X2C_BOOLEAN X2C_IN(X2C_CARD32 i, X2C_CARD16 bits, X2C_CARD32 set)
{
	if (i < (X2C_CARD32)bits)
		return (((1 << (int)i) & set) != 0);
	return 0;
}

X2C_BOOLEAN X2C_INL(X2C_CARD32 i, X2C_CARD16 bits, LSET set)
{
	if (i < (X2C_CARD32)bits)
		return (set[(int)i / X2C_LSET_SIZE] & (1 << ((int)i % X2C_LSET_SIZE))) != 0;
	return 0;
}

X2C_INT32 X2C_MOD_F(X2C_INT32 a, X2C_INT32 b)
{
	X2C_INT32 c;

	if (b <= 0)
		assert(0);
	c = (a % b);
	if (a < 0 && c < 0)
		c += b;
	return c;
}

X2C_CHAR X2C_CAP(X2C_CHAR x)
{
	if ((X2C_CARD8)x >= 'a' && (X2C_CARD8)x <= 'z')
		x = (X2C_CHAR)(((X2C_CARD32)(X2C_CARD8)x + 65) - 97);
	return x;
}

LSET X2C_INCL(LSET set, X2C_CARD32 i, X2C_CARD16 bits)
{
	if (i >= (X2C_CARD32)bits)
		assert(0);
	set[(int)i / X2C_LSET_SIZE] |= 1L << ((int)i % X2C_LSET_SIZE);

	return set;
}
LSET X2C_COMPLEMENT(LSET res, LSET a, X2C_CARD16 length)
{
	LSET c;

	c = res;
	while (length) {
		length--;
		*c++ = ~(*a++);
	}
	return res;
}

void FileSys_Remove(X2C_CHAR fname[], X2C_CARD32 fname_len, X2C_BOOLEAN *done)
{
	int rc;

	rc = remove(fname);
	if (rc == 0)
		*done = 1;
	else
		*done = 0;
}

X2C_BOOLEAN FileSys_Exists(X2C_CHAR fname[], X2C_CARD32 fname_len)
{
	if (access(fname, F_OK) != -1)
		return 1;
	return 0;
}

void FileSys_Rename(X2C_CHAR fname[], X2C_CARD32 fname_len,
		    X2C_CHAR newname[], X2C_CARD32 newname_len,
		    X2C_BOOLEAN *done)
{
	int rc;

	rc = rename(fname, newname);
	if (rc == 0)
		*done = 1;
	else
		*done = 0;
}

struct xrMM_Dynarr {
	X2C_ADDRESS a;
	size_t n[15];
};

size_t xrMM_DynarrDescSize(size_t i)
{
	return 8 + i * 2 * 4;
}

void X2C_InitDesc(struct xrMM_Dynarr *d, size_t *size, size_t lens[], size_t dims)
{
	X2C_CARD32 i;
	X2C_CARD32 tmp;

	d->n[(dims - 1) * 2] = lens[0];
	tmp = dims - 1;
	i = 1;
	if (i <= tmp) {
		for (;; i++) {
			*size =  *size*lens[dims-i];
			d->n[i * 2 - 1] = *size;
			d->n[(dims - i) * 2 - 2] = lens[i];
			if (i == tmp)
				break;
		}
	}
	*size =  *size*lens[0];
}

void X2C_DYNALLOCATE(X2C_ADDRESS *a, size_t size, size_t lens[], size_t dims)
{
	struct xrMM_Dynarr *desc;

	*a = 0;
	Storage_ALLOCATE((X2C_ADDRESS *)&desc, xrMM_DynarrDescSize(dims));
	if (desc) {
		X2C_InitDesc(desc, &size, lens, dims);
		Storage_ALLOCATE(&desc->a, size);
		if (desc->a == 0)
			free((X2C_ADDRESS *)&desc);
		else
			*a = (X2C_ADDRESS)desc;
	}
}

void X2C_DYNDEALLOCATE(X2C_ADDRESS *a)
{
	struct xrMM_Dynarr *d;

	if (*a) {
		d = (struct xrMM_Dynarr *)*a;
		free(&d->a);
		free(a);
	}
}

int X2C_STRCMP_PROC(X2C_pVOID x, size_t alen, X2C_pVOID y, size_t blen)
{
	X2C_pCHAR b;
	X2C_pCHAR a;
	size_t m;
	size_t i;

	a = (X2C_pCHAR)x;
	b = (X2C_pCHAR)y;
	m = alen;
	if (m > blen)
		m = blen;
	i = 0U;
	while ((i < m && *a) && *a == *b) {
		a = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)a + (X2C_INT32)1);
		b = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)b + (X2C_INT32)1);
		++i;
	}
	if (i >= m) {
		if (i < alen && *a == 0)
			alen = i;
		if (i < blen && *b == 0)
			blen = i;
		if (alen < blen)
			return -1;
		if (alen > blen)
			return 1;
		return 0;
	}
	if ((X2C_CARD8)(*a) < (X2C_CARD8)(*b))
		return -1;
	if ((X2C_CARD8)(*a) > (X2C_CARD8)(*b))
		return 1;
	return 0;
}

X2C_INT32 X2C_QUO_F(X2C_INT32 a, X2C_INT32 b)
{
	assert(b);
	if (a >= 0) {
		if (b > 0)
			return (a / b);
		else
			return (-(a / (-b)));
	} else if (b > 0) {
		return (-((-a) / b));
	} else {
		return ((-a) / (-b));
	}
	return 0;
}

X2C_LONGREAL X2C_EXPRI(X2C_LONGREAL base, X2C_INT32 ex)
{
	X2C_LONGREAL res;

	if (ex < 0 || ex > 8)
		return pow(base, (X2C_LONGREAL)ex);
	res = 1.0;
	while (ex > 0) {
		res = res*base;
		--ex;
	}
	return res;
}

X2C_PROC X2C_CHKPROC_F(X2C_PROC p)
{
	assert(p);
	return p;
}