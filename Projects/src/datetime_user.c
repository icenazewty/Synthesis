#include "time.h"

#define _BASE_DOW          4

#define _DAY_SEC           (24L * 60L * 60L)
int _lpdays[] = {
        -1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

int _days[] = {
        -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
};

#define _YEAR_SEC          (365L * _DAY_SEC)    /* secs in a year */
#define _FOUR_YEAR_SEC     (1461L * _DAY_SEC)   /* secs in a 4 year interval */

struct tm tb;
struct tm *   gmtime (
        const time_t *timp
        )
{

        long caltim = *timp;            /* calendar time to convert */
        int islpyr = 0;                 /* is-current-year-a-leap-year flag */
          int tmptim;
          int *mdays;                /* pointer to days or lpdays */

#if 0 //def _MT

         struct tm *ptb;            /* will point to gmtime buffer */
        _ptiddata ptd = _getptd();

#else  /* _MT */
         struct tm *ptb = &tb;
#endif  /* _MT */

        if ( caltim < 0L )
                return(NULL);

#ifdef _MT

        /* Use per thread buffer area (malloc space, if necessary) */

        if ( (ptd->_gmtimebuf != NULL) || ((ptd->_gmtimebuf =
            _malloc_crt(sizeof(struct tm))) != NULL) )
                ptb = ptd->_gmtimebuf;
        else
                ptb = &tb;      /* malloc error: use static buffer */

#endif  /* _MT */

        /*
         * Determine years since 1970. First, identify the four-year interval
         * since this makes handling leap-years easy (note that 2000 IS a
         * leap year and 2100 is out-of-range).
         */
        tmptim = (int)(caltim / _FOUR_YEAR_SEC);
        caltim -= ((long)tmptim * _FOUR_YEAR_SEC);

        /*
         * Determine which year of the interval
         */
        tmptim = (tmptim * 4) + 70;         /* 1970, 1974, 1978,...,etc. */

        if ( caltim >= _YEAR_SEC ) {

            tmptim++;                       /* 1971, 1975, 1979,...,etc. */
            caltim -= _YEAR_SEC;

            if ( caltim >= _YEAR_SEC ) {

                tmptim++;                   /* 1972, 1976, 1980,...,etc. */
                caltim -= _YEAR_SEC;

                /*
                 * Note, it takes 366 days-worth of seconds to get past a leap
                 * year.
                 */
                if ( caltim >= (_YEAR_SEC + _DAY_SEC) ) {

                        tmptim++;           /* 1973, 1977, 1981,...,etc. */
                        caltim -= (_YEAR_SEC + _DAY_SEC);
                }
                else {
                        /*
                         * In a leap year after all, set the flag.
                         */
                        islpyr++;
                }
            }
        }

        /*
         * tmptim now holds the value for tm_year. caltim now holds the
         * number of elapsed seconds since the beginning of that year.
         */
        ptb->tm_year = tmptim;

        /*
         * Determine days since January 1 (0 - 365). This is the tm_yday value.
         * Leave caltim with number of elapsed seconds in that day.
         */
        ptb->tm_yday = (int)(caltim / _DAY_SEC);
        caltim -= (long)(ptb->tm_yday) * _DAY_SEC;

        /*
         * Determine months since January (0 - 11) and day of month (1 - 31)
         */
        if ( islpyr )
            mdays = _lpdays;
        else
            mdays = _days;


        for ( tmptim = 1 ; mdays[tmptim] < ptb->tm_yday ; tmptim++ ) ;

        ptb->tm_mon = --tmptim;

        ptb->tm_mday = ptb->tm_yday - mdays[tmptim];

        /*
         * Determine days since Sunday (0 - 6)
         */
        ptb->tm_wday = ((int)(*timp / _DAY_SEC) + _BASE_DOW) % 7;

        /*
         *  Determine hours since midnight (0 - 23), minutes after the hour
         *  (0 - 59), and seconds after the minute (0 - 59).
         */
        ptb->tm_hour = (int)(caltim / 3600);
        caltim -= (long)ptb->tm_hour * 3600L;

        ptb->tm_min = (int)(caltim / 60);
        ptb->tm_sec = (int)(caltim - (ptb->tm_min) * 60);

        ptb->tm_isdst = 0;
        return( (struct tm *)ptb );

}

struct tm * localtime2 (
    const 
	time_t *ptime2
){
	time_t time3;
        register struct tm *ptm;
        long ltime;

        /*
         * Check for illegal time_t value
         */
        if ( (long)*ptime2 < 0L )
                return( NULL );

//#ifdef _WIN32
        //__tzset();
//#else  /* _WIN32 */
//#if defined (_M_MPPC) || defined (_M_M68K)
        //_tzset();
//#endif  /* defined (_M_MPPC) || defined (_M_M68K) */
//#endif  /* _WIN32 */
//#if 0
//        if ( (*ptime > 3 * _DAY_SEC) && (*ptime < LONG_MAX - 3 * _DAY_SEC) ) {
//                /*
//                 * The date does not fall within the first three, or last
//                 * three, representable days of the Epoch. Therefore, there
//                 * is no possibility of overflowing or underflowing the
//                 * time_t representation as we compensate for timezone and
//                 * Daylight Savings Time.
//                 */
//                ltime = (long)*ptime - _timezone;
//                ptm = gmtime( (time_t *)&ltime );
//                /*
//                 * Check and adjust for Daylight Saving Time.
//                 */
//                if ( _daylight && _isindst( ptm ) ) {
//                        ltime -= _dstbias;
//                        ptm = gmtime( (time_t *)&ltime );
//                        ptm->tm_isdst = 1;
//                }
//        }
//        else 
//#endif					
					{
								time3 = (*ptime2);//	 + 8*3600;//Zone 8 adjust 
                ptm = gmtime( &time3 );

                /*
                 * The date falls with the first three, or last three days
                 * of the Epoch. It is possible the time_t representation
                 * would overflow or underflow while compensating for
                 * timezone and Daylight Savings Time. Therefore, make the
                 * timezone and Daylight Savings Time adjustments directly
                 * in the tm structure. The beginning of the Epoch is
                 * 00:00:00, 01-01-70 (UTC) and the last representable second
                 * in the Epoch is 03:14:07, 01-19-2038 (UTC). This will be
                 * used in the calculations below.
                 *
                 * First, adjust for the timezone.
                 */
//                if ( _isindst(ptm) )
//                        ltime = (long)ptm->tm_sec - (_timezone + _dstbias);
//                else
//                        ltime = (long)ptm->tm_sec - _timezone;
//                ptm->tm_sec = (int)(ltime % 60);
//                if ( ptm->tm_sec < 0 ) {
//                        ptm->tm_sec += 60;
//                        ltime -= 60;
//                }
//                ltime = (long)ptm->tm_min + ltime/60;
//                ptm->tm_min = (int)(ltime % 60);
//                if ( ptm->tm_min < 0 ) {
//                        ptm->tm_min += 60;
//                        ltime -= 60;
//                }
//                ltime = (long)ptm->tm_hour + ltime/60;
//                ptm->tm_hour = (int)(ltime % 24);
//                if ( ptm->tm_hour < 0 ) {
//                        ptm->tm_hour += 24;
//                        ltime -=24;
//                }
//                ltime /= 24;
//                if ( ltime > 0L ) {
//                        /*
//                         * There is no possibility of overflowing the tm_mday
//                         * and tm_yday fields since the date can be no later
//                         * than January 19.
//                         */
//                        ptm->tm_wday = (ptm->tm_wday + ltime) % 7;
//                        ptm->tm_mday += ltime;
//                        ptm->tm_yday += ltime;
//                }
//                else if ( ltime < 0L ) {
//                        /*
//                         * It is possible to underflow the tm_mday and tm_yday
//                         * fields. If this happens, then adjusted date must
//                         * lie in December 1969.
//                         */
//                        ptm->tm_wday = (ptm->tm_wday + 7 + ltime) % 7;
//                        if ( (ptm->tm_mday += ltime) <= 0 ) {
//                                ptm->tm_mday += 31;
//                                ptm->tm_yday = 364;
//                                ptm->tm_mon = 11;
//                                ptm->tm_year--;
//                        }
//                        else {
//                                ptm->tm_yday += ltime;
//                        }
//                }


        }
        return(ptm);
}

#define ChkAdd(dest, src1, src2)   ( ((src1 >= 0L) && (src2 >= 0L) \
    && (dest < 0L)) || ((src1 < 0L) && (src2 < 0L) && (dest >= 0L)) )
