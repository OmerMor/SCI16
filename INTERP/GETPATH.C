
/* GETPATH.C   PATH AROUND AN OBSTACLE PLOYGON
**
** Algorithm by Larry Scott 
**
** NAME---GETPATH---
**    POLYGON must be closed and not self intersecting (JORDAN ploygon) 
**    all points of polygon must lie with screen limits and the polygon
**    must be defined in a screen clockwise direction.
*/

#     include  "getpath.h"

#  ifdef TEST
#     include  "malloc.h"
#     include  "stdio.h"
#     include  "stdlib.h"
#  else
#     include  "start.h"
#     include  "math.h"
#     include  "memmgr.h"
#     include  "window.h"
#     include  "trig.h"
#		include	"errmsg.h"
#		include	"picture.h"
#  endif


static AvdPoint * near  avdpath(AvdPoint *,AvdPoint *,polygon *,int);
static polyNode * near  avdpoly(AvdPoint *,AvdPoint *,polygon *);
static AvdPoint * near  copyPath(AvdPoint *);
static void       near  delPoly(polygon *,int);
static int        near  distEstimate(AvdPoint *,AvdPoint *,int *);
static int        near  distsq(int,int,int,AvdPoint *,int,AvdPoint *,AvdPoint *,int *);
static int        near  dominates(polyPatch *,polyPatch *,AvdPoint *);
static void       near  endPath(AvdPoint *,AvdPoint *);
static void       near  freeNpath(polyNode *);
static int        near  getPolyDirections(polyNode *);
static int        near  getPolyDistance(AvdPoint *);
static int        near  intpoly(AvdPoint *,AvdPoint *,AvdPoint *,int,AvdPoint *,AvdPoint *,int *, int *);
static int        near  intsegms(AvdPoint *,AvdPoint *,AvdPoint *,AvdPoint *,AvdPoint *);
static void       near  invertPolygon(AvdPoint *,int);
static int        near  lineOnScreenEdge(AvdPoint *,AvdPoint *);
static void       near  optpath(AvdPoint *,AvdPoint *,AvdPoint *,polyNode *,polygon  *,int);
static void       near  MergePoly(AvdPoint *,int,polygon *);
static long       near  nearPoint(AvdPoint *,AvdPoint *,int,AvdPoint *,int);
static int        near  PatchNode(polyPatch *,int,AvdPoint *);
static int        near  nodeTest(AvdPoint *,AvdPoint *,AvdPoint *,AvdPoint *);
static int        near  polypath(AvdPoint *,AvdPoint *,AvdPoint *,int,AvdPoint *,AvdPoint *,int *, int *);
static void       near  reducePolyList(polygon*,polyNode *);
static void       near  removeNode(polygon *,int);
static void       near  setPolyDirections(polyNode *,int,int);
static void       near  startPath(AvdPoint *,AvdPoint *);
static void       near  v_add(AvdPoint *, AvdPoint *, AvdPoint *);
static int        near  v_cross3rd_comp(AvdPoint *, AvdPoint *);
static long       near  v_dot(AvdPoint *, AvdPoint *);
static long       near  v_sizesqrd(AvdPoint *, AvdPoint *);
static void       near  v_subtract(AvdPoint *, AvdPoint *, AvdPoint *);


static AvdPoint picWindPoly[4];

#  ifdef TEST
static int near ATan(int,int,int,int);
static int near getMajor(int,int,int,int);
static int near prorateATan(long);
static int near testColinear(AvdPoint *,int *);
static int near testClockwise(AvdPoint *,int);

#define TrigScale       (long)10000

static long near ATanArray[]=
      {
        875,
       1763,
       2679,
       3640,
       4663,
       5774,
       7002,
       8391,
      10000
      };
#  endif

AvdPoint * 
getpath(A,B,polylist,opt)
AvdPoint *A,*B;
polygon  *polylist;
int opt;
   {
   int                 pathType;
   int                 i,j,k;
	AvdPoint            exitPoint,entryPoint,*P,Pt;
   AvdPoint            I1,I2;
   int                 nodeI1, nodeI2;
   enum PathStartType  PathStart;
   enum PathEndType    PathEnd;

#  ifdef TEST
   int                 count;

   /* make ploygons clockwise */
	/* The following code, TestClockwise and InvertPolygon are
      only needed if some polygons may be defined in a counter
      clockwise direction. Clockwise refers to users view point
      which is counter clockwise mathematically.
   */
   for (i=0;polylist[i].n != 0;++i)
      {
      if (testClockwise(polylist[i].polyPoints,polylist[i].n) < 0)
         {
         if(polylist[i].type!=CAP)
            {
            invertPolygon(polylist[i].polyPoints,polylist[i].n);
            printf("Direction of !CAP polygon counter clockwise. Fixed!");
            }
         }
      else
         {
         if(polylist[i].type==CAP)
            {
            invertPolygon(polylist[i].polyPoints,polylist[i].n);
            printf("Direction of CAP polygon clockwise. Fixed!");
            }

         }
      /* also test for adjacent colinear line segments */
		if (count = testColinear(polylist[i].polyPoints,&polylist[i].n))
         {
         printf("%d adjacent colinear line segments found. Fixed!",count);
         }
      }
   /* set screen polygon */
   picWindPoly[0].x = 0;
   picWindPoly[0].y = 0;
   picWindPoly[1].x = 319;
   picWindPoly[1].y = 0;
   picWindPoly[2].x = 319;
   picWindPoly[2].y = 189;
   picWindPoly[3].x = 0;
   picWindPoly[3].y = 189;
#  else
   /* set screen polygon */
   picWindPoly[0].x = picWind->port.portRect.left;
   picWindPoly[0].y = picWind->port.portRect.top;
   picWindPoly[1].x = picWind->port.portRect.right - 1;
   picWindPoly[1].y = picWind->port.portRect.top;
   picWindPoly[2].x = picWind->port.portRect.right - 1;
   picWindPoly[2].y = picWind->port.portRect.bottom - 1;
   picWindPoly[3].x = picWind->port.portRect.left;
   picWindPoly[3].y = picWind->port.portRect.bottom - 1;
#  endif

   PathStart = fromA;
   PathEnd = toB;

   /* TAP polygons are invisable when optimization is off */
      for (i=0;polylist[i].n != 0;++i)
         {
         if (
               (polylist[i].type >= MTP) ||
               ((polylist[i].type == TAP) && (opt == FALSE))
            )
            {
            delPoly(polylist,i);
            --i;
            }

         }
   /* are we starting in a polygon? */
   for (i=0;polylist[i].n != 0;++i)
      {
      if (
            ptIntr(A,polylist[i].polyPoints,polylist[i].n) ||
            (
               (polylist[i].type==CAP) &&
               (nearPoint(A,polylist[i].polyPoints,polylist[i].n,&I2,TRUE)<2)
            )
         )
         {
         switch (polylist[i].type)
            {
            case TAP:
               delPoly(polylist,i);
               --i;
               break;
            case NAP:
               PathStart = fromAtoEXIT;
               if (opt)
                  nearPoint(A,polylist[i].polyPoints,polylist[i].n,&exitPoint,TRUE);
               else
                  delPoly(polylist,i);
               break;
            case BAP:
               PathStart = fromBAPtoEXIT;
               nearPoint(A,polylist[i].polyPoints,polylist[i].n,&exitPoint,TRUE);
               break;
            case CAP:
               if (!ptIntr(B,polylist[i].polyPoints,polylist[i].n))
                  {
                  /* Point A is interior and point B is exterior */
                  PathEnd = toENTRY;
                  /* Find the last exit point */
                  if (opt)
                     {
                     nearPoint(B,polylist[i].polyPoints,polylist[i].n,&entryPoint,FALSE);
                     }
                  else
                     {
                     intpoly(A,B,polylist[i].polyPoints,polylist[i].n,&I1,&I2,&nodeI1,&nodeI2);
                     entryPoint = I1;
                     }
                  }
               break;
            }
         }
      else
         {
         /* Turn CAP in BAP */
         if(polylist[i].type==CAP)
            {
            invertPolygon(polylist[i].polyPoints,polylist[i].n);
            polylist[i].type=BAP;
            polylist[i].info = polylist[i].info | INVERTED;
      
            }
         }
      }
   /* are we ending in a polygon? */
   for (i=0;polylist[i].n != 0;++i)
      {
      if (polylist[i].type!=CAP && ptIntr(B,polylist[i].polyPoints,polylist[i].n))
         {
         switch (polylist[i].type)
            {
            case TAP:
               PathEnd = toB;
               delPoly(polylist,i);
               --i;
               break;
            case NAP:
               PathEnd = toENTRYtoB;
               if (opt)
                  nearPoint(B,polylist[i].polyPoints,polylist[i].n,&entryPoint,TRUE);
               else
                  delPoly(polylist,i);
               break;
            case BAP:
               // If PathEnd == toENTRY we must be trying to get out of a CAP
               if(PathEnd != toENTRY)
                  {
                  PathEnd = toENTRY;
                  nearPoint(B,polylist[i].polyPoints,polylist[i].n,&entryPoint,TRUE);
                  }
               break;
            }
         }
      }

   /* Change saved CAPs into BAPs */
   for (i=0;polylist[i].n != 0;++i)
      {
      if(polylist[i].type==CAP)
         {
         polylist[i].type=BAP;
         }
      }

   pathType = (PathStart << 2) + PathEnd;
   switch (pathType)
      {
      case 0:
         // fromA ---> toB
         P = avdpath(A,B,polylist,opt);
         break;
      case 1:
         // fromA ---> toENTRYtoB
         if (opt)
            {
            P = avdpath(A,&entryPoint,polylist,opt);
            endPath(B,P);
            }
         else
            {
            P = avdpath(A,B,polylist,opt);
            }
         break;
      case 2:
         // fromA ---> toENTRY
         P = avdpath(A,&entryPoint,polylist,opt);
         break;
      case 4:
         // fromAtoEXIT ---> toB
         if (opt)
            {
            P = avdpath(&exitPoint,B,polylist,opt);
            startPath(A,P);
            }
         else
            {
            P = avdpath(A,B,polylist,opt);
            }
         break;
      case 5:
         // fromAtoEXIT ---> toENTRYtoB
         if (opt)
            {
            P = avdpath(&exitPoint,&entryPoint,polylist,opt);
            startPath(A,P);
            endPath(B,P);
            }
         else
            {
            P = avdpath(A,B,polylist,opt);
            }
         break;
      case 6:
         // fromAtoEXIT ---> toENTRY
         if (opt)
            {
            P = avdpath(&exitPoint,&entryPoint,polylist,opt);
            startPath(A,P);
            }
         else
            {
            P = avdpath(A,&entryPoint,polylist,opt);
            }
         break;
      case 8:
         // fromBAPtoEXIT ---> toB
         P = avdpath(&exitPoint,B,polylist,opt);
         if (opt) startPath(A,P);
         break;
      case 9:
         // fromBAPtoEXIT ---> toENTRYtoB
         if (opt)
            {
            P = avdpath(&exitPoint,&entryPoint,polylist,opt);
            startPath(A,P);
            endPath(B,P);
            }
         else
            {
            P = avdpath(&exitPoint,B,polylist,opt);
            }
         break;
      case 10:
         // fromBAPtoEXIT ---> toENTRY
         P = avdpath(&exitPoint,&entryPoint,polylist,opt);
         if (opt) startPath(A,P);
         break;
      } 
   /* This may not be needed anymore */
   /* kludge to prevent deleting of second point */
   if (!opt && P[2].x != ENDOFPATH) 
      {
      /******************************************************/
      /* if first two points are the same and if direction
      ** not blocked eliminate duplicate point, otherwise
      ** leave duplicate in to stop motion around obstacle.
      */
      if (
         FALSE              &&
         (P[0].x == P[1].x) &&
         (P[0].y == P[1].y)
         )
         {
         Pt = *A;
         if (A->x < B->x) Pt.x += 2;
         if (B->x < A->x) Pt.x -= 2;
         if (A->y < B->y) Pt.y += 2;
         if (B->y < A->y) Pt.y -= 2;
         for (i=0,j=0;polylist[i].n != 0;++i)
            {
            if (ptIntr(&Pt,polylist[i].polyPoints,polylist[i].n))
               {
               j=1;
               break;
               }
            }
         if (!j)
            {
            for (k=0;P[k].x != ENDOFPATH;++k)
               {
               P[k] = P[k+1];
               }
            }
         }
      }
      /******************************************************/
    /* Re invert previously inverted polygons  */
    for (i=0;polylist[i].n != 0;++i)
      {
      if(polylist[i].info & INVERTED)
         {
          invertPolygon(polylist[i].polyPoints,polylist[i].n);
         }
      }
   return (P);
   }

