/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)STextIO.c Feb  3 14:30:28 2012" */
/* Generated by XDS Modula-2 to ANSI C v4.20 translator */

#define X2C_int32
#define X2C_index32
#ifndef STextIO_H_
#include "STextIO.h"
#endif
#define STextIO_C_
#ifndef TextIO_H_
#include "TextIO.h"
#endif
#ifndef StdChans_H_
#include "StdChans.h"
#endif


extern void STextIO_ReadChar(X2C_CHAR * ch)
{
   TextIO_ReadChar(StdChans_InChan(), ch);
} /* end ReadChar() */


extern void STextIO_ReadRestLine(X2C_CHAR s[], X2C_CARD32 s_len)
{
   TextIO_ReadRestLine(StdChans_InChan(), s, s_len);
} /* end ReadRestLine() */


extern void STextIO_ReadString(X2C_CHAR s[], X2C_CARD32 s_len)
{
   TextIO_ReadString(StdChans_InChan(), s, s_len);
} /* end ReadString() */


extern void STextIO_ReadToken(X2C_CHAR s[], X2C_CARD32 s_len)
{
   TextIO_ReadToken(StdChans_InChan(), s, s_len);
} /* end ReadToken() */


extern void STextIO_SkipLine(void)
{
   TextIO_SkipLine(StdChans_InChan());
} /* end SkipLine() */


extern void STextIO_WriteChar(X2C_CHAR ch)
{
   TextIO_WriteChar(StdChans_OutChan(), ch);
} /* end WriteChar() */


extern void STextIO_WriteLn(void)
{
   TextIO_WriteLn(StdChans_OutChan());
} /* end WriteLn() */


extern void STextIO_WriteString(X2C_CHAR s[], X2C_CARD32 s_len)
{
   TextIO_WriteString(StdChans_OutChan(), s, s_len);
} /* end WriteString() */


extern void STextIO_BEGIN(void)
{
   static int STextIO_init = 0;
   if (STextIO_init) return;
   STextIO_init = 1;
   StdChans_BEGIN();
   TextIO_BEGIN();
}
