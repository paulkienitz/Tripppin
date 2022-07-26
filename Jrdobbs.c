/* This handles drawing the pictures (gels) in the game Trippin. */

#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/sprite.h>
#include <graphics/rastport.h>
#include <graphics/gels.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <functions.h>
#include "trip.h"


#define TMPIZE (long) RASSIZE(200, 50)


void ShoveMenus(void);
void Die(str s);


import ubyte board[8][8];

import piece oo, bb;

import long black, white;

import bool oisdark, bisdark;

import struct Menu manyou;

import char pubscrname[];


struct GfxBase *GfxBase;


struct NewWindow boardwin = {
    81, 12, 16 * SQIZE + 4 + MARGINWID, 8 * SQIZE + 12, -1, -1,
    MOUSEBUTTONS | MOUSEMOVE | MENUPICK | CLOSEWINDOW,
    REPORTMOUSE | WINDOWDRAG | WINDOWDEPTH | WINDOWCLOSE
			| SMART_REFRESH | ACTIVATE,
    null, null, (ubyte *) "Tripppin 4         by Paul Kienitz", null, null,
    0, 0, 0, 0, PUBLICSCREEN
};


struct Window *win;


/* image data created by GetImage program from fish 345 */

local UWORD ecks[23] = {
    0, 0,
    0, 0x60c0,
    0, 0x71c0,
    0, 0x3b80,
    0, 0x1f00,
    0, 0x0e00,
    0, 0x1f00,
    0, 0x3b80,
    0, 0x71c0,
    0, 0x60c0,
    0, 0, 0
};


UWORD rawimage[IMHITE * 4] = {		/* for non-interlace */
    0x01fe,0x0000,
    0x03eb,0x0000,
    0x07f5,0x8000,
    0x03fb,0x0000,
    0x01fe,0x0000,
    0x00f4,0x0000,
    0x01f6,0x0000,
    0x1ffb,0xe000,
    0x7ffe,0xb800,
    0xffff,0x6c00,
    0xffff,0xfc00,
    0x7fff,0xf800,
    0x07ff,0x8000,
 
    0x0000,0x0000,
    0x00fc,0x0000,
    0x017e,0x0000,
    0x00bc,0x0000,
    0x0058,0x0000,
    0x0038,0x0000,
    0x0078,0x0000,
    0x00bc,0x0000,
    0x057f,0xc000,
    0x2bff,0xf000,
    0x15ff,0xf000,
    0x02b5,0x0000,
    0x0000,0x0000
};


UWORD rawimage2[IMHITE * 8] = {		/* for interlace */
    0x00fc,0x0000,
    0x01fe,0x0000,
    0x03d7,0x0000,
    0x07eb,0x8000,
    0x07f5,0x8000,
    0x07fb,0x8000,
    0x07fd,0x8000,
    0x03ff,0x0000,
    0x01fe,0x0000,
    0x00fc,0x0000,
    0x00f4,0x0000,
    0x00f4,0x0000,
    0x01f6,0x0000,
    0x03f7,0x0000,
    0x0ffb,0xc000,
    0x3ff5,0xf000,
    0x7ffa,0xb800,
    0xffff,0x5c00,
    0xffff,0xac00,
    0xffff,0xdc00,
    0xffff,0xfc00,
    0xffff,0xfc00,
    0x7fff,0xf800,
    0x7fff,0xf800,
    0x1fff,0xe000,
    0x03ff,0x0000,

    0x0000,0x0000,
    0x0000,0x0000,
    0x00fc,0x0000,
    0x01fe,0x0000,
    0x00fe,0x0000,
    0x017e,0x0000,
    0x00be,0x0000,
    0x005c,0x0000,
    0x0028,0x0000,
    0x0058,0x0000,
    0x0038,0x0000,
    0x0078,0x0000,
    0x0038,0x0000,
    0x0078,0x0000,
    0x00fc,0x0000,
    0x01ff,0x0000,
    0x0bff,0xc000,
    0x17ff,0xe000,
    0x2bff,0xf000,
    0x17ff,0xf000,
    0x2bff,0xf000,
    0x15ff,0xe000,
    0x0aaa,0x8000,
    0x0155,0x0000,
    0x0000,0x0000,
    0x0000,0x0000
};


