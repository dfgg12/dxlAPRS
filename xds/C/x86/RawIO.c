/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)RawIO.c Feb  3 14:30:28 2012" */
/* Generated by XDS Modula-2 to ANSI C v4.20 translator */

#define X2C_int32
#define X2C_index32
#ifndef RawIO_H_
#include "RawIO.h"
#endif
#define RawIO_C_
#ifndef IOChan_H_
#include "IOChan.h"
#endif
#ifndef IOConsts_H_
#include "IOConsts.h"
#endif


extern void RawIO_Read(IOChan_ChanId cid, X2C_LOC to[], X2C_CARD32 to_len)
{
   X2C_CARD32 n;
   IOChan_RawRead(cid, (X2C_ADDRESS)to, (to_len-1)+1UL, &n);
   if (n>0UL && n<=to_len-1) IOChan_SetReadResult(cid, IOConsts_wrongFormat);
} /* end Read() */


extern void RawIO_Write(IOChan_ChanId cid, X2C_LOC from[],
                X2C_CARD32 from_len)
{
   X2C_PCOPY((void **)&from,from_len);
   IOChan_RawWrite(cid, (X2C_ADDRESS)from, (from_len-1)+1UL);
   X2C_PFREE(from);
} /* end Write() */


extern void RawIO_BEGIN(void)
{
   static int RawIO_init = 0;
   if (RawIO_init) return;
   RawIO_init = 1;
   IOConsts_BEGIN();
   IOChan_BEGIN();
}
