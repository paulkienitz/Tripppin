/* TO ADD:
    choice of aggressive or defensive computer play (aggressive = assume you
		will fuck up if given the chance) ?
    requester to tell you the rules?  AmigaGuide help?
    raw masks separate for each color, to be blapped into different planes of
		images to create arbitrary color combinations?
    strategy that favors charging ahead when in the lead, and obstructionism
		when the other player is ahead?
*/


/*
This is sort of the game of TRIPPPLES ® that was marketed by some game
company I forget which, and I can't find it in the stores so this
implementation is based purely on my memory of having played the game years
and years ago ...  the computer opponent uses a straightforward lookahead
strategy of variable depth.  I wrote this mainly for practice with the
graphics.library which I hadn't hardly used much yet, and just to prove I
COULD write an honest-to-god event loop ... most everything else I've been
writing has been CLI utilities ... It was easier than I expected; day 1 got
the board displayed pretty well (the tricky part was keeping everything
proportioned right on both interlaced and non-laced displays), day 2 I made
player tokens that could be moved around with the mouse (took some trial and
error to get the hang of using Bobs), the next evening got it to know all the
rules of the game, and display the game status in the right margin, another
evening got the computer opponent working, including the task synchronization
... after that it was just a matter of adding bells and whistles, like
putting arrowheads on the arrows, adding a menu, flashing X's to show where
the pieces are trying to go, the suggest-a-move and take-back-a-move
features, stuff to make it adapt gracefully to different sized fonts ... 
After the first release users gave me some suggestions which prompted me to
add an option to turn off the prevention of loops, and stifle the sprites
when the window is inactive or the menus are up.  I also found two bugs.
Then in early 1994 I redid it to be no longer DOS 1.x compatible and made its
thinker no longer bog down at priority -1 except when thinking of a suggestion
instead of its own move, added tooltypes, and twiddled the cosmetics a bit.

This program is written by Paul Kienitz and is placed in the public domain.
*/


#include <hardware/intbits.h>
#include <exec/interrupts.h>
#include <dos/dosextens.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>
#include <pragmas/exec_lib.h>
#include <pragmas/dos_lib.h>
#include <pragmas/icon_lib.h>
#include <stdlib.h>
#include "trip.h"


#define FOREPRI 1
#define BACKPRI 0
#define HINTPRI -1

/* priorities for foreground and background tasks */


long Now(void);
void KillAnts(void);
void Play(void);
void MakePrettyPictures(void);
void DumpPrettyPictures(void);
void Shuffle(void);
void ShowBoard(void);
void TellTurn(void);
void LiftBob(piece *who);
void DrawSquare(short x, short y);
void DropBob(piece *who);

void geta4(void);


import struct Library *GfxBase, *IntuitionBase;

import long startedthinking;


piece oo, bb, *turn;

history ohist, bhist;

ubyte board[8][8];			/* grid of 8 bit masks */

bool won = false, bluefirst = false, looprevent = true;
bool thinking = false, abort_think = false, notadrill;

struct WBStartup *wbs;			/* Workbench startup message */

short difficulty = 3, depth, thinkx, thinky, suggx, suggy, oldpri;

short sigdone = -1, sigkillme = -1;		/* child -> parent */

struct Task *child, *parent;

char pubscrname[256];


void VBsignal_func();

struct Interrupt VBsignal = {
    { null, null, NT_INTERRUPT, 6, "Tripppin signaler" },	/* node */
    null,			/* data - will be address of task to signal */
    VBsignal_func		/* code */
};



void Die(str s)
{
    if (child) {
		RemIntServer(INTB_VERTB, &VBsignal);
		Signal(child, sigfdie);
		Wait(bit(sigkillme));
		DeleteTask(child);
    }
    if (~sigdone)
		FreeSignal((long) sigdone);
    if (~sigkillme)
		FreeSignal((long) sigkillme);
    KillAnts();
    DumpPrettyPictures();
    if (IntuitionBase)
		CloseLibrary(IntuitionBase);
    if (GfxBase)
		CloseLibrary(GfxBase);
    if (s && *s && Output())		/* may happen under dos 1.x */
		Write(Output(), s, strlen(s));
    if (wbs) {
		Forbid();
		ReplyMsg((struct Message *) wbs);
    } else
		SetTaskPri(parent, (long) oldpri);
    Exit(s ? 20 : 0);
}



