/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef X2C_H_
#include "X2C.h"
#endif
#define profile_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#include <math.h>
#ifndef libsrtm_H_
#include "libsrtm.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef pngwrite_H_
#include "pngwrite.h"
#endif
#ifndef imagetext_H_
#include "imagetext.h"
#endif

/* altitude profile for radio link with srtm data by oe5dxl */
/*, wgs84r, wgs84s*/
#define profile_FRAMEXR 5

#define profile_MINM 20

#define profile_NOALT 10000

#define profile_PI 3.1415926535

#define profile_PI2 6.283185307

#define profile_LF "\012"

struct POSITIONL;


struct POSITIONL {
   double long0;
   double lat;
};

/*
     PIX=RECORD r,g,b:CARD16 END;
     IMAGELINE=ARRAY OF PIX;
     IMAGE=ARRAY OF IMAGELINE;
     pIMAGE=POINTER TO IMAGE;
*/
struct PATH;


struct PATH {
   struct POSITIONL pos;
   double optalt;
   double fresm;
   double refrm;
   double zero;
   double wood;
   double alt;
};


struct _0 {
   struct PATH * Adr;
   size_t Len0;
};

typedef struct _0 * pPATH;

static imagetext_pIMAGE image;

static char imagefn[1024];

static int32_t xsize;

static int32_t ysize;

static int32_t linksize;

static int32_t fontx;

static int32_t fonty;

static int32_t fonttyp;

static int32_t framexl;

static int32_t frameyu;

static int32_t frameyd;

static struct POSITIONL posa;

static struct POSITIONL posb;

static double alta;

static double altb;

static double anta;

static double antb;

static double refraction;

static double mhz;

static double dist;

static double igamma;

static double treesize;

static pPATH path;

static char gammatab[1024];

static char labela[100];

static char labelb[100];

static char opt;

static char treedrawn;


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static double sqr(double x)
{
   return x*x;
} /* end sqr() */


static void posinval(struct POSITIONL * pos)
{
   pos->long0 = 0.0;
   pos->lat = 0.0;
} /* end posinval() */


static char posvalid(const struct POSITIONL pos)
{
   return pos.lat!=0.0 || pos.long0!=0.0;
} /* end posvalid() */


static void wgs84s(double lat, double long0, double nn,
                double * x, double * y, double * z)
/* km */
{
   double c;
   double h;
   h = nn+6370.0;
   *z = h*sin(lat);
   c = cos(lat);
   *y = h*sin(long0)*c;
   *x = h*cos(long0)*c;
} /* end wgs84s() */


static void wgs84r(double x, double y, double z,
                double * lat, double * long0,
                double * heig)
/* km */
{
   double h;
   h = x*x+y*y;
   if (fabs(x)>fabs(y)) {
      *long0 = atan(X2C_DIVL(y,x));
      if (x<0.0) {
         if (y>0.0) *long0 = 3.1415926535+*long0;
         else *long0 = *long0-3.1415926535;
      }
   }
   else {
      *long0 = 1.57079632675-atan(X2C_DIVL(x,y));
      if (y<0.0) *long0 = *long0-3.1415926535;
   }
   *lat = atan(X2C_DIVL(z,sqrt(h)));
   *heig = sqrt(h+z*z)-6370.0;
} /* end wgs84r() */


static char StrToFixl(double * x, const char s[],
                uint32_t s_len)
{
   float r;
   if (!aprsstr_StrToFix(&r, s, s_len)) return 0;
   *x = (double)r;
   return 1;
} /* end StrToFixl() */


