/* This handles initializing and showing the game board in the game Trippin. */


#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <clib/graphics_protos.h>
#include <pragmas/graphics_lib.h>
#include <stdlib.h>
#include "trip.h"

/* a little substitute cause we don't need to know the contents: */
struct Image { long image; };

/* so we don't need clib/intuition_protos.h: */
void DrawImage(struct RastPort *, struct Image	*, WORD, WORD);
#pragma amicall(IntuitionBase, 0x72, DrawImage(a0,a1,d0,d1))


#define SQWID  (SQIZE << 1)
#define BOWID  (SQIZE << 4)
#define ARIZE  ((SQIZE >> 1) - 2)
#define RADZ   (SQIZE / 3L)

#define ANY 0xFF


void LiftBob(piece *who);
void DropBob(piece *who);


import struct Image olabel, blabel;

import ubyte board[8][8];

import bool lace, won, eightcolor;

import short sqite, sthite, gthite, ocolor, bcolor;

import struct RastPort *r;

import piece bb, oo, *turn;


long black, white;

bool oisdark, bisdark;

local ubyte combos[58] = {        /* all possible three-bits-true bytes */
    0x07, 0x0B, 0x0D, 0x0E, 0x13, 0x15, 0x16, 0x19, 0x1A, 0x1C, 0x23, 0x25,
    0x26, 0x29, 0x2A, 0x2C, 0x31, 0x32, 0x34, 0x38, 0x43, 0x45, 0x46, 0x49,
    0x4A, 0x4C, 0x51, 0x52, 0x54, 0x58, 0x61, 0x62, 0x64, 0x68, 0x70, 0x83,
    0x85, 0x86, 0x89, 0x8A, 0x8C, 0x91, 0x92, 0x94, 0x98, 0xA1, 0xA2, 0xA4,
    0xA8, 0xB0, 0xC1, 0xC2, 0xC4, 0xC8, 0xD0, 0xE0, ANY, ANY
};



void Shuffle(void)
{
    ubyte deck[58];
    register short x, y;
    int rand(), i = 0;

    movmem(combos, deck, 58);
/*  for (i = 0; i < 2; i++)		/* some extra randomness */
	for (x = 0; x < 58; x++)
	    for (y = 0; y < 58; y++)
			if (rand() & 1) {
			    register ubyte t = deck[x];
			    deck[x] = deck[y];
			    deck[y] = t;
			}
    for (x = 0; x < 8; x++)
		for (y = 0; y < 8; y++)
		    if ((x == 1 || x == 6) && (y == 1 || y == 6))
				board[x][y] = ANY;
		    else if (y == 7 && (x == 4 || x == 3))
				board[x][y] = ANY;
		    else
				board[x][y] = deck[i++];
}



