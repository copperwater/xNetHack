## xNetHack 9.0 Changelog

This is a major version of xNetHack. It is based directly on xNetHack 8.0, and
is a fork off the vanilla NetHack 3.7.0 development version release.

The most recent vanilla commit incorporated into xNetHack 9.0 is 2abe156. Note
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
made in previous xNetHack versions, xNetHack 9.0 contains the following
changes:

### Gameplay changes

- The blessed scroll of gold detection also detects gems across the level.
- Every lock in the game may be unopenable by using a credit card. Boxes have a
  40% chance, and doors have a chance that starts at 10% and increases with
  depth.
- Also, if the card is cursed or you are fumbling, the card may slip through the
  lock, ending up inside a box or on the other side of a door.
- Cockatrice nests may contain chickatrices and cockatrice eggs.
- Eating troll meat provides 2d6 turns of intrinsic regneneration.
- Drinking from a magic fountain when either HP or Pw is below 60% may give a
  healing spring effect that increases both maximums by 1 and fully restores
  both, instead of the usual gain ability effect. The fountain still becomes
  nonmagical afterward.
- Monsters will consider if their regular melee attacks might deal a lot more
  damage to the hero than their offensive items would, and may subsequently
  decide to attack in melee rather than using that offensive item.
- Rolling boulders, eating carrots, and occasionally trying to read a dusty
  spellbook (via the dust making you sneeze) causes enough noise to wake nearby
  monsters.
- Monsters stepping on a squeaky board near you can wake you up.
- Leprechauns have a one-sided grudge against gold golems and gold dragons.
- Rock moles can eat gems and rocks (and other objects made of stone, like
  marble wands). Dilithium crystals grant them extra speed. The player can eat
  boulders and (empty) statues when polymorphed into one, but other rock moles
  won't.
- Gain energy potions are guaranteed to restore more energy. A blessed one
  restores at least 40% of your energy maximum and an uncursed one restores at
  least 25%.
- The potion of gain energy can be alchemized by combining the potions of full
  healing and gain ability.
- Sitting down can partly wipe engravings on your space.
- The chance of finding a secret door or passage via searching is no longer
  dependent on Luck.
- Zombies cannot open closed doors.

### Interface changes

- Wishing for "leather armor" will give the player a leather light armor (its
  equivalent from vanilla).
- Pets are referred to with their correct gender pronoun if they have one,
  rather than "it".
- When you #chat and are next to only one observable monster, you will
  automatically attempt to chat to that monster without being prompted for a
  direction to chat in.
- When you are a few turns from temporary invisibility expiring, you receive a
  message warning you that you're becoming visible again.
- When farlooking a closed and unlocked door you are next to, it will show up as
  "unlocked door" instead of the generic "closed door".
- Produce a message when a nymph teleports away after a successful theft.
- When polymorphed into a monster with an explosion attack, you can #monster to
  intentionally explode. (This was already possible by attacking a monster or
  force-fighting thin air.)
- Escaping out of a polymorph prompt no longer causes a random polymorph; it
  instead gives instructions on how to explicitly request that and prompts
  again. (However, escaping or failing to enter a valid form 5 times still
  causes a random polymorph.)
- The legacy text at the start of the game uses different role-based opening
  lines that replace the standard "It is written in the Book of [deity]:"
- YAFMs:
  - Chatting to a troll (even though you can't see it's a troll) while
    hallucinating.
  - Being life-saved while hallucinating.
  - Monster drinking a potion where you can't see while hallucinating.

### Architectural changes

