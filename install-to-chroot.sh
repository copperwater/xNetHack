#!/bin/bash
# 
# Ideas and some parts from the original dgl-create-chroot (by joshk@triplehelix.org, modifications by jilles@stack.nl)
# More by <paxed@alt.org>
# More by Michael Andrew Streib <dtype@dtype.org>
# Licensed under the MIT License
# https://opensource.org/licenses/MIT

# autonamed chroot directory. Can rename.
DATESTAMP=`date +%Y%m%d-%H%M%S`
NAO_CHROOT="/opt/nethack/chroot"
# config outside of chroot
DGL_CONFIG="/opt/nethack/dgamelaunch.conf"
# already compiled versions of dgl and nethack
DGL_GIT="/home/build/dgamelaunch"
NETHACK_GIT="/home/build/NetHack37"
# the user & group from dgamelaunch config file.
USRGRP="games:games"
# COMPRESS from include/config.h; the compression binary to copy. leave blank to skip.
COMPRESSBIN="/bin/gzip"
# fixed data to copy (leave blank to skip)
NH_GIT="/home/build/NetHack37"
NH_BRANCH="3.7-hdf" # will probably need to make another hdf branch for this one
# HACKDIR from include/config.h; aka nethack subdir inside chroot
NHSUBDIR="nh370a-hdf"
# VAR_PLAYGROUND from include/unixconf.h
NH_VAR_PLAYGROUND="/nh370a-hdf/var/"
# only define this if dgl was configured with --enable-sqlite
SQLITE_DBFILE="/dgldir/dgamelaunch.db"
# END OF CONFIG
##############################################################################

errorexit()
{
    echo "Error: $@" >&2
    exit 1
}

findlibs()
{
  for i in "$@"; do
      if [ -z "`ldd "$i" | grep 'not a dynamic executable'`" ]; then
         echo $(ldd "$i" | awk '{ print $3 }' | egrep -v ^'\(' | grep lib)
         echo $(ldd "$i" | grep 'ld-linux' | awk '{ print $1 }')
      fi
  done
}

set -e

umask 022

echo "Creating inprogress and extrainfo directories"
mkdir -p "$NAO_CHROOT/dgldir/inprogress-nh370-hdf"
chown "$USRGRP" "$NAO_CHROOT/dgldir/inprogress-nh370-hdf"
mkdir -p "$NAO_CHROOT/dgldir/extrainfo-nh370"
chown "$USRGRP" "$NAO_CHROOT/dgldir/extrainfo-nh370"

echo "Making $NAO_CHROOT/$NHSUBDIR"
mkdir -p "$NAO_CHROOT/$NHSUBDIR"

NETHACKBIN="$NETHACK_GIT/src/nethack"
if [ -n "$NETHACKBIN" -a ! -e "$NETHACKBIN" ]; then
  errorexit "Cannot find NetHack binary $NETHACKBIN"
fi

if [ -n "$NETHACKBIN" -a -e "$NETHACKBIN" ]; then
  echo "Copying $NETHACKBIN"
  cd "$NAO_CHROOT/$NHSUBDIR"
  NHBINFILE="`basename $NETHACKBIN`-$DATESTAMP"
  cp "$NETHACKBIN" "$NHBINFILE"
  ln -fs "$NHBINFILE" nethack
  LIBS="$LIBS `findlibs $NETHACKBIN`"
  cd "$NAO_CHROOT"
fi

echo "Copying NetHack playground stuff"
cp "$NETHACK_GIT/dat/nhdat" "$NAO_CHROOT/$NHSUBDIR"
chmod 644 "$NAO_CHROOT/$NHSUBDIR/nhdat"
cp "$NETHACK_GIT/dat/symbols" "$NAO_CHROOT/$NHSUBDIR"
chmod 644 "$NAO_CHROOT/$NHSUBDIR/symbols"
cp "$NETHACK_GIT/dat/NHdump.css" "$NAO_CHROOT/$NHSUBDIR"
chmod 644 "$NAO_CHROOT/$NHSUBDIR/NHdump.css"

echo "Copying sysconf file"
SYSCF="$NAO_CHROOT/$NHSUBDIR/sysconf"
cp "$NETHACK_GIT/sys/unix/sysconf" "$SYSCF"
chmod 644 $SYSCF

echo "Creating NetHack variable dir stuff."
mkdir -p "$NAO_CHROOT/$NHSUBDIR/var"
chown -R "$USRGRP" "$NAO_CHROOT/$NHSUBDIR/var"
mkdir -p "$NAO_CHROOT/$NHSUBDIR/var/save"
chown -R "$USRGRP" "$NAO_CHROOT/$NHSUBDIR/var/save"
mkdir -p "$NAO_CHROOT/$NHSUBDIR/var/whereis"
chown -R "$USRGRP" "$NAO_CHROOT/$NHSUBDIR/var/whereis"

touch "$NAO_CHROOT/$NHSUBDIR/var/logfile"
chown -R "$USRGRP" "$NAO_CHROOT/$NHSUBDIR/var/logfile"
touch "$NAO_CHROOT/$NHSUBDIR/var/perm"
chown -R "$USRGRP" "$NAO_CHROOT/$NHSUBDIR/var/perm"
touch "$NAO_CHROOT/$NHSUBDIR/var/record"
chown -R "$USRGRP" "$NAO_CHROOT/$NHSUBDIR/var/record"
touch "$NAO_CHROOT/$NHSUBDIR/var/xlogfile"
chown -R "$USRGRP" "$NAO_CHROOT/$NHSUBDIR/var/xlogfile"
touch "$NAO_CHROOT/$NHSUBDIR/var/livelog"
chown -R "$USRGRP" "$NAO_CHROOT/$NHSUBDIR/var/livelog"

RECOVER="$NETHACK_GIT/util/recover"

if [ -n "$RECOVER" -a -e "$RECOVER" ]; then
  echo "Copying $RECOVER"
  cp "$RECOVER" "$NAO_CHROOT/$NHSUBDIR/var"
  LIBS="$LIBS `findlibs $RECOVER`"
  cd "$NAO_CHROOT"
fi

LIBS=`for lib in $LIBS; do echo $lib; done | sort | uniq`
echo "Copying libraries:" $LIBS
for lib in $LIBS; do
        mkdir -p "$NAO_CHROOT`dirname $lib`"
        if [ -f "$NAO_CHROOT$lib" ]
	then
		echo "$NAO_CHROOT$lib already exists - skipping."
	else
		cp $lib "$NAO_CHROOT$lib"
	fi
done

echo "Finished."

