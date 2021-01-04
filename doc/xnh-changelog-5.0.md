## xNetHack 5.0 Changelog

This is a major version of xNetHack. It is a fork off of the vanilla NetHack
3.6.5 development version release, and is also based directly on xNetHack 4.1.
See the doc/fixes36.\* files for the devteam's changes.

The xNetHack page at the NetHackWiki, https://nethackwiki.com/wiki/XNetHack,
attempts to describe these changes in a way that's better formatted and more
friendly to players. However, the wiki page might be out of date; in case of
conflicting information, this changelog and others in this directory are more
up-to-date than the wiki page, and the commit messages are more up-to-date than
this changelog.

On top of any changes made by the NetHack devteam on 3.6.1, and any changes
made in previous xNetHack versions, xNetHack 5.0 contains the following
changes:

### Gameplay changes

- You can sometimes recover from food poisoning with a Con/100 chance.
- If you have hungerless casting, you will never be too hungry to cast a spell.
- Corpses dropped by zombies can revive on their own.
- When you are killed by a zombie, you will arise from the grave as one in
  bones (25% chance to keep playing as a zombie, similar to other undead).
- When a zombie or lich kills a monster that has a zombie counterpart in melee,
  it resurrects as a zombie that is either tame or hostile depending on the
  attacking zombie.
- Zombies will grudge monsters that they can zombify, and vice versa.
- The pet-off-level-untaming timer is fuzzed so it's not totally predictable.
- Pets' willingness to attack foes of a certain level decreases linearly with
  their hit point percentage.
- Trees can generate in the rooms of room-and-corridor levels.
- Lord monsters' weapons and armor is never negatively enchanted.
- If you're carrying sufficient food, being Weak isn't a major trouble.
- If you're turning to slime and opening a tin of chameleon meat, you won't be
  interrupted (you could turn into something fiery).
- Blessed potions of hallucination grant enlightenment 50% of the time; cursed
  ones do 20% of the time, uncursed ones never do.
- Grimtooth is now permanently poisoned and has a base cost of 1000.
- Poisoned weapons in melee do d10+6 damage to non-resistant creatures, but
  only 25% of the time.
- Dragonbane now gives vision of dragons when wielded and does +d10 damage to
  dragons. Its cost is raised to 2500.
- Demonbane now gives vision of demons when wielded, glowing white.
- Werebane now gives vision of weres when wielded and does +d10 damage to
  weres.
- Giantslayer now gives vision of giants when wielded and does +d10 damage to
  giants. Its cost is raised to 500.
- Trollsbane now gives hungerless regeneration when wielded, gives vision of
  trolls when wielded, and does +d10 damage to trolls. Its cost is raised to
  1000.
- Ogresmasher's cost is raised to 1200.
- Sitting or manually teleporting via ^T while standing on a known level
  teleporter will activate it, bypassing magic resistance.
- Intelligent monsters in the Castle know about the trapdoors.
- Reading a confused scroll of earth will summon earth elementals and dust
  vortices rather than mostly harmless rocks.
- Hobbits may generate carrying produce.
- Fire Brand instakills flammable golems and green slimes
- Elven items use their base material 60% of the time and become wood 20% of
  the time rather than defaulting to wood 80% of the time.
- The throne monster in throne rooms has a shiny expensive mace rather than a
  boring iron one.
- The Bell of Opening will turn into a silver regular bell in a bones file,
  rather than a copper one.
- All types of orc monsters (all o) resist poison.
- Pets won't fight each other even if they grudge each other.
- Port RPresser's epitaph patch, enabling you to write your own gravestone
  text.
- Wizard mode players now always get the option to leave bones unless the level
  is ineligible.
- Monsters can gain intrinsics from corpses they eat, identically to players,
  except that they can currently only get resistance-type intrinsics and
  telepathy. The only non-pet monster that can currently eat corpses is the
  gelatinous cube.
- Dragons will roar of their own accord from time to time, which wakes nearby
  monsters.
- Implement cooperative telepathy: if you have any sort of telepathy, even
  extrinsic and you are not blind, you are able to see telepathic monsters.
- Pets will attack monsters they grudge even if they are of too high a level.
- Pets will not attack gas spores that you're adjacent to.
- Magic-liking monsters will pick up all magical items, not just objects from
  classes where all items are magical.
