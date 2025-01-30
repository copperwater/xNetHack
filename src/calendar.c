/* NetHack 3.7	calendar.c	$NHDT-Date: 1706213796 2024/01/25 20:16:36 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.116 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2007. */
/* Copyright (c) Robert Patrick Rankin, 1991                      */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/*
 * Time routines
 *
 * The time is used for:
 *  - seed for rand()
 *  - year on tombstone and yyyymmdd in record file
 *  - phase of the moon (various monsters react to NEW_MOON or FULL_MOON)
 *  - night and midnight (the undead are dangerous at midnight)
 *  - determination of what files are "very old"
 */

/* TIME_type: type of the argument to time(); we actually use &(time_t);
   you might need to define either or both of these to 'long *' in *conf.h */
#ifndef TIME_type
#define TIME_type time_t *
#endif
#ifndef LOCALTIME_type
#define LOCALTIME_type time_t *
#endif

staticfn struct tm *getlt(void);
static int weekday(void);
static int days_since_epoch(int);

time_t
getnow(void)
{
    time_t datetime = 0;

    (void) time((TIME_type) &datetime);
    return datetime;
}

staticfn struct tm *
getlt(void)
{
    time_t date = getnow();

    return localtime((LOCALTIME_type) &date);
}

int
getyear(void)
{
    return (1900 + getlt()->tm_year);
}


long
yyyymmdd(time_t date)
{
    long datenum;
    struct tm *lt;

    if (date == 0)
        lt = getlt();
    else
        lt = localtime((LOCALTIME_type) &date);

    /* just in case somebody's localtime supplies (year % 100)
       rather than the expected (year - 1900) */
    if (lt->tm_year < 70)
        datenum = (long) lt->tm_year + 2000L;
    else
        datenum = (long) lt->tm_year + 1900L;
    /* yyyy --> yyyymm */
    datenum = datenum * 100L + (long) (lt->tm_mon + 1);
    /* yyyymm --> yyyymmdd */
    datenum = datenum * 100L + (long) lt->tm_mday;
    return datenum;
}

long
hhmmss(time_t date)
{
    long timenum;
    struct tm *lt;

    if (date == 0)
        lt = getlt();
    else
        lt = localtime((LOCALTIME_type) &date);

    timenum = lt->tm_hour * 10000L + lt->tm_min * 100L + lt->tm_sec;
    return timenum;
}