struct Image olabel = {
    0, 0, IMWID + 4, IMHITE + 2, 3, null /* will be oonwhite */, 7, 0, null
}, blabel = {
    0, 0, IMWID + 4, IMHITE + 2, 3, null /* bonwhite */, 7, 0, null
};
/* these are used in the menu and margin stuff, not on the board */


struct SimpleSprite ox = { &ecks[0], 1, 0, 0, -1 },
			bx = { &ecks[1], 1, 0, 1, -1 };


extern struct Bob obob, bbob;		/* declared in full below */


struct VSprite ovs = {
    null, null, null, null, 0, 0,	/* system fields */
    SAVEBACK | OVERLAY,			/* is a Bob */
    0, 0, IMHITE, 2, 3,		/* y, x, height, width (in words), planes */
    0, 0, null,			/* memask, hitmask, image = oimd */
    null, null, null,		/* borderline, collmask = shadow, colormap */
    &obob, 7, 0			/* parent Bob, planepick, planeonoff */
}, bvs = {
    null, null, null, null, 0, 0,	/* system fields */
    SAVEBACK | OVERLAY,			/* is a Bob */
    0, 0, IMHITE, 2, 3,		/* y, x, height, width (in words), planes */
    0, 0, null,			/* memask, hitmask, image = bimd */
    null, null, null,		/* borderline, collmask = shadow, colormap */
    &bbob, 7, 0			/* parent Bob, planepick, planeonoff */
};


struct VSprite dummy1 = { 0 }, dummy2 = { 0 };


WORD nextline[8], *(lastcolor[8]);	/* lastcolor is array of pointers */


struct GelsInfo ginfo = {
    0xFC, 0, null, null, &nextline[0], &lastcolor[0],
    null, 0, 0, 0, 0, null, null
};


struct /* J.R. " */ Bob /* " Dobbs */ obob = {
    0, null, null, null, null,		/* flags, save, shadow, before, after */
    &ovs, null, null			/* vsprite, animcomp, dbufpacket */
}, bbob = {
    0, null, null, null, null,		/* flags, save, shadow, before, after */
    &bvs, null, null			/* vsprite, animcomp, dbufpacket */
};


/* pointers into chip ram: */
WORD *oimd, *bimd, *osav, *bsav, *shadow,
		*oonwhite, *bonwhite, *osprat, *bsprat, *nuth;

#define loonwhite ((ulong *) oonwhite)
#define lbonwhite ((ulong *) bonwhite)


short sqite, sthite, gthite, ocolor, bcolor;

bool lace, eightcolor, negative;

struct RastPort *r;
struct ViewPort *vp;

struct TmpRas wtr;
PLANEPTR trp;

long imize = 0;



#define TFree(m, s) if (m) FreeMem(m, (long) s)

void DumpPrettyPictures(void)
{
    if (!imize)
		return;
    TFree(bsprat, 48);
    TFree(osprat, 48);
    TFree(nuth, 24);
    TFree(bonwhite, imize + 48);
    TFree(oonwhite, imize + 48);
    TFree(shadow, imize / 3);
    TFree(bimd, imize);
    TFree(oimd, imize);
    TFree(bsav, imize * 3);
    TFree(osav, imize * 3);
    if (win) {
		ClearMenuStrip(win);
		CloseWindow(win);
    }
    TFree(trp, TMPIZE);
    if (~ox.num)
		FreeSprite((long) ox.num);
    if (~bx.num)
		FreeSprite((long) bx.num);
}



short Brightness(ushort pen)	/* weighted; scale from 0 to 100 */
{
    ushort rgb = GetRGB4(vp->ColorMap, pen);
    return ((rgb >> 8) * 30 + ((rgb >> 4) & 0xF) * 40 + (rgb & 0xF) * 20) / 15;
}