static void Parms(void)
{
   char err;
   char h[1024];
   struct aprsstr_POSITION posr;
   uint32_t label;
   err = 0;
   label = 0UL;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='i') {
            osi_NextArg(imagefn, 1024ul);
            if (imagefn[0U]==0 || imagefn[0U]=='-') {
               Error("-i <imagefilename>", 19ul);
            }
         }
         else if (h[1U]=='x') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &xsize) || xsize<120L) || xsize>=8192L) {
               Error("-x <size>", 10ul);
            }
         }
         else if (h[1U]=='y') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &ysize) || ysize<50L) || ysize>=8192L) {
               Error("-y <size>", 10ul);
            }
         }
         else if (h[1U]=='p') {
            osi_NextArg(libsrtm_srtmdir, 1024ul);
            if (libsrtm_srtmdir[0U]==0 || libsrtm_srtmdir[0U]=='-') {
               Error("-p <osm-folder-path>", 21ul);
            }
         }
         else if (h[1U]=='a') {
            osi_NextArg(h, 1024ul);
            aprsstr_loctopos(&posr, h, 1024ul);
            posa.lat = (double)posr.lat;
            posa.long0 = (double)posr.long0;
            if (!posvalid(posa)) {
               if (!StrToFixl(&posa.lat, h, 1024ul) || fabs(posa.lat)>=90.0) {
                  Error("-a <lat> <long> or <locator>", 29ul);
               }
               osi_NextArg(h, 1024ul);
               if (!StrToFixl(&posa.long0, h,
                1024ul) || fabs(posa.long0)>180.0) {
                  Error("-a <lat> <long> or <locator>", 29ul);
               }
               posa.lat = posa.lat*1.7453292519444E-2;
               posa.long0 = posa.long0*1.7453292519444E-2;
            }
            label = 0UL;
         }
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            aprsstr_loctopos(&posr, h, 1024ul);
            posb.lat = (double)posr.lat;
            posb.long0 = (double)posr.long0;
            if (!posvalid(posb)) {
               if (!StrToFixl(&posb.lat, h, 1024ul) || fabs(posb.lat)>=90.0) {
                  Error("-b <lat> <long> or <locator>", 29ul);
               }
               osi_NextArg(h, 1024ul);
               if (!StrToFixl(&posb.long0, h,
                1024ul) || fabs(posb.long0)>180.0) {
                  Error("-b <lat> <long> or <locator>", 29ul);
               }
               posb.lat = posb.lat*1.7453292519444E-2;
               posb.long0 = posb.long0*1.7453292519444E-2;
            }
            label = 1UL;
         }
         else if (h[1U]=='A') {
            osi_NextArg(h, 1024ul);
            if ((!StrToFixl(&anta, h, 1024ul) || anta<0.0) || anta>10000.0) {
               Error("-A <meter> (0..10000)", 22ul);
            }
         }
         else if (h[1U]=='B') {
            osi_NextArg(h, 1024ul);
            if ((!StrToFixl(&antb, h, 1024ul) || antb<0.0) || antb>10000.0) {
               Error("-B <meter> (0..10000)", 22ul);
            }
         }
         else if (X2C_CAP(h[1U])=='R') {
            opt = h[1U]=='R';
            osi_NextArg(h, 1024ul);
            if ((!StrToFixl(&refraction, h,
                1024ul) || refraction<0.0) || refraction>1.0) {
               Error("-r <refraction> [0.0..1.0]", 27ul);
            }
         }
         else if (h[1U]=='f') {
            osi_NextArg(h, 1024ul);
            if (!StrToFixl(&mhz, h, 1024ul) || mhz<1.0) {
               Error("-f <MHz>", 9ul);
            }
         }
         else if (h[1U]=='F') {
            osi_NextArg(h, 1024ul);
            if (h[0U]=='1') fonttyp = 6L;
            else if (h[0U]=='2') fonttyp = 8L;
            else if (h[0U]=='3') fonttyp = 10L;
            else Error("-F <font> (1:6x10 2:10x20)", 27ul);
         }
         else if (h[1U]=='g') {
            osi_NextArg(h, 1024ul);
            if ((!StrToFixl(&igamma, h,
                1024ul) || igamma<0.01) || igamma>10.0) {
               Error("-g <gamma> [0.1..10]", 21ul);
            }
         }
         else if (h[1U]=='w') {
            osi_NextArg(h, 1024ul);
            if ((!StrToFixl(&treesize, h,
                1024ul) || treesize<0.0) || treesize>100.0) {
               Error("-w <m> [0..100]", 16ul);
            }
         }
         else if (h[1U]=='L') {
            osi_NextArg(h, 1024ul);
            if (label==0UL) aprsstr_Assign(labela, 100ul, h, 1024ul);
            else if (label==1UL) aprsstr_Assign(labelb, 100ul, h, 1024ul);
            ++label;
         }
         else {
            if (h[1U]=='h') {
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("Geoprofile from Position A to Position B", 41ul);
               osi_WrStrLn(" -A <m>                            Antenna A over\
 ground [m] (10)", 66ul);
               osi_WrStrLn(" -a <lat> <long> | [locator]       Position A lat\
 long (degrees) or qth locator", 80ul);
               osi_WrStrLn(" -B <m>                            Antenna B over\
 ground [m] (10)", 66ul);
               osi_WrStrLn(" -b <lat> <long> | [locator]       Position B lat\
 long (degrees) or qth locator", 80ul);
               osi_WrStrLn(" -F <font>                         Font Size (1) \
1: 6x10, 2: 8x14, 3: 10x20", 76ul);
               osi_WrStrLn(" -f <MHz>                          Frequency for \
Fresnelzone (145)", 67ul);
               osi_WrStrLn(" -g <gamma>                        Image Gamma 0.\
1..10 (2.2)", 61ul);
               osi_WrStrLn(" -i <filename>                     Image File Nam\
e", 51ul);
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrStrLn(" -L <text>                         Label, apply a\
fter -a and -b", 64ul);
               osi_WrStrLn(" -p <pathname>                     folder with /s\
rtm1 /srtm3 /srtm30", 69ul);
               osi_WrStrLn(" -r <refraction>                   0.0(vacuum)..1\
.0(earth is a disk) (0.25)", 76ul);
               osi_WrStrLn(" -R <refraction>                   same but earth\
 curvature added to Ground", 76ul);
               osi_WrStrLn(" -w <m>                            Wood higth (0)\
 (from 1000 to 2000NN reduced till 0", 86ul);
               osi_WrStrLn(" -x <size>                         Image size (60\
0)", 52ul);
               osi_WrStrLn(" -y <size>                         Image size (40\
0)", 52ul);
               osi_WrStrLn(" use -a and -p Parameter for Altitude as String o\
utput", 55ul);
               osi_WrStrLn("", 1ul);
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else err = 1;
      if (err) break;
   }
   if (err) {
      osi_Werr(">", 2ul);
      osi_Werr(h, 1024ul);
      osi_Werr("< use -h\012", 10ul);
      X2C_ABORT();
   }
} /* end Parms() */


static double fresnel(double a, double b,
                double lambda)
{
   if ((lambda<=0.0 || a<=0.0) || b<=0.0) return 0.0;
   else return sqrt(X2C_DIVL(lambda*a*b,a+b));
   return 0;
} /* end fresnel() */