static void near delPoly(polylist,i)
polygon * polylist;
int   i;
   {
   for (;polylist[i].n != 0;++i)
      {
      polylist[i] = polylist[i+1];
      }
   }

static void near startPath(A,P)
AvdPoint *P,*A;
   {
   int i;
   i = 0;
   do
      ++i;
   while (P[i].x != ENDOFPATH);
   for (++i;i!= 0;--i)
      {
      P[i] = P[i-1];
      }
   P[0] = *A;
   }

static void near endPath(B,P)
AvdPoint *P,*B;
{
   int i;
   i = 0;
   do
      ++i;
   while (P[i].x != ENDOFPATH);
   P[i++] = *B;
   P[i].x = ENDOFPATH;
}
static long near nearPoint(P,Poly,n,R,Edge)
AvdPoint *P,*Poly,*R;
int      Edge;
int   n;
   {
   int         i;
   long        d=0x07FFFFFFF,dot1,dot2,dot3;
   AvdPoint    A,B,tmp1,tmp2,tmp3,tmp4;
   int         offScreen;
   for (i=0;i<n;++i)
      {
      A = Poly[i];
      if (i==n-1)
         B = Poly[0];
      else
         B = Poly[i+1];
      if 
         (
            Edge                        
         &&
            (
            (A.x == B.x) && ((A.x == 0) || 
            (A.x == picWindPoly[1].x))  ||
            (A.y == B.y) && ((A.y == 0) ||
            (A.y == picWindPoly[2].y))
            )
         )

         {
         /* do nothing, the line is on the screen edge */
         }
      else
         {
         /* If P is closest to this line save 
            the closest on this line to P */
         v_subtract(&B,&A,&tmp1);
         v_subtract(&A,P,&tmp2);
         v_subtract(&B,P,&tmp3);
         if ( (v_dot(&tmp1,&tmp2) <= 0) && (v_dot(&tmp1,&tmp3) >= 0) )
            /* P-R is normal to A-B 
          
                          A
            ***************************
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
              P----------*R
                         *
                         *
                         * 
            ***************************
                         B
            */

            {
            /* Distance P-R is:
               ABS{[(B-A)^(0,0,1)]#(A-P)/|B-A|}
            */

            /* Make N = tmp4 normal to A-B */
            tmp4.x = tmp1.y;
            tmp4.y = -tmp1.x;
            dot1 = v_dot(&tmp2,&tmp4);
            dot2 = dot1/(long)distEstimate(&A,&B,&offScreen);
            if (dot2 <0) dot2 = -dot2;
            if (dot2 < d)
               {
               d = dot2;
               
               /* R = P + [(N#(A-P))/(N#N)]N */
               /* round to force point exterior to polygon */
               dot2 = v_dot(&tmp4,&tmp4);
               dot3 = dot1*(long)tmp4.x;
               R->x = P->x + (int) 
                  (
                  (
                  dot3 + sign(dot3)*(dot2 - 1)
                  )
                  /dot2
                  );
               dot1 = dot1*(long)tmp4.y;
               R->y = P->y + (int)
                  (
                  (
                  dot1 + sign(dot1)*(dot2 - 1)
                  )
                  /dot2
                  );
               }
            }
         else
            {
            /* P-R is not normal to A-B, for example 
          
                 P*
                   *
                    *
                     *
                      *
                       *
                      R * A
            ***************************
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         * 
                         *
                         *
                         * 
            ***************************
                         B
            */

            dot1 = (long)distEstimate(&A,P,&offScreen);
            dot2 = (long)distEstimate(&B,P,&offScreen);


//            dot1 = v_dot(&tmp2,&tmp2);
//            dot2 = v_dot(&tmp3,&tmp3);
            if (dot1 < d)
               {
               d = dot1;
               *R = A;
               }
            if (dot2 < d)
               {
               d = dot2;
               *R = B;
               }
            }
         }
      }
   return(d);
   }

static void near invertPolygon(Points,n)
AvdPoint *Points;
int   n;
   {
   int   i,j;
   AvdPoint P;
   for (i=0,j=n-1;i<j;)
      {
      P = Points[i];
      Points[i++] = Points[j];
      Points[j--] = P;
      }
   }

#  ifdef TEST
static int near testClockwise(Points,n)
AvdPoint *Points;
int   n;
   {
   int   i;
   int   angle =0;
   int   angleStart,angleIn,angleOut,delta;
   angleStart = ATan(Points[0].x,Points[0].y,Points[1].x,Points[1].y);
   angleIn = angleStart;
   for (i=1;i<n-1;++i)
      {
      angleOut = ATan(Points[i].x,Points[i].y,Points[i+1].x,Points[i+1].y);
      delta = angleOut - angleIn;
      if (delta > 180) delta -= 360;
      if (delta < -180) delta += 360;
      angle += delta;
      angleIn = angleOut;
      }
   /* next nine lines only needed for initial testing */
   angleOut = ATan(Points[i].x,Points[i].y,Points[0].x,Points[0].y);
   delta = angleOut - angleIn;
   if (delta > 180) delta -= 360;
   if (delta < -180) delta += 360;
   angle += delta;
   delta = angleStart - angleOut;
   if (delta > 180) delta -= 360;
   if (delta < -180) delta += 360;
   angle += delta;
   /* at this point if previous nine lines
      are compiled then angle = +-360 */
   return(angle);
   }

static int near ATan(x1,y1,x2,y2)
int   x1,y1,x2,y2;
/* returns heading from (x1,y1) to (x2,y2)
   in degrees (always in the range 0-359) */
   {
   int major;
   major = getMajor(x1,y1,x2,y2);
   /*    0 <= major <= 90   */
   if (x2 < x1)
      {
      if (y2 <= y1)
         {
         major += 180;
         }
      else
         {
         major = 180 - major;
         }
      }
   else
      {
      if (y2 < y1) major = 360 - major;
      if (major == 360) major = 0;
      }
   return(major);
   }

static int near getMajor(x1,y1,x2,y2)
int   x1,y1,x2,y2;
/* returns angle in the range 0-90 */
   {
   long  deltaX,deltaY,index;
   int   major;
   deltaX = (long) (abs(x2 - x1));
   deltaY = (long) (abs(y2 - y1));
   /* if (x1,y1) = (x2,y2) return 0 */
   if 
      (
      (deltaX == 0) &&
      (deltaY == 0)
      )
      return(0);

   if (deltaY <= deltaX)
      {
      index = ((TrigScale*deltaY)/deltaX);
      if (index < (long) 1000)
         {
         major = (int)
         (
         ((long)57*deltaY + (deltaX/(long)2))
         /deltaX
         );
         }
      else
         {
         major =  prorateATan(index);
         }
      }
   else
      {
      major = 90 - getMajor(y1,x1,y2,x2);
      }
   return(major);
   }

static int near prorateATan(index)
long  index;
   {
   int   i=0;
   while (ATanArray[i] < index) ++i;
   return
      (
      (5*i) +
      (int) 
      (
      (((long)5*(index - ATanArray[i-1])) + ((ATanArray[i] - ATanArray[i-1])/2))
      /(ATanArray[i] - ATanArray[i-1])
      )
      );
   }

static int near testColinear(P,n)
AvdPoint *P;
int  *n;
   {
   int  i,j;
   int  ret_value = 0;
   int  nodes;
   AvdPoint P1,P2,P3,tmp1,tmp2;

   nodes = *n;
   P1 = P[0];                                            
   P2 = P[1];
   P3 = P[2];
   for (i=3;i<(nodes+2);++i)
      {
      v_subtract(&P2,&P1,&tmp1);
      v_subtract(&P3,&P2,&tmp2);
      if (v_cross3rd_comp(&tmp1,&tmp2) == 0)
         {
         /* count of adjacent colinear line segments encountered */
         ++ret_value;
         /* eliminate the extra node */
         --*n;
         for (j=i;j<(nodes+1);++j)
            {
            P[j-2] = P[j-1];
            }
         --nodes;
         /* retry this node */
         --i;
         P2 = P3;
         }
      else
         {
         P1 = P2;
         P2 = P3;
         }
      P3 = P[i % nodes];
      }
   return (ret_value);
   }

#     endif



/*                  PATH AROUND ALL OBSTACLE PLOYGONS
**
**
** NAME---AVDPATH---
**    polygons must be closed and not self intersecting (JORDAN ploygon) 
**    and defined in a clockwise direction.
** input:
**   line segment A-B & polygonList
** output:
**   path
**
**
**   EXAMPLE:
**        AVOIDPATH(A,B,opt,polyList) ;polyList = pathM,pathN (any order)
**
**        returns path:
**         NO OPTIMIZATION: opt = FALSE
**            A,I1,N4,N3,N2,I2,I3,M5,M4,M3,M2,I4,B
**         WITH OPTIMIZATION: opt = TRUE
**            A,N4,M5,M4,B
**        
**                  
**
**
**                                              M7 ********* M8
**                                                *        *
**               N0     N1                       *         *
**               ********                       *          * M0
**              *        *                     *            *
**             *          *                   *              *
**            *            *                 *                *
**           *              *               *                  *
**       N5 *                *          M6 *                    *
**          *I1               * I2        I3*                 I4 * M1
**  A-------*------------------*-------------*------------------*----------B
**          *                   *N2           *                * M2
**          *                  *               *               *
**          *                 *                 *              *
**       N4 ****************** N3                *             *
**                                                *            * M3
**                                                 *          *
**                                                  *        *
**                                                   *      *
**                                                 M5 ****** M4
**
**
**
**  - DESIGNATES SUBTRACTION OF 2 VECTORS      (returns a vector)
**  + DESIGNATES ADDITION OF 2 VECTORS         (returns a vector)
**  # DESIGNATES DOT PRODUCT OF 2 VECTORS      (returns a scalar)
**  ^ DESIGNATES CROSS PRODUCT OF 2 VECTORS    (returns a vector)
**  | V | DESIGNATES SIZE OF VECTOR V          (returns a scalar)
**  V(i) DESIGNATES COORDINATE i of VECTOR V   (returns a scalar)
**  ! DESIGNATES LARGEST POSSIBLE VALUE
**
**
**   polyNode:
**      next           :next polyNode
**      prev           :previous polyNode
**      I1             :(x,y) value of nearest intersection with A
**      I2             :(x,y) value of nearest intersection with B
**      d              :1 normal direction, -1 reverse direction
**      I1             :entry node
**      I2             :exit node
**      poly           :pointer to polygon
**
**
**   polyList:
**      polygon0       :pointer to possible polygon obstacle
**      polygon1       :pointer to possible polygon obstacle
**      polygon2       :pointer to possible polygon obstacle
**        .
**        .
**        .
**      polygonN       :pointer to possible polygon obstacle
**        0            :indicates end of polyList
**
*/

