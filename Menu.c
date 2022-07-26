/* this file is got the menu stuff for the game Trippin. */


#include <intuition/intuition.h>
#include <functions.h>
#include "trip.h"


#define MENWID 204
#define SUBWID 154


void MakeAnts(short x, short y);
void KillAnts(void);
void Allow(piece *from, piece *tu);
void SetGoal(piece *p);
void Ding(void);
void LiftBob(piece *who);
void DropBob(piece *who);
void DrawSquare(short x, short y);
void StopThinking(void);
void StartThinking(void);
void TellTurn(void);
void Restart(void);
void ShowBoard(void);


import struct Image olabel, blabel;

import bool thinking, abort_think, lace, won, looprevent;

import short difficulty, sigdone, suggx, suggy, sthite;

import piece bb, oo, *turn;

import struct Window *win;

import struct Task *child;



local struct IntuiText xquit = {
    2, 1, JAM2, 4, 1, null, (ubyte *) "Quit the game", null
};


local struct MenuItem mlast = {
    null, 0, (IMHITE << 1) + 68, MENWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ,
    0L, (APTR) &xquit, null, 'Q', null, 0
};


local struct IntuiText xrestart = {
    2, 1, JAM2, 4, 1, null, (ubyte *) "Start a new game", null
};


local struct MenuItem m7 = {
    &mlast, 0, (IMHITE << 1) + 48, MENWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ,
    0L, (APTR) &xrestart, null, 'N', null, 0
};


local struct IntuiText xtakeback = {
    2, 1, JAM2, 4, 1, null, (ubyte *) "Take back move", null
};


local struct MenuItem m6 = {
    &m7, 0, (IMHITE << 1) + 38, MENWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ,
    0L, (APTR) &xtakeback, null, 'T', null, 0
};


local struct IntuiText xsuggest = {
    2, 1, JAM2, 4, 1, null, (ubyte *) "Suggest move", null
};


local struct MenuItem m5 = {
    &m6, 0, (IMHITE << 1) + 28, MENWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ,
    0L, (APTR) &xsuggest, null, 'S', null, 0
};


local struct IntuiText xprevent = {
    2, 1, JAM2, 24, 1, null, (ubyte *) "Prevent loops", null
};


local struct MenuItem m4 = {
    &m5, 0, (IMHITE << 1) + 18, MENWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT | CHECKED | MENUTOGGLE,
    0L, (APTR) &xprevent, null, 0, null, 0
};


local struct IntuiText xd9 = {
    2, 1, JAM2, 24, 1, null, (ubyte *) "9  ( zzz...)", null
};


local struct IntuiText xd8 = {
    2, 1, JAM2, 24, 1, null, (ubyte *) "8  (savage)", null
};


local struct IntuiText xd7 = {
    2, 1, JAM2, 24, 1, null, (ubyte *) "7  (brutal)", null
};


local struct IntuiText xd6 = {
    2, 1, JAM2, 24, 1, null, (ubyte *) "6  (mean)", null
};


local struct IntuiText xd5 = {
    2, 1, JAM2, 24, 1, null, (ubyte *) "5  (tough)", null
};


local struct IntuiText xd4 = {
    2, 1, JAM2, 24, 1, null, (ubyte *) "4  (slick)", null
};


local struct IntuiText xd3 = {
    2, 1, JAM2, 24, 1, null, (ubyte *) "3  (wary)", null
};


local struct IntuiText xd2 = {
    2, 1, JAM2, 24, 1, null, (ubyte *) "2  (sloppy)", null
};


local struct IntuiText xd1 = {
    2, 1, JAM2, 24, 1, null, (ubyte *) "1  (duuh...)", null
};


local struct IntuiText xdifficulty = {
    2, 1, JAM2, 4, 1, null, (ubyte *) "Difficulty level:", null
};


local struct MenuItem m3i = {
    null, MENWID - 26, 40, SUBWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT,
    0xfffffeffL, (APTR) &xd9, null, 0, null, 0
};