static void calcpath(void)
{
   float res;
   struct aprsstr_POSITION posr;
   double lambda;
   double a;
   double stepm;
   double refrac;
   double ks;
   double k;
   double dz;
   double dy;
   double dx;
   double z1;
   double y1;
   double x1;
   double z0;
   double y00;
   double x0;
   int32_t substeps;
   int32_t maxsteps;
   int32_t j;
   int32_t i;
   struct PATH * anonym;
   struct PATH * anonym0;
   int32_t tmp;
   int32_t tmp0;
   posr.lat = (float)posa.lat;
   posr.long0 = (float)posa.long0;
   alta = (double)libsrtm_getsrtm(posr, 1UL, &res);
   if (alta>=10000.0) Error("no altitude at antanna A", 25ul);
   posr.lat = (float)posb.lat;
   posr.long0 = (float)posb.long0;
   altb = (double)libsrtm_getsrtm(posr, 1UL, &res);
   if (altb>=10000.0) Error("no altitude at antanna B", 25ul);
   alta = alta+anta;
   altb = altb+antb;
   /*
     IF alta<altb THEN
       a:=alta; alta:=altb; altb:=a;
       posh:=posa; posa:=posb; posb:=posh;
     END; 
   */
   wgs84s(posa.lat, posa.long0, alta*0.001, &x0, &y00, &z0);
   wgs84s(posb.lat, posb.long0, altb*0.001, &x1, &y1, &z1);
   ks = (double)(X2C_DIVR(1.0f,(float)linksize));
   dx = x1-x0;
   dy = y1-y00;
   dz = z1-z0;
   dist = sqrt(dx*dx+dy*dy+dz*dz);
   stepm = dist*ks*1000.0;
   maxsteps = (int32_t)(1UL+(uint32_t)
                X2C_TRUNCC(stepm*3.3333333333333E-2,0UL,X2C_max_longcard));
   if (dist<0.2) Error("distance too less", 18ul);
   if (dist>600.0) Error("distance too long", 18ul);
   refrac = -((6370.0-sqrt(4.05769E+7+dist*dist*0.25))*4000.0);
   lambda = X2C_DIVL(300.0,mhz);
   tmp = linksize-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      path->Adr[i].alt = (-2.E+4);
      substeps = maxsteps;
      if (i>0L) {
         substeps = (int32_t)(1UL+(uint32_t)
                X2C_TRUNCC(X2C_DIVL(stepm*4.0,
                1.0+fabs(path->Adr[i-1L].alt-path->Adr[i-1L].optalt)),0UL,
                X2C_max_longcard));
      }
      if (substeps>maxsteps) substeps = maxsteps;
      tmp0 = substeps-1L;
      j = 0L;
      if (j<=tmp0) for (;; j++) {
         k = (double)((float)i+X2C_DIVR((float)j,
                (float)substeps))*ks;
         { /* with */
            struct PATH * anonym = &path->Adr[i];
            wgs84r(x0+dx*k, y00+dy*k, z0+dz*k, &anonym->pos.lat,
                &anonym->pos.long0, &anonym->optalt);
            posr.lat = (float)anonym->pos.lat;
            posr.long0 = (float)anonym->pos.long0;
            a = (double)libsrtm_getsrtm(posr,
                (uint32_t)X2C_TRUNCC(stepm,0UL,X2C_max_longcard), &res);
            if (a>anonym->alt) anonym->alt = a;
         }
         if (j==tmp0) break;
      } /* end for */
      { /* with */
         struct PATH * anonym0 = &path->Adr[i];
         k = (double)(X2C_DIVR((float)i,(float)(linksize-1L)));
         anonym0->optalt = anonym0->optalt*1000.0;
         anonym0->zero = (k-k*k)*refrac;
         anonym0->refrm = anonym0->zero*refraction;
         anonym0->fresm = fresnel(dist*1000.0*k, dist*1000.0*(1.0-k),
                lambda)*0.5;
         anonym0->wood = 0.0;
         if (anonym0->alt>=1.0) {
            if (anonym0->alt>=1000.0) {
               anonym0->wood = X2C_DIVL(treesize*(2000.0-anonym0->alt),
                1000.0);
               if (anonym0->wood<0.0) anonym0->wood = 0.0;
            }
            else anonym0->wood = treesize;
         }
      }
      if (i==tmp) break;
   } /* end for */
} /* end calcpath() */


static void makegammatab(void)
{
   uint32_t c;
   double g;
   g = X2C_DIVL(1.0,igamma);
   gammatab[0U] = 0;
   for (c = 1UL; c<=1023UL; c++) {
      gammatab[c] = (char)(uint32_t)X2C_TRUNCC(exp(log((double)
                (X2C_DIVR((float)c,1024.0f)))*g)*255.5,0UL,
                X2C_max_longcard);
   } /* end for */
} /* end makegammatab() */


static uint8_t pngc(uint16_t c)
{
   if (c<=1023U) return gammatab[c];
   else return 255U;
   return 0;
} /* end pngc() */


static void wrpng(void)
{
   struct PNGPIXMAP pngimg;
   int32_t ret;
   int32_t y;
   int32_t x;
   struct imagetext_PIX * anonym;
   struct PNGPIXEL * anonym0;
   int32_t tmp;
   int32_t tmp0;
   osic_alloc((char * *) &pngimg.image, (uint32_t)(xsize*ysize*3L));
   if (pngimg.image) {
      makegammatab();
      tmp = ysize-1L;
      y = 0L;
      if (y<=tmp) for (;; y++) {
         tmp0 = xsize-1L;
         x = 0L;
         if (x<=tmp0) for (;; x++) {
            { /* with */
               struct imagetext_PIX * anonym = &image->Adr[(x)
                *image->Len0+((ysize-1L)-y)];
               { /* with */
                  struct PNGPIXEL * anonym0 = &pngimg.image[x+y*xsize];
                  anonym0->red = pngc(anonym->r);
                  anonym0->green = pngc(anonym->g);
                  anonym0->blue = pngc(anonym->b);
               }
            }
            if (x==tmp0) break;
         } /* end for */
         if (y==tmp) break;
      } /* end for */
      pngimg.width = (uint32_t)xsize;
      pngimg.height = (uint32_t)ysize;
      ret = writepng(imagefn, &pngimg);
      osic_free((char * *) &pngimg.image, (uint32_t)(xsize*ysize*3L));
   }
   else osi_Werr("png write out of memory\012", 25ul);
} /* end wrpng() */