static AvdPoint * near avdpath(A,B,polylist,opt)
AvdPoint    *A,*B;
polygon  *polylist;
int opt;
   {
   polygon     *newPolylist;
   int         i, j;
   AvdPoint    startPath;
   polyNode    *Npath;
   polyNode    *polyNpath;
   AvdPoint    Path[MAXPATH];
   int         totalNodes = 0;
   int         polyDirections,firstPolyDirection,bestPolyDirection;
   int         polyDistance,bestPolyDistance;
   polyNode    *node,*tmp;

   Npath = 0;
   /* duplicate polylist so that we can eliminate polygons while
      creating unoptimized path yet still retain original polylist
      for optimization pass.
   */
   startPath = *A;
   for (i=0;polylist[i].n != 0;++i);
   ++i;
   ++i;
#  ifdef TEST
      newPolylist = (polygon *) calloc(i, sizeof(polygon));
      if (newPolylist == 0) printf("no memory available");
#  else
      newPolylist = (polygon *) RNewPtr((i*sizeof(polygon)));
#  endif
/*   HOOK                               */
/* this is a good place to get rid of unwanted polygons ie ctp,cnp etc. */
   i = -1;
   j = -1;
   do
      {
      ++i; ++j;
      if (polylist[i].type >= MTP) 
         --j;
      else
          newPolylist[j] = polylist[i];

      }

   while (polylist[i].n != 0);

   while (polyNpath = avdpoly(&startPath,B,newPolylist))
      {
      ++totalNodes;
      /* add first node in polyNpath to Npath */
      /* note that Npath points to last node until path complete. */
      if (Npath)
         {
         Npath->next = polyNpath;
         polyNpath->prev = Npath;
         }
/*      else
         {
         polyNpath->prev = 0;
         }                     */
      /* free any unused nodes */
      for (node=polyNpath->next;node != 0;)
         {
         tmp = node->next;
#        ifdef TEST
            free(node);
#        else
            DisposePtr(node);
#        endif
         node=tmp;
         }
      polyNpath->next = 0;
      Npath = polyNpath;
      startPath = polyNpath->I2;


      /* Now reduce list of polygons to only those which can interfere
         with the path starting from the exit of the first polygon and
         going to the point B. We only generate a path around the first
         encountered polygon and then recurse to generate a path around
         the next etc. until there are no more polygons in the way.
      */
      reducePolyList(newPolylist,polyNpath);
      }

   /* free up space allocated for newPolylist */
#  ifdef TEST
      free(newPolylist);
#  else
      DisposePtr(newPolylist);
#  endif
   /* set Npath to point to the first node */
   if (Npath)
      {
      while (Npath->prev != 0)
         {
         Npath = Npath->prev;
         }
      }

   /* Now chain together the paths around the polygons
      and optimize out any unnecessary line segments. */
   if ((opt>1) && (totalNodes > 1))
      {
      /* optimize as much as possible to the level 2**n */
      /* set up current path directions and try to improve
         on the total distance from A to B */
      if(totalNodes > MAXOPTIMIZEDNODES) totalNodes = MAXOPTIMIZEDNODES;
      polyDirections = getPolyDirections(Npath) % (1<<totalNodes);
      bestPolyDirection = polyDirections;
      firstPolyDirection = polyDirections;
      optpath(A,B,Path,Npath,polylist,opt);
      bestPolyDistance = getPolyDistance(Path);
      while (firstPolyDirection != (polyDirections = (polyDirections + 1) % (1<<totalNodes)))
         {
         setPolyDirections(Npath,polyDirections,totalNodes);
         optpath(A,B,Path,Npath,polylist,opt);
         polyDistance = getPolyDistance(Path);
         if (bestPolyDistance > polyDistance)
            {
            bestPolyDirection = polyDirections;
            bestPolyDistance = polyDistance;
            }
         }
      setPolyDirections(Npath,bestPolyDirection,totalNodes);
      }
   optpath(A,B,Path,Npath,polylist,opt);
   freeNpath(Npath);

   return (copyPath(Path));
   }

static AvdPoint * near copyPath(Path)
AvdPoint *Path;
   {
   int      i;
   AvdPoint *P;
   i = 0;
   do
      {
      }
   while  (Path[i++].x != ENDOFPATH);
   /* getpath may add two points to the path
      when A and B in NAP polygons. */
#  ifdef TEST
   P = (AvdPoint *) calloc(i+2,sizeof(AvdPoint));
#  else
   P = (AvdPoint *) RNewPtr((i+2)*sizeof(AvdPoint));
#  endif
   i = 0;
   do
      {
      P[i] = Path[i]; 
      }
   while  (Path[i++].x != ENDOFPATH);
   return(P);
   }

static polyNode  * near avdpoly(A,B,polylist)
AvdPoint    *A,*B;
polygon  *polylist;
   {
   int         n1,n2,d,i;
   AvdPoint    I1,I2;
   polyNode    *node,*newNode;
   polyNode    *Npath;

   Npath = 0;
   if 
      (!
         (
         (A->x == B->x) &&
         (A->y == B->y)
         )
      )
      {
      for (i=0;polylist[i].n != 0;++i)
         {
         if (d = polypath(A,B,polylist[i].polyPoints,polylist[i].n,&I1,&I2,&n1,&n2))
            /* this polygon is an obstruction, create path node */
            {
#           ifdef TEST
               newNode = (polyNode *) calloc(1, sizeof(polyNode));
               if (newNode == 0) printf("no memory available");
#           else
               newNode = (polyNode *) RNewPtr(sizeof(polyNode));
#           endif
            newNode->I1 = I1;
            newNode->I2 = I2;
            newNode->d = d;
            newNode->n1 = n1;
            newNode->n2 = n2;
            newNode->poly = polylist[i].polyPoints;
            newNode->n = polylist[i].n;
            newNode->next = 0;
            /* add path node to list */
            if (!Npath)
               /* newNode first node in path */
               {
               Npath = newNode;
               newNode->next = 0;
               newNode->prev = 0;
               }
            else
               /* insert path node so that the list is ordered from A to B */
               {
               for (node = Npath;TRUE;node = node->next)
                  {
                  if (
                     (
                     (node->I1.x != A->x) 
                     && 
                     (abs(newNode->I1.x - A->x) < abs(node->I1.x - A->x))
                     )
                     ||
                     (
                     (node->I1.y != A->y) 
                     && 
                     (abs(newNode->I1.y - A->y) < abs(node->I1.y - A->y))
                     )
                     )
                        /* insert newNode before node in path */
                        {
                        newNode->prev = node->prev;
                        newNode->next = node;
                        node->prev = newNode;
                        if (newNode->prev == 0)
                           Npath = newNode;
                        else
                           (newNode->prev)->next = newNode;
                        break;
                        }
                  if (node->next == 0) break;
                  }
               if (newNode->next == 0)
                  /* insert newNode at end of list */
                  {
                  node->next = newNode;
                  newNode->prev = node;
                  }

               }
            }
         }
      }
   return(Npath);
   }

static void near reducePolyList(polylist,Npath)
polygon     *polylist;
polyNode    *Npath;
   {
   polyNode  *temp;
   int       deleted;
   AvdPoint  I1,I2;
   AvdPoint  tmp1,tmp2;
   /* Eliminate any overlaped polygons.
      For example polygon2 can be eliminated in the following case:


                       ********************
                       *                  *
                       *                  *
                       *     polygon2     *
                        *                 *
                         *               *
                          *             *
                *********  *           *  **********
                *        *  *         *  *         *
              I1*         *  *I3   I4*  *          *I2
     A----------*----------*--*-----*--*-----------*---------B
                *           *  *   *  *            *
                *            *  * *  *             *
                *             *  *  *              *
                *              *   *               *
                *               * *                *
                *                *                 *
                *                                  *
                *            polygon1              *
                *                                  *
                ************************************    
   */
   /* First polygon in polylist is checked againest the others to
      see if any polygons in the list can be eliminated. */
   deleted = 0;
   I2 = Npath->I2;
   I1 = Npath->I1;
   Npath = Npath->next;
   while (Npath)
      {
      temp = Npath->next;
      v_subtract(&I2,&I1,&tmp1);
      v_subtract(&I2,&Npath->I2,&tmp2);
      if (v_dot(&tmp1,&tmp2) <= 0l)      
         {
         /* remove Npath->i polygon from polylist */
         removeNode(polylist,Npath->i - deleted);
         ++deleted;
         }
      Npath = temp;
      }
   }

static void near removeNode(polylist,i)
polygon  *polylist;
int      i;
   {
   do
      {
      polylist[i] = polylist[i+1];
      ++i;
      }
   while (polylist[i].n != 0);
   }

static void near freeNpath(Npath)
polyNode    *Npath;
   {
   polyNode   *node,*tmp;
   for (node=Npath;node != 0;)
      {
      tmp = node->next;
#     ifdef TEST
         free(node);
#     else
         DisposePtr(node);
#     endif
      node=tmp;
      }
   }

static int near getPolyDirections(Npath)
polyNode    *Npath;
   {
   int        polyDirections = 0;
   int        d,i;
   polyNode   *node;
   for (node=Npath,i=0;(node != 0) && (i<=MAXOPTIMIZEDNODES);++i)
      {
      d = 1;
      if(node->d<0) d =0;
      polyDirections += (d<<i);
      node = node->next;
      }
   return(polyDirections);
   }

static int near getPolyDistance(Path)
AvdPoint    Path[];
   {
   int   distance = 0,offScreen = 0;
   int   i;
   for (i=0;Path[i+1].x != ENDOFPATH;++i)
      {
      distance += distEstimate(&Path[i],&Path[i+1],&offScreen);
      }
   if (offScreen) return(MAXVALUEINT);
   return(distance);
   }

static void near setPolyDirections(Npath,polyDirections,totalNodes)
polyNode    *Npath;
int         polyDirections,totalNodes;
   {
   polyNode   *node;
   int        i,d;
   for (node=Npath,i=0;(node != 0) && (i<totalNodes);++i)
      {
      d = (polyDirections>>i) & 1;
      if (!d) d = -1;
      if (node->d != d)
         {
         if (node->d == 1)
            {
            node->d = -1;
            node->n1 = (node->n1 - 1 + node->n) % node->n;
            node->n2 = (node->n2 + 1 + node->n) % node->n;
            }
         else
            {
            node->d = 1;
            node->n1 = (node->n1 + 1 + node->n) % node->n;
            node->n2 = (node->n2 - 1 + node->n) % node->n;
            }
         }
      node = node->next;
      }
   }


