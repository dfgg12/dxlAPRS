/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)MathLib0.c Feb  3 14:30:28 2012" */
/* Generated by XDS Modula-2 to ANSI C v4.20 translator */

#define X2C_int32
#define X2C_index32
#ifndef MathLib0_H_
#include "MathLib0.h"
#endif
#define MathLib0_C_
#ifndef RealMath_H_
#include "RealMath.h"
#endif


extern X2C_REAL MathLib0_sqrt(X2C_REAL x)
{
   return RealMath_sqrt(x);
} /* end sqrt() */


extern X2C_REAL MathLib0_exp(X2C_REAL x)
{
   return RealMath_exp(x);
} /* end exp() */


extern X2C_REAL MathLib0_ln(X2C_REAL x)
{
   return RealMath_ln(x);
} /* end ln() */


extern X2C_REAL MathLib0_sin(X2C_REAL x)
{
   return RealMath_sin(x);
} /* end sin() */


extern X2C_REAL MathLib0_cos(X2C_REAL x)
{
   return RealMath_cos(x);
} /* end cos() */


extern X2C_REAL MathLib0_arctan(X2C_REAL x)
{
   return RealMath_arctan(x);
} /* end arctan() */


extern X2C_INT32 MathLib0_entier(X2C_REAL x)
{
   return (X2C_INT32)X2C_TRUNCI(x,X2C_min_longint,X2C_max_longint);
} /* end entier() */


extern void MathLib0_BEGIN(void)
{
   static int MathLib0_init = 0;
   if (MathLib0_init) return;
   MathLib0_init = 1;
   RealMath_BEGIN();
}
