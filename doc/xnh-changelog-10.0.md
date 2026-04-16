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
- Glass chests and large boxes are referred to as "crystal" and have the
  following properties:
  - They are always generated locked.
  - Randomly generated ones generate 1 additional item in their contents.
  - The lock is magical and cannot be broken by force or unlocked via mundane
    means such as a key or lock pick. It requires a locking or unlocking spell
    or wand.
  - They are see-through: when you move onto a space containing one of these and
    no other objects, you will be shown the contents of the container. This
    doesn't trigger again by walking over it as long as you know the contents,
    but you can look at the contents again by using the near-look command while
    standing over it.
- Archfiends' chests that may store wands of wishing are always crystal chests.
- The chest in the Castle containing the wand of wishing is now a crystal chest
  25% of the time, iron 37.5% of the time, and wood 37.5% of the time.
- Ordinary soldiers have a 1% chance of generating with a leather drum or cheap
  flute. Those generated at level creation time in Fort Ludios will never
  generate with drums.

### Interface changes

- If you know the beatitude of a horn of plenty or tinning kit, you will
  automatically know the beatitude of the food items they produce.
- Non-cursed charging methods identify the charge count on the item they
  charged.
- Copper items are now colored orange, to better distinguish them from gold
  items.
- The Amulet of Yendor (and fakes) are colored bright magenta.

### Architectural changes

