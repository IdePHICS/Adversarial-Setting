/* Written by Charles Harris charles.harris@sdl.usu.edu */
/* Modified to this project by Kasimir Tanner kasimir.tanner@gmail.com */

#include <math.h>
#include "zeros.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/*
  At the top of the loop the situation is the following:

    1. the root is bracketed between xa and xb
    2. xa is the most recent estimate
    3. xp is the previous estimate
    4. |fp| < |fb|

  The order of xa and xp doesn't matter, but assume xp < xb. Then xa lies to
  the right of xp and the assumption is that xa is increasing towards the root.
  In this situation we will attempt quadratic extrapolation as long as the
  condition

  *  |fa| < |fp| < |fb|

  is satisfied. That is, the function value is decreasing as we go along.
  Note the 4 above implies that the right inequlity already holds.

  The first check is that xa is still to the left of the root. If not, xb is
  replaced by xp and the interval reverses, with xb < xa. In this situation
  we will try linear interpolation. That this has happened is signaled by the
  equality xb == xp;

  The second check is that |fa| < |fb|. If this is not the case, we swap
  xa and xb and resort to bisection.

*/

// Compilation command used: gcc -shared -o brentq.so brentq.c

double optim(double z, double y, double V, double w_prime)
{
    double a = y*z;
    if (a < 0) {
        return (y*V) / (1.0 + exp(a)) + w_prime - z;
    }
    else {
        return (y*V*exp(-a)) / (1.0 + exp(-a)) + w_prime - z;
    }
}

double brentq(double xa, double xb, double xtol, double rtol,
       int iter, double y, double V, double w_prime)
{
    double xpre = xa, xcur = xb;
    double xblk = 0., fpre, fcur, fblk = 0., spre = 0., scur = 0., sbis;
    /* the tolerance is 2*delta */
    double delta;
    double stry, dpre, dblk;
    int i;
    
    fpre = optim(xpre, y, V, w_prime);
    fcur = optim(xcur, y, V, w_prime);
    
    if (fpre == 0) {
        return xpre;
    }
    if (fcur == 0) {
        return xcur;
    }
    if (signbit(fpre)==signbit(fcur)) {
        return 0.;
    }
    
    for (i = 0; i < iter; i++) {
        
        if (fpre != 0 && fcur != 0 &&
	    (signbit(fpre) != signbit(fcur))) {
            xblk = xpre;
            fblk = fpre;
            spre = scur = xcur - xpre;
        }
        if (fabs(fblk) < fabs(fcur)) {
            xpre = xcur;
            xcur = xblk;
            xblk = xpre;

            fpre = fcur;
            fcur = fblk;
            fblk = fpre;
        }

        delta = (xtol + rtol*fabs(xcur))/2;
        sbis = (xblk - xcur)/2;
        if (fcur == 0 || fabs(sbis) < delta) {
            
            return xcur;
        }

        if (fabs(spre) > delta && fabs(fcur) < fabs(fpre)) {
            if (xpre == xblk) {
                /* interpolate */
                stry = -fcur*(xcur - xpre)/(fcur - fpre);
            }
            else {
                /* extrapolate */
                dpre = (fpre - fcur)/(xpre - xcur);
                dblk = (fblk - fcur)/(xblk - xcur);
                stry = -fcur*(fblk*dblk - fpre*dpre)
                    /(dblk*dpre*(fblk - fpre));
            }
            if (2*fabs(stry) < MIN(fabs(spre), 3*fabs(sbis) - delta)) {
                /* good short step */
                spre = scur;
                scur = stry;
            } else {
                /* bisect */
                spre = sbis;
                scur = sbis;
            }
        }
        else {
            /* bisect */
            spre = sbis;
            scur = sbis;
        }

        xpre = xcur; fpre = fcur;
        if (fabs(scur) > delta) {
            xcur += scur;
        }
        else {
            xcur += (sbis > 0 ? delta : -delta);
        }

        fcur = optim(xcur, y, V, w_prime);
        
    }
    
    return xcur;
}