void LearnScreenName(void)
{
    pubscrname[0] = 0;
    if (wbs) {
		struct Library *IconBase = OpenL("icon");
		long v;
		str n;
		if (IconBase) {
		    BPTR ocd = CurrentDir(wbs->sm_ArgList->wa_Lock);
		    struct DiskObject *bob = GetDiskObject(wbs->sm_ArgList->wa_Name);
		    if (bob) {
				if (n = FindToolType((ustr *) bob->do_ToolTypes, "PUBSCREEN")) {
				    while (*n == ' ')
						n++;
				    strcpy(pubscrname, n);
				    n = pubscrname + strlen(pubscrname);
				    while (--n >= pubscrname && *n == ' ')
						*n = 0;
				}
				if ((n = FindToolType((ustr *) bob->do_ToolTypes,
										"DIFFICULTY")) && (v = atol(n))) {
				    if (v >= 1 && v <= 9)
						difficulty = v;
				}
				FreeDiskObject(bob);
		    }
		    CurrentDir(ocd);
		    CloseLibrary(IconBase);
		}
    } else {
		long argz[2];
		struct RDArgs *ra;
		argz[0] = argz[1] = 0;
		if (ra = ReadArgs("DIFFICULTY/N,PUBSCREEN", argz, null)) {
		    if (argz[0]) {
				long v = *((long *) argz[0]);
				if (v >= 1 && v <= 9)
				    difficulty = v;
		    }
		    if (argz[1])
			strcpy(pubscrname, (str) argz[1]);
			FreeArgs(ra);
		} else {
		    PrintFault(IoErr(), "Tripppin");
		    Die("");
		}
    }
}



void Init(void)
{
    struct DateStamp d;
    void ThinkerTask();
    struct Task *CreateTask();

    DateStamp(&d);
    LearnScreenName();
    srand((int) ((d.ds_Days << 2) + (d.ds_Minute << 4) + d.ds_Tick));
    if (!(GfxBase = OpenL("graphics")))
		Die("No graphics!!\n");
    if (GfxBase->lib_Version < 37)
		Die("This version requires AmigaDOS 2.04.\n");
    if (!(IntuitionBase = OpenL("intuition")))
		Die("No intuition!!\n");
    MakePrettyPictures();
    if (!~(sigdone = AllocSignal(-1L)) || !~(sigkillme = AllocSignal(-1L)))
		Die("Couldn't allocate signals!\n");
    if (!(child = CreateTask("Thinkin and Trippin", BACKPRI, ThinkerTask, 8000L)))
		Die("Couldn't create subtask!\n");
    parent = FindTask(null);
    VBsignal.is_Data = (adr) parent;
    AddIntServer(INTB_VERTB, &VBsignal);
    oo.hist = &ohist;
    bb.hist = &bhist;
    oo.other = &bb;
    bb.other = &oo;
    oo.machine = false;
    bb.machine = true;
}



void StopThinking(void)
{
    if (thinking) {
		abort_think = true;
		Wait(bit(sigdone));		/* thinking = false */
		abort_think = false;
    }
}



void StartThinking(void)
{
    StopThinking();
    suggx = suggy = -1;
    abort_think = false;
    notadrill = turn->machine;
    depth = notadrill ? difficulty : 3;
    /* just an initial value that   ^^^    goes up with time */
    startedthinking = Now();
    thinking = true;
    SetTaskPri(child, notadrill ? BACKPRI : HINTPRI);
    Signal(child, sigfthink);
}



short xgoals[6] = { 4, 6, 6, 1, 1, 3 };
short ygoals[6] = { 7, 6, 1, 1, 6, 7 };

void SetGoal(piece *p)
{
    short i;
    i = (p == &oo ? p->reached + 1 : 4 - p->reached);
    p->goalx = xgoals[i];
    p->goaly = ygoals[i];
}



