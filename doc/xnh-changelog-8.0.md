## xNetHack 8.0 Changelog

This is a major version of xNetHack. It is based directly on xNetHack 7.0, and
is a fork off the vanilla NetHack 3.7.0 development version release.

The most recent vanilla commit incorporated into xNetHack is f592b9d. Note that
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
- Vampires will not move onto sinks, including if they are shapeshifted.
- Foocubi gain a level when they drain one of yours, making future encounters
  less likely to go well for you.
- Magic portals are never hidden; they are readily visible like holes are.
- Gnomes generated outside the Mines may be carrying a few gems.
- Eating disenchanter meat while hallucinating will remove the hallucination
  instead of an intrinsic. If permanently hallucinating, this will break you out
  of it as well, similar to wielding Grayswandir.
- The Castle drawbridge may fail to close 20% of the time when using the
  passtune.
- The Castle drawbridge responds to passtune attempts 2 squares away from
  either of the two drawbridge squares, instead of only directly adjacent
  squares.
- Sleep spell level raised from 2 to 3 (which is what it is in vanilla).
- Item encyclopedia lookup from the inventory is now an item in the menu of
  things you can do with an item you select when looking at the inventory, under
  'l', rather than the default behavior.
- Kicking into empty air (which may strain a muscle) no longer wakes monsters.
- Anti-magic fields no longer deal HP damage when magic resistant. They instead
  drain additional energy the more sources of magic resistance and half damage
  you have.
- Corpses of monsters that cause petrification only grow green mold on them.
- Monsters that spawn with weapons get a free turn to wield them.
- Steam vortexes leave a trail of steam clouds behind them and burst into a
  medium sized steam cloud when they die. Canceling them prevents both of these.
- Add the #shout command, which lets you shout a string of your choice to get
  recorded in the game chronicle and livelog (which public servers may
  broadcast).
- Rangers at or above XL 10 automatically identify the enchantment of ammo that
  they pick up, provided they have at least Basic skill with its associated
  launcher.
- Exploding yellow and black lights affect all monsters immediately adjacent to
  them, not just the single target they are nominally attacking.
- The hunger intrinsic halves the nutritional value of anything you eat. It does
  not affect beverages, and other intrinsics that have fast hungering as a side
  effect do not cause this.
- Engraving with a wand of probing or wand of secret door detection produces a
  unique message and identifies the wand (assuming it has a charge to expend, as
  is standard).
- 1% of tigers drop a tiger eye ring when killed.
- Dwarves get a +1 alignment bonus when they chop down a tree.
- Jumping boots automatically identify themselves when worn.
- Iron-hating monsters will avoid stepping on spaces with iron chains.

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
- Steam vortices' engulf attack produces "boiling" messages rather than
  "burning".
- When you begin the game in polyinit mode and using the tty windowport, the "Is
  this ok?" prompt before launching the game is always shown, and displays the
  monster you will be polyinitted into.
- Detect unseen is no longer a messageless effect. You now get messages about
  revealing one or more secret doors, corridors, traps, or monsters.

### Architectural changes

Fuzzer logging is now a compile-time switch, and is off by default, rather than
being a silent user of CPU in all regular games.