/*              OPTIMIZE PATH AROUND ALL OBSTACLE PLOYGONS
**
**
** NAME---OPTPATH---
** input:
**   line segment A-B, path nodes created by avdpath, polygonList, opt
** output:
**   path
**
*/

static void near optpath(A,B,Path,Npath,polylist,opt)
AvdPoint    *A,*B;
AvdPoint    Path[];
polyNode    *Npath;
polygon     *polylist;
int         opt;
   {
   int         i,j,k,x;
   int         M,P0,PN,PG;
   polyNode    *node;
   AvdPoint    I1, I2;
   int         nodeI1, nodeI2;


   /* Chain path nodes together to make one large path */
   /* first point is A */
   Path[0].x = A->x;
   Path[0].y = A->y;

   i = 1;
   for (node = Npath;node != 0;)
      {
      Path[i].x = node->I1.x;
      Path[i++].y = node->I1.y;
      j = (node->n1 - node->d + node->n) % node->n;

      do
         {
         j = (j + node->d + node->n) % node->n;
         Path[i].x = (node->poly)[j].x;
         Path[i++].y = (node->poly)[j].y;
         }
      while (j != node->n2);

      Path[i].x = node->I2.x;
      Path[i++].y = node->I2.y;
      node = node->next;
      }

   /* last point is B */
   Path[i].x = B->x;
   Path[i++].y = B->y;
   Path[i].x = ENDOFPATH;
   if (i >= MAXPATH)
      {
      #  ifndef TEST
         Panic(E_POLY_AVOID);
      #  else
         printf("Poly Avoider generated too large of path!");
      #  endif
      }


   /* start optimization of path */
   if (!opt || (i < 3))
      {
      return;
      }
   else
      {

      /* get rid of any adjacent dupicate points */
      for (j=0;Path[j].x != ENDOFPATH;++j)
         {
         if ((Path[j].x == Path[j+1].x) && (Path[j].y == Path[j+1].y))
            {
               for (k=j;Path[k].x != ENDOFPATH;++k)
                  {
                  Path[k] = Path[k+1];
                  }
               --j;
               --i;
            }
         }

      M = i;
      P0 = 0;
      for (PG = 0,PN=M-1;P0 < M-2;PG = 0 )
         {
         x = FALSE;
         for (;polylist[PG].n != 0;++PG)
            {
            if (intpoly(&Path[P0],&Path[PN],polylist[PG].polyPoints,polylist[PG].n,&I1,&I2,&nodeI1,&nodeI2))
               {
               x = TRUE;
               break;
               }
            }
         if (x)
            {
            if (PN > P0+2) 
               PN -= 1;
            else
               {
               P0 += 1;
               PN = M-1;
               }
            }
         else
            {
            /* eliminate nodes P0+1 through PN-1 */
            for (i=P0+1,j=PN,k=M-PN+1;k!=0;--k)
               {
               Path[i].x = Path[j].x;
               Path[i].y = Path[j].y;
               ++i;
               ++j;
               }
            M = M-PN+P0+1;
            PN = M-1;
            P0 += 1;
            }
         }
      }
   }


AvdPoint * 
MergePolygons(poly,polylist)
AvdPoint *poly;
polygon  *polylist;
   {
   AvdPoint    P[MAXPATH];
   AvdPoint    *newPoly;
   int         i,j,n;

   /* copy polygon to merge into P */
   for (i=0;(poly[i].x != ENDOFPATH) && (i < MAXPATH);++i) P[i] = poly[i];
   if (i >= MAXPATH)
      {
      #  ifndef TEST
         Panic(E_POLY_MERGE);
      #  else
         printf("Poly Merge generated too large of path!");
      #  endif
      }
   P[i].x = ENDOFPATH;
   P[i].y = ENDOFPATH;
   n = i;

   /* merge given polygon with each polygon in the list */
   /* A merged polygon in the list will be marked as
      MTP, MNP, MBP or MCP. The resultant polygon will be
      marked BAP.
   */


   for (i=0;polylist[i].n != 0;++i)
      {
      MergePoly(P,n,&polylist[i]);
      for (n=0;P[n].x != ENDOFPATH;++n);
      }

   /* return the new polygon */
   for (i=0;P[i].x != ENDOFPATH;++i);
   ++i;
#  ifdef TEST
      newPoly = (AvdPoint *) calloc(i, sizeof(AvdPoint));
      if (newPoly == 0) printf("no memory available");
#  else
      newPoly = (AvdPoint *) RNewPtr((i*sizeof(AvdPoint)));
#  endif
   j = 0 ;
   for (i=0;P[i].x != ENDOFPATH;++i) 
      {
         if((P[i].x != P[i+1].x) || (P[i].y != P[i+1].y))
            {
               newPoly[j] = P[i];
               j ++ ;

            }
      }

   newPoly[i].x = ENDOFPATH;
   newPoly[i].y = ENDOFPATH;




   return (newPoly);
   }


static void  near
MergePoly(P,n,poly)
AvdPoint  *P;
polygon  *poly;
int      n;
   {
   int         i,j,k,m; 
   int         AddNode; 
   int         t1,P_i,Q_i,P_j,Q_j; 
   int         angleIn,angleOut,delta,angle; 
   AvdPoint    *Q,P_U,Q_U,Result[MAXPATH];
   AvdPoint    tmp1,tmp2;
   polyPatch   Patch[MAXPATCHES],newPatch;
   int         patches = 0,p;
   

   /* hook */
   /* make sure that merged polygons are marked as mtp,mnp etc. */
   Q = poly->polyPoints;
   m = poly->n;
   for (P_i=0;P_i < n;++P_i)
      {
      for (Q_i=0;Q_i < m;++Q_i)
         {
         t1 = intsegms(&P[P_i],&P[(P_i+1+n) % n],&Q[Q_i],&Q[(Q_i+1+m) % m],&P_U);
         if ((t1 != NOINTERSECT) && (t1 != INTERSECT+INTERSECTD))
            {
            /* Is this an exit from P intersection?
                  For example:
                                *P[i+1]
                               *      
                              *           
                             *           
                   INSIDE   *   OUTSIDE
                           *     
                          *   
            Q[j]---------*---------------->Q[j+1]   This is an exiting
                        *                           intersection
                       *
                      *                     
                     *
                P[i]*

                                *P[i+1]
                               *      
                              *           
                             *           
                  INSIDE    *   OUTSIDE
                           *     
                          *   
            Q[j+1]<------*-----------------Q[j]     This is an entering
                        *                           intersection
                       *
                      *                     
                     *
                P[i]*


            */
            /* test for exit or entry */
            v_subtract(&P[(P_i+1+n) % n],&P[P_i],&tmp1);
            v_subtract(&Q[(Q_i+1+m) % m],&Q[Q_i],&tmp2);
            if (v_cross3rd_comp(&tmp1,&tmp2) < 0)
               {
               /* now test for clock wise or counter clockwise */
               angle = 0;
               angleIn = ATan(Q[Q_i].x,Q[Q_i].y,Q[(Q_i+1+m) % m].x,Q[(Q_i+1+m) % m].y);
               for (Q_j=Q_i+1;Q_j <= Q_i + m;++Q_j)
                  {
                  angleOut = ATan(Q[(Q_j+m) % m].x,Q[(Q_j+m) % m].y,Q[(Q_j+1+m) % m].x,Q[(Q_j+1+m) % m].y);
                  delta = angleOut - angleIn;
                  if (delta > 180) delta -= 360;
                  if (delta < -180) delta += 360;
                  angle += delta;
                  angleIn = angleOut;
                  for (P_j = P_i;P_j <= P_i + n ;++P_j)
                     {
                     t1 = intsegms(&P[(P_j + n) % n],&P[(P_j+1+n) % n],&Q[(Q_j+m) % m],&Q[(Q_j+1+m) % m],&Q_U);
                     if ((t1 != NOINTERSECT) && (t1 != INTERSECT+INTERSECTD))
                        {
                        /* test for exit or entry */
                        v_subtract(&P[(P_j+1+n) % n],&P[(P_j + n) % n ],&tmp1);
                        v_subtract(&Q[(Q_j+1+m) % m],&Q[(Q_j+m) % m],&tmp2);
                        if (v_cross3rd_comp(&tmp1,&tmp2) > 0)
                           {
                           break;
                           }
                        else
                           {
                           t1 = NOINTERSECT;
                           }
                        }
                     else
                        {
                        t1 = NOINTERSECT;
                        }
                     }
                  if (t1 != NOINTERSECT) break;
                  }
               /* we need to find an intersection */
               if (t1 != NOINTERSECT)
               /* if loop to add is screen clockwise add to P patches*/

                  {

               if (angle > 0)
                  {
                  if (patches >= MAXPATCHES)
                     {
                      #  ifndef TEST
                            Panic(E_MAX_PATCHES);
                      # else
                            printf("Too many patches!");
                      #  endif

                     }
                  /* add loop to patches */
                  if(patches == 0)
                     {
                     Patch[patches].P_i   = P_i;
                     Patch[patches].Q_i   = Q_i;
                     Patch[patches].P_U   = P_U;
                     Patch[patches].P_j   = (P_j + n) % n;
                     Patch[patches].Q_j   = (Q_j + m) % m;
                     Patch[patches].Q_U = Q_U;
                     Patch[patches++].delete = FALSE; 
                     }
                  else
                     {
                     newPatch.P_i   = P_i;
                     newPatch.Q_i   = Q_i;
                     newPatch.P_U   = P_U;
                     newPatch.P_j   = (P_j + n) % n;
                     newPatch.Q_j   = (Q_j + m) % m;
                     newPatch.Q_U   = Q_U;
                     for(p = 0;p < patches; p ++)
                        if (dominates(&Patch[p],&newPatch,P)) break;
                     if(p == patches)
                        {
                        Patch[patches].P_i   = P_i;
                        Patch[patches].Q_i   = Q_i;
                        Patch[patches].P_U   = P_U;
                        Patch[patches].P_j   = (P_j + n) % n;
                        Patch[patches].Q_j   = (Q_j + m) % m;
                        Patch[patches].Q_U = Q_U;  
                        Patch[patches++].delete = FALSE; 
                        for(p = 0;p < patches-1 ; p ++)
                           if (dominates(&newPatch, &Patch[p], P)) Patch[p].delete = TRUE;
                        }
                     }

                  }


                  }    
               }
            }
         }
      }
   if (patches)
      {
      /* merge the polygons */

      /* mark as merged polygon */
      poly->type = poly->type + MTP; 
      j = 0;

      for (P_i=0;P_i < n;++P_i)
         {
         /* if P_i is not in a patch add node */
         for (i=0,AddNode=TRUE;(i<patches) && (AddNode == TRUE);++i)
            if(Patch[i].delete == FALSE)
               if (PatchNode(&Patch[i],P_i,P))
                  AddNode = FALSE;
         if (AddNode) Result[j++] = P[P_i];
         /* if at a patch, add the patch */
         for (i=0;i<patches;++i)
            {
            if(Patch[i].delete == FALSE)
               {
               if (P_i == Patch[i].P_i)
                  {
                  if (j >= MAXPATH)
                     {
                        #  ifndef TEST
                              Panic(E_MAX_POINTS);
                        #  else
                              printf("Too many Points!");
                        #  endif
                     }
                  if (!(Patch[i].P_U.x == P[P_i].x && Patch[i].P_U.y == P[P_i].y))
                     Result[j++] = Patch[i].P_U;
                  for 
                     (
                     k=(Patch[i].Q_i+1+m) % m;
                     k != Patch[i].Q_j;
                     k = (k+1+m) % m,++j
                     ) 
                     Result[j] = Q[(k+m) % m];
                  Result[j++] = Q[Patch[i].Q_j];
                  if (!(Patch[i].Q_U.x == Q[Patch[i].Q_j].x && Patch[i].Q_U.y == Q[Patch[i].Q_j].y))                                           
                     Result[j++] = Patch[i].Q_U;
                  }
               }
            }
         }
      for (i=0;i<j;++i) P[i] = Result[i];
      if ((P[i-1].x ==  P[0].x) && (P[i-1].y == P[0].y))
         {
         i -- ;
         }
      P[i].x = ENDOFPATH;
      P[i].y = ENDOFPATH;
      }
   }


