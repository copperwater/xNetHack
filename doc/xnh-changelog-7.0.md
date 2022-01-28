## xNetHack 7.0 Changelog

This is a major version of xNetHack. It is based directly on xNetHack 6.0, and
is a fork off the vanilla NetHack 3.7.0 development version release.

The most recent vanilla commit incorporated into xNetHack is dd6ed502. Note that
because 3.7.0 is still in development status, xNetHack may contain major changes
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
made in previous xNetHack versions, xNetHack 7.0 contains the following
changes:

### Gameplay changes

- 5% of humanoid non-lord A monsters (Angels and Aleaxes) generate with a harp,
  which is magic 20% of the time.
- 1/40 of dwarves in the Mines and 1/8 of dwarves outside the Mines generate
  with d3 booze potions.
- French fried and deep fried tins give more nutrition (80 and 100
  respectively.)
- New random tin flavor "stale", giving only 25 nutrition.
- Skeletons rarely drop bone skeleton keys.
- Blowing up a bag of holding gives some experience points, identifies the bag
  of holding, and identifies the wand of cancellation if that is the cause of
  the explosion.
- Ghosts will not make themselves uninvisible in an attempt to frighten you if
  you are already paralyzed.
- Elven ring mail, in its default copper form, provides 2 AC, and provides 4 AC
  if made of mithril.
- Covetous monsters will attempt to equip the item they covet if it comes into
  their possession and it is equippable.
- Itlachiayaque replaces the Orb of Detection as the Archeologist quest
  artifact. It is a gold shield of reflection that confers fire resistance
  and warning when carried and reflection when worn. Archeologists do not get
  a spellcasting penalty for wearing it. When invoked, it either summons a
  stinking cloud, identical to a scroll of the same beatitude, or you can gaze
  into it for the same effect as a crystal ball.
- Monsters can throw potions of oil as an offensive item. Typically they light
  them so that they explode on contact, but a confused monster may forget to do
  this.
- If you are hit by an unlit potion of oil, you get covered in oil and your
  fingers become slippery.
- Schliemann replaces the Minion of Huhetotl as the Archeologist quest nemesis.
  He is a human, and unlike other quest nemeses does not warp. He carries a
  pick-axe and a number of potions of oil to throw at you.
- The Archeologist quest has received a full overhaul:
  - The home, locate, goal, and filler levels are all replaced. The lower filler
    levels use an all-new level generator implemented entirely in Lua.
  - The story of the quest is changed. It's now a bit like a mash-up of Raiders
    of the Lost Ark and National Treasure: the location of Itlachiayaque has
    just been uncovered, and Schliemann and his better equipped team are trying
    to get it first! Unlike the player, they have no qualms about being ruthless
    and destructive in their eagerness to get there first.
  - Enemy-wise, the quest is a lot more mummy-heavy than it used to be, with
    human mummies in particular appearing a lot in the tomb in the second half
    of the quest.
  - Much of the quest loot consists of gold or other precious metal variants of
    regular items. (To this end, gold is now a valid material on normally-wooden
    items.)
  - Level sounds of explosions appear on the lower floors of the quest as you
    and Schliemann's team converge on Itlachiayaque.
  - Monster generation probabilities in the quest are now 55% human mummy, 14%
    random M, 17% random S, 14% random monster. (Not exact percentages.)
- Mummies now have a touch attack that inflicts withering, a new status effect.
  Withering has the following effects:
  - Every turn, a withering creature loses 1 or 2 hit points (0 or 1 if it
    regenerates), until the withering times out.
  - Withering creatures do not regenerate HP like normal, though items such as
    healing potions work the same to restore HP.
  - Creatures that die to withering attacks do not leave a corpse.
  - When the player is withering, a "Wither" status is shown on the status line.
    Status hilites can be used to configure this status like normal. It's also
    covered under 'major-troubles' in status hilites.
  - Prayer can fix withering, but unicorn horns cannot. Life-saving also does
    not cure it.
  - The various mummies inflict increasingly higher durations of withering as
    they increase in difficulty.
  - Certain chests are trapped in such a way that they spawn a number of mummies
    around you when triggered.
- You can #rub silver items while withering to reduce the remaining duration:
  - The amount of withering duration cancelled generally equals the total weight
    of the rubbed items. E.g. a weight-11 silver dagger will shorten withering
    by up to 11 turns.
  - This corrodes and/or destroys the silver items in the process. Each
    application of the silver causes 1 level of corrosion per 1/4 of the
    object's weight. If it goes past thoroughly corroded it disintegrates.
  - Items that are not subject to corrosion (e.g. the silver wand/ring) just get
    destroyed, regardless of how much withering they stopped.
  - Erodeproof silver and artifacts are not protected from corrosion or
    destruction.
  - The Bell of Opening cannot corrode or be destroyed, so using it to fix
    withering instead uses up its charges, and it won't work when out of
    charges.
