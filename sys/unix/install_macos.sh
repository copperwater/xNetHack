#!/usr/bin/env bash
#
# Builds and installs xNetHack with TTY, Curses, and Qt interfaces on macOS.
#
# Requirements:
# - homebrew (https://brew.sh/) to install build dependencies
#
# Tested on:
# - macOS Monterey 12.3

# Errors if a variable is referenced before being set
set -u

abort() {
  printf "%s\n" "$@"
  exit 1
}

### Default settings
# Default installation path of xNetHack.
# Must be consistent with the HACKDIR settings in `sys/unix/hints/macOS.370`.
declare -r DEFAULT_HACKDIR="$HOME/xnethackdir/"
# Default path of launch script.
# Must be consistent with the SHELLDIR settings in `sys/unix/hints/macOS.370`.
declare -r LAUNCH_SCRIPT="$HOME/bin/xnethack"
declare -r USER_CONFIG="$HOME/.xnethackrc"

### Environment validation
# Check operating system.
declare -r OS="$(uname)"
if [[ "${OS}" != "Darwin" ]]; then
    abort "ERROR: This script is only supported on macOS"
fi

# Check user.
#
# Background: The script has not been tested to be run as root yet, as this
# will result in different default paths than for regular users, see
# `sys/unix/hints/macOS.370`. Contributions welcome!
if [[ "${USER}" == "root" ]]; then
    abort "ERROR: This script was not tested to be run as user 'root'"
fi

if ! command -v brew &> /dev/null; then
    abort "ERROR: This script requires homebrew (https://brew.sh/)"
fi

### Build xNetHack with tty+ncurses+qt (macOS)
echo "Building xNetHack with TTY, Curses, and Qt interfaces ..."
# Install dependencies
declare -r FORMULAE=(
    # Required for curses version
    ncurses
    # Required for qt version
    qt@5
    libxpm
)
for FORMULA in ${FORMULAE[@]}; do
    brew ls "$FORMULA" &>/dev/null || brew install "$FORMULA" || \
        abort "ERROR: $FORMULA could not be installed, exiting ..."
done

(cd sys/unix && sh setup.sh hints/macOS.370) || \
    abort "ERROR: Failed to apply macOS hints file, exiting ..."
make fetch-Lua || \
    abort "ERROR: 'make fetch-Lua' step failed, exiting ..."
make WANT_WIN_TTY=1 WANT_WIN_CURSES=1 WANT_WIN_QT=1 QTDIR=$(brew --prefix)/opt/qt@5 all || \
    abort "ERROR: 'make all' step failed, exiting ..."

### Install xNetHack
echo "====================================================================="
echo "About to install xNetHack:"
echo
echo "- Install directory: $DEFAULT_HACKDIR"
echo "- Launch script:     $LAUNCH_SCRIPT"
echo
echo "***                   WARNING WARNING WARNING                    ***"
echo "***  Backup your existing install directory before proceeding,   ***"
echo "***  because it will be OVERWRITTEN by the new installation.     ***"
echo "***  Without a backup, you lose your save files, record,         ***"
echo "***  logfile, sysconf files, etc.                                ***"
echo
read -r -p "I do have a backup of my previous installation? [y/N] " response
if [[ $response =~ ^(yes|y|Yes|Y) ]]; then
    make WANT_WIN_TTY=1 WANT_WIN_CURSES=1 WANT_WIN_QT=1 QTDIR=$(brew --prefix)/opt/qt@5 install || \
        abort "ERROR: Installation failed, exiting ..."
    echo "====================================================================="
	echo "                 _   __       __   __  __          __     A"
	echo "               /  | / /      / /  / / / /         / /    ,X,"
	echo "      __  __  /   |/ /___  _/ /_ / /_/ /_   ____ / /___ <=V=>"
	echo "      \ \/ / / /| / // _ \/  __// _   /  \ / __//   __/  |||"
	echo "       /  / / / |  /|  __// /  / / / / / // /_ / /\ \    |||"
	echo "      /_/\_/_/  |_/ \___//_/  /_/ /_/\___\\__//_/  \_\   |||"
	echo "       ,_______________________________________________  |||"
	echo "                                                       '  \|"
	echo
    echo "Installation completed!  You have three options to run xNetHack."
    echo
    echo "1) GUI version, with tile set:"
    echo
    echo "       $ XNETHACKOPTIONS=windowtype:qt $LAUNCH_SCRIPT"
    echo
    echo "2) Curses version:"
    echo
    echo "       $ XNETHACKOPTIONS=windowtype:curses $LAUNCH_SCRIPT"
    echo
    echo "3) TTY version:"
    echo
    echo "       $ XNETHACKOPTIONS=windowtype:tty $LAUNCH_SCRIPT"
    echo
    echo "You can also set your launch preference in $USER_CONFIG:"
    echo
    echo "    # Example: launch Qt version"
    echo "    OPTIONS=windowtype:qt"
    echo
    echo "Questions? Bugs? Suggestions? Go to https://github.com/copperwater/xNetHack"
fi
