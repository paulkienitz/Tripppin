#include <exec/types.h>
#include <intuition/intuition.h>

/*   Image ecks_image  
          Width:    11
         Height:    11
          Depth:    2
     TransColor:    0  */

/* Color Map */

USHORT map[] = {
   0x0fff,/* USED */
   0x0000,
   0x055f,
   0x0f80/* USED */
};

/* Image Data */

UWORD ecks[22] =
{
/* Bit Plane #0 */

   0x0000,
   0x60c0,
   0x71c0,
   0x3b80,
   0x1f00,
   0x0e00,
   0x1f00,
   0x3b80,
   0x71c0,
   0x60c0,
   0x0000,

/* Bit Plane #1 */

   0x0000,
   0x60c0,
   0x71c0,
   0x3b80,
   0x1f00,
   0x0e00,
   0x1f00,
   0x3b80,
   0x71c0,
   0x60c0,
   0x0000
};
struct Image ecks_image =
{
	0,0,			/* LeftEdge, TopEdge  */
	11,11,2,		/* Width, Height, Depth */
	&ecks[0],		/* Pointer to Image data */
	3,0,			/* PlanePick, PlaneOnOff */
	NULL,			/* NextImage pointer */
};