char *
yyyymmddhhmmss(time_t date)
{
    long datenum;
    static char datestr[15];
    struct tm *lt;

    if (date == 0)
        lt = getlt();
    else
        lt = localtime((LOCALTIME_type) &date);

    /* just in case somebody's localtime supplies (year % 100)
       rather than the expected (year - 1900) */
    if (lt->tm_year < 70)
        datenum = (long) lt->tm_year + 2000L;
    else
        datenum = (long) lt->tm_year + 1900L;
    Snprintf(datestr, sizeof datestr, "%04ld%02d%02d%02d%02d%02d",
             datenum, lt->tm_mon + 1,
             lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
    //debugpline1("yyyymmddhhmmss() produced date string %s", datestr);
    return datestr;
}

time_t
time_from_yyyymmddhhmmss(char *buf)
{
    int k;
    time_t timeresult = (time_t) 0;
    struct tm t, *lt;
    char *d, *p, y[5], mo[3], md[3], h[3], mi[3], s[3];

    if (buf && strlen(buf) == 14) {
        d = buf;
        p = y; /* year */
        for (k = 0; k < 4; ++k)
            *p++ = *d++;
        *p = '\0';
        p = mo; /* month */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = md; /* day */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = h; /* hour */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = mi; /* minutes */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = s; /* seconds */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        lt = getlt();
        if (lt) {
            t = *lt;
            t.tm_year = atoi(y) - 1900;
            t.tm_mon = atoi(mo) - 1;
            t.tm_mday = atoi(md);
            t.tm_hour = atoi(h);
            t.tm_min = atoi(mi);
            t.tm_sec = atoi(s);
            timeresult = mktime(&t);
        }
        if (timeresult == (time_t) -1)
            ;
#if 0
TODO: set_debugpline1, debugpline1 -> function pointer
            debugpline1("time_from_yyyymmddhhmmss(%s) would have returned -1",
                        buf ? buf : "");
#endif
        else
            return timeresult;
    }
    return (time_t) 0;
}

/*
 * moon period = 29.53058 days ~= 30, year = 365.2422 days
 * days moon phase advances on first day of year compared to preceding year
 *      = 365.2422 - 12*29.53058 ~= 11
 * years in Metonic cycle (time until same phases fall on the same days of
 *      the month) = 18.6 ~= 19
 * moon phase on first day of year (epact) ~= (11*(year%19) + 29) % 30
 *      (29 as initial condition)
 * current phase in days = first day phase + days elapsed in year
 * 6 moons ~= 177 days
 * 177 ~= 8 reported phases * 22
 * + 11/22 for rounding
 */
int
phase_of_the_moon(void) /* 0-7, with 0: new, 4: full */
{
    struct tm *lt = getlt();
    int epact, diy, goldn;

    diy = lt->tm_yday;
    goldn = (lt->tm_year % 19) + 1;
    epact = (11 * goldn + 18) % 30;
    if ((epact == 25 && goldn > 11) || epact == 24)
        epact++;

    return ((((((diy + epact) * 6) + 11) % 177) / 22) & 7);
}

boolean
friday_13th(void)
{
    struct tm *lt = getlt();

    /* tm_wday (day of week; 0==Sunday) == 5 => Friday */
    return (boolean) (lt->tm_wday == 5 && lt->tm_mday == 13);
}

int
night(void)
{
    int hour = getlt()->tm_hour;

    return (hour < 6 || hour > 21);
}

int
midnight(void)
{
    return (getlt()->tm_hour == 0);
}

/* Returns current day of week (0==Sunday through 6==Saturday) */
static int
weekday(void)
{
    struct tm *lt = getlt();
    return lt->tm_wday;
}

/* Return the number of days since 01/01/0000 on the Gregorian calendar,
 * inclusive of the start date but not the current date.
 * Argument should be a yyyymmdd int. */
static int
days_since_epoch(int ymd)
{
    const int year = ymd / 10000;
    const int month = (ymd % 10000) / 100;
    const int date = ymd % 100;
    int monthlen[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    int i;

    /* insert Feb 29 if this is a leap year */
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        monthlen[1] += 1;
    }

    /* baseline: 365 days per year */
    int days = 365 * year;

    /* Gregorian leap years.
     * These will ignore that year 0 was a leap year, but that shouldn't matter
     * if all you're doing with this function is subtracting two epoch dates
     * from each other.
     * This does year - 1 so as NOT to count the current year, which the end of
     * February may or may not have happened in yet. */
    /* 1 per year divisible by 4. */
    days += (year - 1) / 4;
    /* Minus a leap year for each year divisible by 100. */
    days -= (year - 1) / 100;
    /* Plus a leap year for each year divisible by 400. */
    days += (year - 1) / 400;
    /* Plus the partial amount of days this year. */
    for (i = 0; i < (month - 1); ++i) {
        days += monthlen[i];
    }
    /* Plus the amount of days so far this month. */
    days += date;
    return days;
}

/* Return a bitmask of holidays it is today.
 * This uses a bitmask rather than an enum so that it can extend to "seasons"
 * which extend over multiple days and possibly intersect with different
 * holidays (for instance, Hanukkah can overlap with Christmas). */
int
current_holidays(void)
{
    const int ymd = yyyymmdd((time_t) 0);
    const int year = ymd / 10000;
    const int month = (ymd % 10000) / 100; /* 1..12 */
    const int date = ymd % 100;            /* 1..31 */
    const int today_epoch = days_since_epoch(ymd);
    const int hour = getlt()->tm_hour;
    int retmask = 0;

    /* These cache the value of the last time this function did a full holiday
     * computation, to avoid recomputing it all over again unless needed. */
    static int cached_ymd = 0, cached_hour = 0, cached_retmask = 0;
    boolean recompute = FALSE;
    /* recompute if:
     * 1. first ever call (no need to check cached_hour == -1 because cached_ymd
     *    of 0 is always less than ymd in that case)
     * 2. date has changed
     * 3. this was last called before 6 pm and it is now after 6 pm (this
     *    changes the Jewish day which can begin or end a holiday, see below)
     */
    if (cached_ymd < ymd || (cached_hour < 18 && hour >= 18)) {
        recompute = TRUE;
    }
    cached_ymd = ymd;
    cached_hour = hour;
    if (!recompute) {
        return cached_retmask;
    }

    /* Simple holidays observed yearly on the Gregorian calendar. */
    if (month == 1 && date == 1) {
        retmask |= HOLIDAY_NEW_YEARS;
    }
    if (month == 2 && date == 2) {
        retmask |= HOLIDAY_GROUNDHOG_DAY;
    }
    if (month == 2 && date == 14) {
        retmask |= HOLIDAY_VALENTINES_DAY;
    }
    if (month == 3 && date == 14) {
        retmask |= HOLIDAY_PI_DAY;
    }
    if (month == 4 && date == 1) {
        retmask |= HOLIDAY_APRIL_FOOLS;
    }
    if (month == 7 && date == 1) {
        retmask |= HOLIDAY_CANADA_DAY;
    }
    if (month == 10 && date == 31) {
        retmask |= HOLIDAY_HALLOWEEN;
    }
    if (month == 11 && date >= 1 && date <= 2) {
        retmask |= HOLIDAY_LOS_MUERTOS;
    }
    if (month == 11 && date >= 22 && date <= 28 && weekday() == 4) {
        retmask |= HOLIDAY_THANKSGIVING;
    }
    if (month == 12 && date >= 24 && date <= 25) {
        /* counts Christmas Eve too */
        retmask |= HOLIDAY_CHRISTMAS;
    }

    /* Now for the tough stuff. */
    {
        /* Modified form of Gauss's algorithm to compute the date of Easter, also
         * known as the "Computus" algorithm. Relies on a not-inconsiderable amount
         * of voodoo magic. */
        int a = year % 19;
        int b = year / 100;
        int c = year % 100;
        int d = b / 4;
        int e = b % 4;
        int f = (b + 8) / 25;
        int g = (b - f + 1) / 3;
        int h = ((19 * a) + b - d - g + 15) % 30;
        int ii = c / 4; /* note: 'i' declared already in this function */
        int k = c % 4;
        int L = (32 + (2 * e) + (2 * ii) - h - k) % 7;
        int m = (a + (11 * h) + (22 * L)) / 451;
        int eastermonth = (h + L - (7 * m) + 114) / 31;
        int easterday = (h + L - (7 * m) + 114) % 31 + 1;
        if (month == eastermonth && date == easterday) {
            retmask |= HOLIDAY_EASTER;
        }
        /* Mardi Gras is based on Easter, 47 days before it */
        {
            int easterymd = year * 10000 + eastermonth * 100 + easterday;
            if (days_since_epoch(easterymd) - today_epoch == 47) {
                retmask |= HOLIDAY_MARDI_GRAS;
            }
        }
    }

    {
        /* The Islamic calendar begins on 16 Jul 622 in the Julian calendar (19
         * July in the Gregorian calendar). This is only one of many rule-based
         * reckonings for it.
         * The (lunar) year contains 12 months, in a 30 29 30 29... pattern,
         * except in leap years where the final month is 30 days. Leap years
         * repeat every 30-year cycle, on years 2, 5, 7, 10, 13, 16, 18, 21, 24,
         * 26, 29. This closely but imperfectly approximates the lunar period.
         * TODO: in year 3000 or so, check if it's drifted off by a day or two,
         * and fix it if it has.
         * Begin by calculating a delta number of days since the start of the
         * Islamic calendar.
         * Every 30-year period contains 10631 days, so we can immediately take
         * that delta modulo 10631 to ignore however many 30-year periods that
         * is.
         * This algorithm is not going to pinpoint Ramadan or other holidays
         * exactly, particularly since they depend on manual observation. But it
         * should get pretty close.
         */
        int lunar_leap[30] = { 0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,
                               1,0,1,0,0,1,0,0,1,0,1,0,0,1,0 };

        /* There are 166 days in the partial year 622. Start with that. */
        int date_delta = today_epoch - days_since_epoch(6220719);
        int cycyear;
        int month_ctr;

        /* Now cut off as many 30-year periods as possible. */
        date_delta = date_delta % 10631;
        /* Then cut off year by year until we reach the current lunar year. */
        for (cycyear = 0; cycyear < 30; ++cycyear) {
            int this_year_len = 354 + lunar_leap[cycyear];
            if (date_delta < this_year_len) {
                break;
                /* cycyear stays in scope so we can tell below if it is
                 * currently a leap year and need to adjust the last month
                 * accordingly */
            }
            date_delta -= this_year_len;
        }
        if (date_delta < 0 || cycyear == 30) {
            impossible("holiday: bad math finding lunar year");
            date_delta = 0;
        }
        /* Then using whatever is remaining, find the month and date of the
         * current day. */
        int islam_month = 0, islam_date = 0;
        for (month_ctr = 0; month_ctr < 12; ++month_ctr) {
            int month_len = (month_ctr % 2 == 1) ? 29 : 30;
            if (month_ctr == 11)
                month_len += lunar_leap[cycyear];
            if (date_delta < month_len) {
                islam_month = month_ctr + 1;        /* convert back to human-readable */
                islam_date = date_delta + 1;
                break;
            }
            date_delta -= month_len;
        }
        if (date_delta < 0 || month_ctr >= 12) {
            impossible("holiday: bad math finding lunar month/date");
        }
        if (islam_month == 9) {
            retmask |= HOLIDAY_RAMADAN;
        }
        if (islam_month == 10 && islam_date == 1) {
            retmask |= HOLIDAY_EID_AL_FITR;
        }
    }

    {
        /* The Hebrew calendar is even more complicated than the Islamic
         * calendar, but the real problem with it is that it depends on
         * obtaining the precise instant of the new moon. This is difficult to
         * do because the lunar synodic period fluctuates, and getting a new
         * moon wrong by even an hour can throw off the date of the new year by
         * up to two days. In lieu of piling a bunch of orbital mechanics
         * calculations on top of the Hebrew calendar to make it work for
         * arbitrary dates, this will just use dates for the Hebrew new year
         * obtained from hebcal.com.
         */
        const int heb_new_year[30] = {
            20200919, 20210907, 20220926, 20230916, 20241003, /* 2020-2024 */
            20250923, 20260912, 20271002, 20280921, 20290910, /* 2025-2029 */
            20300928, 20310918, 20320906, 20330924, 20340914, /* 2030-2034 */
            20351004, 20360922, 20370910, 20380930, 20390919, /* 2035-2039 */
            20400908, 20410926, 20420915, 20431005, 20440922, /* 2040-2044 */
            20450912, 20461001, 20470921, 20480908, 20490927, /* 2045-2049 */
        };

        if (year > 2048) {
            pline("This game is still being played after 2048?  Cool.");
            impossible("no data for Hebrew calendar in year %d", year);
        }
        else if (year < 2020) {
            pline("Time travel to the past?  Or fix your system clock.");
            impossible("no data for Hebrew calendar in year %d", year);
        }
        else {
            int tmp_epoch_today = today_epoch;
            /* The Gregorian day begins at midnight, but the Hebrew day begins
             * at sunset. Assume sunset is at 6 PM; if it's after that, advance
             * the day by 1. */
            if (hour >= 18) {
                tmp_epoch_today += 1;
            }
            /* ymd is no longer safe to use in this computation */

            /* advance index in heb_new_year until it points at the current
             * Hebrew year */
            int index = 0;
            int epoch_last_newyear, epoch_next_newyear;
            do {
                epoch_last_newyear = days_since_epoch(heb_new_year[index]);
                epoch_next_newyear = days_since_epoch(heb_new_year[index + 1]);
                index++;
            } while (epoch_next_newyear <= tmp_epoch_today);

            int heb_year_length = epoch_next_newyear - epoch_last_newyear;

            /* The leap year inserts a 30-day month in the middle of the year,
             * represented by a 0 here. */
            int heb_month_len[13] = { 30,29,30,29,30,0,29,30,29,30,29,30,29 };
            if (heb_year_length >= 383 && heb_year_length <= 385) {
                heb_month_len[5] = 30; /* insert Adar I */
            }
            else if (heb_year_length < 353 || heb_year_length > 355) {
                /* if not a leap year, year must be 353 to 355 days */
                impossible("illegal Hebrew year length %d", heb_year_length);
                heb_year_length = 354; /* try for graceful fallback */
            }
            if (heb_year_length % 10 == 3) { /* short year */
                heb_month_len[2] -= 1; /* deduct 1 day from Kislev */
            }
            else if (heb_year_length % 10 == 5) { /* full year */
                heb_month_len[1] += 1; /* add 1 day to Cheshvan */
            }
            int hebrew_month = 0, hebrew_date = 0;
            int date_delta = tmp_epoch_today - epoch_last_newyear;
            int month_ctr;
            for (month_ctr = 0; month_ctr < 13; ++month_ctr) {
                if (date_delta < heb_month_len[month_ctr]) {
                    hebrew_month = month_ctr + 1;
                    hebrew_date = date_delta + 1;
                    break;
                }
                date_delta -= heb_month_len[month_ctr];
            }
            if (date_delta < 0 || month_ctr == 13) {
                impossible("holiday: bad math finding hebrew month/date");
            }
            if (hebrew_month == 1 && hebrew_date >= 1 && hebrew_date <= 2) {
                retmask |= HOLIDAY_ROSH_HASHANAH;
            }
            if (hebrew_month == 1 && hebrew_date == 10) {
                retmask |= HOLIDAY_YOM_KIPPUR;
            }
            /* There are a number of different ways to observe Passover; this
             * tracks the first two days but not the rest of its week. */
            if (hebrew_month == 8 && hebrew_date >= 15 && hebrew_date <= 16) {
                retmask |= HOLIDAY_PASSOVER;
            }
            /* Judging the end date of Hanukkah depends on whether Kislev was
             * reduced in length or not. */
            if ((hebrew_month == 3 && hebrew_date >= 25)
                || (hebrew_month == 4
                    && (hebrew_date <= (heb_month_len[2] == 30 ? 2 : 3)))) {
                retmask |= HOLIDAY_HANUKKAH;
            }
        }
    }
    cached_retmask = retmask; /* for next time */
    return retmask;
}

/* calendar.c */

