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

- Randomly generated amulets other than the amulet of reflection now get
  assigned a material from an amulet-specific list rather than using the same
  material list as shields of reflection. This list has a more even distribution
  of possible materials, and also includes gemstone, stone, and bone.
  - Amulets of reflection still use the shield of reflection material list.
- Human Rangers can now be played as Lawful.
- Rangers are 4 times as good as other roles at finding pits and spiked pits.
- Add the giant scorpion.
  - It is basically a scaled-up, faster version of the regular scorpion.
  - Unlike the smaller scorpion, it has a grapple attack.
  - Regular scorpions can grow up into them.
  - They appear on the Ranger quest.
  - They are orange to be distinguishable from regular scorpions.
- Allow large s-class monsters (giant spider and giant scorpion) to be saddled
  and ridden.

### Interface changes

- An extended achievement (which does not correspond to any bits in the xlogfile
  achieve field) is awarded for solving the Wizard's Puzzle, which is encoded in
  achieveX as "solved_wiz_puzzle".

### Architectural changes

