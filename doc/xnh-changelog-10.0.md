## xNetHack 10.0 Changelog

This is a major version of xNetHack. It is based directly on xNetHack 9.0, and
is a fork off the vanilla NetHack 3.7.0 development version release.

The most recent vanilla commit incorporated into xNetHack 10.0 is df06fc3. Note
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

- The Ides of March (March 15th) is recognized as one of the in-game holidays,
  and slime molds will generate as "Caesar salads" on that day.
- Items with negative enchantment or charges can be dipped in a noncursed potion
  of restore ability to raise their enchantment, to 0 if the potion is blessed
  or by 1 point if not. If the item is eroded, that will be fixed first and a
  second potion will be required to address any negative enchantment.
- Spellbooks can be dipped in a noncursed potion of restore ability to fully
  refresh the ink on their pages.
- Quaffing restore ability will heal wounded legs, confusion, stunning, and
  hallucinating (unless you are perma-hallucinating). If it's blessed, it will
  also cure illness.
- The spell of restore ability has the same effects, except for curing illness.
- Monsters hit by a potion of restore ability get cured of any blindness,
  confusion, or stunning, instead of being healed to full HP.

### Interface changes


### Architectural changes

- Refactored the bit of common logic for "is this a weapon, armor, weptool, or
  charged ring - an object that is + or - something?" into a macro,
  spe_means_plus.
