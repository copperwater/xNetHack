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
- The potion of restore ability has received many buffs:
  - Items with negative enchantment or charges can be dipped in a noncursed
    potion of restore ability to raise their enchantment, to 0 if the potion is
    blessed or by 1 point if not. If the item is eroded, that will be fixed
    first and a second potion will be required to address any negative
    enchantment.
  - Spellbooks can be dipped in a noncursed potion of restore ability to fully
    refresh the ink on their pages.
  - Quaffing noncursed restore ability has the following effects in addition to
    restoring lost ability scores:
    - Heals wounded legs, temporary blindness, temporary deafness, confusion,
      stunning, and temporary hallucination.
    - A blessed potion will also cure illness.
    - The existing level-restoring function of this potion now restores levels
      up to your *peak* XL reached, meaning that if you reduced your *max* XL by
      quaffing full healing, restore ability can still recover the rest.
    - If you are not missing any experience levels and your HP or energy maximum has
      been reduced below its highest ever value, it will also restore half of
      the difference.
  - Monsters can also quaff potions of restore ability they happen to have to
    cure blindness, confusion, stunning, and cancellation.
  - Monsters hit by a potion of restore ability get cured of any blindness,
    confusion, or stunning, instead of being healed to full HP. (This will not
    cure cancellation.)
- The spell of restore ability can now also cure the same status ailments as the
  potion, except for curing illness (since that has its own spell).
- If the hero is hit with cancellation, they lose all their magic energy.
- Throwing gems that are type-named but not fully identified at a coaligned
  unicorn no longer gives +2 luck; they are treated the same as non-type-named
  gems, giving +1 luck.
- Items with a base iron or metal type generate as silver less often (2% of the
  time instead of 5%), and as mithril less often (1% of the time instead of 3%).
  Their chance of generating as copper, wood, or iron is slightly increased.
- Several vanilla themed rooms are tweaked:
  - The "Pillars" and "Mausoleum" room can now be filled with ordinary room
    contents (monsters, objects, etc) rather than being barren.
  - The "Statuary" fill now generates a number of statues in proportion to its
    area, and will never place 2 statues (or a statue combined with a statue
    trap) on the same space.
  - The "Ghost of an Adventurer" fill curses 80% of the items in its fake bones
    pile, the same as normal bones piles.
- Thiefstones can no longer teleport you to a location on the same level if it's
  a no-teleport level, and cannot levelport you if the levelport would normally
  be blocked (like when within Gehennom).

### Interface changes

- There is a visual effect associated with the monster (or the hero) being
  hit with cancellation.

### Architectural changes

- Refactored the bit of common logic for "is this a weapon, armor, weptool, or
  charged ring - an object that is + or - something?" into a macro,
  spe_means_plus.