local struct MenuItem m3h = {
    &m3i, MENWID - 26, 30, SUBWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT,
    0xffffff7fL, (APTR) &xd8, null, 0, null, 0
};


local struct MenuItem m3g = {
    &m3h, MENWID - 26, 20, SUBWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT,
    0xffffffbfL, (APTR) &xd7, null, 0, null, 0
};


local struct MenuItem m3f = {
    &m3g, MENWID - 26, 10, SUBWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT,
    0xffffffdfL, (APTR) &xd6, null, 0, null, 0
};


local struct MenuItem m3e = {
    &m3f, MENWID - 26, 0, SUBWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT,
    0xffffffefL, (APTR) &xd5, null, 0, null, 0
};


local struct MenuItem m3d = {
    &m3e, MENWID - 26, -10, SUBWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT,
    0xfffffff7L, (APTR) &xd4, null, 0, null, 0
};


local struct MenuItem m3c = {
    &m3d, MENWID - 26, -20, SUBWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT,
    0xfffffffbL, (APTR) &xd3, null, 0, null, 0
};


local struct MenuItem m3b = {
    &m3c, MENWID - 26, -30, SUBWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT,
    0xfffffffdL, (APTR) &xd2, null, 0, null, 0
};


local struct MenuItem m3a = {
    &m3b, MENWID - 26, -40, SUBWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT,
    0xfffffffeL, (APTR) &xd1, null, 0, null, 0
};


local struct MenuItem m3 = {
    &m4, 0, (IMHITE << 1) + 8, MENWID, 10, ITEMTEXT | ITEMENABLED | HIGHNONE,
    0L, (APTR) &xdifficulty, null, 0, &m3a, 0
};


local struct IntuiText xcomputer = {
    2, 1, JAM2, 24, 1, null, (ubyte *) "Computer", null
};


local struct IntuiText xhuman = {
    2, 1, JAM2, 24, 1, null, (ubyte *) "Human", null
};


local struct IntuiText xplayedby = {
    2, 1, JAM2, 38, 3, null, (ubyte *) "is played by:", null
};


local struct MenuItem m2b = {
    null, MENWID - 26, 10, SUBWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT | CHECKED, bit(0),
    (APTR) &xcomputer, null, 0, null, 0
};


local struct MenuItem m2a = {
    &m2b, MENWID - 26, 0, SUBWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT, bit(1),
    (APTR) &xhuman, null, 0, null, 0
};


local struct MenuItem m1b = {
    null, MENWID - 26, 10, SUBWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT, bit(0),
    (APTR) &xcomputer, null, 0, null, 0
};


local struct MenuItem m1a = {
    &m1b, MENWID - 26, 0, SUBWID, 10,
    ITEMTEXT | ITEMENABLED | HIGHCOMP | CHECKIT | CHECKED, bit(1),
    (APTR) &xhuman, null, 0, null, 0
};


local struct MenuItem m2i = {
    &m3, 4, IMHITE + 5, 32, IMHITE + 2, ITEMENABLED | HIGHNONE, 0L,
    (APTR) &blabel, null, 0, null, 0
};


local struct MenuItem m2 = {
    &m2i, 0, IMHITE + 4, MENWID, IMHITE + 3, ITEMTEXT | ITEMENABLED | HIGHNONE,
    0L, (APTR) &xplayedby, null, 0, &m2a, 0
};


local struct MenuItem m1i = {
    &m2, 4, 1, 32, IMHITE + 2, ITEMENABLED | HIGHNONE, 0L,
    (APTR) &olabel, null, 0, null, 0
};


local struct MenuItem m1 = {
    &m1i, 0, 0, MENWID, IMHITE + 3, ITEMTEXT | ITEMENABLED | HIGHNONE, 0L,
    (APTR) &xplayedby, null, 0, &m1a, 0
};