local void DrawArrows(short xc, short yc, short bits)
{
    short sx = xc + (SQWID >> 1) + 1, sy = yc + (sqite >> 1);
    short dx, dy, len, i;

    RectFill(r, sx - 3L, sy - (1L << lace),
				sx + 2L, sy + (2L << lace) - 1);
    for (i = 0; i < 8; i++)
		if (bits & (1 << i)) {
		    short yo = lace && i >= 2 && i <= 6;
		    dx = sx;
		    dy = sy;
		    len = i & 1 ? ARIZE - (ARIZE >> 2) : ARIZE;
		    if (i >= 1 && i <= 3)
				dx += len << 1;
		    else if (i >= 5)
				dx -= (len << 1);
		    if (i >= 3 && i <= 5)
				dy += len << lace;
		    else if (i <= 1 || i == 7)
				dy -= len << lace;
		    if (lace && sx != dx) {		/* thicken lace non-verticals */
				Move(r, (long) sx - (i >= 5), sy + 1L - yo);
				Draw(r, (long) dx - (i >= 5), dy + 1L - yo);
		    }
		    dy += yo;
		    sy += yo;
		    Move(r, (long) sx, (long) sy);
		    Draw(r, (long) dx - (i == 6), (long) dy);
		    if (sy != dy) {			/* thicken non-horizontals */
				Move(r, sx - 1L, (long) sy);
				Draw(r, dx - 1L, (long) dy);
		    }
		    sy -= yo;
		    if (i & 1) {			/* diagonal line arrowheads */
				long ye = dy + ((i == 3 || i == 5 ? -3L : 3L) << lace);
				Move(r, (long) dx, (long) dy);
				Draw(r, (long) dx, ye);
				Move(r, dx - 1L, (long) dy);
				Draw(r, dx - 1L, ye);
				Move(r, (long) dx, (long) dy);
				Draw(r, dx + (i >= 5 ? 6L - lace : -7L + lace), (long) dy);
				if (lace) {
				    if (yo) dy--;
				    else dy++;
				    Move(r, (long) dx, (long) dy);
				    Draw(r, dx + (i >= 5 ? 6L - lace : -7L + lace), (long) dy);
				}
		    } else {
				long pull;
				if (sx == dx) {				/* vertical lines */
				    pull = (i == 4 ? -2 : 2) << lace;
				    Move(r, (long) dx, (long) dy);
				    Draw(r, dx + 4L, dy + pull);
				    Move(r, dx - 1L, (long) dy);
				    Draw(r, dx - 5L, dy + pull);
				    if (lace) {
						if (i) dy--;
						else dy++;
						Move(r, (long) dx, (long) dy);
						Draw(r, dx + 4L, dy + pull);
						Move(r, dx - 1L, (long) dy);
						Draw(r, dx - 5L, dy + pull);
		    		}
				} else {				/* horizontal lines */
				    pull = i == 2 ? -4 : 4;
				    if (i == 6) dx--;
				    if (lace) {
						Move(r, (long) dx, (long) dy);
						Draw(r, dx + pull, dy + (2L << lace));
						Move(r, (long) dx, (long) dy - yo);
						Draw(r, dx + pull, dy - yo - (2L << lace));
				    }
				    dx += (short) pull >> 2;
				    Move(r, (long) dx, (long) dy);
				    Draw(r, dx + pull, dy + (2L << lace));
				    Move(r, (long) dx, (long) dy - yo);
				    Draw(r, dx + pull, dy - yo - (2L << lace));
				}
		    }
		}
}



void DrawSquare(short x, short y)
{
    long xc = 3 + x * SQWID, yc = sthite + 2 + y * sqite,
							    hafite = sqite >> 1;

    SetAPen(r, white);
    RectFill(r, xc + 2, yc + 1 + lace, xc + SQWID - 1, yc + sqite - 1);
    SetAPen(r, black);
    if ((x == 1 || x == 6) && (y == 1 || y == 6)) {
		DrawEllipse(r, xc + (SQWID >> 1) + 1, yc + hafite,
						RADZ << 1, RADZ << lace);
		DrawEllipse(r, xc + (SQWID >> 1), yc + hafite,
						RADZ << 1, RADZ << lace);
		if (lace) {
		   DrawEllipse(r, xc + (SQWID >> 1) + 1, yc + hafite + 1,
						RADZ << 1, RADZ << lace);
		   DrawEllipse(r, xc + (SQWID >> 1), yc + hafite + 1,
						RADZ << 1, RADZ << lace);
		}
    } else if (y == 7 && (x == 3 || x == 4)) {
		RectFill(r, xc + 8, yc + (4 << lace),
					xc + SQWID - 7, yc + sqite - (4 << lace) + lace);
		SetAPen(r, white);
		RectFill(r, xc + 14, yc + (7 << lace),
					xc + SQWID - 13, yc + sqite - (7 << lace) + lace);
    } else if (board[x][y] != ANY)
		DrawArrows((short) xc, (short) yc, board[x][y]);
	    SetAPen(r, black);
	    Move(r, xc + SQWID, yc);
	    Draw(r, xc, yc);
	    Draw(r, xc, yc + sqite);
	    Move(r, xc + 1, yc + sqite);
	    Draw(r, xc + 1, yc + 1);
	    if (lace)
			Draw(r, xc + SQWID, yc + 1);
}



local void CenterText(str say, long y)
{
    long sl = strlen(say);
    short tl = TextLength(r, say, sl);
    Move(r, BOWID + (MARGINWID >> 1) - (tl >> 1) + 5L,
						y + ((r->TxBaseline + 1) >> 1));
    Text(r, say, sl);
}



local long yoff(short y)
{
    return (long) (sthite + 2 + ((y) << lace));
}



