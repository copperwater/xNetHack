/* NetHack 3.7	extralev.c	$NHDT-Date: 1737345573 2025/01/19 19:59:33 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.28 $ */
/*      Copyright 1988, 1989 by Ken Arromdee                      */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file used to contain the support code for the Rogue level.
 * In xNetHack, this level has been removed thus making the code no longer
 * needed. For a while, this file was just removed entirely.
 * However, I've gotten tired of fixing merge conflicts introduced in various
 * system build files due to the removal of extralev.c and thus extralev.o,
 * which is frequently hardcoded along with all the other source files. So
 * I've brought back extralev.c even though it contains no code.
 */
/* -Werror=pedantic will complain that this file is empty, so provide a dummy
 * typedef to make the compiler happy */
typedef int satisfy_iso_c_compilers;
