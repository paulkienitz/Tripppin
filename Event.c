/* this handles player mouse input in the game Trippin. */

#include <exec/memory.h>
#include <intuition/intuition.h>
#include <graphics/sprite.h>
#include <functions.h>
#include "trip.h"


#define THINKDELAY 200
#define ANTTIME    27
#define BLINKTIME  75

/* THINKDELAY is the minimum time in 300ths of a second for which we should
say "thinking" before we make a machine move (it embarasses the users when we
move before they finish letting go of the mouse button, with them low
difficulty levels).  ANTTIME is the minimum duration of each frame of the
animation of the ants, if any.  The ants can't animate any faster than ten
per second (reliably) without opening the timer device ... but that's fast
enough and setting up a tiny vblank interrupt is less trouble than opening
the timer.  BLINKTIME is the time between toggling the X markers on or off. */


bool TryMove(piece *who, short x, short y);
bool DoMenu(short c);
void Win2Square(short x, short y, short *xp, short *yp);
void LiftBob(piece *who);
void DropBob(piece *who);
void DragBob(piece *who, short x, short y);
void DrawSquare(short x, short y);
void MachineMove(void);


import piece oo, bb, *turn;

import bool notadrill, thinking, abort_think;

import bool won, lace;

import short sigdone, suggx, suggy, thinkx, thinky, depth, sthite;

import short *osprat, *bsprat, *nuth;

import struct SimpleSprite ox, bx;

import struct Task *child;

import struct Window *win;

import struct ViewPort *vp;


adr IntuitionBase;


/* image data created with GetImage */

local UWORD ant0[26] = {
    0x5280, 0x6300,
    0x5298, 0x6318,
    0x0000, 0x0018,
    0x0018, 0x0000,
    0xc000, 0x0000,
    0x0000, 0xc000,
    0xc018, 0xc018,
    0x0000, 0x0018,
    0x0018, 0x0000,
    0xc000, 0x0000,
    0x0000, 0xc000,
    0xca50, 0xc630,
    0x0a50, 0x0630
};


local UWORD ant1[26] = {
    0x2520, 0x8428,
    0x2520, 0x4630,
    0xc018, 0xc000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0xc018, 0x0018,
    0x0000, 0xc018,
    0xc018, 0xc000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0xc018, 0x0018,
    0x2520, 0x6310,
    0x2520, 0xa308
};


local UWORD ant2[26] = {
    0x0a50, 0x0c60,
    0xca50, 0x0c60,
    0x0000, 0xc000,
    0xc000, 0xc000,
    0x0018, 0x0018,
    0x0000, 0x0018,
    0xc018, 0x0000,
    0x0000, 0xc000,
    0xc000, 0xc000,
    0x0018, 0x0018,
    0x0000, 0x0018,
    0x5298, 0x3180,
    0x5280, 0x3180
};


local UWORD ant3[26] = {
    0x14a0, 0x18c0,
    0x14a0, 0x18c0,
    0xc000, 0x0000,
    0x0018, 0xc018,
    0xc000, 0xc018,
    0x0018, 0x0000,
    0x0000, 0x0000,
    0xc000, 0x0000,
    0x0018, 0xc018,
    0xc000, 0xc018,
    0x0018, 0x0000,
    0x2940, 0x18c0,
    0x2940, 0x18c0
};


local UWORD ant4[26] = {
    0x2940, 0x3180,
    0x2940, 0x3180,
    0x0018, 0x0018,
    0xc000, 0x0018,
    0x0018, 0xc000,
    0xc000, 0xc000,
    0x0000, 0x0000,
    0x0018, 0x0018,
    0xc000, 0x0018,
    0x0018, 0xc000,
    0xc000, 0xc000,
    0x14a0, 0x0c60,
    0x14a0, 0x0c60
};

local UWORD *(antses[5]) = { ant0, ant1, ant2, ant3, ant4 };


piece *grabee;

long startedthinking = 0, antsmoved, blinked = 0;

local short sprum = -1, antnum, *sproint = null;	/* points to chip ram */

local struct SimpleSprite suggestion = {
    &ant0[0] /* sproint */, 13, 0, 0, -1
};



void Ding(void)
{
    DisplayBeep(win->WScreen);
}



long Now(void)	/* returns time since game started measured in 300ths of sec */
{
    static long starts = 0, startm = 0;
    long ss, mm;
    CurrentTime((ulong *) &ss, (ulong *) &mm);
    if (!starts) {
		starts = ss;
		startm = mm;
		return 0;
    }
    return 300 * (ss - starts) + (mm - startm) / 3333;
}



void KillAnts(void)
{
    if (~sprum)
		FreeSprite((long) sprum);
    if (sproint)
		FreeMem(sproint, 64L);
    sproint = null;
    sprum = -1;
}



local short spx, spy;