void TellTurn(void)
{
    short xc, yc;
    long a;
    struct Image *label;
    str say;
    bool blueturn = turn == &bb;
    static bool lastwon = true;

    SetAPen(r, white);
    RectFill(r, BOWID + 7L, yoff(2), BOWID + MARGINWID, yoff(21) - 1);
    SetAPen(r, black);
    SetBPen(r, white);
    CenterText("To move:     ", yoff(11));
    label = blueturn ? &blabel : &olabel;
    DrawImage(r, label, BOWID - (IMWID >> 1) + 130L, yoff(10 - (IMHITE >> 1)));
    SetAPen(r, blueturn ? bcolor : ocolor);
    RectFill(r, BOWID + 17L, yoff(32), BOWID + MARGINWID - 10L, yoff(56));
    SetAPen(r, (blueturn ? bisdark : oisdark) ? white : black);
    SetBPen(r, blueturn ? bcolor : ocolor);
    if (won)
		say = "Game Over.";
    else if (!turn->allowed)
		say = "CAN'T MOVE!";
    else if (turn->machine)
		say = "thinking...";
    else
		say = "MAKE A MOVE";
    CenterText(say, yoff(44));
    SetAPen(r, eightcolor ? 7 : white);
    SetBPen(r, black);
    CenterText("Legal Moves:", yoff(74));
    xc = BOWID + 4 + ((MARGINWID) >> 1) - SQIZE;
    yc = yoff(82);
    SetAPen(r, white);
    RectFill(r, xc + 2L, yc + lace + 1L, xc + SQWID - 1L, yc + sqite - 1L);
    SetAPen(r, black);
    DrawArrows(xc, yc, won ? 0 : turn->allowed);
    if (!turn->allowed)
		won = true;
    if (won == lastwon)
		return;
    lastwon = won;
    if (won) {
		SetAPen(r, blueturn ? ocolor : bcolor);
		RectFill(r, BOWID + 9L, yoff(112), BOWID + MARGINWID - 2L, yoff(174));
		SetAPen(r, white);
		RectFill(r, BOWID + 19L, yoff(117), BOWID + MARGINWID - 12L, yoff(169));
		label = blueturn ? &olabel : &blabel;
		a = yoff(119);
		DrawImage(r, label, BOWID + 24L, a);
		DrawImage(r, label, BOWID + MARGINWID - IMWID - 20L, a);
		SetAPen(r, black);
		SetBPen(r, white);
		a = yoff(143) - ((gthite + 1) >> 1);
		CenterText("WE HAVE", a);
		CenterText("A WINNER!", a + gthite + 1);
		a = yoff(166 - IMHITE);
		DrawImage(r, label, BOWID + 24L, a);
		DrawImage(r, label, BOWID + MARGINWID - IMWID - 20L, a);
    } else {
		SetAPen(r, black);
		RectFill(r, BOWID + 9L, yoff(112), BOWID + MARGINWID - 2L, yoff(174));
		SetAPen(r, eightcolor ? 6 : BLUE);
		SetBPen(r, black);
		CenterText("Try to reach", yoff(135) - gthite);
		CenterText("the blinking", yoff(136));
		CenterText(" X  of your ", yoff(137) + gthite);
		CenterText("piece's color", yoff(138) + (gthite << 1));
    }
}



void ShowBoard(void)
{
    short x, y;
    long bot = 2 + sthite + (sqite << 3);

    SetDrMd(r, JAM2);
    SetAPen(r, black);
    RectFill(r, 5L + BOWID, sthite + 2L,
				3L + MARGINWID + BOWID, 2L + sthite + (sqite << 3) + lace);
    for (x = 0; x < 8; x++)
		for (y = 0; y < 8; y++)
		    DrawSquare(x, y);
    Move(r, BOWID + 4L, 2L + sthite);
    Draw(r, BOWID + 4L, bot);
    Draw(r, 4L, bot);
    Move(r, BOWID + 3L, 2L + sthite);
    Draw(r, BOWID + 3L, bot);
    if (lace) {
		Move(r, BOWID + 4L, bot + 1);
		Draw(r, 3L, bot + 1);
    }
    LiftBob(&oo);			/* paint tokens in start position */
    DropBob(&oo);
    LiftBob(&bb);
    DropBob(&bb);
    TellTurn();
}