static void fresnelfree(double * airshadow, double * woodshadow)
/* guess freeness of fresnel zone */
{
   int32_t wd;
   int32_t i;
   double as;
   double ws;
   double gnd;
   double fs;
   double fz;
   struct PATH * anonym;
   int32_t tmp;
   wd = (int32_t)X2C_TRUNCI(X2C_DIVL((double)(float)linksize,
                dist),X2C_min_longint,X2C_max_longint); /* steps per km */
   *airshadow = 0.0;
   *woodshadow = 0.0;
   tmp = linksize-1L;
   i = 1L;
   if (i<=tmp) for (;; i++) {
      { /* with */
         struct PATH * anonym = &path->Adr[i];
         fz = (anonym->optalt+anonym->refrm)-anonym->fresm;
         gnd = anonym->alt;
         fs = anonym->fresm*2.0;
         if (fs<1.0) fs = 1.0;
         as = X2C_DIVL(gnd-fz,fs);
         if (i>=wd && i<linksize-wd) {
            /* wood only 1km from endpoints */
            ws = X2C_DIVL((gnd+anonym->wood)-fz,fs);
            if (ws>*woodshadow) *woodshadow = ws;
         }
         if (as>*airshadow) *airshadow = as;
      }
      if (i==tmp) break;
   } /* end for */
} /* end fresnelfree() */


static void pix(int32_t cb, int32_t cg, int32_t cr, int32_t x,
                int32_t y, float lum)
{
   struct imagetext_PIX * anonym;
   if (lum>0.0f) {
      { /* with */
         struct imagetext_PIX * anonym = &image->Adr[(x)*image->Len0+y];
         anonym->r += (uint16_t)(uint32_t)X2C_TRUNCC((float)cr*lum,
                0UL,X2C_max_longcard);
         anonym->g += (uint16_t)(uint32_t)X2C_TRUNCC((float)cg*lum,
                0UL,X2C_max_longcard);
         anonym->b += (uint16_t)(uint32_t)X2C_TRUNCC((float)cb*lum,
                0UL,X2C_max_longcard);
      }
   }
} /* end pix() */


static void drawcolon(int32_t x, double y00, double y1,
                int32_t cr, int32_t cg, int32_t cb)
{
   int32_t y;
   double d;
   double yi;
   double k;
   double fy1;
   double fx1;
   double fx;
   double fy;
   k = y1-y00;
   d = X2C_DIVL(0.25,sqrt(1.0+sqr(k)));
   fx = 0.0;
   do {
      yi = y00+k*fx;
      y = (int32_t)(uint32_t)X2C_TRUNCC(yi,0UL,X2C_max_longcard);
      fy = yi-(double)(float)y;
      fy1 = 1.0-fy;
      fx1 = 1.0-fx;
      pix(cb, cg, cr, x, y+1L, (float)(fy*fx1));
      pix(cb, cg, cr, x, y, (float)(fy1*fx1));
      pix(cb, cg, cr, x+1L, y+1L, (float)(fy*fx));
      pix(cb, cg, cr, x+1L, y, (float)(fy1*fx));
      fx = fx+d;
   } while (fx<=1.0);
} /* end drawcolon() */


static int32_t leftbound(const char s[], uint32_t s_len)
{
   return (xsize-(int32_t)imagetext_strsize((uint32_t)fonttyp, s,
                s_len))-2L;
} /* end leftbound() */


static int32_t Scale125(double pix0)
{
   int32_t d;
   int32_t y;
   d = 1L;
   y = 1L;
   for (;;) {
      if (pix0<(double)(float)(y*d)) return y*d;
      if (y==1L) y = 2L;
      else if (y==2L) y = 5L;
      else {
         y = 1L;
         d = d*10L;
      }
   }
   return 0;
} /* end Scale125() */


static void drawtree(int32_t x, double dalt, double treesize0)
{
   int32_t bty;
   int32_t branches;
   int32_t branchdense;
   int32_t y;
   int32_t i;
   double wtt;
   double wty;
   double treelum;
   struct imagetext_PIX * anonym; /* trunk of tree */
   struct imagetext_PIX * anonym0;
   struct imagetext_PIX * anonym1;
   int32_t tmp;
   branchdense = 2L+(int32_t)X2C_TRUNCI(treesize0*0.05,X2C_min_longint,
                X2C_max_longint);
   branches = (int32_t)X2C_TRUNCI(treesize0*0.5+sqrt(treesize0),
                X2C_min_longint,X2C_max_longint);
   treelum = X2C_DIVL(2.0,log(treesize0));
   tmp = (int32_t)X2C_TRUNCI(treesize0,X2C_min_longint,X2C_max_longint)-1L;
                
   y = 0L;
   if (y<=tmp) for (;; y++) {
      bty = (int32_t)X2C_TRUNCI(dalt+treesize0,X2C_min_longint,
                X2C_max_longint)-y;
      if (bty>=ysize) bty = ysize-1L;
      { /* with */
         struct imagetext_PIX * anonym = &image->Adr[(x+framexl)
                *image->Len0+bty];
         anonym->r += (uint16_t)(int32_t)X2C_TRUNCI(300.0*treelum,
                X2C_min_longint,X2C_max_longint);
         anonym->g += (uint16_t)(int32_t)X2C_TRUNCI(200.0*treelum,
                X2C_min_longint,X2C_max_longint);
         anonym->b += 0U;
      }
      wty = 1.0+(double)(float)y*(0.2+X2C_DIVL(2.0,treesize0));
      i = 1L;
      for (;;) {
         if (y-i<0L || y-i>=branches) break;
         if ((y-i)%branchdense==0L) {
            /* branch */
            wtt = wty-(double)(float)i;
                /* branchs last pixel brightness */
            if (wtt<0.0) break;
            if (wtt>1.0) wtt = 1.0;
            if (x>i) {
               { /* with */
                  struct imagetext_PIX * anonym0 = &image->Adr[((x-i)
                +framexl)*image->Len0+bty];
                  anonym0->r += 0U;
                  anonym0->g += (uint16_t)(int32_t)
                X2C_TRUNCI(500.0*treelum*wtt,X2C_min_longint,
                X2C_max_longint);
                  anonym0->b += 0U;
               }
            }
            if (x+i<xsize-framexl) {
               { /* with */
                  struct imagetext_PIX * anonym1 = &image->Adr[(x+i+framexl)
                *image->Len0+bty];
                  anonym1->r += 0U;
                  anonym1->g += (uint16_t)(int32_t)
                X2C_TRUNCI(500.0*treelum*wtt,X2C_min_longint,
                X2C_max_longint);
                  anonym1->b += 0U;
               }
            }
         }
         ++i;
      }
      if (y==tmp) break;
   } /* end for */
} /* end drawtree() */


