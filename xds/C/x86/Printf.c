/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)Printf.c Feb  3 14:30:28 2012" */
/* Generated by XDS Modula-2 to ANSI C v4.20 translator */

#define X2C_int32
#define X2C_index32
#ifndef Printf_H_
#include "Printf.h"
#endif
#define Printf_C_
#ifndef IOChan_H_
#include "IOChan.h"
#endif
#ifndef FormOut_H_
#include "FormOut.h"
#endif
#ifndef StdChans_H_
#include "StdChans.h"
#endif

static void write_file(X2C_ADDRESS, const X2C_CHAR [], X2C_CARD32,
                X2C_INT32);


static void write_file(X2C_ADDRESS x, const X2C_CHAR s[], X2C_CARD32 s_len,
                X2C_INT32 l)
{
   IOChan_ChanId cid;
   cid = (IOChan_ChanId)x;
   IOChan_TextWrite(cid, (X2C_ADDRESS)s, (X2C_CARD32)l);
} /* end write_file() */


extern void Printf_printf(X2C_CHAR f[], X2C_CARD32 f_len, X2C_LOC x[],
                X2C_CARD32 x_len)
{
   IOChan_ChanId out;
   X2C_PCOPY((void **)&f,f_len);
   out = StdChans_OutChan();
   FormOut_format((X2C_ADDRESS)out, (FormOut_write_proc)write_file, f, f_len,
                 '2', (X2C_ADDRESS)x, x_len);
   X2C_PFREE(f);
} /* end printf() */


extern void Printf_fprintf(IOChan_ChanId cid, X2C_CHAR f[], X2C_CARD32 f_len,
                 X2C_LOC x[], X2C_CARD32 x_len)
{
   X2C_PCOPY((void **)&f,f_len);
   FormOut_format((X2C_ADDRESS)cid, (FormOut_write_proc)write_file, f, f_len,
                 '2', (X2C_ADDRESS)x, x_len);
   X2C_PFREE(f);
} /* end fprintf() */

struct str_buf;


struct str_buf {
   X2C_ADDRESS adr;
   X2C_CARD32 len;
   X2C_CARD32 pos;
};

typedef struct str_buf * str_buf_ptr;

static void write_str(X2C_ADDRESS, const X2C_CHAR [], X2C_CARD32, X2C_INT32);


static void write_str(X2C_ADDRESS x, const X2C_CHAR s[], X2C_CARD32 s_len,
                X2C_INT32 l)
{
   str_buf_ptr buf;
   X2C_CARD32 i;
   buf = (str_buf_ptr)x;
   i = 0UL;
   while (l>0L && buf->pos<buf->len) {
      *(X2C_CHAR *)buf->adr = s[i];
      ++buf->pos;
      --l;
      ++i;
      buf->adr = (X2C_ADDRESS)((X2C_ADDRESS)buf->adr+(X2C_INT32)1UL);
   }
} /* end write_str() */


extern void Printf_sprintf(X2C_CHAR o[], X2C_CARD32 o_len, X2C_CHAR f[],
                X2C_CARD32 f_len, X2C_LOC x[], X2C_CARD32 x_len)
{
   struct str_buf buf;
   X2C_PCOPY((void **)&f,f_len);
   buf.adr = (X2C_ADDRESS)o;
   buf.pos = 0UL;
   buf.len = (o_len-1)+1UL;
   FormOut_format((X2C_ADDRESS) &buf, (FormOut_write_proc)write_str, f,
                f_len, '2', (X2C_ADDRESS)x, x_len);
   if (buf.pos<buf.len) o[buf.pos] = 0;
   X2C_PFREE(f);
} /* end sprintf() */


extern void Printf_BEGIN(void)
{
   static int Printf_init = 0;
   if (Printf_init) return;
   Printf_init = 1;
   StdChans_BEGIN();
   FormOut_BEGIN();
   IOChan_BEGIN();
}