static int  near
PatchNode(Patch,theNode,thePoly)
   polyPatch * Patch;
   int         theNode;
   AvdPoint  * thePoly;
   {
   int   offScreen;
   if (Patch->P_i < Patch->P_j)
      if ((Patch->P_i < theNode) && (theNode <= Patch->P_j))
         return (TRUE);
   if (Patch->P_j < Patch->P_i)
      if ((Patch->P_i < theNode) || (theNode <= Patch->P_j))
         return (TRUE);
   if (Patch->P_i == Patch->P_j)
      if (
         distEstimate(&Patch->P_U,&thePoly[Patch->P_i],&offScreen) > 
         distEstimate(&Patch->Q_U,&thePoly[Patch->P_i],&offScreen)
         )
         return (TRUE);
   return (FALSE);
   }


static int  near
dominates(A,B, poly)
   polyPatch *A, *B;
   AvdPoint *poly;
   {

   int Api, Bpi, Apj, Bpj, offScreen= 0;

	Api = A->P_i;
   Apj = A->P_j;
   Bpi = B->P_i;
   Bpj = B->P_j;
   if((A->P_U.x == B->P_U.x) &&
      (A->P_U.y == B->P_U.y) &&
      (A->Q_U.x == B->Q_U.x) &&
      (A->Q_U.y == B->Q_U.y) 
     )
     /* The same patch! */
      return(TRUE);

   if(Api != Apj)
      {
      if(
         ((Api<Apj) && ((Api < Bpi) && (Bpi < Apj))) ||
         ((Api>Apj) && ((Api < Bpi) || (Bpi < Apj))) 
        )
         /*
              patch A
         *-----------------------------*
         |                             |
         |    patch B                  |
         |  *----------...             |
         |  |                          |
         |  |                          |
         |  |       ********           |
         |  |      *        *          |
         |  *-----*          *         |
         |       *            *--------*
         |      *              *
         |     *                *
         |     *                 * 
         *-----*                  *
               *                   *  
               *                  *
               *                 *
               ******************   

         */
         return(TRUE);


      if(
         ((Api<Apj) && ((Api < Bpj) && (Bpj < Apj))) ||
         ((Api>Apj) && ((Api < Bpj) || (Bpj < Apj))) 
        )
         /*
              patch A
         *-----------------------------*
         |                             |
         |    patch B                  |
         |  *----------*               |
         |             |               |
         |             |               |
         |          ********           |
         |         *        *          |
         |        *          *         |
         |       *            *--------*
         |      *              *
         |     *                *
         |     *                 * 
         *-----*                  *
               *                   *  
               *                  *
               *                 *
               ******************   

         */
         return(TRUE);
      }
   if(Bpi != Bpj)
      {
      if(
         ((Bpi<Bpj) && ((Bpi < Api) && (Api < Bpj))) ||
         ((Bpi>Bpj) && ((Bpi < Api) || (Api < Bpj))) 
       )
         /*
              patch B
         *-----------------------------*
         |                             |
         |    patch A                  |
         |  *----------...             |
         |  |                          |
         |  |                          |
         |  |       ********           |
         |  |      *        *          |
         |  *-----*          *         |
         |       *            *--------*
         |      *              *
         |     *                *
         |     *                 * 
         *-----*                  *
               *                   *  
               *                  *
               *                 *
               ******************   

         */
         return(FALSE);
      if(
     
         ((Bpi<Bpj) && ((Bpi < Apj) && (Apj < Bpj))) ||
         ((Bpi>Bpj) && ((Bpi < Apj) || (Apj < Bpj))) 
       )
         /*
              patch B
         *-----------------------------*
         |                             |
         |    patch A                  |
         |  *----------*               |
         |             |               |
         |             |               |
         |          ********           |
         |         *        *          |
         |        *          *         |
         |       *            *--------*
         |      *              *
         |     *                *
         |     *                 * 
         *-----*                  *
               *                   *  
               *                  *
               *                 *
               ******************   

         */
         return(FALSE);
      }
   if(Api != Apj)
      {
      if(Bpi == Bpj)
         {
         if(Api == Bpi)
            if(distEstimate(&poly[A->P_i], &A->P_U, &offScreen) <  distEstimate(&poly[A->P_i], &B->P_U,&offScreen))
               /*
                 patch A
               *----------------*       
               |                |      
               |  ************************
               |  *                      *
               |  *                      *-------*
               |  *                      *       |
               |  *                      *       |
               |  *                      *---*p  |
               |  *                      *   |a  |
               |  *                      *   |t  |
               |  *                      *   |c  |
               |  *                      *   |h  |
               |  *                      *   |B  |
               |  *                      *---*   |
               |  *                      *       |
               |  *                      *       |
               |  *                     *        |
               |  *                    *         |
               |  *********************          |
               |                                 |
               *---------------------------------*  
               */
               return(TRUE);
            else
               /*
                 patch A
               *----------------*       
               |                |      
               |  ************************
               |  *                      *
               |  *                      *              
               |  *                      *     
               |  *                      *     
               |  *                      *---*p
               |  *                      *   |a
               |  *                      *   |t
               |  *                      *   |c
               |  *                      *   |h
               |  *                      *   |B
               |  *                      *---*          
               |  *                      *              
               |  *                      *-------*
               |  *                      *       |
               |  *                      *       |
               |  *                     *        |
               |  *                    *         |
               |  *********************          |
               |                                 |
               *---------------------------------*  
               */
               return(FALSE);
         if(Apj == Bpi)
            if(distEstimate(&poly[A->P_j], &A->Q_U, &offScreen) >  distEstimate(&poly[A->P_j], &B->P_U,&offScreen))
               /*
                 patch A
               *----------------*       
               |                |      
               |    patch B     |      
               |    *-----*     |      
               |    |     |     |      
               |    |     |     |      
               |  ************************
               |  *                      *
               |  *                      *
               |  *                      *
               |  *                      *---*
               |  *                      *   |
               |  *                      *   |
               |  *                      *   |
               |  *                      *   |
               |  *                     *    |
               |  *                    *     |
               |  *********************      |
               |                             |
               *-----------------------------*  
               */
               return(TRUE);
            else
               /*
                 patch A
               *---------*       
               |         |      
               |         |   patch B   
               |         |   *-----*   
               |         |   |     |   
               |         |   |     |   
               |  ************************
               |  *                      *
               |  *                      *
               |  *                      *
               |  *                      *---*
               |  *                      *   |
               |  *                      *   |
               |  *                      *   |
               |  *                      *   |
               |  *                     *    |
               |  *                    *     |
               |  *********************      |
               |                             |
               *-----------------------------*  
               */
               return(FALSE);
         }
      if(Api != Bpi) 
         /*
                                        
              patch A                            patch B                   
         *-------------*                    *-------------*                
         |             |                    |             |                
         |             |                    |             |                
         |          ********                |          ********            
         |         *        *               |         *        *           
         |        *          *              |        *          *          
         |       *            *--------*    |       *            *--------*
         |      *              *       |p   |      *              *       |p
         |     *                *      |a   |     *                *      |a
         |     *                 *     |t   |     *                 *     |t
         *-----*                  *    |c   *-----*                  *    |c
               *                   *   |h         *                   *   |h
               *                  *    |B         *                  *    |A
               *                 *-----*          *                 *-----*
               ******************                 ******************   

         */
         return(FALSE);
      if(distEstimate(&poly[A->P_i], &A->P_U, &offScreen) <  distEstimate(&poly[A->P_i], &B->P_U,&offScreen))
         /*
              patch A
         *-----------------------------*
         |                             |
         |    patch B                  |
         |  *----------*               |
         |  |          |               |
         |  |          |               |
         |  |       ********           |
         |  |      *        *          |
         |  |     *          *         |
         |  |    *            *--------*
         |  |   *              *
         |  |  *                *
         |  *--*                 * 
         *-----*                  *
               *                   *  
               *                  *
               *                 *
               ******************   

         */
         return(TRUE);
      else
         /*
              patch B
         *-----------------------------*
         |                             |
         |    patch A                  |
         |  *----------*               |
         |  |          |               |
         |  |          |               |
         |  |       ********           |
         |  |      *        *          |
         |  |     *          *         |
         |  |    *            *--------*
         |  |   *              *
         |  |  *                *
         |  *--*                 * 
         *-----*                  *
               *                   *  
               *                  *
               *                 *
               ******************   

         */
         return(FALSE);
      }
   if((Api == Apj) && (Api != Bpi))
      {
      /* if A wraps around it dominates B. */
      if(distEstimate(&poly[A->P_i], &A->P_U, &offScreen) >  distEstimate(&poly[A->P_i], &A->Q_U,&offScreen))
         /*
              patch A
         *--------------------------------*
         |                                |
         |          ********              |
         |         *        *             |
         |        *          *--------*   |
         |       *            *       |p  |
         |      *              *      |a  |
         |     *                *     |t  |
         |     *                 *    |c  |
         *-----*                  *   |h  |
               *                   *  |B  |
         *-----*                  *---*   |
         |     *                 *        |
         |     ******************         |
         |                                |
         *--------------------------------*

         */
         return(TRUE);
      }

   /* if either patches wrap around then their can not be a dominance. */
   if(distEstimate(&poly[A->P_i], &A->P_U, &offScreen) >  distEstimate(&poly[A->P_i], &A->Q_U,&offScreen))
      /*
           patch A
      *-----------------------------*
      |                             |
      |          ********           |
      |         *        *          |
      |        *          *         |
      |       *            *        |
      |      *              *       |
      |     *                *      |
      |     *                 *     |
      *-----*                  *    |
            *                   *   |
      *-----*                  *    |
      |     *                 *     |
      |     ******************      |
      |                             |
      *-----------------------------*

      */
      return(FALSE);
   if(distEstimate(&poly[B->P_i], &B->P_U, &offScreen) >  distEstimate(&poly[B->P_i], &B->Q_U,&offScreen))
      /*
           patch B
      *-----------------------------*
      |                             |
      |          ********           |
      |         *        *          |
      |        *          *         |
      |       *            *        |
      |      *              *       |
      |     *                *      |
      |     *                 *     |
      *-----*                  *    |
            *                   *   |
      *-----*                  *    |
      |     *                 *     |
      |     ******************      |
      |                             |
      *-----------------------------*

      */
      return(FALSE);

   if(distEstimate(&poly[B->P_i], &B->P_U, &offScreen) >  distEstimate(&poly[B->P_i], &A->P_U,&offScreen))
      if(distEstimate(&poly[B->P_i], &A->Q_U, &offScreen) >  distEstimate(&poly[B->P_i], &B->P_U,&offScreen))
         /*
                                         
                        *************         
                        *            *        
           *------------*             *       
         p |            *              *      
         a |            *               *     
         t |            *                *    
         c |   patchB   *                 *   
         h |  *---------*                  *  
         A |  |         *                   * 
           |  *---------*                  *  
           *------------*                 *   
                        ******************    
                                          

         */
         return(TRUE);
   /*
                                                                
                  ********                          ********         
                  *       *                         *       *        
     *------------*        *           *------------*        *       
   p |            *         *          |            *         *      
   a |            *          *         |            *          *     
   t |            *           *        *------------*           *    
   c |   patchB   *            *                    *            *   
   h |  *---------*             *      *------------*             *  
   A |  |         *              *     |            *              * 
     |  *---------*             *      |            *             *  
     *------------*            *       *------------*            *   
                  *************                     *************    
                                                                 

   */
   return(FALSE);
   }


