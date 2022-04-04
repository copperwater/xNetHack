## xNetHack

xNetHack is a fork of the dungeon exploration game NetHack. It is a distant
descendent of Rogue and Hack, and a direct descendant of the development
version of NetHack 3.7. For more information on NetHack, visit
http://nethack.org, and for its public Git repository, see
https://github.com/NetHack/NetHack.

The main goals of xNetHack are to take vanilla NetHack and:
1. Fix gameplay balance issues.
2. Remove tedious and frustrating parts of the game.
3. Make uninteresting parts of the game more interesting.
4. Experiment with new ideas from the community.

In general, the game design takes a conservative approach to changing the
gameplay compared to other variants, with focus on deepening elements of the
game rather than broadening them. For example, this philosophy would prefer to
differentiate monsters in a class that all tend to play the same way, instead
of adding new monsters to that class.

Popular features of xNetHack have tended to be good candidates for feature
suggestions to the DevTeam; a number of features in vanilla NetHack either
originated in xNetHack or used its implementation of something that originated
in another variant.

xNetHack's public Git repository can be accessed at
https://github.com/copperwater/xNetHack. Its changelogs (one per each version)
can be found in the doc/ folder. A more human-readable version of this is
available on the NetHack wiki: https://nethackwiki.com/wiki/xNetHack.

The ideas and proposals that xNetHack draws from are archived on [the NetHack
YANI Archive](https://nethack-yanis.github.io), which is maintained by the
xNetHack author.

## Installation

For installation instructions, please refer to the `README` file also found in
this directory, which is the unchanged README provided by the NetHack devteam.
It will direct you to the appropriate documentation for your system, in
particular the `sys/*/Install.*` files.

* Users running macOS can build and install xNetHack with the script
  [install_macos.sh](sys/unix/install_macos.sh) (requires
  [homebrew](https://brew.sh/) to install build dependencies).