static void placetree(int32_t * x, double treesize0)
{
   int32_t mindist;
   int32_t ff;
   int32_t ix;
   int32_t m;
   double max0;
   /*OR (ix=m+1)*/
   struct PATH * anonym;
   ix = *x+(int32_t)X2C_TRUNCI(treesize0*0.25,X2C_min_longint,
                X2C_max_longint)+5L; /* min distance */
   mindist = (int32_t)X2C_TRUNCI(treesize0*0.25,X2C_min_longint,
                X2C_max_longint)+3L;
   max0 = (-1.E+4);
   m = 0L;
   ff = 0L;
   while (ix<linksize && (ff==0L || ix-ff<mindist)) {
      { /* with */
         struct PATH * anonym = &path->Adr[ix];
         if ((((anonym->alt>=1.0 && anonym->alt<anonym->optalt+anonym->fresm)
                 && anonym->alt+anonym->wood>anonym->optalt-anonym->fresm)
                && anonym->wood>anonym->fresm*0.5)
                && anonym->alt-anonym->optalt>max0) {
            /* tree is in fresnel zone and 1/4 as high */
            m = ix;
            max0 = anonym->alt-anonym->optalt;
            if (ff==0L) ff = ix;
         }
      }
      ++ix;
   }
   /*    IF (ff<>0) & (ix-ff>=mindist)
                THEN x:=ff ELSIF m>0 THEN x:=m ELSE x:=ix END; */
   if (ix==m+1L) *x = ff;
   else if (m>0L) *x = m;
   else *x = ix;
} /* end placetree() */


static double sc(double scale, double min0,
                double y)
{
   return (y-min0)*scale+(double)(float)frameyd;
} /* end sc() */