int
ptIntr(M,P,n)
AvdPoint *M,*P;
int   n;
   {
   AvdPoint    N3,P1,P2,P3,P4;
   AvdPoint    tmp1,tmp2;
   int      nextnode,intr;
   intr = 0;
   P1 = P[0];
   P2 = P[1];
   nextnode = 3;
   P3 = N3 = P[2];
   if (n>3)
      P4 = P[3];
   else
      P4 = P[0];
start:
   /* If M is interior to some line segment of
      the polygon consider it as interior. */
   if (
         (
            (
            (P1.y<=M->y)       &&
            (M->y<=P2.y) 
            )
         ||
            (
            (P2.y<=M->y)       &&
            (M->y<=P1.y)
            )
         )
      &&
         (
            (
            (P1.x<=M->x)       &&
            (M->x<=P2.x) 
            )
         ||
            (
            (P2.x<=M->x)       &&
            (M->x<=P1.x)
            )
         )
      )

      {
      v_subtract(M,&P1,&tmp1);
      v_subtract(M,&P2,&tmp2);
      if (v_cross3rd_comp(&tmp1,&tmp2) == 0) return(TRUE);
      }

   /* The alogorithm counts the number of intersections with the
      given polygon going from M in the negative x direction.
      If this number is odd M must be interior to the polygon. */

   /* if M is above or below P1-P2 then there can't be 
      an intersection of the ray from M in the negative x
      direction with the line segment P1-P2. */
   if (
         (
         (P1.y<M->y)       &&
         (M->y<P2.y) 
         )
      ||
         (
         (P2.y<M->y)       &&
         (M->y<P1.y)
         )
      )

      {
      /* If P1-P2 or P2-P1 is chosen so that the y delta is 
         positive and this vector is crossed with the vector
         P1-M then the z component is positive if M lies on
         the negative side of P1-P2 and negative if M lies
         on the positive side of P1-P2. */
      v_subtract(&P2,&P1,&tmp1);
      if (tmp1.y < 0 )
         {
         tmp1.x = -tmp1.x;
         tmp1.y = -tmp1.y;
         }
      v_subtract(M,&P1,&tmp2); 
      if (v_cross3rd_comp(&tmp1,&tmp2) > 0) ++intr;
      }
   else
      {
      /* If the ray in the negative x direction from M passes
         through the point P2 then one must look at the next
         segment of the polygon which is not parallel with the
         x axis in order to determine if the is an intersection
         at P2. Note that consecutive line segment are not allowed
         to be parallel in a given polygon. */

      if (
         (P2.y == M->y) &&
         (M->x < P2.x)
         )
         {
         /* types of intersections checked here:

            *           *       *    *             *            *
             *           *     *      *             *          *
              *           *   *        *             *        *
               *           * *          *             *      *
      <---------*-----------*------------*****---------******-------------M
                *                             *
                *                              *
                *                               *           
                *                                *
                *                                 *

intersection?  yes          no             yes           no          */



         if (P3.y != P2.y)
            {
            if ((long)(P2.y-P1.y)*(long)(P3.y-P2.y) > 0L) ++intr;
            }
         else
            {
            if ((long)(P2.y-P1.y)*(long)(P4.y-P3.y) > 0L) ++intr;
            }
         }
      }
   P1 = P2;
   P2 = P3;
   P3 = P4;
   /* See if there are any more line segments in 
      the polygon to be tested. */
   if ((P3.x == N3.x)  && (P3.y == N3.y))
      {
      if (intr & 1)
         return(TRUE);
      else
         return(FALSE);
      }
   else
      P4 = P[(++ nextnode) % n];
   goto start;
   }


/* POLYPATH.C   PATH AROUND AN OBSTACLE PLOYGON
**
** Algorithm by Larry Scott
**
** NAME---POLYPATH---
**    POLYGON must be closed and not self intersecting (JORDAN ploygon) 
** input:
**   line segment A-B & POLYGON P1,P2,...PN
** output:
**   return       0 polygon not an obstacle, 1 path of nodes returned  
**   nodeI1       first node 
**   nodeI2       last node
**   direction    1 increasing order, -1 decreasing order
**   I1           intersecting point closest to A
**   I2           intersecting point closest to B
**
**
**               EXAMPLE:
**                    POLYPATH(A,B,path) returns (1,N4,N2,-1,x1,y1,x2,y2)
**
**
**
**                 N0     N1
**                 ********
**                *        *
**               *          *
**              *            *
**             *              *
**         N5 *                *
**            *(x1,y1)=I1       * (x2,y2)=I2
**  A---------*------------------*---------------B
**            *                   *N2
**            *                  *
**            *                 *
**         N4 ****************** N3
**
*/

static int near polypath(A, B, P, n, I1, I2, nodeI1, nodeI2)
AvdPoint *A, *B, *P, *I1, *I2;
int n;
int * nodeI1;
int * nodeI2;

{
   int   distA, distB;
   int   offScreenA=FALSE,offScreenB=FALSE;

   if (intpoly(A,B,P,n,I1,I2,nodeI1,nodeI2) == 0) return(0);
   if (*nodeI1 == *nodeI2) return(0);
   /*  nodeI1 = P[0]; first node */
   /*  nodeI2 = P[n - i]; last node */
   distA = distsq(1, *nodeI1 + 1, *nodeI2, P, n, I1, I2,&offScreenA); 
   distB = distsq(-1, *nodeI1, *nodeI2 + 1, P, n, I1, I2,&offScreenB);
   if (offScreenA && (!offScreenB))
      {
      *nodeI2 = (*nodeI2+1) % n;
      return(-1);
      }
   if ((!offScreenA) && offScreenB)
      {
      *nodeI1 = (*nodeI1+1) % n;
      return(1);
      }
   if (distA < distB)
      {
      *nodeI1 = (*nodeI1+1) % n;
      return(1);
      }
   else
      {
      *nodeI2 = (*nodeI2+1) % n;
      return(-1);
      }
}

static int near distsq(d, nodeF, nodeL, P, n, I1, I2, offScreen)
/* 
   d = direction
   nodeF = first node
   nodeL = last node
   P = polygon
   offScreen = pointer to offscreen TRUE or FALSE
*/
int d, nodeF, nodeL, n, *offScreen;
AvdPoint *P;
AvdPoint *I1;
AvdPoint *I2;
   {
   int      distance = 0;
   int      nodeM;
   AvdPoint P1, P2;
   nodeM = nodeF = nodeF % n;
   nodeL = nodeL % n;
   P1 = P[nodeM];
   for (;nodeM != nodeL;)
      {
      /* adding n to mod because % will return a negative */
      nodeM = (nodeM + d + n) % n;
      P2 = P[nodeM];
   /* distEstimate is more accurate than the sum 
      of the squares of the distances for deciding 
      which way around a polygon is shorter.
   */
      distance += distEstimate(&P2,&P1,offScreen);
      P1 = P2;
      }
   if (d>0)
      {
      distance += distEstimate(I1,&P[nodeF],offScreen);
      distance += distEstimate(&P[nodeL],I2,offScreen);
      }
   else
      {
      distance += distEstimate(I1,&P[nodeF],offScreen);
      distance += distEstimate(&P[nodeL],I2,offScreen);
      }
   return(distance);
   }

static int near distEstimate(P2,P1,offScreen)
AvdPoint *P1,*P2;
int      *offScreen;
   {
   int   deltaX,deltaY,temp;
   /* if line lies on screen edge offScreen set to TRUE */
   if (lineOnScreenEdge(P1,P2))  *offScreen = TRUE;
   /* estimate distance by the formula:
      If max(deltaX,deltaY) <= (10*min(deltaX,deltaY))/6)
         then use (13*max(deltaX,deltaY))/10
         else use max(deltaX,deltaY)
      This formula guarantees the max error is less than
      11% and the average error is about 5%.
   */
   deltaX = abs(P2->x - P1->x);
   deltaY = abs(P2->y - P1->y);
   if (deltaX > deltaY)
      {
      temp = deltaX;
      deltaX = deltaY;
      deltaY = temp;
      }
   if (((deltaX*10)/6) >= deltaY)
      return(((13*deltaY)/10));
   else
      return(deltaY);
   }


static int near lineOnScreenEdge(p1,p2)
AvdPoint *p1;
AvdPoint *p2;
   {  
   if 
      (
         (p1->x == p2->x)              &&
         (p1->y != p2->y)              &&
         (
            (p1->x == 0)               ||
            (p1->x == picWindPoly[1].x) 
         )
      )

      return(1);

   if 
      (
         (p1->y == p2->y)              &&
         (p1->x != p2->x)              &&
         (
            (p1->y == 0)               ||
            (p1->y == picWindPoly[2].y) 
         )
      )

      return(1);

   return(0);
   }

/* INTPOLY.C  INTERSECT LINE SEGMENT AND A POLYGON
**
** Algorithm by Larry Scott
**
** NAME---INTPOLY---
**      POLYGON must be closed and not self intersecting (JORDAN polygon) 
** input:
**      line segment A-B & POLYGON P1,P2,...PN
** output: 
**      return    number ofintersections
**      I1        intersection closest to A
**      I2        intersection closest to B
**      nodeI1    start node of line intersecting at I1
**      nodeI2    start node of line intersecting at I2
*/