- Monsters can use magic flutes.
- Dissolving a cursed mold corpse in fruit juice will create sickness.
- Port the Free Fortune Cookie Patch: half of szechuan tins, if eaten, give you
  a free fortune cookie.
- Getting a critical hit on a vampire with a wooden piercing weapon can stake
  it through the heart and destroy it instantly.
- Magic lamps always release a djinni when rubbed instead of 1/3 of the time.
- 1/30 of randomly generated tins are booby-trapped and will explode when
  opened.
- 1/5 of nurses carry a scalpel.
- Wearing a charged ring of carrying will auto-identify it.
- Hobbits and Nazguls grudge each other.
- All s-class monsters grudge x-class and a-class monsters, but not vice versa.
- Bats grudge all flying x-class and a-class monsters (currently just bees).
- Guarantee graffiti of a true rumor in the first room of the dungeon.
- When hallucinating, all rays appear as blasts of something nonsensical.
- Rings that aren't obviously iron by their appearance are made of metal
  instead.
- Helms made of wood, bone and stone protect your head the same as a metal
  helm.
- Helms made of glass may be shattered if a heavy object drops on them.
- Helms reduce a lot less of the damage if the falling object is very heavy.
- Helms only protect you from half of the damage from a falling piercer.
- Acidic corpses can go moldy, but produce only green mold.
- Plastic has a much lower relative cost as a material.
- Group size now throttles on low dungeon levels rather than experience levels.
- Glass body armor may shatter if you are hit with a heavy object moving
  sideways.
- Hallucination is effective at blocking all gaze attacks made by monsters.
- Falling rock traps no longer generate on outdoors levels.
- Amulets are worth 150 zorkmids each when the game ends regardless of
  material.
- Amulets of life saving will not set your HP or max HP higher than 100.
- Chopping a web with a blade now factors in the weapon's enchantment.
- Shopkeepers' base level is adjusted from 12 to 13.
- Cockatrice hissing attacks begin stoning 1/5 of the time, but deafness blocks
  the attack entirely.
- Aligned priests' maces are never cursed.
- Ghosts generate invisible 1/3 of the time.
- Gas clouds are a little random in how they spread out from a point; the edges
  of the cloud may be fuzzy and hard to predict.
- The chest that generates with a thiefstone now contains random contents.
- Random statues can occasionally be copper, or rarely gold.
- Quarterstaves no longer block spellcasting if a cursed one is welded to both
  of your hands.
- Quarterstaves only do d4 damage to small monsters.
- Felines now attack rats.
- The portal to Fort Ludios now generates in the first vault generated below
  level 10.
- The titan on one Medusa level has been replaced with two golden nagas.
- Drop the level of the spellbook of invisibility from 4 to 1.
- Yeenoghu's magic missile attack damage is increased to 6d6.
- Revise ammo breakage rules: as in NetHack Fourk, the chance of breakage
  decreases with higher skill and skill cap. Negatively enchanted and cursed
  ammo breaks much more often; positively enchanted and blessed ammo breaks
  much less often.
- Demon princes gate in twice as many demons as lesser demons do; demon lords
  gate in some intermediate amount.
- Goblins now generate in small groups (but won't on very shallow levels due to
  automatic group size scaling).
- The Wizard of Yendor now has a 2d6 weapon attack.
- Restore passive rust attack to water elementals.
- Scale the size of monsters' hit dice by their in-game size. Medium monsters
  still use a d8; smaller monsters use smaller dice and larger monsters use
  larger dice.
- Adjust the special monster maximum HP formula for Riders to 40 + 8d8. Remove
  the special monster maximum HP formla for dragons (they follow normal rules
  for any gigantic creature of their level).
- Adjust monster sizes:
  - Giant ants are Small rather than Tiny.
  - Lemures are Small rather than Medium.
  - Baby crocodiles are Small rather than Medium.
  - Mumakil are Gigantic rather than Large.
  - Mastodons are Huge rather than Large.
  - Giant rats are Small rather than Tiny.
  - Centipedes are Small rather than Tiny.
  - All v-class monsters are Large rather than Huge.
  - Electric eels are Large rather than Huge. (Giant eels remain Huge.)
  - Krakens are Gigantic rather than Huge.
  - All baby dragons are Large rather than Huge.
  - All elementals are Large rather than Huge.
  - All naga hatchlings are Small rather than Large.
  - Cobras are Large rather than Medium.
  - Xorns are Large rather than Medium.
  - Medusa is Medium rather than Large.
  - Balrogs are Huge rather than Large.
  - Juiblex is Gigantic rather than Large.