static void drawimage(void)
{
   int32_t lasttree;
   int32_t maxi;
   int32_t dk;
   int32_t cb;
   int32_t cg;
   int32_t cr;
   int32_t xc;
   int32_t m;
   int32_t n;
   int32_t st;
   int32_t y;
   int32_t x;
   int32_t i;
   double woodshadow;
   double airshadow;
   double maxw;
   double w;
   double mpp;
   double ah;
   double a1;
   double a0;
   double ao;
   double scale;
   double max0;
   double min0;
   char sss[100];
   char ss[100];
   char s[100];
   struct imagetext_PIX * anonym;
   struct imagetext_PIX * anonym0;
   struct PATH * anonym1;
   /* fill earth */
   struct imagetext_PIX * anonym2;
   struct imagetext_PIX * anonym3; /* fill fresnel area */
   /* vertical scale line left */
   struct imagetext_PIX * anonym4;
   /* vertical scale line right */
   struct imagetext_PIX * anonym5;
   /* meter lines */
   struct imagetext_PIX * anonym6;
   /* km lines */
   struct imagetext_PIX * anonym7;
   int32_t tmp;
   int32_t tmp0;
   fresnelfree(&airshadow, &woodshadow);
   /*** background ***/
   tmp = xsize-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      tmp0 = ysize-1L;
      y = 0L;
      if (y<=tmp0) for (;; y++) {
         { /* with */
            struct imagetext_PIX * anonym = &image->Adr[(i)*image->Len0+y];
            anonym->r = 20U;
            anonym->g = 80U;
            anonym->b = 220U;
         }
         if (y==tmp0) break;
      } /* end for */
      if (i==tmp) break;
   } /* end for */
   /*** bottom line ***/
   tmp = (xsize-1L)-5L;
   i = framexl;
   if (i<=tmp) for (;; i++) {
      { /* with */
         struct imagetext_PIX * anonym0 = &image->Adr[(i)
                *image->Len0+frameyd];
         anonym0->r = 800U;
         anonym0->g = 500U;
         anonym0->b = 200U;
      }
      if (i==tmp) break;
   } /* end for */
   /*** get scaleing ***/
   min0 = path->Adr[0UL].alt;
   max0 = min0;
   maxw = min0;
   tmp = linksize-1L;
   i = 1L;
   if (i<=tmp) for (;; i++) {
      { /* with */
         struct PATH * anonym1 = &path->Adr[i];
         anonym1->optalt = anonym1->optalt+anonym1->refrm;
         if (opt) {
            anonym1->alt = anonym1->alt+anonym1->zero;
            anonym1->optalt = anonym1->optalt+anonym1->zero;
         }
         if (anonym1->alt>anonym1->optalt+anonym1->fresm) {
            anonym1->wood = 0.0;
         }
         if (anonym1->alt<10000.0) {
            if (anonym1->alt<min0) min0 = anonym1->alt;
            if (anonym1->alt>max0) max0 = anonym1->alt;
            if (anonym1->alt+anonym1->wood>maxw) {
               maxw = anonym1->alt+anonym1->wood;
            }
         }
         if (anonym1->optalt+anonym1->fresm>max0) {
            max0 = anonym1->optalt+anonym1->fresm;
         }
         if (anonym1->optalt-anonym1->fresm<min0) {
            min0 = anonym1->optalt-anonym1->fresm;
         }
      }
      if (i==tmp) break;
   } /* end for */
   ah = max0-min0;
   w = maxw-min0; /* scale trees to top of image */
   if (ah<20.0) ah = 20.0;
   w = X2C_DIVL((double)(float)((ysize-1L)-frameyd),w);
   scale = X2C_DIVL((double)(float)(ysize-(frameyu+frameyd)),ah);
   if (w<scale) scale = w;
   /*** draw graphs ***/
   lasttree = 0L;
   tmp = linksize-2L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      a0 = path->Adr[i].alt;
      a1 = path->Adr[i+1L].alt;
      if (a0<10000.0 && a1<10000.0) {
         ah = sc(scale, min0, a0);
         drawcolon(i+framexl, ah, sc(scale, min0, a1), 80L, 250L, 0L);
         tmp0 = (int32_t)X2C_TRUNCI(ah,X2C_min_longint,X2C_max_longint)-1L;
                
         y = frameyd+1L;
         if (y<=tmp0) for (;; y++) {
            { /* with */
               struct imagetext_PIX * anonym2 = &image->Adr[(i+framexl)
                *image->Len0+y];
               anonym2->r += 200U;
               anonym2->g += 120U;
               anonym2->b += 0U;
            }
            if (y==tmp0) break;
         } /* end for */
         if (((mhz>=30.0 && a0<path->Adr[i].optalt+path->Adr[i].fresm)
                && a0+path->Adr[i].wood>path->Adr[i].optalt-path->Adr[i]
                .fresm) && path->Adr[i].wood>path->Adr[i].fresm*0.5) {
            /* tree is in fresnel zone and 1/4 as high */
            w = path->Adr[i].wood*scale; /* tree size in pixel */
            if (w>4.0 && i>=lasttree) {
               /* space between trees */
               placetree(&lasttree, w);
               if (lasttree<(int32_t)(path->Len0-1)) {
                  drawtree(lasttree, sc(scale, min0,
                path->Adr[lasttree].alt), path->Adr[lasttree].wood*scale);
                  treedrawn = 1;
               }
            }
         }
      }
      ao = path->Adr[i].optalt;
      drawcolon(i+framexl, sc(scale, min0, ao), sc(scale, min0,
                path->Adr[i+1L].optalt), 90L, 150L, 150L);
      drawcolon(i+framexl, sc(scale, min0, ao+path->Adr[i].fresm), sc(scale,
                min0, path->Adr[i+1L].optalt+path->Adr[i+1L].fresm), 190L,
                100L, 0L);
      drawcolon(i+framexl, sc(scale, min0, ao-path->Adr[i].fresm), sc(scale,
                min0, path->Adr[i+1L].optalt-path->Adr[i+1L].fresm), 190L,
                100L, 0L);
      tmp0 = (int32_t)X2C_TRUNCI(sc(scale, min0, ao+path->Adr[i].fresm),
                X2C_min_longint,X2C_max_longint)-1L;
      y = (int32_t)X2C_TRUNCI(sc(scale, min0, ao-path->Adr[i].fresm),
                X2C_min_longint,X2C_max_longint)+1L;
      if (y<=tmp0) for (;; y++) {
         { /* with */
            struct imagetext_PIX * anonym3 = &image->Adr[(i+framexl)
                *image->Len0+y];
            anonym3->r += 240U;
            anonym3->g += 80U;
            anonym3->b += 0U;
         }
         if (y==tmp0) break;
      } /* end for */
      if (i==tmp) break;
   } /* end for */
   /*** vertical scale ***/
   maxi = linksize-1L;
   if (sc(scale, min0, path->Adr[0UL].optalt)>sc(scale, min0,
                path->Adr[linksize-1L].optalt)) maxi = 0L;
   a0 = path->Adr[maxi].optalt-min0; /* meters on scale */
   a1 = sc(scale, min0, path->Adr[maxi].optalt)-(double)(float)frameyd;
                 /* pixel on scale */
   if (a1==0.0) mpp = 1.0;
   else mpp = X2C_DIVL(a0,a1);
   st = Scale125(mpp*(double)(fonty*16L+ysize/4L)*0.05);
   m = ((((int32_t)X2C_TRUNCI(min0,X2C_min_longint,
                X2C_max_longint)+st)-1L)/st)*st; /* lowest meters */
   tmp = (int32_t)X2C_TRUNCI(sc(scale, min0, path->Adr[maxi].optalt),
                X2C_min_longint,X2C_max_longint);
   y = frameyd;
   if (y<=tmp) for (;; y++) {
      { /* with */
         struct imagetext_PIX * anonym4 = &image->Adr[(framexl)
                *image->Len0+y];
         anonym4->r += 600U;
         anonym4->g += 600U;
         anonym4->b += 500U;
      }
      if (y==tmp) break;
   } /* end for */
   tmp = (int32_t)X2C_TRUNCI(sc(scale, min0,
                path->Adr[linksize-1L].optalt),X2C_min_longint,
                X2C_max_longint);
   y = frameyd;
   if (y<=tmp) for (;; y++) {
      { /* with */
         struct imagetext_PIX * anonym5 = &image->Adr[(linksize+(framexl-1L))
                *image->Len0+y];
         anonym5->r += 600U;
         anonym5->g += 600U;
         anonym5->b += 500U;
      }
      if (y==tmp) break;
   } /* end for */
   if (opt) i = -3L;
   else i = 0L;
   for (;;) {
      y = frameyd+(int32_t)X2C_TRUNCI(X2C_DIVL((double)(st*i+m)-min0,
                mpp)+0.5,X2C_min_longint,X2C_max_longint);
      if (y>=(int32_t)X2C_TRUNCI(sc(scale, min0, path->Adr[maxi].optalt),
                X2C_min_longint,X2C_max_longint)) break;
      tmp = linksize-2L;
      x = 1L;
      if (x<=tmp) for (;; x++) {
         if (opt) {
            a0 = path->Adr[x].zero*scale+(double)(float)y;
            a1 = path->Adr[x+1L].zero*scale+(double)(float)y;
         }
         else {
            a0 = (double)(float)y;
            a1 = a0;
         }
         if ((a0>(double)(float)frameyd && a0<sc(scale, min0,
                path->Adr[x].alt)) && path->Adr[x].alt<10000.0) {
            drawcolon(x+framexl, a0, a1, 25L, 25L, 25L);
         }
         if (x==tmp) break;
      } /* end for */
      if (y>frameyd/2L+2L) {
         tmp = framexl+2L;
         x = framexl-2L;
         if (x<=tmp) for (;; x++) {
            { /* with */
               struct imagetext_PIX * anonym6 = &image->Adr[(x)
                *image->Len0+y];
               anonym6->r += 600U;
               anonym6->g += 600U;
               anonym6->b += 600U;
            }
            if (x==tmp) break;
         } /* end for */
         aprsstr_IntToStr(st*i+m, 4UL, s, 100ul);
         aprsstr_Append(s, 100ul, "m", 2ul);
         imagetext_writestr(image, 0UL, (uint32_t)(y-fonty/2L),
                (uint32_t)fonttyp, 700UL, 700UL, 500UL, s, 100ul);
      }
      ++i;
   }
   /*** hor scale ***/
   a0 = (X2C_DIVL(dist,(double)(float)linksize))*7000.0*(double)(float)fontx;
   if (dist>=10.0 && a0<1000.0) a0 = 1000.0;
   st = Scale125(a0);
   xc = (int32_t)X2C_TRUNCI(dist*1000.0,X2C_min_longint,
                X2C_max_longint)/st;
   tmp = xc;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      n = framexl+(int32_t)X2C_TRUNCI(X2C_DIVL((double)(float)
                (linksize*i*st),dist*1000.0)+0.5,X2C_min_longint,
                X2C_max_longint);
      tmp0 = frameyd+1L;
      y = frameyd-2L;
      if (y<=tmp0) for (;; y++) {
         { /* with */
            struct imagetext_PIX * anonym7 = &image->Adr[(n)*image->Len0+y];
            anonym7->r += 600U;
            anonym7->g += 600U;
            anonym7->b += 600U;
         }
         if (y==tmp0) break;
      } /* end for */
      if (st>=1000L) dk = 1000L;
      else dk = 1L;
      aprsstr_IntToStr((st*i)/dk, 1UL, s, 100ul);
      if (i==xc) {
         if (dk==1L) aprsstr_Append(s, 100ul, "m", 2ul);
         else aprsstr_Append(s, 100ul, "km", 3ul);
      }
      dk = n-(int32_t)(imagetext_strsize((uint32_t)fonttyp, s,
                100ul)/2UL);
      if (dk+(int32_t)imagetext_strsize((uint32_t)fonttyp, s,
                100ul)>=xsize) {
         dk = (xsize-(int32_t)imagetext_strsize((uint32_t)fonttyp, s,
                100ul))-2L;
      }
      imagetext_writestr(image, (uint32_t)dk,
                (uint32_t)((frameyd-fonty)-3L), (uint32_t)fonttyp, 700UL,
                 700UL, 500UL, s, 100ul);
      if (i==tmp) break;
   } /* end for */
   aprsstr_Assign(s, 100ul, labela, 100ul);
   aprsstr_Append(s, 100ul, " [", 3ul);
   aprsstr_IntToStr((int32_t)X2C_TRUNCI(alta-anta,X2C_min_longint,
                X2C_max_longint), 0UL, ss, 100ul);
   aprsstr_Append(s, 100ul, ss, 100ul);
   aprsstr_Append(s, 100ul, "NN+", 4ul);
   aprsstr_IntToStr((int32_t)X2C_TRUNCI(anta,X2C_min_longint,
                X2C_max_longint), 0UL, ss, 100ul);
   aprsstr_Append(s, 100ul, ss, 100ul);
   aprsstr_Append(s, 100ul, "m]", 3ul);
   imagetext_writestr(image, 0UL, (uint32_t)(ysize-(fonty*12L)/10L),
                (uint32_t)fonttyp, 800UL, 700UL, 100UL, s, 100ul);
   strncpy(s,"[",100u);
   aprsstr_FixToStr((float)(X2C_DIVL(posa.lat,1.7453292519444E-2)), 6UL,
                ss, 100ul);
   aprsstr_Append(s, 100ul, ss, 100ul);
   aprsstr_FixToStr((float)(X2C_DIVL(posa.long0,1.7453292519444E-2)), 6UL,
                 ss, 100ul);
   aprsstr_Append(s, 100ul, "/", 2ul);
   aprsstr_Append(s, 100ul, ss, 100ul);
   aprsstr_Append(s, 100ul, "]", 2ul);
   imagetext_writestr(image, 0UL, (uint32_t)(ysize-(fonty*24L)/10L),
                (uint32_t)fonttyp, 700UL, 700UL, 500UL, s, 100ul);
   aprsstr_FixToStr((float)mhz, 2UL*(uint32_t)(mhz<30.0), s, 100ul);
   aprsstr_Append(s, 100ul, "MHz Refrac=", 12ul);
   aprsstr_FixToStr((float)refraction, 3UL, ss, 100ul);
   aprsstr_Append(s, 100ul, ss, 100ul);
   imagetext_writestr(image, 0UL, (uint32_t)(ysize-(fonty*36L)/10L),
                (uint32_t)fonttyp, 700UL, 700UL, 500UL, s, 100ul);
   aprsstr_Assign(s, 100ul, labelb, 100ul);
   aprsstr_Append(s, 100ul, " [", 3ul);
   aprsstr_IntToStr((int32_t)X2C_TRUNCI(altb-antb,X2C_min_longint,
                X2C_max_longint), 0UL, ss, 100ul);
   aprsstr_Append(s, 100ul, ss, 100ul);
   aprsstr_Append(s, 100ul, "NN+", 4ul);
   aprsstr_IntToStr((int32_t)X2C_TRUNCI(antb,X2C_min_longint,
                X2C_max_longint), 0UL, ss, 100ul);
   aprsstr_Append(s, 100ul, ss, 100ul);
   aprsstr_Append(s, 100ul, "m]", 3ul);
   imagetext_writestr(image, (uint32_t)leftbound(s, 100ul),
                (uint32_t)(ysize-(fonty*12L)/10L), (uint32_t)fonttyp,
                800UL, 700UL, 100UL, s, 100ul);
   strncpy(s,"[",100u);
   aprsstr_FixToStr((float)(X2C_DIVL(posb.lat,1.7453292519444E-2)), 6UL,
                ss, 100ul);
   aprsstr_Append(s, 100ul, ss, 100ul);
   aprsstr_FixToStr((float)(X2C_DIVL(posb.long0,1.7453292519444E-2)), 6UL,
                 ss, 100ul);
   aprsstr_Append(s, 100ul, "/", 2ul);
   aprsstr_Append(s, 100ul, ss, 100ul);
   aprsstr_Append(s, 100ul, "]", 2ul);
   imagetext_writestr(image, (uint32_t)leftbound(s, 100ul),
                (uint32_t)(ysize-(fonty*24L)/10L), (uint32_t)fonttyp,
                700UL, 700UL, 500UL, s, 100ul);
   /*  s:="fl="; */
   aprsstr_FixToStr((float)(32.2+8.685889638065*log(dist*mhz)), 2UL, sss,
                100ul);
   aprsstr_Append(sss, 100ul, "dBi", 4ul);
   cr = 700L;
   cg = 700L;
   cb = 700L;
   if (airshadow>0.2) {
      cr = 800L; /* ground shadow*/
      cg = 100L;
      cb = 0L;
   }
   else if (woodshadow>0.2) {
      cr = 700L; /* wood shadow */
      cg = 700L;
      cb = 0L;
   }
   imagetext_writestr(image, (uint32_t)leftbound(sss, 100ul),
                (uint32_t)(ysize-(fonty*36L)/10L), (uint32_t)fonttyp,
                (uint32_t)cr, (uint32_t)cg, (uint32_t)cb, sss, 100ul);
   if (treedrawn) {
      strncpy(s,"Trees=",100u);
      aprsstr_FixToStr((float)treesize, 0UL, ss, 100ul);
      aprsstr_Append(s, 100ul, ss, 100ul);
      aprsstr_Append(s, 100ul, "m", 2ul);
      aprsstr_Append(sss, 100ul, s, 100ul);
      aprsstr_Append(sss, 100ul, " ", 2ul);
      imagetext_writestr(image, (uint32_t)leftbound(sss, 100ul),
                (uint32_t)(ysize-(fonty*36L)/10L), (uint32_t)fonttyp,
                700UL, 700UL, 700UL, s, 100ul);
   }