short xoffs[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
short yoffs[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };

#define NX(x, i) ((x) + xoffs[i])
#define NY(y, i) ((y) + yoffs[i])



void Allow(piece *from, piece *tu)
{
    register short i, x , y;
    ubyte r = 0;
    for (i = 0; i < 8; i++) {
		x = NX(tu->x, i);
		y = NY(tu->y, i);
		if (!(x & ~7) && !(y & ~7) && !(x == from->x && y == from->y))
		    r |= 1 << i;
    }
    tu->allowed = r & board[from->x][from->y];
}



void Restart(void)
{
    Shuffle();
    KillAnts();
    oo.x = 4;
    bb.x = 3;
    oo.y = bb.y = 7;
    Allow(&bb, &oo);
    Allow(&oo, &bb);
    oo.reached = bb.reached = 0;
    SetGoal(&oo);
    SetGoal(&bb);
    ohist.top = bhist.top = HISTORY - 1;
    ohist.count = bhist.count = 0;
    turn = bluefirst ? &bb : &oo;
    bluefirst = !bluefirst;
    won = false;
    StartThinking();
}



void Mooove(piece *who, short x, short y)
{
    register history *gh = who->hist;

    if (gh->top < HISTORY - 1)
		gh->top++;
    else gh->top = 0;
    if (gh->count < HISTORY)
		gh->count++;
    gh->hx[gh->top] = who->x;
    gh->hy[gh->top] = who->y;
    gh->madegoal[gh->top] = x == who->goalx && y == who->goaly;
    who->x = x;
    who->y = y;
    Allow(who, who->other);
    if (gh->madegoal[gh->top])
		if (!(won = ++who->reached > 4))
		    SetGoal(who);
}



bool TryMove(piece *who, short x, short y)
{
    short i, xd, yd;

    xd = x - who->x;
    yd = y - who->y;
    if (!xd && !yd)
		return true;				/* give player another try */
    if (xd > 1 || xd < -1 || yd > 1 || yd < -1)
		return false;				/* moved too far */
    if (!xd)
		i = yd > 0 ? 4 : 0;
    else if (xd > 0)
		i = 2 + yd;
    else
		i = 6 - yd;
    if (!(who->allowed & (1 << i)))
		return false;				/* forbidden move */
    KillAnts();
    StopThinking();
    Mooove(who, x, y);
    turn = turn->other;
    if (!won)
		StartThinking();
    TellTurn();
    return true;
}



#define max(a, b) ((a) > (b) ? (a) : (b))

#define abs(a) ((a) < 0 ? -(a) : (a))

short WayToGo(piece *p)
{
    register short d, dx, dy;

    dx = abs(p->x - p->goalx);
    dy = abs(p->y - p->goaly);
    d = max(dx, dy);
    if (p->reached < 4)
		d += 5 * (4 - p->reached) - 2;
    return d;
}



/* true if p's last move makes a forbidden loop - twice through is okay,
starting on a third time through is forbidden.  Does not detect loops longer
than HISTORY / 2. */

bool Loopin(piece *p)
{
    short len, pt, ot, pi, oi, n;
    history *hp = p->hist, *ho = p->other->hist;

    for (len = 2; len <= HISTORY >> 1; len++) {
		if (hp->count < len << 1)
		    break;
		pt = (hp->top + HISTORY + 1 - len) % HISTORY;
		ot = (ho->top + HISTORY + 1 - len) % HISTORY;
		if (p->other->x == ho->hx[ot] && p->other->y == ho->hy[ot]
							&& p->x == hp->hx[pt] && p->y == hp->hy[pt]) {
		    /* possible loop -- confirm: */
		    pi = hp->top;
		    oi = ho->top;
		    for (n = 0; n < len; n++) {
				pt = (pt + HISTORY - 1) % HISTORY;
				ot = (ot + HISTORY - 1) % HISTORY;
				if (hp->hx[pt] != hp->hx[pi] || hp->hy[pt] != hp->hy[pi] ||
						 ho->hx[ot] != ho->hx[oi] || ho->hy[ot] != ho->hy[oi])
				    break;
				pi = (pi + HISTORY - 1) % HISTORY;
				oi = (oi + HISTORY - 1) % HISTORY;
		    }
	    	if (n >= len)
				return true;
		}
    }
    return false;
}



/* here we have the recursive lookahead strategy -- this function tries all
the moves available to player "me" against player "it" and sets rx and ry to
the best move available to "me", and returns a number that rates how well off
"me" ends up.  It abandons computation and returns meaningless values if the
global variable abort_think ever becomes true.  It does recursive lookahead
to depth "d". */

#define EXTREME 9999

short PickBestMove(short d, piece *me, piece *it, short *rx, short *ry)
{
    short i, besti, best = -EXTREME, t, go, bgo = 999;
    short otop, ntop, oxx, oyy, oco;
    piece p, *oooth;
    history *h = me->hist;

    if (!me->allowed)
		return -EXTREME;			/* I lose */
    d--;
    if (rx) {
		t = 0;
		for (i = 0; i <= 7; i++)
		    if (me->allowed & (1 << i)) {
				t++;
				*rx = NX(me->x, i);
				*ry = NY(me->y, i);
		    }
		if (t == 1)
		    return 0;     /* only legal move, skip other thinking */
    }

    otop = h->top;
    oco = h->count;
    if (h->count < HISTORY)
		h->count++;
    if (otop < HISTORY - 1)
		ntop = otop + 1;
    else ntop = 0;
    h->top = ntop;
    oxx = h->hx[ntop];
    oyy = h->hy[ntop];
    h->hx[ntop] = me->x;
    h->hy[ntop] = me->y;
    oooth = it->other;
    it->other = &p;

    for (i = 0; i <= 7; i++)
		if (me->allowed & (1 << i)) {
		    if (abort_think)
				return 0;
		    p = *me;
		    p.x = NX(p.x, i);
		    p.y = NY(p.y, i);
		    if (p.x == p.goalx && p.y == p.goaly)
				if (++p.reached > 4) {
				    if (rx)
						*rx = p.x, *ry = p.y;
				    best = EXTREME;		/* I win */
				    break;
				} else
				    SetGoal(&p);
		    if (d <= 0)
				t = WayToGo(it) - WayToGo(&p);
		    else {
				ubyte a = it->allowed;
				Allow(&p, it);
				t = - PickBestMove(d, it, &p, null, null);
				it->allowed = a;
		    }			   /* VVV oops, forgot that! */
		    if (rx && looprevent && notadrill && Loopin(&p))
				t -= EXTREME << 1;
		    go = WayToGo(&p);
		    if (t > best || (t == best && go < bgo)) {
				best = t;
				besti = i;
				bgo = go;
				if (rx)
				    *rx = p.x, *ry = p.y;
		    }
		}

    it->other = oooth;
    h->hx[ntop] = oxx;
    h->hy[ntop] = oyy;
    h->count = oco;
    h->top = otop;
    return best;
}



/* the following is started up as a separate task to calculate the best move.
When it gets the "sigfthink" signal it looks in the global variables for what
to think about.  It signals the parent when finished or aborted.  When not
aborted it puts the move that the player whose turn it is should make in the
global vars thinkx and thinky. */

void ThinkerTask(void)
{
    piece who, whom;
    history wo, wm;

    geta4();
    for (;;) {
		if (Wait(sigfthink | sigfdie) & sigfdie) {
		    Forbid();
		    Signal(parent, bit(sigkillme));
		    Wait(0L);
		}
		thinking = true;
		who = *turn;
		whom = *who.other;
		who.other = &whom;
		whom.other = &who;
		wo = *who.hist;
		wm = *whom.hist;
		who.hist = &wo;
		whom.hist = &wm;
		PickBestMove(depth, &who, &whom, &thinkx, &thinky);
		Disable();
		thinking = false;
		Signal(parent, bit(sigdone));
		Enable();
    }
}



void MachineMove(void)
{
    short oldx, oldy;

    oldx = turn->x;
    oldy = turn->y;
    Mooove(turn, thinkx, thinky);
    LiftBob(turn);
    DrawSquare(oldx, oldy);		/* erase old image from rastport */
    DropBob(turn);				/* draw in new position */
    turn = turn->other;
    if (!won)
		StartThinking();
    TellTurn();
}



void _main(void)
{
    struct Process *me = ThisProcess();
    if (!me->pr_CLI) {
		WaitPort(&me->pr_MsgPort);
		wbs = (struct WBStartup *) GetMsg(&me->pr_MsgPort);
    }
    Init();
    Restart();
    ShowBoard();
    if (me->pr_Task.tc_Node.ln_Pri < FOREPRI)
		oldpri = SetTaskPri((struct Task *) me, FOREPRI);
    Play();
    Die(null);
}