- Mumaks' base level is now 10, and their difficulty is increased to 15.
- Material hatred damage is assessed for monsters made of a certain material
  touching (hitting, kicking, etc) a monster that hates that material.
- Stone to fleshed objects now turn into an appropriate amount of meatballs,
  proportional to the monster's weight if a statue and proportional to the
  object's base weight otherwise.
- There is no longer an Elbereth engraving under the Sokoban prize and the
  Castle chest. (The scrolls of scare monster are still there.)
- Scrolls of scare monster no longer work on uniques.
- Pets will not eat food if they are already satiated.
- Centaurs prefer to keep their distance from you and won't close to melee
  combat.
- Intrinsic invisibility and see invisible are no longer obtainable:
  - Extrinsics (rings, cloak, artifacts) and role-based intrinsics are
    unchanged.
  - Stalkers now grant 200+d200 turns of both intrinsics.
  - Random throne and fountain effects now grant 1000 + d1000 turns.
  - The potions grant 200+d200 turns, plus 200 if uncursed / 400 if blessed.
  - The wand grants 50+d50 turns per zap, or (charges)d250 turns if broken.
- Elven boots will always be leather.
- Orcish gear will be either iron, wood, bone, plastic, or glass.
- Gazing into a crystal ball is now an occupation that doesn't leave you
  helpless. It takes 6-10 turns to use one, rather than 1-10.
- Crystal balls now have many more charges and weigh only 100.
- A blessed or cursed crystal ball counts as +5 or -5 to your Int when
  determining whether you successfully use it.
- Non-cursed crystal balls will never explode.
- Uncursed charging a crystal ball, horn of plenty, bag of tricks or can of
  grease will add d5+5 charges instead of just d5.
- Dart, arrow, and rock traps produce ammunition from a stack, so it will all
  have the same material, enchantment, etc. The number of darts/arrows/rocks
  from a given trap is constant, regardless of whether produced by firing or by
  untrapping.
- Dart and arrow traps generate with 15+1d20 ammo, rock traps with 5+1d10.
- Poison darts from traps deal Con damage more often (60% of the time), and
  higher HP damage when they don't. However, dart traps won't generate with
  poison darts until depth 7.
- Any iron items that elvish players would start with are copper instead.
- Remove the rogue tribute level.
- Add Frontier Town (what Orcish Town was before being ransacked) as an eighth
  variant of Minetown.
- Monsters that wear helms of telepathy become telepathic.
- Monsters can wear amulets of ESP, becoming telepathic.
- Celibate conduct (broken when you lie with an incubus/succubus)
- The Chromatic Dragon is now named Tiamat.
- The Caveman quest narration and dialogue have been changed to use caveman
  speech patterns.
- The Dark One is now named Anaraxis the Black.
- Loadstones are removed.
- Conflictless conduct (broken when you generate conflict).
- Add homosexual and bisexual orientations. These can be specified by
  OPTIONS=orientation:[orientation]. It also accepts "straight", "gay" and
  "bi".  The only thing your orientation controls is what sort of foocubi you
  can interact with.
- If you are bisexual, foocubi roll twice after each encounter to see if they
  get a severe headache.
- The foocubus gained from kicking a sink will always be compatible with your
  orientation.

### Interface changes

- Potion bottles are called by nonsensical names when hallucinating.
- Your god will belch, bray, squeak, and so on instead of booming or thundering
  while you are hallucinating.
- Confused scrolls of fire underwater evaporate the water like a regular
  scroll.
- A message is printed when you fail to walk into liquid, and the first time
  this happens in the game it informs you that you can use m to walk in.
- Add a new paranoid confirmation option "throw"; when on, it prompts for
  confirmation when you try to throw ammo with no matching launcher for it.
- Dwarves think all rats are delicious.
- YAFM for hearing a dwarf digging while hallucinating.
- YAFM for eating long worms.
- YAFM for dying to green slime (permanently) while hallucinating.
- YAFM for failing to get a wished-for artifact while hallucinating.
- YAFM for ascending while hallucinating.
- YAFM for trying to #chat to a wall.
- YAFM for a cosmetic magic trap effect if you are a hallucinating valkyrie.
- YAFM for hearing a wererat change form while hallucinating.
- YAFM for getting your brains eaten while hallucinating.
- Usual periodic assortment of new bogusmons, bogus gods, bogus currencies,
  candy bar labels, t-shirt texts, rumors, engravings, and epitaphs.