- It is impossible to destroy a metal box or chest by forcing it.
- The Oracle may generate with a potion of hallucination.
- Several tweaks to the spell Pw cost formula:
  - There is no more "effective Int" calculation, or a cap on it. Just your
    normal Int is used.
  - An equipped robe, quarterstaff, or wand of nothing halves the total penalty
    you take from worn armor. These do not stack with each other.
  - A wielded wand matching the spell you want to cast provides a direct boost
    rather than lumping into "effective Int".
- Grass patches appear in the Ranger quest start level.
- Add the Oily Corpses Patch by L: there is a 20% chance upon starting to eat a
  corpse of certain slippery monsters (e.g. most slithy, non-snakelike monsters
  and most blobby monsters) that your fingers get slippery. Tins of them are not
  slippery unless the tin preparation method is slippery.
- Cursed wands explode more often, 1/30 of the time instead of 1/100.
- The Vibrating Square level is no longer a random maze; the player always
  generates in one of two small rooms at the bottom, and the square is always
  found in the center of one of three larger rooms.
- New monster "black mold": a black F that is sessile and has a passive deathly
  illness attack. It also causes illness when eaten, and like other molds can be
  used to ferment fruit juice, turning it into sickness.
- Air terrain can now appear in levels other than the Planes. In such levels
  where gravity is in effect, it poses a potentially fatal hazard.
  - Anyone who steps into air will fall either to a designated level beneath
    that level, take 12d20 fall damage, become stunned, and wound their legs for
    60+10d10 turns.
    or if there is no designated level beneath it, simply die instantly.
  - Air can be crossed by flying or levitating or being a wumpus, or riding a
    steed that can cross air.
  - Items that fall into air fall to the level below, but have a 25% chance of
    being lost or broken instead. If there is no level below, they are lost
    forever.
  - Unique items required to win the game never break when falling to a lower
    level and will hover in midair if there is no lower level.
  - Monsters cannot be pushed or hurtle out into midair. This appears to be due
    to vanilla code (they also cannot hurtle out over water or lava).
  - It behaves like water or lava in terms of safeguards to prevent stepping
    into air (requires an m prefix if it would be unsafe, and ParanoidSwim
    additionally prompts for confirmation even when using an m prefix).
  - Air is now colored black; this does not change anything when using its
    default ASCII symbol of " " but will hopefully allow players who wish to
    define it as something else to show it less ambiguously than if it were
    cyan.
- The Vlad's Tower branch now has a fourth level: a deep perilous chasm beneath
  the tower proper consisting mostly of air terrain. The level beneath it is the
  level in Gehennom containing the stairs to the tower. There are a couple
  demons and several vampires and vampire leaders inhabiting this area.
- New artifact Mirror Brand, an unaligned glass short sword with the following
  properties:
  - Confers reflection.
  - When used to hit a monster wielding a weapon, rolls the damage for that
    weapon if it had been used against the monster and adds it to the
    preexisting damage. (This includes base damage, enchantment, object
    material, and specific weapon-versus-monster damage adjustments such as
    blessed weapons versus undead.)
  - Can be dual wielded with the other Brands.
- New monster "phoenix": an orange B that is quite powerful with difficulty 20.
  Strongly lawful (thus is likely to be peaceful to lawful players), attacks
  with bites and fire claws, and explodes in a ball of fire when killed, which
  produces a phoenix egg that will soon hatch anew into a phoenix.

### Interface changes

- Upon first entering the bottom Sokoban level, you get a message about grooves
  on the floor.
- Shopkeepers address certain players with a role-specific title, such as
  Knights or high-level Monks or Healers.
- The "You hear someone cursing shoplifters" message uses a shopkeeper's real
  name if you have visited their shop previously.
- Monks will stop seeing "You feel guilty" messages after eating meat enough
  times.
- The levels with the stairs to the Mines, Sokoban, and Vlad's Tower have
  special ambient level sounds. The Sokoban and Vlad ones only persist until you
  have finished the first level of Sokoban and killed Vlad, respectively.
- Swap the chaotic and unaligned altar colors to be in line with vanilla:
  chaotic is now black and unaligned red.
- For Junethack, begin recording quest completion (returning to one's leader
  with the quest artifact) as "completed_quest" in the achieveX xlogfile field.
- Magic flutes and harps are automatically identified when played with charges
  remaining.
- A new message prints when playing a fire or frost horn describing the
  emanating bolt.
- YAFMs:
  - Attempting to chop down a petrified tree while hallucinating.

### Architectural changes

- New functions:
  - sokoban_solved()
  - arti_starts_with_the()
  - undiscovered_bones()
- Implement covetous_nonwarper(), a macro that controls whether a covetous
  monster should not warp or use the other code in tactics(), and should behave
  like a normal monster.
- Allow specification of "waiting" on monsters in level file, which makes them
  wait until you're in visual range before moving.
- Refactor mktrap()'s mazeflag argument into a general flags field, and add a
  flag that disables generating a spider on a web. Also allow specification of
  "no_spider_on_web" in a table-form des.trap() command that passes this flag.
- New struct monst fields mwither (byte) and mwither_from_u (bit).
- Artifacts that warn against a monster class are properly supported with a new
  g.context.warntype.obj_mlet field.
