## xNetHack 9.0 Changelog

This is a major version of xNetHack. It is based directly on xNetHack 8.0, and
is a fork off the vanilla NetHack 3.7.0 development version release.

The most recent vanilla commit incorporated into xNetHack 9.0 is 2abe156. Note
that because 3.7.0 is still in development status, xNetHack contains major
changes including new monsters, new objects, themed rooms, and other things
*not* documented in this file or other xNetHack changelogs. See doc/fixes37.0
for the DevTeam's changes.

The xNetHack page at the NetHackWiki, https://nethackwiki.com/wiki/XNetHack,
attempts to describe these changes in a way that's better formatted and more
friendly to players. However, the wiki page might be out of date; in case of
conflicting information, this changelog and others in this directory are more
up-to-date than the wiki page, and the commit messages are more up-to-date than
this changelog.

On top of any changes made by the NetHack devteam on 3.7, and any changes
made in previous xNetHack versions, xNetHack 9.0 contains the following
changes:

### Gameplay changes

- The blessed scroll of gold detection also detects gems across the level.
- Every lock in the game may be unopenable by using a credit card. Boxes have a
  40% chance, and doors have a chance that starts at 10% and increases with
  depth.
- Cockatrice nests may contain chickatrices and cockatrice eggs.
- Eating troll meat provides 2d6 turns of intrinsic regneneration.
- Rock moles can eat gems and rocks (and other objects made of stone, like
  marble wands). Dilithium crystals grant them extra speed. The player can eat
  boulders and (empty) statues when polymorphed into one, but other rock moles
  won't.

### Interface changes

- Wishing for "leather armor" will give the player a leather light armor (its
  equivalent from vanilla).
- Pets are referred to with their correct gender pronoun if they have one,
  rather than "it".

### Architectural changes