PUBLIC struct Menu manyou = {
    null, 210, 0, MENWID, 10, MENUENABLED, "Tripppin out!", &m1, 0, 0, 0, 0
};

/* ------------------ JEEZ!  All that for just ONE menu?! ------------ */



local void Spread(struct MenuItem *m, short h)
{
    short t;
    for (t = 0; m; m = m->NextItem) {
		m->TopEdge += t;
		m->Height = sthite + 1;
		t += h;
    }
}



void ShoveMenus(void)		/* compensate for lace and/or tall font */
{
    struct MenuItem *mi;
    short t = (IMHITE << 2) + 14, h = sthite - 9;

    if (lace) {
		m1.Height = m2.Height = m2.TopEdge = (IMHITE << 1) + 6;
		xplayedby.TopEdge = (IMHITE >> 1) + 5;
		m2i.TopEdge = (IMHITE << 1) + 7;
		m1i.Height = m2i.Height = (IMHITE << 1) + 4;
		for (mi = &m3; mi; mi = mi->NextItem) {
		    mi->TopEdge = t;
		    t += 10;
		}
		mlast.TopEdge += 10;
    }
    if (h > 0) {
		Spread(&m3, h);			/* main items below imaged ones */
		Spread(&m3a, h);		/* difficulty subitems */
		manyou.Height = sthite + 1;
		Spread(&m1a, h);
		Spread(&m2a, h);
    }
    /* this function assumes that sthite < IMHITE + 2 */
    for (mi = &m3a, h = 1; mi; mi = mi->NextItem, h++)
		if (h == difficulty) {
		    mi->Flags |= CHECKED;
		    break;
		}
}



local void Suck(piece *who)
{
    history *h = who->hist;
    if (!h->count)
		return;
    turn = turn->other;
    LiftBob(who);
    DrawSquare(who->x, who->y);
    who->x = h->hx[h->top];
    who->y = h->hy[h->top];
    if (h->madegoal[h->top]) {
		--who->reached;
		SetGoal(who);
    }
    if (!h->top)
		h->top = HISTORY - 1;
    else --h->top;
	    --h->count;
    DropBob(who);
}



local void TakeBack(void)
{
    piece *oldturn = turn;

    if ((oo.machine && bb.machine) || !turn->other->hist->count) {
		Ding();
		return;
    }
    StopThinking();
    KillAnts(); 
    won = false;
    Suck(turn->other);
    if (oldturn->other->machine)
		Suck(oldturn);
    Allow(turn->other, turn);
    TellTurn();
    StartThinking();
}



bool DoMenu(short c)
{
    struct MenuItem *eye;
    short i;

    switch (c) {
		case 0:		/* blue player */
		    if ((oo.machine ? m1a.Flags : m1b.Flags) & CHECKED) {
				oo.machine = !oo.machine;
				if (turn == &oo) {
				    KillAnts();
				    StartThinking();
				    TellTurn();
				}
		    }
		    break;
		case 2:		/* gray player */
		    if ((bb.machine ? m2a.Flags : m2b.Flags) & CHECKED) {
				bb.machine = !bb.machine;
				if (turn == &bb) {
				    KillAnts();
				    StartThinking();
				    TellTurn();
				}
		    }
		    break;
		case 4:		/* difficulty level */
		    for (i = 1, eye = &m3a; eye; eye = eye->NextItem, i++)
				if (eye->Flags & CHECKED) {
				    difficulty = i;
				    break;
				}
			/* do *NOT* trust SUBITEM to be same as the one now checked! */
		    break;
		case 6:		/* suggest a move */
		    if (suggx >= 0)
				MakeAnts(suggx, suggy);
		    break;
		case 7:		/* take back a move */
		    TakeBack();
		    break;
		case 8:		/* restart the game */
		    Restart();
		    ShowBoard();
		    break;
		case 9:		/* quit the game */
		    abort_think = true;
		    return true;
    }
    looprevent = !!(m4.Flags & CHECKED);
    return false;
}