#define _BASE_YEAR         70L                  /* 1970 is the base year */
#define _MAX_YEAR          138L                 /* 2038 is the max year */
#define _LEAP_YEAR_ADJUST  17L                  /* Leap years 1900 - 1970 */
#define ChkMul(dest, src1, src2)   ( src1 ? (dest/src1 != src2) : 0 )

static time_t   _make_time_t (
        struct tm *tb,
        int ultflag
        )
{
        long tmptm1, tmptm2, tmptm3;
        struct tm *tbtemp;

        /*
         * First, make sure tm_year is reasonably close to being in range.
         */
        if ( ((tmptm1 = tb->tm_year) < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR
          + 1) )
            goto err_mktime;


        /*
         * Adjust month value so it is in the range 0 - 11.  This is because
         * we don't know how many days are in months 12, 13, 14, etc.
         */

        if ( (tb->tm_mon < 0) || (tb->tm_mon > 11) ) {

            /*
             * no danger of overflow because the range check above.
             */
            tmptm1 += (tb->tm_mon / 12);

            if ( (tb->tm_mon %= 12) < 0 ) {
                tb->tm_mon += 12;
                tmptm1--;
            }

            /*
             * Make sure year count is still in range.
             */
            if ( (tmptm1 < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR + 1) )
                goto err_mktime;
        }

        /***** HERE: tmptm1 holds number of elapsed years *****/

        /*
         * Calculate days elapsed minus one, in the given year, to the given
         * month. Check for leap year and adjust if necessary.
         */
        tmptm2 = _days[tb->tm_mon];
        if ( !(tmptm1 & 3) && (tb->tm_mon > 1) )
                tmptm2++;

        /*
         * Calculate elapsed days since base date (midnight, 1/1/70, UTC)
         *
         *
         * 365 days for each elapsed year since 1970, plus one more day for
         * each elapsed leap year. no danger of overflow because of the range
         * check (above) on tmptm1.
         */
        tmptm3 = (tmptm1 - _BASE_YEAR) * 365L + ((tmptm1 - 1L) >> 2)
          - _LEAP_YEAR_ADJUST;

        /*
         * elapsed days to current month (still no possible overflow)
         */
        tmptm3 += tmptm2;

        /*
         * elapsed days to current date. overflow is now possible.
         */
        tmptm1 = tmptm3 + (tmptm2 = (long)(tb->tm_mday));
        if ( ChkAdd(tmptm1, tmptm3, tmptm2) )
            goto err_mktime;

        /***** HERE: tmptm1 holds number of elapsed days *****/

        /*
         * Calculate elapsed hours since base date
         */
        tmptm2 = tmptm1 * 24L;
        if ( ChkMul(tmptm2, tmptm1, 24L) )
            goto err_mktime;

        tmptm1 = tmptm2 + (tmptm3 = (long)tb->tm_hour);
        if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
            goto err_mktime;

        /***** HERE: tmptm1 holds number of elapsed hours *****/

        /*
         * Calculate elapsed minutes since base date
         */

        tmptm2 = tmptm1 * 60L;
        if ( ChkMul(tmptm2, tmptm1, 60L) )
            goto err_mktime;

        tmptm1 = tmptm2 + (tmptm3 = (long)tb->tm_min);
        if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
            goto err_mktime;

        /***** HERE: tmptm1 holds number of elapsed minutes *****/

        /*
         * Calculate elapsed seconds since base date
         */

        tmptm2 = tmptm1 * 60L;
        if ( ChkMul(tmptm2, tmptm1, 60L) )
            goto err_mktime;

        tmptm1 = tmptm2 + (tmptm3 = (long)tb->tm_sec);
        if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
            goto err_mktime;

        /***** HERE: tmptm1 holds number of elapsed seconds *****/
#if 0
        if  ( ultflag ) {

            /*
             * Adjust for timezone. No need to check for overflow since
             * localtime() will check its arg value
             */

#ifdef _WIN32
            __tzset();
#else  /* _WIN32 */
#if defined (_M_MPPC) || defined (_M_M68K)
            _tzset();
#endif  /* defined (_M_MPPC) || defined (_M_M68K) */
#endif  /* _WIN32 */

            tmptm1 += _timezone;

            /*
             * Convert this second count back into a time block structure.
             * If localtime returns NULL, return an error.
             */
            if ( (tbtemp = localtime(&tmptm1)) == NULL )
                goto err_mktime;

            /*
             * Now must compensate for DST. The ANSI rules are to use the
             * passed-in tm_isdst flag if it is non-negative. Otherwise,
             * compute if DST applies. Recall that tbtemp has the time without
             * DST compensation, but has set tm_isdst correctly.
             */
            if ( (tb->tm_isdst > 0) || ((tb->tm_isdst < 0) &&
              (tbtemp->tm_isdst > 0)) ) {
                tmptm1 += _dstbias;
                tbtemp = localtime(&tmptm1);    /* reconvert, can't get NULL */
            }

        }
        else 
#endif
        {
            if ( (tbtemp = gmtime(&tmptm1)) == NULL )
                goto err_mktime;
        }
        /***** HERE: tmptm1 holds number of elapsed seconds, adjusted *****/
        /*****       for local time if requested                      *****/

        *tb = *tbtemp;
        return (time_t)tmptm1;

err_mktime:
        /*
         * All errors come to here
         */
        return (time_t)(-1);
}
time_t   mktime2 (
        struct tm *tb
        )
{
        return( _make_time_t(tb, 1) );//-8*3600);
}
time_t   _mkgmtime (
        struct tm *tb
        )
{
        return( _make_time_t(tb, 0) );
}