static int near intpoly(A,B,P,n,INT1,INT2,nodeI1,nodeI2)
AvdPoint *A,*B,*P,*INT1,*INT2;
int * nodeI1;
int * nodeI2;
int n;

   {
   long     distA = MAXVALUELONG, distB = MAXVALUELONG,d;
   AvdPoint N1,N2,P1,P2,P3,V,W,F1,tmp1,tmp2,tmp3,tmp4,I1,I2;
   int i, node, nextnode, f1, t1, t2;                             
   N1 = P[0];                                            
   P1 = N1;                                              
   N2 = P[1];
   P2 = P[1];
   P3 = P[2];
   i = 0; node = 0; /* first three nodes already taken */
   f1 = intsegms(A,B,&P1,&P2,&V);
   t1 = f1;
   F1 = V;
   nextnode = 2;

start:
   t2 = intsegms(A,B,&P2,&P3,&W);
   if ((t1 == NOINTERSECT) || (t1 == INTERSECT+INTERSECTC))
      {
getnextnode:
      t1 = t2;
      P1 = P2;
      P2 = P3;
      V  = W;
      ++ node;
      if ((n-1) <= nextnode)
         if (P3.x == N2.x && P3.y == N2.y)
            {
            *INT1 = I1;
            *INT2 = I2;
            return(i);
            }
         else  
            if (P3.x == N1.x && P3.y == N1.y)
               {
               P3 = N2;
               t2 = f1;
               W = F1;
               }
            else 
               {
               P3 = N1;
               }
      else
         P3 = P[++ nextnode];
      goto start;
      }
   else
      {
      if (t1 == COLINEAR)
         {
         /* If P1-P2 is colinear with A-B and A-B contains P2, then
            one of the following cases must apply. This is because 
            all polygon must be defined in a mathematical counter
            clockwise direction.
            For example:
            
               P1      P2                           There is an 
         A-----*********----------------------B     intersection at P2
                        *
                         *
               OUTSIDE    *     INSIDE
                           *
                            *
                             *                  
                             P3

               

                             *
                            *
               INSIDE      *
                          *   OUTSIDE
                         *
               P1     P2*                           There is no 
         A-----*********----------------------B     intersection at P2
                         
                          
                                
                            

             
         IF A=P2 OR B=P2


               P1      P2,A                         There is an 
               *********----------------------B     intersection at P2
                        *
                         *
               OUTSIDE    *     INSIDE
                           *
                            *
                             *             
                             P3

               
               P1      P2,A                         There is no 
         B-----*********                            intersection at P2
                        *
                         *
               OUTSIDE    *     INSIDE
                           *
                            *
                             *                    
                             P3



               P1      P2,B                         There is an 
               *********----------------------A     intersection at P2
                        *
                         *
               OUTSIDE    *     INSIDE
                           *
                            *
                             *                            
                             P3

               
               
                             P3
                             *
                            *
               INSIDE      *   OUTSIDE
                          *
                         *
               P1   P2,B*                           There is no 
         A-----*********                            intersection at P2
                         
         */
         /* See if A-B contains P2. */
         v_subtract(&P2,A,&tmp1);
         v_subtract(&P2,B,&tmp2);
         if (v_dot(&tmp1,&tmp1) <= 2L)
            /* A = P2 */
            /* If (P2-P1)#(B-A) > 0 then intersection at A
               otherwise not an intersection */
            {
            v_subtract(&P2,&P1,&tmp3);
            v_subtract(B,A,&tmp4);
            if (v_dot(&tmp3,&tmp4) > 0L)
               {
               V = P2;
               goto intersection;
               }
            else
               {
               goto getnextnode;
               }
            }
         if (v_dot(&tmp2,&tmp2) <= 2L)
            /* B = P2 */
            /* If (P2-P1)#(B-A) < 0 then intersection at B
               otherwise not an intersection */
            {
            v_subtract(&P2,&P1,&tmp3);
            v_subtract(B,A,&tmp4);
            if (v_dot(&tmp3,&tmp4) < 0L)
               {
               V = P2;
               goto intersection;
               }
            else
               {
               goto getnextnode;
               }
            }
         /* If (P2-A)#(P2-B) < 0 then A-B contains P2. */
         if (v_dot(&tmp1,&tmp2) < 0L)
            {   
            /* If (P2-P1)^(P3-P2) < 0 then P2 is an intersection. */
            v_subtract(&P2,&P1,&tmp1);
            v_subtract(&P3,&P1,&tmp2);
            if (v_cross3rd_comp(&tmp1,&tmp2) < 0)
               {
               V = P2;
               goto intersection;
               }
            }
         goto getnextnode;
         }

      if (t1 == INTERSECT+INTERSECTD)
         {
         if (t2 == COLINEAR)
            {
            /* If intersection at P2, A == P2 and P2-P3 is colinear,
               Then if (P3-P2)#(B-A) < 0 and (P2-P1)^(P3-P2) < 0 then 
               P2 is an intersection.

               If intersection at P2, A != P2 and P2-P3 is colinear,
               then if (P2-P1)^(P3-P2) < 0 then P2 is an intersection.

               Otherwise D is not an intersection.
               This is because all polygons must be defined in a counter
               clockwise direction.
               For example:


                          A,P2     P3             There is an no
            B---------------*******               intersection at P2
                           *                      
                          *             
                OUTSIDE  *   INSIDE    
                        *               
                     P1*           


                     P1*
                        *   OUTSIDE
                         *
                 INSIDE   *
                           *P2     P3             There is an 
            B---------------*******               intersection at P2
                            A                     

                P1*
                   *
                    *   OUTSIDE
              INSIDE *
                      *P2     P3                  There is no
                       *******-------------B      intersection at P2
                       A                          


                     A,P2     P3                  There is no
                       *******-------------B      intersection at P2
                      *                           
                     *
            OUTSIDE *   INSIDE
                   *
                P1*



                P1*
                   *    OUTSIDE
                    *
             INSIDE  *
                      *P2    P3                   There is an
            A----------*******-------------B      intersection at P2
                                                  

                      P2     P3
            A----------*******-------------B      There is no
                      *                           intersection at P2
                     *
            OUTSIDE *   INSIDE
                   *
                P1*

                  *
                   *
                    *  OUTSIDE
              INSIDE *
                      *P2    P3                   There is an
            B----------*******-------------B      intersection at P2
                                                  


                      P2     P3
            B----------*******-------------B      There is no
                      *                           intersection at P2
                     *
            OUTSIDE *   INSIDE
                   *
                P1*

            */
            /* See if P2 == A */
            v_subtract(&P2,A,&tmp1);
            d = v_dot(&tmp1,&tmp1);
            v_subtract(&P2,&P1,&tmp1);
            v_subtract(&P3,&P2,&tmp2);
            if (d <= CLOSETOSQR)
               {
               /* if (P3-P2)#(B-A) > 0 no intersection */
               v_subtract(B,A,&tmp3);
               if (v_dot(&tmp2,&tmp3) > 0L) goto getnextnode;
               }
            /* If (P2-P1)^(P3-P2) < 0 then P2 is an intersection. */
            if (v_cross3rd_comp(&tmp1,&tmp2) < 0)
               goto intersection;
            else
               goto getnextnode;
            }



         /* If intersection at P2 then check crossing node or not.
            For example:
                                  *P3
                               *    
                            *   
                         *  
                   P2 *     
         A----------*--------------------      Not crossing node
                   *                           
                  *
                 *                     
                *
             P1*

                       P2       
             A----------*--------------------  Crossing node    
                       * *                         
                      *   *            
                     *     *     
                    *       *
                 P1*         * P3
         */
         v_subtract(B,A,&tmp1);
         v_subtract(&P3,A,&tmp2);
         v_subtract(&P1,A,&tmp3);
         if ((v_cross3rd_comp(&tmp1,&tmp2) * v_cross3rd_comp(&tmp1,&tmp3)) > 0)
            {
            /* not crossing node */
            v_subtract(&P2,A,&tmp4);
            if (v_dot(&tmp4,&tmp4) <= 2L)
               {
               /* P2 = A */
               /*
                          P2       
                          A*--------------------      There is an
                          * *                         intersection at P2
                         *   *      INSIDE
                        *     *     
                       *       *
                    P1*         * P3

                          P2       
                          A*--------------------      There is no
                          * *                         intersection at P2
                         *   *     OUTSIDE
                        *     *     
                       *       *
                    P3*         * P1
               */
               if (nodeTest(&tmp1,&P1,&P2,&P3))
                  {
                  goto intersection;
                  }
               else
                  {
                  goto getnextnode;
                  }
               }
            v_subtract(&P2,B,&tmp4);
            if (v_dot(&tmp4,&tmp4) <= 2L)
               {
               /* P2 = B */
               /*
                          P2       
                 A--------B*                          There is an
                          * *                         intersection at P2
                         *   *      INSIDE
                        *     *     
                       *       *
                    P1*         * P3

                          P2       
                A---------B*                          There is no
                          * *                         intersection at P2
                         *   *     OUTSIDE
                        *     *     
                       *       *
                    P3*         * P1

               */
               v_subtract(A,B,&tmp1);
               if (nodeTest(&tmp1,&P1,&P2,&P3))
                  {
                  goto intersection;
                  }
               else
                  {
                  goto getnextnode;
                  }
               }
            /*
                       P2       
             A----------*--------------------      There is an
                       * *                         intersection at P2
                      *   *      INSIDE
                     *     *     
                    *       *
                 P1*         * P3

                       P2       
             A----------*--------------------      There is no
                       * *                         intersection at P2
                      *   *     OUTSIDE
                     *     *     
                    *       *
                 P3*         * P1
            */
            v_subtract(A,&P2,&tmp1);
            if (nodeTest(&tmp1,&P1,&P2,&P3))
               {
               goto intersection;
               }
            else
               {
               goto getnextnode;
               }
            }
         else
            {
            /* Crossing node using known inside determine if there
               is an intersection.
                                      *P3
                                   *    
                INSIDE          *   
                             *  
                       P2 *     
             A----------*--------------------      There is an
                       *                           intersection at P2
                      *
                     *                     
                    *
                 P1*

                       P3*        
                        *   
                       *     OUTSIDE
            INSIDE    *                           There is no       
                   P2*A--------------B            intersection at P2
                     *  
                     *                           
                     *
                   P1*                

                             P3*        
                              *   
                             *     OUTSIDE
                  INSIDE    *                     There is an       
              B----------P2*A                     intersection at P2
                           *  
                           *                
                           *
                         P1*               

                       P3*        
                        *   
                       *     OUTSIDE
            INSIDE    *                           There is no       
                   P2*B--------------A            intersection at P2
                     *  
                     *                           
                     *
                   P1*                
                             P3*        
                              *   
                             *     OUTSIDE
                  INSIDE    *                     There is an       
              A----------P2*B                     intersection at P2
                           *  
                           *                
                           *
                         P1*               

            */

            v_subtract(&P2,A,&tmp4);
            if (v_dot(&tmp4,&tmp4) <= 2L)
               {
               /* P2 = A */
               /* If (P2-P1)^(B-A) > 0 then there is an intersection at P2. */
               v_subtract(&P2,&P1,&tmp2);
               if (v_cross3rd_comp(&tmp2,&tmp1) > 0)
                  {
                  goto intersection;
                  }
               else
                  {
                  goto getnextnode;
                  }
               }
            v_subtract(&P2,B,&tmp4);
            if (v_dot(&tmp4,&tmp4) <= 2L)
               {
               /* P2 = B */
               /* If (P2-P1)^(B-A) < 0 then there is an intersection at P2. */
               v_subtract(&P2,&P1,&tmp2);
               if (v_cross3rd_comp(&tmp2,&tmp1) < 0)
                  {
                  goto intersection;
                  }
               else
                  {
                  goto getnextnode;
                  }
               }
            }
         }


      /* If intersection is at A then see if leaving or
         entering polygon.
         For example:
                        *P2
                       *      
                      *           
                     *           
          INSIDE    *   OUTSIDE
                   *     
                  *   
                 *A-------------------      There is no
                *                           intersection at P2
               *
              *                     
             *
          P1*         *P2
                     *
                    *
          INSIDE   *    OUTSIDE
                  *      
      -----------*A                         There is an
                *                           intersection at P2
               *    
              *           
             *        
          P1*              

      */
      if (t1 == INTERSECT+INTERSECTA)
         {
         v_subtract(&P2,&P1,&tmp1);
         v_subtract(B,A,&tmp2);
         if (v_cross3rd_comp(&tmp1,&tmp2)<=0) goto getnextnode;
         }


      /* If intersection is at B then see if leaving or
         entering polygon.
         For example:
                        *P2
                       *      
                      *           
                     *           
          INSIDE    *   OUTSIDE
                   *     
                  *   
                 *B-------------------      There is no
                *                           intersection at P2
               *
              *                     
             *
          P1*         *P2
                     *
                    *
          INSIDE   *    OUTSIDE
                  *      
      -----------*B                         There is an
                *                           intersection at P2
               *    
              *           
             *        
          P1*              

      */
      if (t1 == INTERSECT+INTERSECTB)
         {
         v_subtract(&P2,&P1,&tmp1);
         v_subtract(B,A,&tmp2);
         if (v_cross3rd_comp(&tmp1,&tmp2)>0) goto getnextnode;
         }



intersection:
      /* intersection, see if closer to A or B than other intersections */
      v_subtract(A,&V,&tmp1);
      if (v_sizesqrd(A,&V) < distA)
         {
         distA = v_sizesqrd(A,&V);
         I1 = V;
         *nodeI1 = node ;
         }
      if (v_sizesqrd(B,&V) < distB)
         {
         distB = v_sizesqrd(B,&V);
         I2 = V;
         *nodeI2 = node ;
         }
      /* count intersections */
      ++ i;
      goto getnextnode;
      }
   }

      
