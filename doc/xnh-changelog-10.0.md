## xNetHack 10.0 Changelog

This is a major version of xNetHack. It is based directly on xNetHack 9.1, and
is a fork off the vanilla NetHack 3.7.0 development version release.

The most recent vanilla commit incorporated into xNetHack 10.0 is deec8317. Note
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
made in previous xNetHack versions, xNetHack 10.0 contains the following
changes:

### Gameplay changes

- The wooden material list is extended to include metal and glass. Launchers
  cannot be made out of glass, but all other base-wooden items can be.
- Cursed weapons are no longer immune to breaking when used to force a lock.
- Chests and large boxes can no longer generate made of bone.
- Chests and large boxes made of stone no longer have any lock at all. They
  cannot be locked (but may still be trapped) through magic or mundane means,
  and they are always easily openable. They no longer display as "unlocked" even
  when fully identified.
- Metal chests' or large boxes' locks cannot be broken open by kicking, and
  forcing them with a bladed weapon is significantly more likely to result in
  the weapon breaking.
- Weapons have different odds of breaking when used to pry open a chest or large
  box.
  - Iron and metal are unchanged from the usual odds.
  - Mithril is somewhat less likely to break than iron.
  - Copper is a little worse than iron.
  - Silver is a little worse than copper.
  - Gold, wood, plastic, and stone are worse than iron, but not to the point
    where they will reliably break.
  - Glass is very likely to break, unless it has been shatterproofed.
- Blunt glass weapons have a 5% chance per turn of cracking when being used to
  smash the lock on a box. If it cracks, you will stop trying to force the lock,
  but may continue if you wish.

### Interface changes


### Architectural changes