/*WrFixed(airshadow,2, 9);WrFixed(woodshadow,2, 9);WrStrLn(" air wood"); */
} /* end drawimage() */


static void wralt(void)
{
   struct aprsstr_POSITION posr;
   float res;
   float a;
   posr.lat = (float)posa.lat;
   posr.long0 = (float)posa.long0;
   a = libsrtm_getsrtm(posr, 1UL, &res);
   if (a>=10000.0f) Error("no altitude for this position", 30ul);
   osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(a+0.5f,X2C_min_longint,
                X2C_max_longint), 1UL);
} /* end wralt() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   size_t tmp[1];
   size_t tmp0[2];
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   imagetext_BEGIN();
   aprsstr_BEGIN();
   aprspos_BEGIN();
   libsrtm_BEGIN();
   osi_BEGIN();
   imagefn[0] = 0;
   xsize = 600L;
   ysize = 400L;
   posinval(&posa);
   posinval(&posb);
   anta = 10.0;
   antb = 10.0;
   refraction = 0.25;
   mhz = 145.0;
   opt = 0;
   igamma = 2.2;
   treesize = 0.0;
   fonttyp = 6L;
   labela[0] = 0;
   labelb[0] = 0;
   Parms();
   fontx = (int32_t)imagetext_fontsizex((uint32_t)fonttyp);
   fonty = (int32_t)imagetext_fontsizey((uint32_t)fonttyp);
   framexl = fontx*5L+2L;
   frameyd = fonty+5L;
   frameyu = (fonty*39L)/10L;
   treedrawn = 0;
   if (posa.lat==0.0 && posa.long0==0.0) Error("need Position A", 16ul);
   if (posb.lat==0.0 && posb.long0==0.0) {
      if (imagefn[0U]==0) wralt();
      else Error("need Position B", 16ul);
   }
   else {
      if (imagefn[0U]==0) Error("need Image Filename", 20ul);
      if (xsize<framexl+5L+30L) Error("xsize too less", 15ul);
      if (ysize<30L) Error("ysize too less", 15ul);
      linksize = xsize-(framexl+5L);
      X2C_DYNALLOCATE((char **) &path,sizeof(struct PATH),
                (tmp[0] = (size_t)linksize,tmp),1u);
      if (path==0) Error("out of memory", 14ul);
      calcpath();
      libsrtm_closesrtmfile(); /* free srtm cache */
      X2C_DYNALLOCATE((char **) &image,sizeof(struct imagetext_PIX),
                (tmp0[0] = (size_t)xsize,tmp0[1] = (size_t)ysize,tmp0),2u);
      if (image==0) Error("out of memory", 14ul);
      drawimage();
      wrpng();
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
