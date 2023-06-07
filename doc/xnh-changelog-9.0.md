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
- New artifact The Amulet of Storms: a chaotic amulet of flying that grants
  shock resistance when worn and allows you to chat to hostile vortices, air
  elementals, and storm giants to pacify them. It also prevents you from being
  paralyzed by lightning on the Plane of Air.

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

### Architectural changes

