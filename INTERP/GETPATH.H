#include "types.h"

struct   AvdPoint
   {
	int	x;
	int	y;
   };
typedef struct AvdPoint AvdPoint;
struct 	polygon
   {
	AvdPoint   	         *polyPoints;/* points to points of polygon*/
   ubyte                type;       /* number of nodes in polygon*/
	ubyte						info;			/* bit flag info */	
   int                  n;          /* number of nodes in polygon*/
	};
typedef struct polygon polygon;
struct 	polyNode
   {
	struct	polyNode 	*next;		/* pointer to next path in list*/
	struct	polyNode 	*prev;		/* pointer to previous path in list*/
	AvdPoint   	         I1;			/* intersection nearest to A*/
	AvdPoint   	         I2;			/* intersection nearest to B*/
   int                  d;          /* direction to travel on polygon*/
   int                  n1;         /* first node encountered when traversing polygon*/
   int                  n2;         /* last node encountered when traversing polygon*/
   AvdPoint             *poly;      /* points to polygon*/
   int                  n;          /* number of nodes in polygon*/
   int                  i;          /* index into polylist for this polygon */
	};
typedef struct polyNode polyNode;

struct 	polyPatch
   {
   int                  P_i;        /* Intersection node of given polygon*/
	int      	         Q_i;			/* start intersection node of patch loop*/
	AvdPoint   	         P_U;			/* start intersection point of patch loop*/
   int                  P_j;        /* end intersection node of given polygon*/
   int                  Q_j;        /* end intersection node of patch loop*/
   AvdPoint             Q_U;        /* end intersection point of patch loop*/
	int						delete ;
	};
typedef struct polyPatch polyPatch;

#ifdef	LINT_ARGS

	extern AvdPoint *  getpath(AvdPoint *,AvdPoint *,polygon *,int);
	extern int         ptIntr(AvdPoint *,AvdPoint *,int);
	extern AvdPoint *  MergePolygons(AvdPoint *,polygon *);

#else

	extern AvdPoint *  getpath();
	extern int         ptIntr();
	extern AvdPoint *  MergePolygons();
   
#endif
#define sign(x) ((x)>0 ? 1:(x)<0 ? -1:0)
#define ENDOFPATH 0x7777
#define FALSE 0
#define NOINTERSECT -1
#define INTERSECT    1
#define COLINEAR     0
#define INTERSECTA   2
#define INTERSECTB   4
#define INTERSECTC   8
#define INTERSECTD   16
#define INTERSECTI   32
#define MAXPOLY 30 	
#define MAXPATH 100
#define MAXVALUELONG 0x7FFFFFFF
#define MAXVALUEINT  0x7FFF
#define TRUE 1
#define MAXPATCHES 8

/* Types of polygons */
#define TAP 0  /* Total access polygon */
#define NAP 1  /* Near point access polygon */
#define BAP 2  /* Barred access polygon */
#define CAP 3  /* Contained accessable polygon */

#define MTP 16 /* Merged total access polygon */
#define MNP 17 /* Merged near point access polygon */
#define MBP 18 /* Merged barred access polygon */
#define MCP 19 /* Merged contained accessable polygon */

#define PInPolygon 1
#define MAXOPTIMIZEDNODES 3
#define CLOSETOSQR 2    /* allows 1 x and/or y delta error  */

/* Defines for info bit flags */

#define	INVERTED	1


// Types of polygon exits and polygon entries
enum PathStartType
   {
   fromA,
   fromAtoEXIT,
   fromBAPtoEXIT
   };

enum PathEndType 
   {
   toB,
   toENTRYtoB,
   toENTRY
   };