static int near nodeTest(tmp1,P1,P2,P3)
AvdPoint *P1,*P2,*P3,*tmp1;
   {
   AvdPoint tmp2,tmp3;

   /*

            P2              tmp1
             *------------------->      There is no
            * *                         intersection at P2
           *   *     OUTSIDE
          *     *     
         *       *
      P3*         * P1


    tmp1    P2       
   <---------*                          There is an
            * *                         intersection at P2
           *   *      INSIDE
          *     *     
         *       *
      P1*         * P3

   */       
   v_subtract(P3,P2,&tmp2);
   v_subtract(P2,P1,&tmp3);
   if (v_cross3rd_comp(&tmp2,&tmp3) > 0)
      {
      if (v_cross3rd_comp(&tmp2,tmp1) > 0)
         {
         return (TRUE);
         }
      else
         {
         if (v_cross3rd_comp(tmp1,&tmp3) < 0)
            {
            return (TRUE);
            }
         else
            {
            return (FALSE);
            }
         }
      }
   else
      {
      if (v_cross3rd_comp(&tmp2,tmp1) > 0)
         {
         if (v_cross3rd_comp(tmp1,&tmp3) < 0)
            {
            return (TRUE);
            }
         else
            {
            return (FALSE);
            }
         }
      else
         {
         return (FALSE);
         }
      }
   }


/* INTSEGMS.C INTERSECT TWO LINE SEGMENTS
**
** Algorithm by Larry Scott
**
** NAME---INTSEGMS---
**      POLYGON must be closed and not self intersecting (JORDAN polygon) 
** input: 2 line segments A-B & C-D
**        address of where to place intersection point
**        It's assumed that A-B is the motion and C-D
**        is part of a polygon.
** output: return    NOINTERSECT               no intersection
**                   COLINEAR                  segments are colinear
**                   INTERSECT + INTERSECTA    intersection exactly at the point A
**                   INTERSECT + INTERSECTB    intersection exactly at the point B
**                   INTERSECT + INTERSECTC    intersection exactly at the point C
**                   INTERSECT + INTERSECTD    intersection exactly at the point D
**                   INTERSECT + INTERSECTI    intersection internal to segments
**                   interpt           intersection point
*/ 

static int near intsegms(A, B, C, D, interpt)
AvdPoint *A, *B, *C, *D, *interpt;
{
   int      x1,x2;
   long     dot1,dot2,dot3,d;
   AvdPoint U,V,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7;

   v_subtract(B, A, &tmp1);
   v_subtract(C, A, &tmp2);
   v_subtract(D, A, &tmp3);

   x1 = v_cross3rd_comp(&tmp1, &tmp2);
   x2 = v_cross3rd_comp(&tmp1, &tmp3);

   /* check for colinear */
   if (x1 == 0 && x2 == 0)
      /* COLINEAR LINE SEGMENTS */
      {
      return(COLINEAR);
      }

   /* check for intersection at C */
   if (x1 == 0)
      /* C lies on the line through A-B find out if it
         is in the line segment A-B. */
      if (A->x == B->x)
         {
         if (
               (
               (A->y <= C->y)    &&
               (C->y <= B->y)
               )
               ||
               (
               (B->y <= C->y)    &&
               (C->y <= A->y)
               )
            )
            {
            *interpt = *C;
            return(INTERSECT+INTERSECTC);
            }
         }
      else
         {
         if (
               (
               (A->x <= C->x)    &&
               (C->x <= B->x)
               )
               ||
               (
               (B->x <= C->x)    &&
               (C->x <= A->x)
               )
            )
            {
            *interpt = *C;
            return(INTERSECT+INTERSECTC);
            }
         }

   /* check for intersection at D */
   if (x2 == 0)
      /* D lies on the line through A-B find out if it
         is in the line segment A-B. */
      if (A->x == B->x)
         {
         if (
               (
               (A->y <= D->y)    &&
               (D->y <= B->y)
               )
               ||
               (
               (B->y <= D->y)    &&
               (D->y <= A->y)
               )
            )
            {
            *interpt = *D;
            return(INTERSECT+INTERSECTD);
            }
         }
      else
         {
         if (
               (
               (A->x <= D->x)    &&
               (D->x <= B->x)
               )
               ||
               (
               (B->x <= D->x)    &&
               (D->x <= A->x)
               )
            )
            {
            *interpt =  *D; 
            return(INTERSECT+INTERSECTD);
            }
         }

   /* check for intersection at A */
   v_subtract(C,D,&tmp4);
   v_subtract(B,C,&tmp5);
   v_subtract(B,D,&tmp6);
   dot3 = v_dot(&tmp4,&tmp4);
   if (dot3==0L)
   #  ifndef TEST
      Panic(E_AVOID);
   #  else
      printf("Poly Avoider internal error number 1!");
   #  endif    
   if ( ((d = v_dot(&tmp4,&tmp2)) >= 0) && (v_dot(&tmp4,&tmp3) <= 0) )
      {
      tmp7.x = (int) ((d*(long)tmp4.x + (long) (sign(tmp4.x)*dot3/2))/dot3);
      tmp7.y = (int) ((d*(long)tmp4.y + (long) (sign(tmp4.y)*dot3/2))/dot3);
      v_subtract(&tmp7,&tmp2,&tmp7);
      d = v_dot(&tmp7,&tmp7);
      }
   else
      {
      if ((dot1 = v_dot(&tmp2,&tmp2)) <= (dot2 = v_dot(&tmp3,&tmp3)))
         d = dot1; 
      else
         d = dot2; 
      }
   if (d <= CLOSETOSQR)
      {
      *interpt =  *A; 
      return(INTERSECT+INTERSECTA);
      }

   /* check for intersection at B */
   if ( ((d = v_dot(&tmp4,&tmp6)) >= 0) && (v_dot(&tmp4,&tmp5) <= 0) )
      {
      tmp7.x = (int) ((d*(long)tmp4.x + (long) (sign(tmp4.x)*dot3/2))/dot3);
      tmp7.y = (int) ((d*(long)tmp4.y + (long) (sign(tmp4.y)*dot3/2))/dot3);
      v_subtract(&tmp7,&tmp6,&tmp7);
      d = v_dot(&tmp7,&tmp7);
      }
   else
      {
      if ((dot1 = v_dot(&tmp5,&tmp5)) <= (dot2 = v_dot(&tmp6,&tmp6)))
         d = dot1; 
      else
         d = dot2; 
      }
   if (d <= CLOSETOSQR) 
      {
      *interpt =  *B; 
      return(INTERSECT+INTERSECTB);
      }

   /* check that C,D on opposite side of line through A-B  */
   if (x1*x2 >= 0)
      {
      return(NOINTERSECT);
      }

   /* check that A,B on opposite side of line through C-D  */
   x1 = v_cross3rd_comp(&tmp2, &tmp4);
   x2 = v_cross3rd_comp(&tmp4, &tmp5);
   if (x1*x2 >= 0)
      {
      return(NOINTERSECT);
      }

   /* intersection interior to both line segments
      calculate intersection point
   */

   /* U is normal to C-D pointing toward the outside of the polygon */
   U.x = -tmp4.y;
   U.y = tmp4.x;
   /* U dot (A-C) */
   dot1 = v_dot(&U, &tmp2);
   /* U dot (A-B) */
   dot2 = v_dot(&U, &tmp1);
   dot3 = dot2;
   if (dot1 < 0) 
      dot1 = - dot1;
   if (dot2 < 0) 
      dot2 = - dot2;

   if (dot3>0)
      {
      /* round toward outside of polygon */
      tmp6.x = (int) (((long) tmp1.x*dot1 + (long) sign(tmp1.x)*(dot2-1))/dot2);
      tmp6.y = (int) (((long) tmp1.y*dot1 + (long) sign(tmp1.y)*(dot2-1))/dot2);
      v_add(A,  &tmp6 ,&V);   
      }
   if (dot3<0)
      {
      /* truncate so that intersection is outside polygon */
      tmp6.x = (int) ((((long) tmp1.x)*dot1)/dot2) ;
      tmp6.y = (int) ((((long) tmp1.y)*dot1)/dot2) ;
      v_add(A,  &tmp6 ,&V);   
      }
   if (dot3==0)
      {
      V = *C;
      }  
   *interpt = V;
   return(INTERSECT+INTERSECTI);
}

      
/* vectops.c vector arithmatic operations */


/* vector subtraction for 2 components */
static void near v_subtract(X, Y, Res)
AvdPoint *X, *Y, *Res;
{
   Res->x = X->x - Y->x;   
   Res->y = X->y - Y->y;   
}

/* vector addition for 2 components */

static void near v_add(X, Y, Res)
AvdPoint *X, *Y, *Res;
{
   Res->x = X->x + Y->x;   
   Res->y = X->y + Y->y;   
}

/*  2 component dot product */
static long near v_dot(X, Y)
AvdPoint *X, *Y;
{
   return (((long) X->x) * ((long) Y->x) + ((long) X->y) * ((long) Y->y));
}
      
/*  return the third component of a cross product */
static int near v_cross3rd_comp(X, Y)
AvdPoint *X, *Y;
{
   long direction;
   direction = (
               (((long) X->x) * ((long) Y->y)) 
               - 
               (((long) X->y) * ((long) Y->x))
               );
   if (direction < 0)
      return(-1);
   if (direction > 0)
      return(1);
   return(0);
}

/* return the magnitude (squared) of vector X */

static long near v_sizesqrd(X,Y)
AvdPoint *X,*Y;
{
   long x,y;
   x = (((long) X->x) - ((long) Y->x)) * (((long) X->x) - ((long) Y->x));
   y = (((long) X->y) - ((long) Y->y)) * (((long) X->y) - ((long) Y->y));
   return (x + y);
}
