/* a few global definitions for the game Trippin. */


#include <Paul.h>


#define HISTORY 100

typedef struct {
    short top, count;
    ubyte hx[HISTORY], hy[HISTORY], madegoal[HISTORY];
} history;


typedef struct _pIeCe {
    short x, y, goalx, goaly;
    bool machine;
    ubyte reached, allowed;
    struct Bob *face;
    struct _pIeCe *other;
    history *hist;
} piece;


#define sigfdie   SIGBREAKF_CTRL_C
#define sigfthink SIGBREAKF_CTRL_F

#define sigftof   SIGBREAKF_CTRL_E


#define SQIZE 22

#define IMHITE 26
#define IMWID  22

#define MARGINWID 160