- Give better directions for reporting impossible() calls.
- Give gnomes a gnomeish message when chatted to.
- Auto-identify a potion of acid when it explodes due to water contact.
- Add L's Colored Walls Patch, turning the walls in various special levels and
  branches different colors. Also in some types of special rooms.
- Don't unconditionally delete the save file in case of a version mismatch.
- Zombies groan instead of being silent.
- Trying to attack a monster in melee with pacifist conduct intact after 100
  turns will ask you if you really want to attack.
- Livelog when the player changes alignment (by helm or permaconversion).
- YAFM for staking a vampire's heart while hallucinating.
- You can try indefinitely to enter a valid wish, rather than the game giving
  you a random object after 5 failed tries.
- YAFM for hitting a devil with an egg (thrown or melee).
- Message that you can't reenter the dungeon after negative levelport
- Give a special artifact message when Grimtooth poisons a target.
- YAFM for playing a cursed magic whistle while hallucinating.
- YAFM for being startled and dropping a heavy iron ball while hallucinating.
- YAFM for eating globs of brown pudding.
- YAFM for playing a magic flute while hallucinating.
- YAFM for fumbling noisily while hallucinating.
- YAFM for seeing a lantern on the floor get dim while hallucinating.
- YAFM for observing something step on a squeaky board while hallucinating.
- Remove "[Un]lock it?" prompts for applying a key to a door.
- Failed and cancelled scrolls of stinking cloud give a new message (and also a
  YAFM for doing this while hallucinating).
- Stat increase/decrease messages are comparative ("You feel stronger", etc)
- More explicit hunger messages.
- YAFM for hallucinating and doing nothing in particular.
- Add weapon skills and intended launchers to object lookup.
- Display skill caps and the percent towards the next skill in #enhance.
- When worn or wielded items become uncursed, you get a message.
- Crystal balls prompt you again if they don't recognize the character you want
  to look for. You no longer get vague statements about the location of special
  levels.
- Crystal balls give unique messages if they are uncharged or cancelled.

### Architectural changes

- Refactor spell hunger reduction logic into its own function.
- Refactor confusing logic of what food adjective to choose.
- Promote obj_nutrition to a non-static function.
- Allow artifacts which target a monster class (S_\* instead of M2_\*) to warn
  versus that monster class. The save-stable version of this is kludgy.
- Change all obj->material = foo statements into a function set_material which
  handles weight recalculation and erosion differences between the materials.
- Move bones depth logic out of can_make_bones into really_done.
- Move corpse-intrinsic-selection logic into its own function.
- Extract part of givit's logic (the yes/no of whether an intrinsic should be
  given) into its own function.
- Define a new MR2_TELEPATHY monster intrinsic and set it by default for all
  inherently-telepathic monsters.
- Add a parameter to put_monsters_to_sleep defining the monster that is causing
  the effect.
- Refactor mpickstuff to use a callback function rather than a string of object
  classes, allowing for finer control of objects monsters will pick up.
- Refactor mm_aggression into clear one-directional and two-directional cases.
- Pass lines retrieved via get_rnd_text and get_rumor (rumors, bogusmons,
  graffiti, epitaphs, etc) through the quest text parser, letting them use %
  arguments.
- Move angelic and demonic maledictions out of quest.txt into wizard.c string
  constants, and scrap using com_pager for them.
- Objects will only be given a randomized object material if called with
  init=TRUE in mksobj.
- Add a struct crystalball_info to struct context, and the appropriate bits
  into the save and restore code to save and restore it.
- Add an ammo field to struct trap, which represents the object contained in
  the trap (darts, beartrap, etc). Interactions with the trap draw on this
  rather than on generic objects.
- Rename cnv_trap_obj to remove_trap_ammo. It now extracts and places the ammo
  from a trap.
- Split monster intrinsics off the mextrinsics field, preventing monsters' worn
  armor from clobbering their intrinsics from eaten corpses.
- Generalize loadstone undroppability behavior to work on arbitrary objects.