local void AnimAnts(void)
{
    ushort *new;
    long now = Now(), offf = 9L + sthite + win->TopEdge + 8 * lace, blate;
    bool stifle = (win->Flags & MENUSTATE) || !(win->Flags & WINDOWACTIVE);
    short hi = stifle ? 1 : 9;

    if (ox.x == bx.x && ox.y == bx.y)
		blate = BLINKTIME << 1;
    else if (ox.height > 1)
		blate = 3 * BLINKTIME;
    else blate = BLINKTIME;
    if (stifle || now - blinked >= blate) {
		blinked = now;
		if (blate == (BLINKTIME << 1) && !won) {
		    if (ox.height == 1)
				ox.height = hi, bx.height = 1;
	   		else
				ox.height = 1, bx.height = hi;
		} else if (ox.height == 1 && !won)
	    	ox.height = bx.height = hi;
		else
	    	ox.height = bx.height = 1;
		((long *) osprat)[10] = ((long *) bsprat)[10] = ((long *) nuth)[2] = 0;
		ChangeSprite(vp, &ox, (PLANEPTR) (ox.height == 1 ? nuth : osprat));
		ChangeSprite(vp, &bx, (PLANEPTR) (bx.height == 1 ? nuth : bsprat));
    }
    MoveSprite(vp, &ox, oo.goalx * (SQIZE << 1) + 14L + win->LeftEdge,
						oo.goaly * (SQIZE << lace) + offf);
    MoveSprite(vp, &bx, bb.goalx * (SQIZE << 1) + 14L + win->LeftEdge,
						bb.goaly * (SQIZE << lace) + offf);
    /* MoveSprite them repeatedly to follow window / viewport movement */

    if (!~sprum)
		return;
    /* Should we wait for the video beam to be well placed?     NAAAAH... */
    if (now - antsmoved >= ANTTIME) {
		antsmoved = now;
		if (++antnum > 4)
		    antnum = 0;
		new = antses[antnum];
		if (stifle)
		    setmem(sproint + 2, 52, 0);		/* invisible */
		else
		    movmem(new, sproint + 2, 52);
    }
    MoveSprite(vp, &suggestion, spx * (SQIZE << 1) + 12L + win->LeftEdge,
				spy * (SQIZE << lace) + 7L + sthite + win->TopEdge + 6 * lace);
}



void MakeAnts(short x, short y)
{
    struct SimpleSprite placeholder;
    long cum;
    bool gotsp1;

    KillAnts();
    placeholder = suggestion;
    gotsp1 = GetSprite(&placeholder, 1L) != -1;      /* reserve this sprite */
    if (~(sprum = GetSprite(&suggestion, -1L)))
		if (!(sproint = AllocCPZ(64))) {
			FreeSprite((long) sprum);
			sprum = -1;
		}
    if (gotsp1)
		FreeSprite(1L);
    if (sprum < 0)
		return;
    ChangeSprite(vp, &suggestion, (PLANEPTR) sproint);
    spx = x; spy = y;
    antnum = 0;
    cum = 17 + ((sprum << 1) & ~3);
    SetRGB4(vp, cum, 0L, 0L, 15L);			/* blue */
    SetRGB4(vp, cum + 1, 0L, 15L, 0L);		/* green */
    SetRGB4(vp, cum + 2, 15L, 0L, 0L);		/* red */
    antsmoved = 0;
    AnimAnts();
}



void PickUpToken(struct IntuiMessage *m)
{
    piece *ogre = grabee;
    short xxx, yyy;

    Win2Square(m->MouseX, m->MouseY, &xxx, &yyy);
    if (xxx == oo.x && yyy == oo.y)
		grabee = &oo;
    else if (xxx == bb.x && yyy == bb.y)
		grabee = &bb;
    else grabee = null;
    if (won)
		grabee = null;
    if (grabee != ogre) {
		if (ogre) /* probably not */
		    DropBob(ogre);
		if (grabee) {
		    if (grabee != turn || grabee->machine) {	/* not your turn! */
				grabee = null;
				Ding();
	    	} else {
				LiftBob(grabee);
				DrawSquare(grabee->x, grabee->y);	/* erase old image */
				DragBob(grabee, m->MouseX, m->MouseY);
	    	}
		}
    }
}



void SetTokenDown(struct IntuiMessage *m)
{
    short xx, yy;
    Win2Square(m->MouseX, m->MouseY, &xx, &yy);
    if (!TryMove(grabee, xx, yy))
		Ding();						/* illegal move */
    DropBob(grabee);
    grabee = null;
}



void Play(void)
{
    struct IntuiMessage *orig, m;
    long sigmund, mask;
    bool machineready = false;

    mask = bit(win->UserPort->mp_SigBit) | bit(sigdone) | sigftof
										 | SIGBREAKF_CTRL_C;
    for (;;) {		/* intuition event loop */
		sigmund = Wait(mask);
		while (orig = (adr) GetMsg(win->UserPort)) {
		    m = *orig;
		    ReplyMsg((struct Message *) orig);
		    switch (m.Class) {
				case IDCMP_CLOSEWINDOW:
				    abort_think = true;
				    return;
				case IDCMP_MOUSEBUTTONS:
				    if (m.Code == SELECTDOWN)
						PickUpToken(&m);
				    else if (m.Code == SELECTUP && grabee)
						SetTokenDown(&m);
					break;
				case IDCMP_MOUSEMOVE:
		    		if (m.Qualifier & IEQUALIFIER_LEFTBUTTON && grabee)
						DragBob(grabee, m.MouseX, m.MouseY);
		    		break;
				case IDCMP_MENUPICK:
		    		if (grabee) /* probably not */
						DropBob(grabee);
		    		grabee = null;
		    		if (DoMenu(ITEMNUM(m.Code)))
						return;
		    		break;
	    	}
		}
		AnimAnts();
		if (sigmund & bit(sigdone)) {
		    if (!abort_think && !won) {
				if (notadrill) {
				    machineready = true;
				    abort_think = false;
				} else {
				    suggx = thinkx;
				    suggy = thinky;
				    depth++;
				    if (depth <= 10) {
						startedthinking = Now();
						thinking = true;
						Signal(child, sigfthink);	/* resume, deeper */
				    }
				}
		    } else
		    	abort_think = false;
		}
		if (machineready && Now() - startedthinking >= THINKDELAY) {
		    machineready = false;
		    MachineMove();
		}
		if (sigmund & SIGBREAKF_CTRL_C) {
		    abort_think = true;
		    return;
		}
    }
}