void MakePrettyPictures(void)
{
    register short i;
    short imhite;
    ulong *o, *b, *rr;
    ulong rgb;
    struct Screen *wbench;

    if (!pubscrname[0] || !(wbench = LockPubScreen(pubscrname)))
		wbench = LockPubScreen(null);
    if (!wbench)
		Die("Workbench inaccessible!\n");
    sthite = wbench->Font->ta_YSize;
    /* that's the height of the font our title bar and menu will use */
    if (sthite < 8 || sthite > 40)
		sthite = 8;			/* crude */
    gthite = GfxBase->DefaultFont->tf_YSize;
    /* that's the height of the font that text rendered in the window uses */
    lace = wbench->Height >= 400;	/* SHOULD CHECK ASPECT RATIO */
    sqite = SQIZE << lace;
    boardwin.Height = sthite + wbench->WBorTop + wbench->WBorBottom
						+ (sqite << 3) + lace + 1;	/* room for title bar */
    boardwin.TopEdge -= sthite - 6;		      /* constant bottom edge */
    boardwin.Width += wbench->WBorRight + wbench->WBorLeft - 4;
    if (boardwin.TopEdge < 0)
		boardwin.TopEdge = 0;
    vp = &wbench->ViewPort;
    negative = Brightness(1) > Brightness(2);
    boardwin.Screen = wbench;
    if (!(win = OpenWindow(&boardwin)))
		Die("Can't open window!\n");
    ScreenToFront(wbench);
    eightcolor = wbench->BitMap.Depth > 2;
    UnlockPubScreen(null, wbench);
    if (win->WScreen->BitMap.Depth > 8)
		Die("Sorry; can't handle a screen of more than 256 colors.\n");

    ShoveMenus();
    SetMenuStrip(win, &manyou);
    if (!(trp = AllocCP(TMPIZE)))
		Die("Can't allocate TmpRas for window!\n");
    InitTmpRas(&wtr, trp, TMPIZE);
    r = win->RPort;
    r->TmpRas = &wtr;
    imhite = IMHITE << lace;
    imize = RASSIZE(IMWID, imhite) * 3;
    /* probably should have used AllocEntry, but it just sorta grew ... */
    if (!(oimd = AllocCP(imize)) || !(bimd = AllocCP(imize)) ||
				!(osav = AllocCP(imize * 3)) ||	  /* 256 color WB! */
				!(bsav = AllocCP(imize * 3)) ||
				!(shadow = AllocCP(imize / 3)) ||
				!(oonwhite = AllocCPZ(imize + 48)) ||
				!(bonwhite = AllocCPZ(imize + 48)) ||
				!(osprat = AllocCP(48)) || !(bsprat = AllocCP(48)) ||
				!(nuth = AllocCPZ(24)))
		Die("Can't allocate chip ram for image data!\n");
    o = (ulong *) (ovs.ImageData = oimd);
    b = (ulong *) (bvs.ImageData = bimd);
    obob.SaveBuffer = osav;
    bbob.SaveBuffer = bsav;
    obob.ImageShadow = ovs.CollMask = bbob.ImageShadow = bvs.CollMask = shadow;
    ovs.BorderLine = bvs.BorderLine = nuth + 8;
    ovs.Height = bvs.Height = imhite;
    oo.face = &obob;
    bb.face = &bbob;
    if (eightcolor)
		ocolor = 5, bcolor = 6;
    else
		ocolor = 3, bcolor = 0;
    oisdark = Brightness(ocolor) < 40;
    bisdark = Brightness(bcolor) < 40;
    if (negative)
		white = BLACK, black = WHITE;
    else
		white = WHITE, black = BLACK;

    rr = (ulong *) (lace ? rawimage2 : rawimage);
    for (i = 0; i < imhite; i++) {
		short i2 = i + imhite, i3 = i + 2 * imhite;
		register short ri1, ri2;
		register long m, dw;
		if (negative)					/* swap bitplanes, so */
		    ri1 = i2, ri2 = i;			/* pens 1 & 2 swapped */
		else
		    ri1 = i, ri2 = i2;
		m = o[i] = rr[ri1];
		m &= (dw = rr[ri2]);			/* bits on in m are color 3 */
		if (eightcolor)
		    dw &= ~m;
		b[i] = rr[ri1] ^ m;				/* ^ m turns color 3 into 0 */
		o[i2] = dw;
		b[i2] = eightcolor ? rr[ri2] : rr[ri2] ^ m;
		o[i3] = b[i3] = m;
    }
    InitMasks(&ovs);					/* fill in bob shadow mask */

    /* turn bobs into images, filling in white around the sides: */
    for (i = 0; i < imhite; i++) {
		register short i2 = i + imhite, i3 = i2 + imhite;
		register ulong m, w1, w2;
		m = w1 = o[i2] >> 2;
		m |= w2 = o[i] >> 2;			/* ~m is true for bits of color 0 */
		if (negative) {
		    loonwhite[i + 1 + lace] = w2 | ~m;
		    lbonwhite[i + 1 + lace] = (b[i] >> 2) | ~m;
		    loonwhite[i2 + (3 << lace)] = w1;
		    lbonwhite[i2 + (3 << lace)] = b[i2] >> 2;
		} else {
		    loonwhite[i + 1 + lace] = w2;
		    lbonwhite[i + 1 + lace] = b[i] >> 2;
		    loonwhite[i2 + (3 << lace)] = w1 | ~m;
		    lbonwhite[i2 + (3 << lace)] = (b[i2] >> 2) | ~m;
		}
		loonwhite[i3 + (5 << lace)] = o[i3] >> 2;
		lbonwhite[i3 + (5 << lace)] = b[i3] >> 2;
    }
    for (i = 0; i <= lace; i++) {	/* top and bottom edges */
		register short i1 = (negative ? 0 : ((IMHITE + 2) << lace)) + i;
		register short i2 = i1 + ((IMHITE + 1) << lace);
		loonwhite[i1] = loonwhite[i2] = lbonwhite[i1] = lbonwhite[i2] = ~0;
    }
    olabel.ImageData = (ushort *) oonwhite;
    blabel.ImageData = (ushort *) bonwhite;
    olabel.Height = blabel.Height = (IMHITE + 2) << lace;
    ginfo.sprRsrvd &= ~GfxBase->SpriteReserved;
    r->GelsInfo = &ginfo;
    InitGels(&dummy1, &dummy2, &ginfo);

    for (i = 0; i < 11; i++) {
		((long *) osprat)[i] = ((long *) (&ecks[0]))[i];
		((long *) bsprat)[i] = ((long *) (&ecks[1]))[i];
    }
    for (i = 2; i <= 6; i += 2) {
		if (~GetSprite(&ox, (long) i) && ~GetSprite(&bx, (long) i + 1)) {
		    i = (i << 1) + 17;
		    rgb = GetRGB4(vp->ColorMap, bcolor);
		    SetRGB4(vp, (long) i, rgb >> 8, (rgb >> 4) & 0xF, rgb & 0xF);
		    rgb = GetRGB4(vp->ColorMap, ocolor);
		    SetRGB4(vp, i + 1L, rgb >> 8, (rgb >> 4) & 0xF, rgb & 0xF);
		    return;
		}
		if (~ox.num)
		    FreeSprite((long) ox.num);
		if (~bx.num)
		    FreeSprite((long) bx.num);
		ox.num = bx.num = -1;
    }
    Die("Can't find a free pair of sprites!\n");
}



void Win2Square(short x, short y, short *xp, short *yp)
{
    *xp = (x + (SQIZE << 1) - 4) / (SQIZE << 1) - 1;
    *yp = (y + sqite - 2 - sthite - lace) / sqite - 1;
}



void DropBob(piece *who)
{
    register struct Bob *b = who->face;
    b->BobVSprite->X = who->x * (SQIZE << 1) + 15;
    b->BobVSprite->Y = who->y * sqite + 2 + sthite + (5 << lace);
    /* SortGList(r); */
    DrawGList(r, vp);			/* paint it in center of square */
    b->Flags |= SAVEBOB;
    RemIBob(b, r, vp);			/* dump it on the background */
}



void LiftBob(piece *who)
{
    who->face->Flags &= ~SAVEBOB;
    AddBob(who->face, r);
    /* SortGList(r); */
}



void DragBob(piece *who, short x, short y)
{
    who->face->BobVSprite->X = x - 10;
    who->face->BobVSprite->Y = y - (((IMHITE >> 2) + 3) << lace);
    /* SortGList(r); */
    DrawGList(r, vp);		/* must already be LiftBobbed */
}
