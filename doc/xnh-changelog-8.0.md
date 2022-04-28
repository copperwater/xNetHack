## xNetHack 8.0 Changelog

This is a major version of xNetHack. It is based directly on xNetHack 7.0, and
is a fork off the vanilla NetHack 3.7.0 development version release.

The most recent vanilla commit incorporated into xNetHack is a971244. Note that
because 3.7.0 is still in development status, xNetHack contains major changes
including new monsters, new objects, themed rooms, and other things *not*
documented in this file or other xNetHack changelogs. See doc/fixes37.0 for the
DevTeam's changes.

The xNetHack page at the NetHackWiki, https://nethackwiki.com/wiki/XNetHack,
attempts to describe these changes in a way that's better formatted and more
friendly to players. However, the wiki page might be out of date; in case of
conflicting information, this changelog and others in this directory are more
up-to-date than the wiki page, and the commit messages are more up-to-date than
this changelog.

On top of any changes made by the NetHack devteam on 3.7, and any changes
made in previous xNetHack versions, xNetHack 8.0 contains the following
changes:

### Gameplay changes

- Trappers and lurkers above are mindless.
- If hallucinating, a skeleton rattling its bones will not scare and paralyze
  you.
- Cost of several magic tools increased:
  - Magic horns and magic harps 50 => 200
  - Drum of earthquake 25 => 100
  - Magic whistle 10 => 100
  - Magic flute 36 => 144
- New terrain type "magic platform". This will currently only appear in special
  levels. It is effectively a hovering walkable space over open air. It cannot
  be dug down on or engraved on.
- The Valkyrie quest has received an overhaul:
  - All quest levels are redesigned to some extent; the locate and goal levels
    especially have received heavy redesigns, and the filler levels use new
    Lua-based level generators.
  - While the overall story of the quest remains the same (stop Lord Surtur from
    starting Ragnarok), the goal of the quest is changed - kill Lord Surtur.
    This may be a fine distinction, but the point is not to retrieve an artifact
    stolen from the Norn.
  - There is still technically a quest artifact: Sol Valtiva, the flaming sword
    wielded by Lord Surtur. It is a mithril two-handed sword, retaining the half
    damage and invoke for levelport provided by the Orb of Fate, but does not
    provide warning or act as a luckstone. It has +d3 to hit and +d5 fire
    damage, burning things it hits similar to Fire Brand. It is chaotic, but as
    a quest artifact, its alignment will change to match the player's if they
    are a Valkyrie.
  - In terms of monsters, the quest has many more giants and fewer fire ants.
    Insects no longer generate randomly over time.
  - The locate level, a broken bridge, is magically restored after Surtur is
    killed.
  - There is a throne in the goal level (and only one drawbridge).
- Valkyries start with a +1 spear or dwarvish spear instead of a long sword.
- Valkyries can now reach only Skilled in two-handed sword instead of Expert.
- Valkyries can reach Expert in spear, instead of Skilled.
- Enchantment is ignored when wishing for dragon scales.
- Artifacts created by naming retain the original material of the object.

### Interface changes

- Engravings now properly use the new vanilla glyph system. There is one glyph
  per type of engraving (except graffiti, which has 3 glyphs). There are new
  tiles for each new glyph.
- When hallucinating, squeaky boards now appear to be chickens, and squawk
  instead of squeak.
- #holidays extended command which displays which holidays tracked by the game
  are active.

### Architectural changes

