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
- Enchantment is ignored when wishing for dragon scales.
- Artifacts created by naming retain the original material of the object.
- Vampires will not move onto sinks, including if they are shapeshifted.
- Foocubi gain a level when they drain one of yours, making future encounters
  less likely to go well for you.
- Magic portals are never hidden; they are readily visible like holes are.
- Gnomes generated outside the Mines may be carrying a few gems.
- Eating disenchanter meat while hallucinating will remove the hallucination
  instead of an intrinsic. If permanently hallucinating, this will break you out
  of it as well, similar to wielding Grayswandir.

### Interface changes

- Engravings now properly use the new vanilla glyph system. There is one glyph
  per type of engraving (except graffiti, which has 3 glyphs). There are new
  tiles for each new glyph.
- When hallucinating, squeaky boards now appear to be chickens, and squawk
  instead of squeak.
- #holidays extended command which displays which holidays tracked by the game
  are active.
- Cold rays and explosions render as bright blue instead of white, to
  distinguish them from lightning.
- Quantum mechanics are given physicist names when farlooked, or "Dr. Science"
  if cancelled. (Similar to coyotes, not like nymphs or foocubi.)

### Architectural changes

