## xNetHack 10.0 Changelog

This is a major version of xNetHack. It is based directly on xNetHack 9.1, and
is a fork off the vanilla NetHack 5.0.0 release and subsequent bugfix
development.

The most recent upstream NetHack commit incorporated into xNetHack 10.0 is
c22b21e. This changelog does not attempt to document upstream changes from the
5.0.0 release and postrelease. See doc/fixes5-0-0.txt and doc/fixes5-0-1.txt
for the DevTeam's changes.

The xNetHack page at the NetHackWiki, https://nethackwiki.com/wiki/XNetHack,
attempts to describe these changes in a way that's better formatted and more
friendly to players. However, the wiki page might be out of date; in case of
conflicting information, this changelog and others in this directory are more
up-to-date than the wiki page, and the commit messages are more up-to-date than
this changelog.

On top of any changes made by the NetHack DevTeam on 5.0, and any changes
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
- Add a new material: hard light, which is a magic construct that is
  lightweight and fairly strong.
- Rename the Longbow of Diana to the Longbow of Orion.
- Add a new object: arrow of light. It is a weightless arrow made of hard light
  that deals exceptionally high damage but disintegrates after hitting
  something or falling to the floor. Monsters it kills in one shot will leave
  no corpse, and monsters it fails to kill will be blinded and stunned.
  - The arrow of light deals half its normal damage if you are not a Ranger who
    has killed Scorpius.
- The Longbow of Orion's invoke effect now creates arrows of light, granting
  either 3 of them, or if that would make there be more than 5 arrows of light
  in the game, as many as can be created to bring the total to 5.
  - If you are not a Ranger, only one arrow of light will be created.
  - If there are already 5 in the game, it creates ordinary arrows enchanted
    equal to the Longbow's enchantment.
  - These ordinary arrows will be elven arrows if you are an elf and orcish
    arrows if you are an orc.
- Orion is replaced as the Ranger quest leader with Cedalion, his lieutenant
  and guide.
- Wielding the Longbow of Orion puts a floor on the amount of multishot you get
  from arrows fired from it. This floor is 1 (no multishot) if you wouldn't be
  able to multishoot with it, or 2 if you can only shoot a maximum of 2 arrows,
  but otherwise it's 3.
- The Ranger quest has received an overhaul:
  - The home level is lightly redesigned (from when it was redone in xNetHack
    3.0); monsters will no longer spawn in the sacred grove.
  - The upper filler level is no longer a cavern fill with trees as the
    background but is still somewhat similar, consisting of randomized trails
    through a forest leading into one of two caves.
  - The story has not been severely changed, but more closely tracks the
    mythological conflict between Orion and Scorpius. The largest change is
    that Orion is already dead and the mission is one of vengeance.
  - You receive the Longbow of Orion from Cedalion as soon as you get assigned
    the quest. However, it will not create arrows of light until Scorpius is
    killed, though it can still create normal arrows and its passive extrinsics
    still apply.
  - Scorpions and other s-class monsters generate more frequently on the quest,
    with centaurs generating less frequently.
  - The wumpus on the locate level is much stronger than ordinary wumpuses. It
    has much more HP, is significantly faster, and its bite does a huge amount
    of damage, enough to kill most characters.
  - The locate level is changed to be more cave-like and less
    artificial-feeling while still being a series of 20 chambers arranged like
    the vertices of a dodecahedron.
    - Passages between the chambers are now corridor terrain rather than room
      terrain.
    - The upstairs and downstairs cavern are now randomized instead of fixed.
    - The wumpus is waiting on the downstairs rather than being asleep.
    - The two random "bottomless pit" chambers are open air terrain. Falling
      into these is an instadeath. (The spiked pit traps are removed to avoid
      confusion.)
    - The centaurs spawn in two groups in random chambers rather than being
      spread out.
  - The lower filler level(s) are similar to the upper one, but the map is
    dark with some random clouds, there is no cave, and there are many more
    scorpions than centaurs.
  - The goal level is another forest traverse, but with more fog and a river.
    Scorpius, a few centaurs, and a lot of scorpions inhabit a clearing across
    the river.
  - Scorpius is buffed:
    - He has a higher base level, giving him more HP.
    - His AC is now 0 instead of 10, in line with most other nemeses.
    - His base MR is now 50.
    - He is faster at speed 18 instead of 12.
    - One of his claw attacks is now a grabbing attack.
    - He is now huge, though his attacks do *not* cause knockback.
    - If he stings you a second time while you are already ill and you have
      intrinsic poison resistance, you will lose that intrinsic.
    - When below half health, he can summon scorpions and giant scorpions.
    - On the other hand, he will no longer covetous warp.
  - All levels of the quest have barren trees that contain no fruit or bees.
  - Large dogs have a one-way grudge against Scorpius, so you can use Sirius to
    help attack him.
- Ice devils and cold traps no longer strip temporary cold resistance.
- Rangers have an 80% chance of escaping traps they created rather than the
  standard 20% chance.
- Most trees on the Archeologist and Monk quests can no longer produce fruit or
  bees. There are several trees on both start levels that still can, though.
- When you walk into a peaceful invisible creature you weren't aware of, it
  does not get angry.
- Dark Speech can no longer have no effect; if it tries to blind you and you
  are already blinded, it will fall through to trying to strip an intrinsic; if
  no intrinsic gets removed, it will fall through to the general
  damage-dealing, confusing, and stunning effect.
- Iron objects generate as metal (which is not harmful to elves) 6% of the
  time.
- The FIQHack-inspired energy regeneration formula is reset to NetHack 5.0's
  formula.
- Wizards once again start with a random wand, two random rings, three random
  potions and scrolls, and one random spellbook in addition to their force bolt
  spellbook, the same as in NetHack 5.0. They no longer start with a guaranteed
  magic missile book and extra random spellbook.

### Interface changes

- If you know the beatitude of a horn of plenty or tinning kit, you will
  automatically know the beatitude of the food items they produce.
- Non-cursed charging methods identify the charge count on the item they
  charged.
- Copper items are now colored orange, to better distinguish them from gold
  items.
- The Amulet of Yendor (and fakes) are colored bright magenta.
- An extended achievement (which does not correspond to any bits in the xlogfile
  achieve field) is awarded for solving the Wizard's Puzzle, which is encoded in
  achieveX as "solved_wiz_puzzle".
- Monster lookup shows whether or not a monster can be saddled.
- Trees that are incapable of summoning a swarm of bees will not produce a "low
  buzzing" when kicked.

### Architectural changes

- Add a make_spanning_tree function in the Lua nhlib file, which constructs a
  general spanning tree from a given unweighted graph.
- Change the semantics of the "arboreal" level flag.
  - Instead of meaning "rock should appear and behave as trees", it now means
    "trees should not contain fruit or bees by default".
  - It is still possible for a level designer to create individual trees that
    do contain fruit or bees on an arboreal level by using des.terrain.
- Add a makemon flag MM_NOITEM which suppresses the creation of a random item
  for a snake or scorpion to hide underneath.
