-- NetHack quest.lua	$NHDT-Date: 1652196288 2022/05/10 15:24:48 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.7 $
-- Copyright (c) 2021 by Pasi Kallinen
-- NetHack may be freely redistributed.  See license for details.
-- TODO:
--  - output = "verbalize"
--  - export the quest string replacements to lua, instead of %H etc
--  - allow checking if hero is carrying item (see comments for %Cp Arc 00042)
--  - fold quest_portal, quest_portal_again, quest_portal_demand into one
--  - write tests to check questtext validity?
--  - qt_pager hack(?): if (qt_msg->delivery == 'p' && strcmp(windowprocs.name, "X11"))




-- text = "something"
-- Text is shown to the user.

-- synopsis = "something"
-- Synopsis is inserted into the message history.
--
-- output = "pline" | "menu" | "text"
-- The output can be manually set by using output = "menu"
-- Valid values for output are "pline", "text", and "menu, defaulting to
-- pline, unless the text contains newlines, or is too long to fit a message buffer,
-- then will be shown as a text window instead.



questtext = {
   -- If a role doesn't have a specific message, try a fallback
   msg_fallbacks = {
      goal_alt = "goal_next"
   },
   common = {
      TEST_PATTERN = {
         output = "text",
         text = [[%p:	return(plname);
 %c:	return(pl_character);
 %r:	return((char *)rank_of(u.ulevel));
 %R:	return((char *)rank_of(MIN_QUEST_LEVEL));
 %s:	return((flags.female) ? "sister" : "brother" );
 %S:	return((flags.female) ? "daughter" : "son" );
 %l:	return((char *)ldrname());
 %i:	return(intermed());
 %o:	return(artiname());
 %O:	return(shortened(artiname()));
 %n:	return((char *)neminame());
 %g:	return((char *)guardname());
 %G:	return((char *)align_gtitle(u.ualignbase[1]));
 %H:	return((char *)homebase());
 %a:	return(Alignnam(u.ualignbase[1]));
 %A:	return(Alignnam(u.ualign.type));
 %d:	return((char *)align_gname(u.ualignbase[1]));
 %D:	return((char *)align_gname(A_LAWFUL));
 %C:	return("chaotic");
 %N:	return("neutral");
 %L:	return("lawful");
 %x:	return((Blind) ? "sense" : "see");
 %Z:	return("The Dungeons of Doom");
 %%:	return(percent_sign);
 a suffix:	return an(root);
 A suffix:	return An(root);
 C suffix:	return capitalized(root);
 h suffix:	return pronoun(he_or_she, mon_of(root)); /* for %l,%n,%d,%o */
 H suffix:	return capitalized(pronoun(he_or_she, mon_of(root)));
 i suffix:	return pronoun(him_or_her, mon_of(root));
 I suffix:	return capitalized(pronoun(him_or_her, mon_of(root)));
 j suffix:	return pronoun(his_or_her, mon_of(root));
 J suffix:	return capitalized(pronoun(his_or_her, mon_of(root)));
 p suffix:	return makeplural(root);
 P suffix:	return makeplural(capitalized(root));
 s suffix:	return s_suffix(root);
 S suffix:	return s_suffix(capitalized(root));
 t suffix:	return strip_the_prefix(root);]],
      },
      angel_cuss = {
         "\"Repent, and thou shalt be saved!\"",
         "\"Thou shalt pay for thine insolence!\"",
         "\"Very soon, my child, thou shalt meet thy maker.\"",
         "\"The great %D has sent me to make thee pay for thy sins!\"",
         "\"The wrath of %D is now upon thee!\"",
         "\"Thy life belongs to %D now!\"",
         "\"Dost thou wish to receive thy final blessing?\"",
         "\"Thou art but a godless void.\"",
         "\"Thou art not worthy to seek the Amulet.\"",
         "\"Judgment hath been passed upon thee, %p.\"",
         "\"Thy reckoning is at hand, %p.\"",
         "\"Thou shalt be brought before %D for thy crimes!\"",
         "\"With %D as my witness, I shall strike thee down.\"",
         "\"%d have mercy on thee, for I shall not!\"",
         "\"Abandon this arrogance and accept thy doom!\"",
         "\"Begone, and do not return!\"",
         "\"Depart, thou accursed, into everlasting fire!\"",
         "\"Filthy mortal, thy life is forfeit!\"",
         "\"I find thy lack of faith disturbing.\"",
         "\"Kneel, or thou shalt be knelt!\"",
         "\"Say thy prayers, %p!\"",
         "\"Thou shalt not kill! That's my job!\"",
         "\"Stop! %D commands it!\"",
         "\"Thy fate is sealed, infidel!\""
         -- hallucinatory - not yet implemented
         -- "\"Just one more sinner, and I get a promotion!\"",
         -- "\"No one expects the Spanish Inquisition!\"",
      },
      banished = {
         synopsis = "[You are banished from %H for betraying your allegiance to %d.]",
         output = "text",
         text = [["You have betrayed all those who hold allegiance to %d, as you once did.
My allegiance to %d holds fast and I cannot condone or accept what you
have done.

Leave this place.  You shall never set foot at %H again.
That which you seek is now lost forever, for without the Bell of Opening,
you will never be able to enter the place where he who has the Amulet
resides.

Go now!  You are banished from this place.]],
      },
      demon_cuss = {
         "\"I first mistook thee for a statue, when I regarded thy head of stone.\"",
         "\"Come here often?\"",
         "\"Doth pain excite thee?  Wouldst thou prefer the whip?\"",
         "\"Thinkest thou it shall tickle as I rip out thy lungs?\"",
         "\"Eat slime and die!\"",
         "\"Go ahead, fetch thy mama!  I shall wait.\"",
         "\"Go play leapfrog with a herd of unicorns!\"",
         "\"Hast thou been drinking, or art thou always so clumsy?\"",
         "\"This time I shall let thee off with a spanking, but let it not happen again.\"",
         "\"I've met smarter (and prettier) acid blobs.\"",
         "\"Look!  Thy bootlace is undone!\"",
         "\"Mercy!  Dost thou wish me to die of laughter?\"",
         "\"Run away!  Live to flee another day!\"",
         "\"Thou hadst best fight better than thou canst dress!\"",
         "\"Twixt thy cousin and thee, Medusa is the prettier.\"",
         "\"Methinks thou wert unnaturally stirred by yon corpse back there, eh, varlet?\"",
         "\"Up thy nose with a rubber hose!\"",
         "\"Verily, thy corpse could not smell worse!\"",
         "\"Wait!  I shall polymorph into a grid bug to give thee a fighting chance!\"",
         "\"Why search for the Amulet?  Thou wouldst but lose it, cretin.\"",
         "\"Thou ought to be a comedian, thy skills are so laughable!\"",
         "\"Thy gaze is so vacant, I thought thee a floating eye!\"",
         "\"Thy head is unfit for a mind flayer to munch upon!\"",
         "\"Only thy reflection could love thee!\"",
         "\"Hast thou considered masking thine odour?\"",
         "\"Hold! Thy face is a most exquisite torture!\"",
         "\"I should fart in thy direction, but it might improve thy smell!\"",
         "\"Art thou a dragon, or is that merely thy foul breath?\"",
         "\"Go away, or I shall taunt thee again!\"",
         "\"Hurry up and die already, I'm hungry!\"",
         "\"Thy self-esteem shall crumble beneath my vicious mockery!\"",
         "\"Thy broken corpse shall taste delicious lightly seasoned with nutmeg!\"",
         "\"Thy mother was a hamster and thy father smelt of elderberries!\"",
         "\"Talk to the claw!\"",
         "\"Nyaaah!\"",
         "\"Thou art but a talentless rube!\"",
         "\"Once I am finished with thee, I think I may feed upon thy pet!\"",
         "\"Is thy name truly \'%p?\' Truly, \'dunderhead\' wouldst be more fitting!\"",
         "\"Thou calls thyself a %r? In sooth?\"",
         "\"Thy mother is a sow, and thy father is a cow!\"",
         "\"Wouldst thou assist me with directions? I appear to be surrounded by idiots!\"",
         "\"Thhbbbbbt!!!\"",
         "\"I bite my thumb at thee!\"",
         "\"I cannot wait to drag thy body across a sea of spikes!\"",
         "\"I wonder, what circle awaits thou after I slay thee?\"",
         "\"Nyeah nyeah, thou canst hurt me!\"",
         -- hallucinatory - not yet implemented
         -- "\"Come at me, bro!\"",
         -- "\"Ni!\"",
      },
      leader_malediction = {
         "\"Dead or alive, you shall never abide here again!\"",
         "\"Accept your exile or die!\""
      },
      legacy = {
         synopsis = "[%dC has chosen you to recover the Amulet of Yendor for %dI.]",
         output = "menu",
         text = [[It is written in the Book of %d:

    After the Creation, the cruel god Moloch rebelled
    against the authority of Marduk the Creator.
    Moloch stole from Marduk the most powerful of all
    the artifacts of the gods, the Amulet of Yendor,
    and he hid it in the dark cavities of Gehennom, the
    Under World, where he now lurks, and bides his time.

Your %G %d seeks to possess the Amulet, and with it
to gain deserved ascendance over the other gods.

You, a newly trained %r, have been heralded
from birth as the instrument of %d.  You are destined
to recover the Amulet for your deity, or die in the
attempt.  Your hour of destiny has come.  For the sake
of us all:  Go bravely with %d!]],
      },
      quest_complete_no_bell = {
         text = [["The silver bell which was hoarded by %n will be
essential in locating the Amulet of Yendor."]],
      },
      quest_portal = {
         output = "pline",
         text = [[You receive a faint telepathic message from %l:
Your help is urgently needed at %H!
Look for a ...ic transporter.
You couldn't quite make out that last message.]],
      },
      quest_portal_again = {
         text = "You again sense %l pleading for help.",
      },
      quest_portal_demand = {
         text = "You again sense %l demanding your attendance.",
      },
      --- Boss appearance messages are keyed by their monster name.  It expects
      --- that any spaces in the name will be replaced by underscores and there
      --- will be an "_appears" added to the end of it.
      Juiblex_appears = {
         output = "text",
         text = [[
A massive pillar of slime towers above you, larger than you would believe
to be possible.  It hisses, spits, and bubbles, filling the air around it
with noxious vapours.  Most striking of all is the burning hatred that
emanates from it, so intense that it feels almost like a physical force
and you have to struggle not to let it push you back.

Juiblex, the Faceless Lord, has arrived.
         ]],
         synopsis = "[Juiblex appears!]"
      },
      Yeenoghu_appears = {
         output = "text",
         text = [[
The hulking humanoid that stands before you is covered nearly three meters
tall and has the head of a hyena.  In one hand it grips a flail, and the
other hand crackles with magical energy.  When it opens its mouth, the
unholy din that emerges could charitably be called a laugh.

Yeenoghu has come to call.
         ]],
         synopsis = "[Yeenoghu appears!]"
      },
      Orcus_appears = {
         output = "text",
         text = [[
Blackness cloaks the being in front of you.  All you can %x of it is a
muscular form, two burning yellow eyes, and a pair of claws.  One of the
claws clutches a skull-topped wand, about which a miasma of dark magic
gathers.

"I am Orcus, Lord of the Dead.  Goodbye."
         ]],
         synopsis = "[Orcus appears!]"
      },
      Geryon_appears = {
         output = "text",
         text = [[
The monster you now face is nothing short of horrifying.  The upper body
seems to be a combination of three separate humanoids that have been
forcibly merged.  The lower body is serpentine and distended, tapering off
into an enormous stinger.  The hands of the monster end in wicked claws.

"Greetings, mortal.  I am Geryon."
         ]],
         synopsis = "[Geryon appears!]"
      },
      Dispater_appears = {
         output = "text",
         text = [[
The being in front of you sniffs in disdain and tosses his ram's horns.  He
is dressed in fine clothing, and his skin is covered in bony protrusions.
A flickering gray aura surrounds him, which occasionally curls into a
bestial shape.

"Is this the one who seeks the Amulet of Yendor? How utterly pathetic.  I,
Dispater, shall dispatch thee."
         ]],
         synopsis = "[Dispater appears!]"
      },
      Baalzebub_appears = {
         output = "text",
         text = [[
A maddening buzz fills your ears.  Thousands of flies appear from
everywhere, swarming through the air to coalesce into a horrible,
pulsating mass, which continually spews torrents of insects as it expands.

Baalzebub, Lord of the Flies approaches.
         ]],
         synopsis = "[Baalzebub appears!]"
      },
      Asmodeus_appears = {
         output = "text",
         text = [[
The air around you is filled with a momentary hissing, before a figure
appears in a flash of frost.  He grins at you, displaying a mouth full of
fangs.

"I am Asmodeus.  Come to me, %r."
         ]],
         synopsis = "[Asmodeus appears!]"
      },
      Demogorgon_appears = {
         output = "text",
         text = [[
A great beast drags itself from the earth in a shower of stone, roaring
loudly enough that the world shakes around it.  You cannot wrap your mind
around the entirety of its form, and instead must process bits and pieces
of it at a time.

Four lashing tentacles, thick with rot.
Thick, scaly legs ending in knifelike claws.
The cloying smell of mildew mixed with blood.
A pair of rabid mandrill heads, their eyes trained on you.
Prince of Demons.

Demogorgon.
         ]],
         synopsis = "[Demogorgon appears!]"
      },
      Death_appears = {
         output = "text",
         text = [[
Blocking the way in front of you stands a huge horse, pale as the moon.
Its rider is cloaked in a black robe, taller than a man, and holding a
gleaming scythe.  He turns to face you, revealing his head to be a grinning
skull.  Instead of eyes, two icy blue pinpoints of light shine as brightly
as stars.
         ]],
         synopsis = "[You encounter Death.]"
      },
      Pestilence_appears = {
         output = "text",
         text = [[
A disgusting stench assaults your nose as a blinding white horse thunders
to a stop ahead.  Its rider wears a sickly green cloak covered in splotches
of mildew and circled by several flies.  A glittering crown, looking
rather out of place, rests on his head.  With a ghastly leer he extends a
diseased finger toward you.
         ]],
         synopsis = "[You encounter Pestilence.]"
      },
      Famine_appears = {
         output = "text",
         text = [[
A horse, dark as midnight, rears up before you, with a hunched figure
seated on its back.  He is wearing a thin gray cowl and clutching a set of
black iron balance scales.  He lowers his hood and glares at you, revealing
a badly emaciated face.  Under his fevered gaze, a pang of hunger goes
through your stomach.
         ]],
         synopsis = "[You encounter Famine.]"
      },
      Wizard_of_Yendor_appears = {
         output = "text",
         text = [[
A man stands before you.  He wears an elaborate robe trimmed with gold, and
he regards you with a malevolent stare.  The very air around him pulsates with
a fell power, and sparks fly from his fingers.

"Not since the fall of Yendor has a puny mortal dared to cross my path.
Thou art a fool to challenge me.  Prepare to die!"
         ]],
         synopsis = "[You encounter the Wizard of Yendor!]"
      },
      Vlad_the_Impaler_appears = {
         output = "text",
         text = [[
In a swirl of dark vapor, a tall figure appears, garbed in a high-collared black
velvet cloak.  His skin is very pale, except for some disturbing red stains
around his mouth.  His lips curl into a sneer, revealing long white fangs.

"I am Vlad Dracula, the Impaler.  Tonight I will feast on thy blood."
         ]],
         synopsis = "[You encounter Vlad the Impaler!]"
      },
   },
   Arc = {
      assignquest = {
         synopsis = "[%nC is after %o.  Locate %i and try to get to %O first.]",
         output = "text",
         text = [[
"No sooner had we published our latest research on locating the fabled
%o, than %n announced that %nh and %nj team were heading
to %i to claim it first.

"%nS methods of archeology are abhorrent.  %nH will blast %nj
way into the temple with no regard for anything but %nj prize,
destroying priceless pieces of history.  They are well-funded and outnumber
us.  You are the only one who might stand a chance of beating them to it.

"Go through the great jungle and locate the ancient entrance to
%i.  Somewhere deep within it, you will find
%o.  They will not be watching that way, I think.

"Make haste.  %nC must not be allowed to make off with %o.

"May the wisdom of %d be your guide."]],
      },
      badalign = {
         synopsis = "[\"%pC, you have strayed from the %a path.  Purify yourself!\"]",
         output = "text",
         text = [[
"%pC!  I've heard that you've been using sloppy techniques.  Your
results lately can hardly be called suitable for %ra!

"How could you have strayed from the %a path?  Go from here, and come
back only when you have brushed up on your basics."]],
      },
      badlevel = {
         synopsis = "[%pC, a mere %r is too inexperienced.]",
         output = "text",
         text = [["%p, you are yet too inexperienced to undertake such a demanding
quest.  A mere %r could not possibly face the rigors demanded and
survive.  Go forth, and come here again when your adventures have further
taught you."]],
      },
      discourage = {
         "\"%lC is weak! %lH is blind to the success of my methods.\"",
         "\"Ah, this bell? An ancient tool that has opened the way to many discoveries!\"",
         "\"Surrender to me, %p.  I will let you return to %H in shame.\"",
         "\"If you are the best %l can send, I have nothing to fear.\"",
         "\"What are a few walls worth, compared to the treasure behind them?\"",
         "\"%lC names me chaotic.  I consider myself pragmatic.\"",
         "\"Do you mean to kill me? Are we not professionals?\"",
         "\"Ancient artifacts don't belong in a museum! They're meant to be used!\"",
         "\"Soon, all the pupils at your college will want to study under me!\"",
         "\"Why do you strive against me? Abandon %l and we can work together.\"",
         "\"You'll never get anywhere gingerly dusting in the dirt.\"",
         "\"Oh, you had trouble with mummies? I just blew them up.\"",
         "\"I will blast you to the heavens!\"",
         "\"I did not expect you to demand death if you could not have %o.\"",
         "\"I got here first! Go find some other dig site!\"",
         "\"Is this what it's come to? Taking someone's find by force?\"",
      },
      encourage = {
         "\"To delve into %i, you must pass many traps.\"",
         "\"The jungle trek is dangerous.  You may need to cut your way through it.\"",
         "\"Beware the wild beasts you will surely encounter on your journey.\"",
         "\"The tomb has never been opened.  None can say for sure what lies within.\"",
         "\"You may encounter mummies.  Beware their withering touch.\"",
         "\"%nC has always had a penchant for using explosives.\"",
         "\"Remember not to stray from the true %a path.\"",
         "\"Go forth with the blessings of %d.\"",
         "\"I will have my %gP watch for your return.\"",
         "\"%nS henchmen are just as ruthless as %nh is.\"",
         "\"Do not listen to %ns lies.  The end does not justify the means.\"",
         "\"%o is a mighty scrying device.  %nC will see you coming if %nh gets it.\"",
         "\"Wearing %o should defend against many types of rays.\"",
         "\"Expect %i to be warded against magical mapping.\"",
      },
      firsttime = {
         synopsis = "[You arrive at %H.]",
         output = "text",
         text = [[
You are suddenly in familiar surroundings.  The buildings nearby seem
to be those of your old alma mater.

Everything looks fine on the surface, but you remember %ls plea
for help, and know that something must be wrong.]],
      },
      goal_alt = {
         text = "You have returned to the lowest level of the tomb.",
      },
      goal_first = {
         synopsis = "[Explosions continue deep in the temple.]",
         output = "text",
         text = [[
As you press into the depths of the temple, you notice cracks in the
walls, and the blasts of explosions grow ever more powerful.  You realize
that %n and %nj team must already be nearby.

You hope the continued blasts mean that they have not found
%o yet, and you are not too late.]],
      },
      goal_next = {
         text = "You return to the deepest depths of the temple.",
      },
      gotit = {
         synopsis = "[%o is what you were looking for.  Take it back to %l.]",
         output = "text",
         text = [[
As you hold %o, you feel the weight of its long and storied
history, and feel a profound sense of respect.  You know you must take
it to %l for analysis.]],
      },
      guardtalk_after = {
         "\"Truth be told, I don't actually like fedoras that much.\"",
         "\"Hey, got any spare potions of booze?\"",
         "\"I guess you are guaranteed to make full professor now.\"",
         "\"So, what was worse, %n or your entrance exams?\"",
         "\"%oC is impressive, but nothing like the bones I dug up!\"",
         "\"Snakes.  Why did it have to be snakes?\"",
      },
      guardtalk_before = {
         "\"Did you see Lash LaRue in 'Song of Old Wyoming' the other night?\"",
         "\"Hey, got any potions of hallucination for sale?\"",
         "\"Did you see the bracelet %l brought back from the last dig?\"",
         "\"So what species do *you* think we evolved from?\"",
         "\"So you're %ls prize pupil!  I don't know what he sees in you.\"",
         "\"Ever since we dug up that burial ground, I think we've been cursed.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and sacrifice it at the altar of %d.]",
         output = "text",
         text = [[
"Congratulations, %p.  I wondered if anyone could prevail against
the Wizard and the minions of Moloch.  Now, you must embark on one
final adventure.

"Take the Amulet, and find your way onto the Astral Plane.
There you must find the altar of %d and sacrifice the
Amulet on that altar to fulfill your destiny.

"Remember, your path now should always be upwards."]],
      },
      killed_nemesis = {
         synopsis = "[%nC collapses to the ground and dies.]",
         output = "text",
         text = [[
%nC collapses to the ground, wounded beyond any possibility of
healing.  For a moment, it seems as though %nh is trying to say
something, but the life leaves %ni before %nh can say it.

You are unhappy that it had to come to this, and wonder if you went too
far by killing %ni.]],
      },
      leader_first = {
         synopsis = "[\"You have returned, %p, to a difficult task.\"]",
         output = "text",
         text = [[
"Finally you have returned, %p.  You were always
my most promising student.  Allow me to see if you are ready for the
most difficult task of your career."]],
      },
      leader_last = {
         synopsis = "[\"%pC, you have failed us.  Begone!\"]",
         output = "text",
         text = [[
"%p, you have failed us.  All of my careful training has been in
vain.  Begone!  Your tenure at this college has been revoked!

"You are a disgrace to the profession!"]],
      },
      leader_next = {
         text = [[
"Again, %p, you stand before me.
Let me see if you have gained experience in the interim."]],
      },
      leader_other = {
         text = [[
"Once more, %p, you have returned from the field.
Are you finally ready for the task that must be accomplished?"]],
      },
      locate_first = {
         synopsis = "[You arrive at the ruined edifice, and feel an explosion.]",
         output = "text",
         text = [[
The trees give way ahead, as the ground abruptly steepens into a
mountainside.  Carved into the mountainside are the clear signs of ruins
of a once-great edifice -- %i.  It's not clear
where the entrance is, if indeed there still is one.

A short tremor runs through the ground as you stand there.  Alarmed, you
realize that it was a detonation.  %nS team must already be here
and blasting their way into the other side of the mountain.]],
      },
      locate_next = {
         text = "Once again, you are near the entrance to %i.",
      },
      lowlevel = {
         synopsis = "[Are you really capable of representing the College?]",
         output = "text",
         text = [[
"This is most unusual. One as inexperienced as you are could make a
mockery of the College's reputation. Do you really believe yourself
capable of representing us?"]],
      },
      nemesis_first = {
         synopsis = "[\"The treasure is mine.  You dispute this?  Come, then!\"]",
         output = "text",
         text = [[
%nC levels a piercing gaze at you.  A slow smile comes across %nh
face.

%nH briefly tips %nj hat to you.  "So %l did send someone
after all.  Alas, it seems that I was faster, so the treasure is mine."

%nH lifts %nj arm, admiring the gleam of %o on it in the dim
light, then glances at you again.  "Ah.  I see you will not accept your
loss gracefully.  I should not have expected better from one of
%l's pupils.  Very well then: come!  And you shall see what
I am made of."]],
      },
      nemesis_next = {
         synopsis = "[\"You are foolish.  Accept that %o is not yours.\"]",
         output = "text",
         text = [[
"You are entirely too bloodthirsty, %p.  This is foolishness.

"If you insist on being beaten into the dirt until you accept that
%o belongs to me now, though, I am happy to oblige."]],
      },
      nemesis_other = {
         text = "\"Your misguided persistence is growing tiresome.\"",
      },
      nemesis_wantsit = {
         text = "\"You have no claim to %o!  Return it!\"",
      },
      nexttime = {
         text = "Once again, you are back at %H.",
      },
      offeredit = {
         synopsis = "[%lC instructs you to take %o with you.]",
         output = "text",
         text = [[
You briefly tell the details of your journey to %l.

%lH takes %o and holds it briefly, gazing into its surface.
"So, %n is defeated.  I regret the conflict it caused, though I
cannot say I sympathize with a man who so grievously destroyed the
archaeological record in order to get at one piece of treasure.

"One thing though that I now realize %nh was right about:  %o
cannot sit for decades gathering dust in some storeroom, or even a
museum.  It yearns to be wielded by one who is worthy.  I can think of
none more worthy than you.

"Now that we have photographed %o and recorded its
characteristics, it seems fitting that you should keep it for further
study.  I expect that you will have a fine treatise published soon.

"May the blessings of %d follow you and guard you."]],
      },
      offeredit2 = {
         synopsis = "[\"Resume your search for the Amulet beyond the magic portal to %Z.\"]",
         output = "text",
         text = [["
Lord Carnarvon raises an eyebrow and hands Itlachiayaque back to you.
"We've already documented this as thoroughly as we can right now.
Perhaps some day when your great quest for the Amulet is complete, you
can return it here.

"For now, take it back with you through the magic portal to
%Z, with my blessing."]],
      },
      othertime = {
         text = [[You are back at %H.
You have an odd feeling this may be the last time you ever come here.]],
      },
      posthanks = {
         synopsis = "[\"Have you progressed with your quest to regain the Amulet of Yendor for %d?\"]",
         output = "text",
         text = [["Welcome back, %p.  Have you progressed with your quest to
regain the Amulet of Yendor for %d?"]],
      },
   },
   Bar = {
      assignquest = {
         synopsis = "[\"Find %n, defeat %ni, and return %o to us.\"]",
         output = "text",
         text = [["The world is in great need of your assistance, %p.

"About six months ago, I learned that a mysterious sorcerer, known
as %n, had begun to gather a large group of cutthroats and brigands
about %ni.

"At about the same time, these people you once rode with `liberated' a
potent magical talisman, %o, from a Turanian caravan.

"%nC and %nj Black Horde swept down upon %i and defeated
the people there, driving them out into the desert.  He has taken
%o, and seeks to bend it to %nj will.  I detected the
subtle changes in the currents of fate, and joined these people.
Then I sent forth a summons for you.

"If %n can bend %o to %nj will, he will become
almost indestructible.  He will then be able to enslave the minds of
men across the world.  You are the only hope.  The gods smile upon you,
and with %d behind you, you alone can defeat %n.

"You must go to %i.  From there, you can track down
%n, defeat %ni, and return %o to us.  Only
then will the world be safe."]],
      },
      badalign = {
         synopsis = "[\"You have wandered from the path of the %a.  Come back when you have atoned.\"]",
         output = "text",
         text = [["%pC!  You have wandered from the path of the %a!
If you attempt to overcome %n in this state, he will surely
enslave your soul.  Your only hope, and ours, lies in your purification.
Go forth, and return when you feel ready."]],
      },
      badlevel = {
         synopsis = "[\"You are too inexperienced.  Come back when you are %Ra.\"]",
         output = "text",
         text = [["%p, I fear that you are as yet too inexperienced to face
%n.  Only %Ra with the help of %d could ever hope to
defeat %ni."]],
      },
      discourage = {
         "\"My pets will dine on your carcass tonight!\"",
         "\"You are a sorry excuse for %ra.\"",
         "\"Run while you can, %c.  My next spell will be your last.\"",
         "\"I shall use your very skin to bind my next grimoire.\"",
         "\"%d cannot protect you now.  Here, you die.\"",
         "\"Your %a nature makes you weak.  You cannot defeat me.\"",
         "\"Come, %c.  I shall kill you, then unleash the horde on your tribe.\"",
         "\"Once you are dead, my horde shall finish off %l, and your tribe.\"",
         "\"Fight, %c, or are you afraid of the mighty %n?\"",
         "\"You have failed, %c.  Now, my victory is complete.\"",
      },
      encourage = {
         "\"%nC is strong in the dark arts, but not immune to cold steel.\"",
         "\"Remember that %n is a great sorcerer.  He lived in the time of Atlantis.\"",
         "\"If you fail, %p, I will not be able to protect these people long.\"",
         "\"To enter %i, you must be very stealthy.  The horde will be on guard.\"",
         "\"Call upon %d in your time of need.\"",
         "\"May %d protect you, and guide your steps.\"",
         "\"If you can lay hands upon %o, carry it for good fortune.\"",
         "\"I cannot stand against %ns sorcery.  But %d will help you.\"",
         "\"Do not fear %n.  I know you can defeat %ni.\"",
         "\"You have a great road to travel, %p, but only after you defeat %n.\"",
      },
      firsttime = {
         synopsis = "[You reach the vicinity of %H, but sense evil magic nearby.]",
         output = "text",
         text = [[Warily you scan your surroundings, all of your senses alert for signs
of possible danger.  Off in the distance, you can %x the familiar shapes
of %H.

But why, you think, should %l be there?

Suddenly, the hairs on your neck stand on end as you detect the aura of
evil magic in the air.

Without thought, you ready your weapon, and mutter under your breath:

    "By %d, there will be blood spilt today."]],
      },
      goal_first = {
         synopsis = "[This is surely the lair of %n.]",
         output = "text",
         text = [[
Ahead, you can make out the glow of fire and the dark shapes of
buildings.  The hairs on the nape of your neck lift as you sense an
energy in the very air around you.  You fight down a primordial panic
that seeks to make you turn and run.  This is surely the lair of %n.
         ]],
      },
      goal_next = {
         text = "Yet again you feel the air around you heavy with malevolent magical energy.",
      },
      gotit = {
         synopsis = "[You feel the power of %o flowing through your hands.]",
         output = "text",
         text = [[As you pick up %o, you feel the power of it
flowing through your hands.  It seems to be in two or more places
at once, even though you are holding it.]],
      },
      guardtalk_after = {
         "\"The battles here have been good -- our enemies' blood soaks the soil!\"",
         "\"Remember that glory is crushing your enemies beneath your feet!\"",
         "\"Times will be good again, now that the horde is vanquished.\"",
         "\"You have brought our clan much honor in defeating %n.\"",
         "\"You will be a worthy successor to %l.\"",
      },
      guardtalk_before = {
         "\"The battles here have been good -- our enemies' blood soaks the soil!\"",
         "\"Remember that glory is crushing your enemies beneath your feet!\"",
         "\"There has been little treasure to loot, since the horde arrived.\"",
         "\"The horde is mighty in numbers, but they have little courage.\"",
         "\"%lC is a strange one, but he has helped defend us.\"",
      },
      hasamulet = {
         synopsis = "[\"Take the Amulet to the altar of %d on the Astral Plane and offer it.\"]",
         output = "text",
         text = [["This is wondrous, %p.  I feared that you could not possibly
succeed in your quest, but here you are in possession of the Amulet
of Yendor!

"I have studied the texts of the magi constantly since you left.  In
the Book of Skelos, I found this:

    %d will cause a child to be sent into the world.  This child is to
    be made strong by trial of battle and magic, for %d has willed it so.
    It is said that the child of %d will recover the Amulet of Yendor
    that was stolen from the Creator at the beginning of time.

"As you now possess the amulet, %p, I suspect that the Book
speaks of you.

    The child of %d will take the Amulet, and travel to the Astral
    Plane, where the Great Temple of %d is to be found.  The Amulet
    will be sacrificed to %d, there on %dJ altar.  Then the child will
    stand by %d as champion of all %cP for eternity.

"This is all I know, %p.  I hope it will help you."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you, but you feel the overpowering aura of magic fading.]",
         output = "text",
         text = [[%nC falls to the ground, and utters a last curse at you.  Then %nj
body fades slowly, seemingly dispersing into the air around you.  You
slowly become aware that the overpowering aura of magic in the air has
begun to fade.]],
      },
      leader_first = {
         synopsis = "[\"At last you have returned.  There is a great quest you must undertake.\"]",
         output = "text",
         text = [["Ah, %p.  You have returned at last.  The world is in dire
need of your help.  There is a great quest you must undertake.

"But first, I must see if you are ready to take on such a challenge."]],
      },
      leader_last = {
         synopsis = "[\"You have betrayed %d; soon %n will destroy us.  Begone!\"]",
         output = "text",
         text = [["Pah!  You have betrayed the gods, %p.  You will never attain
the glory which you aspire to.  Your failure to follow the true path has
closed this future to you.

"I will protect these people as best I can, but soon %n will overcome
me and destroy all who once called you %s.  Now begone!"]],
      },
      leader_next = {
         text = "\"%p, you are back.  Are you ready now for the challenge?\"",
      },
      leader_other = {
         text = "\"Again, you stand before me, %p.  Surely you have prepared yourself.\"",
      },
      locate_first = {
         synopsis = "[You have located %i.]",
         output = "text",
         text = [[
As the sun sinks below the horizon, the scent of water comes to you in
the desert breeze.  You know that you have located %i.
         ]],
      },
      locate_next = {
         text = "Yet again you have a chance to infiltrate %i.",
      },
      lowlevel = {
         synopsis = "[Are you sure you can face %n?]",
         output = "text",
         text = [[
"%p, you seem very inexperienced. Are you sure that you can
face %n?"]],
      },
      nemesis_first = {
         synopsis = "[%nC boasts that %nh has slain many.  \"Prepare to die, %c.\"]",
         output = "text",
         text = [["So.  This is what that second rate sorcerer %l sends to do %lj bidding.
I have slain many before you.  You shall give me little sport.

"Prepare to die, %c."]],
      },
      nemesis_next = {
         text = "\"I have wasted too much time on you already.  Now, you shall die.\"",
      },
      nemesis_other = {
         text = "\"You return yet again, %c!  Are you prepared for death now?\"",
      },
      nemesis_wantsit = {
         text = [["I shall have %o back, you pitiful excuse for %ca.
And your life as well."]],
      },
      nexttime = {
         text = [[Once again, you near %H.  You know that %l
will be waiting.]],
      },
      offeredit = {
         synopsis = "[%lC tells you to guard %o, and to return when you have triumphed.]",
         output = "text",
         text = [[When %l sees %o, he smiles, and says:

    Well done, %p.  You have saved the world from certain doom.
    What, now, should be done with %o?

    These people, brave as they are, cannot hope to guard it from
    other sorcerers who will detect it, as surely as %n did.

    Take %o with you, %p.  It will guard you in
    your adventures, and you can best guard it.  You embark on a
    quest far greater than you realize.

    Remember me, %p, and return when you have triumphed.  I
    will tell you then of what you must do.  You will understand when the
    time comes.]],
      },
      offeredit2 = {
         synopsis = "[\"You keep %o.  Return to %Z to search for the Amulet.\"]",
         output = "text",
         text = [[%l gazes reverently at %o, then back at you.

"You are its keeper now, and the time has come to resume your search
for the Amulet.  %Z await your return through the
magic portal which brought you here."]],
      },
      othertime = {
         text = [[Again, and you think possibly for the last time, you approach
%H.]],
      },
      posthanks = {
         text = "\"Tell us, %p, have you fared well on your great quest?\"",
      },
   },
   Cav = {
      assignquest = {
         synopsis = "[Go beat %n.  Get %o.  Bring back.]",
         output = "text",
         text = [[
"Yes, you good now, %p.  Me tell you what happen to us.

"Right after you leave, Bad Men come to caves, fight us.
They come with %n.  This one very bad.  It stomp.
It bite.  It claw.  It breathe on us, burn us, freeze us,
hurt us lots of ways.  It lead the Bad Men to fight us.
Some of us die.

"While we hurt, Bad Men take %o.  They take it to %i.
We can not get it back.  %n will kill us.

"%p, you find %i, take back %o from %n.  Then we can
make caves safe."
         ]],
      },
      badalign = {
         synopsis = "[\"You not acting %a well.  Go fix self.\"]",
         output = "text",
         text = [[
"%pC!  You not do like me teach you.  You not do right things
on true path of %a.  Me send you from our caves.  Go, %p.
Go away and make self right.  Then maybe you can do this quest."
         ]],
      },
      badlevel = {
         synopsis = "[%rA not ready.  Go do other things.]",
         output = "text",
         text = [[
"No, %p, you not ready to go on such hard quest as what me give you.

"%rA can not do what it take to find %i, could not fight %n, could not
bring back %o.

"Go do more things.  You learn stuff you need.
%d say so."
         ]],
      },
      banished = {
         synopsis = "[You cast out from %H for rest of your life.]",
         output = "text",
         text = [[
"You turn your back on us.  You turn your back on %d!
You not part of this tribe no more.

Get out.  You not come back ever.
You never get God Thing now.  You not get Bright Thing from %n.
You not have Bright Thing, you not able to go to place of God Thing.

Go! Or else we eat you.]],
      },
      discourage = {
         -- Note that Tiamat doesn't use caveman speak.
         "\"You are weak, %c.  No challenge for the Mother of all Dragons.\"",
         "\"I grow hungry, %r.  You look like a nice appetizer!\"",
         "\"Join me for lunch?  You're the main course, %c.\"",
         "\"With the power of %o, I will kill your tribe.\"",
         "\"Your mentor, %l has failed.  You are nothing to fear.\"",
         "\"You shall die here, %c.  %rA cannot hope to defeat me.\"",
         "\"You, a mere %r challenge the might of %n?  Hah!\"",
         "\"I am the Mother of all Dragons!  You cannot hope to defeat me.\"",
         "\"My claws are sharp now.  I shall rip you to shreds!\"",
         "\"%d has deserted you, %c.  This is my domain.\"",
      },
      encourage = {
         "\"%nC not hurt by her own breath.  You should use magic that she not use, maybe she not know how to beat that.\"",
         "\"When you fight %n, call to %d for help.\"",
         "\"There no place to hide in %ns cave.\"",
         "\"Best way to not die when %n fight you is, run away.\"",
         "\"You no stop to take pretty things in %ns lair.  Just make sure you get %o.\"",
         "\"If you take %o from %n, maybe you use it to hit her.\"",
         "\"Do not be fooled by %ns size.  She is fast and has magic too.\"",
         "\"Me would send %gP with you, but we need them to fight here, keep our cave.\"",
         "\"If you kill %n, you be big hero here.\"",
         "\"Watch out for %ns breath.  It hurt lots of ways.\"",
      },
      firsttime = {
         synopsis = "[You come to %H.  It wrong.]",
         output = "text",
         text = [[
You come to %H.  It not right.  There no meat in cave.
There no mammoth hides.  Men are sad.  Things broke.  Fire
not lit.  Caves not right, make you sad.  You look for %l.
Him tell you what is wrong.
         ]],
      },
      goal_first = {
         synopsis = "[You come to big cave.  %nC is here.]",
         output = "text",
         text = [[
You come to big cave, with shiny walls.  Walls look like they
have been burned.

You see bones on the floor, and other things.
It smell bad in here, like smoke from fire mountain.

%nC is here, but it seem like she is asleep.
         ]],
      },
      goal_next = {
         text = "You come to lair of %n again.",
      },
      gotit = {
         synopsis = "[%oC make you feel strong and good.]",
         output = "text",
         text = [[
As you pick up %o it seem heavy at first, but then
your arms feel strong.

You feel like you can beat up anyone now.
         ]],
      },
      guardtalk_after = {
         "\"We hunt mammoth again.\"",
         "\"Peace back.  Peace good.  Give thanks to %d!\"",
         "\"You back!  You find %o?\"",
         "\"Strong %p, tell us how you fight %n.\"",
         "\"%lC old.  Maybe you guide us after him die.\"",
      },
      guardtalk_before = {
         "\"We not get much food since Big Men seal off %H.\"",
         "\"Since Bad Men come with %n, we fight all the time.\"",
         "\"You think we beat %n, take back %o?\"",
         "\"Hey, %p, you help?\"",
         "\"%lC old.  We not know who guide us after him die.\"",
      },
      hasamulet = {
         synopsis = "[\"Take God Thing up to %d in place of gods.\"]",
         output = "text",
         text = [[
"Oh, good, %p, you find God Thing for %d.

"Take God Thing up to sky.  It take you where you need to go,
to place of earth, place of fire, place of air, place of water,
and place of gods, where the Great House of %d is.

"Take God Thing up there."
         ]],
      },
      killed_nemesis = {
         text = [[
%nC sink to ground.  She move a lot, then she not move.
Big cloud of smelly smoke come out of all her heads.
         ]],
      },
      leader_first = {
         synopsis = "[\"You back.  We need big help.\"]",
         output = "text",
         text = [[
"You come back, %p.  Thank %d.

"We need big help, my %S.

"But first, me see if you can do things me ask you to do."
         ]],
      },
      leader_last = {
         synopsis = "[\"You not %L.  Leave!\"]",
         output = "text",
         text = [[
"%pC!  You not good enough.  Seems you not able to get ready.
Me pick other one from tribe to take your place.

"Leave %H!  You let us down.  You choose
path of %C over true path of %L.

"You not live in our eyes."
         ]],
      },
      leader_next = {
         text = "\"You back, %p.  Let me see if you ready now.\"",
      },
      leader_other = {
         text = "\"Ah, %p.  You ready?\"",
      },
      locate_first = {
         synopsis = "[You %x many big claw marks and bones, smell dead things.]",
         output = "text",
         text = [[
You %x many big claw marks and bones on ground.  Cave ahead of you
is larger than most cave you have been in before.

You smell dead things in there.
         ]],
      },
      locate_next = {
         text = "Once again, you approach %i.",
      },
      lowlevel = {
         synopsis = "[You very small.  You sure?]",
         output = "text",
         text = [[
"Me not know, %p, you sure you not need to do more stuff first?
It real hard to fight %n and you very small.  You sure you can do it?"]],
      },
      nemesis_first = {
         synopsis = "[%nC threatens to eat you.]",
         output = "text",
         text = [["So, follower of %l, you seek to invade the lair of
%n.  Only my meals are allowed down here.  Prepare
to be eaten!"]],
      },
      nemesis_next = {
         text = [["So, again you face me, %c.  No one has ever before escaped me.
Now I shall kill you."]],
      },
      nemesis_other = {
         text = "\"You are getting annoying, %c.  Prepare to die.\"",
      },
      nemesis_wantsit = {
         text = "\"I'll have %o from you, %c.  You shall die.\"",
      },
      nexttime = {
         text = "You come to %H again.",
      },
      offeredit = {
         synopsis = "[\"Take %o with you.  It maybe help you on big quest.\"]",
         output = "text",
         text = [[
%lC see %o in your hands.
Him smile and say:

    You have it!  We saved now.  But me fear that Bad Men still
    want to take it from us.

    You keep it safe, %p, take %o away with you.  It
    help you find other thing you look for.
         ]],
      },
      offeredit2 = {
         synopsis = "[\"You keep %o now.  Return to %Z.\"]",
         output = "text",
         text = [[
%l take %o, then him look at you.

"You keep it now.  Keep it safe, go look for other
thing, thing for %d.  Go back though magic thing that
brought you here."
         ]],
      },
      othertime = {
         text = "You think this may be last time you come to %H."
      },
      posthanks = {
         text = "%pC!  You back.  Did you find other thing %d told you to look for?"
      },
   },
   Hea = {
      assignquest = {
         synopsis = "[Travel to %i on your way to recover %o from %n.]",
         output = "text",
         text = [[For the first time, you sense a smile on %ls face.

    "You have indeed learned as much as we can teach you in preparation
    for this task.  Let me tell you what I know of the symptoms and hope
    that you can provide a cure.

    "A short while ago, the dreaded %nt was fooled by the gods
    into thinking that %nh could use %o to find a
    cure for old age.  Think of it, eternal youth!  But %nj good
    health is accomplished by drawing the health from those around %ni.

    "He has exhausted %nj own supply of healthy people and now %nh seeks to
    extend %nj influence into our world.  You must recover from %ni
    %o and break the spell.

    "You must travel into the swamps to %i, and from there
    follow the trail to %ns island lair.  Be careful."]],
      },
      badalign = {
         synopsis = "[Return when you are more %a.]",
         output = "text",
         text = [["You have learned much of the remedies that benefit, but you must also
know which physic for which ail.  That is why %ds teachings are a
part of your training.

"Return to us when you have healed thyself."]],
      },
      badlevel = {
         synopsis = "[You are too inexperienced.  Return when you are %Ra.]",
         output = "text",
         text = [["Alas, %p, you are yet too inexperienced to deal with the rigors
of such a task.  You must be able to draw on the knowledge of botany,
alchemy and veterinary practices before I can send you on this quest
with good conscience.

"Return when you wear %Ra's caduceus."]],
      },
      discourage = {
         "\"They might as well give scalpels to wizards as to let you try to use %o!\"",
         "\"If I could strike %l, surrounded by %lj %gP, imagine what I can do to you here by yourself.\"",
         "\"I will put my %Rp to work making a physic out of your ashes.\"",
         "\"As we speak, Hades gathers your patients to join you.\"",
         "\"After I'm done with you, I'll destroy %l as well.\"",
         "\"You will have to kill me if you ever hope to leave this place.\"",
         "\"I will impale your head on my caduceus for all to see.\"",
         "\"There is no materia medica in your sack which will cure you of me!\"",
         "\"Do not fight too hard, I want your soul strong, not weakened!\"",
         "\"You should have stopped studying at vetenary.\"",
      },
      encourage = {
         "\"Remember, %p, to always wash your hands before operating.\"",
         "\"%nC has no real magic of %nj own.  To this %nh is vulnerable.\"",
         "\"If you have been true to %d, you can draw on the power of %o.\"",
         "\"Bring with you antidotes for poisons.\"",
         "\"Remember this, %n can twist the powers of %o to hurt instead of heal.\"",
         "\"I have sent for Chiron, but I am afraid he will come too late.\"",
         "\"Maybe when you return the snakes will once again begin to shed.\"",
         "\"The plague grows worse as we speak.  Hurry, %p!\"",
         "\"Many times %n has caused trouble in these lands.  It is time that %nh was eradicated like the diseases %nh has caused.\"",
         "\"With but one eye, %n should be easy to blind.  Remember this.\"",
      },
      firsttime = {
         synopsis = "[You arrive back at %H and must find %l.]",
         output = "text",
         text = [[What sorcery has brought you back to %H?  The smell
of fresh funeral pyres tells you that something is amiss with the healing
powers that used to practice here.

No rhizotomists are tending the materia medica gardens, and where are the
common folk who used to come for the cures?

You know that you must quickly make your way to the collegium, and
%ls iatreion, and find out what has happened in your absence.]],
      },
      goal_first = {
         synopsis = "[You have reached the lair of %n.  Take %o away from %ni.]",
         output = "text",
         text = [[You stand within sight of the infamous Isle of %n.  Even
the words of %l had not prepared you for this.

Steeling yourself against the wails of the ill that pierce your ears,
you hurry on your task.  Maybe with %o you can
heal them on your return, but not now.]],
      },
      goal_next = {
         text = "Once again, you %x the Isle of %n in the distance.",
      },
      gotit = {
         synopsis = "[You feel the healing power of %o and should return it to %l.]",
         output = "text",
         text = [[As you pick up %o, you feel its healing begin to
warm your soul.  You curse Zeus for taking it from its rightful owner,
but at least you hope that %l can put it to good use once
again.]],
      },
      guardtalk_after = {
         "\"Did you read that new treatise on the therapeutic use of leeches?\"",
         "\"Paint a red caduceus on your shield and monsters won't hit you.\"",
         "\"How are you feeling?  Perhaps a good bleeding will improve your spirits.\"",
         "\"Have you heard the absurd new theory that diseases are caused by microscopic organisms, and not ill humors?\"",
         "\"I see that you bring %o, now you can cure this plague!\"",
      },
      guardtalk_before = {
         "\"Did you read that new treatise on the therapeutic use of leeches?\"",
         "\"Paint a red caduceus on your shield and monsters won't hit you.\"",
         "\"I passed handwriting so they are demoting me a rank.\"",
         "\"I've heard that even %l has not been able to cure Chiron.\"",
         "\"We think %n has used %nj alchemists, and %o, to unleash a new disease we call 'the cold' on Gehennom.\"",
      },
      hasamulet = {
         synopsis = "[\"You have recovered the Amulet.  Travel to the Astral Plane and return it to %d.\"]",
         output = "text",
         text = [["Ah, you have recovered the Amulet, %p.  Well done!

"Now, you should know that you must travel through the Elemental Planes
to the Astral, and there return the Amulet to %d.  Go forth and
may our prayers be as a wind upon your back."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you as %nh dies.]",
         output = "text",
         text = [[The battered body of %n slumps to the ground and gasps
out one last curse:

    "You have defeated me, %p, but I shall have my revenge.
    How, I shall not say, but this curse shall be like a cancer
    on you."

With that %n dies.]],
      },
      leader_first = {
         synopsis = "[%l is weak from the struggle with %n.  %lH wants to examine you.]",
         output = "text",
         text = [[Feebly, %l raises %lj head to look at you.

"It is good to see you again, %p.  I see the concern in your
eyes, but do not worry for me.  I am not ready for Hades yet.  We have
exhausted much of our healing powers holding off %n.
I need your fresh strength to carry on our work.

"Come closer and let me lay hands on you, and determine if you have
the skills necessary to accomplish this mission."]],
      },
      leader_last = {
         synopsis = "[You are a failure as a healer.]",
         output = "text",
         text = [["You have failed us, %p.  You are a quack!  A charlatan!

"Hades will be happy to hear that you are once again practicing your
arts on the unsuspecting."]],
      },
      leader_next = {
         text = [["Again you return to me, %p.  I sense that each trip back
the pleurisy and maladies of our land begin to infect you.  Let us
hope and pray to %d that you become ready for your task before
you fall victim to the bad humors."]],
      },
      leader_other = {
         text = [["Chiron has fallen, Hermes has fallen, what else must I tell you to
impress upon you the importance of your mission!  I hope that you
have come prepared this time."]],
      },
      locate_first = {
         synopsis = "[You have reached %i but all is not well.]",
         output = "text",
         text = [[You stand before the entrance to %i.  Strange
scratching noises come from within the building.

The swampy ground around you seems to stink with disease.]],
      },
      locate_next = {
         text = "Once again you stand at the entrance to %i.",
      },
      lowlevel = {
         synopsis = "[You are very inexperienced.  Really proceed?]",
         output = "text",
         text = [[
"Alas, %p, I fear that you are far too inexperienced to deal with the
rigors of such a task.  Are you certain you are really ready to undertake it?"]],
      },
      nemesis_first = {
         synopsis = "[\"I will take your life, then defeat %l.\"]",
         output = "text",
         text = [["They have made a mistake in sending you, %p.

"When I add your youth to mine, it will just make it easier for me
to defeat %l."]],
      },
      nemesis_next = {
         text = "\"Unlike your patients, you seem to keep coming back, %p!\"",
      },
      nemesis_other = {
         text = "\"Which would you like, %p?  Boils, pleurisy, convulsions?\"",
      },
      nemesis_wantsit = {
         text = [["I'll have %o back from you, %r.  You are
not going to live to escape this place."]],
      },
      nexttime = {
         text = [[After your last experience you expected to be here, but you certainly
did not expect to see things so much worse.  This time you must succeed.]],
      },
      offeredit = {
         synopsis = "[%l touches %o and tells %lj %gP to do so too, then tells you to take it with you.]",
         output = "text",
         text = [[As soon as %l sees %o %lh summons %lj
%gP.

Gently, %l reaches out and touches %o.
He instructs each of the assembled to do the same.  When everyone
has finished %lh speaks to you.

    "Now that we have been replenished we can defeat this plague.  You must
    take %o with you and replenish the worlds you have
    been called upon to travel next.  I wish you could ride Chiron to the
    end of your journey, but I need him to help me spread the cure.  Go
    now and continue your journey."]],
      },
      offeredit2 = {
         synopsis = "[%l tells you to keep %o and return to %Z to search for the Amulet.]",
         output = "text",
         text = [[%l cautiously handles %o while watching you.

"You are its keeper now, and the time has come to resume your search
for the Amulet.  %Z await your return through the
magic portal which brought you here."]],
      },
      othertime = {
         text = [[Again, you %x %H in the distance.

The smell of death and disease permeates the air.  You do not have
to be %Ra to know that %n is on the verge of victory.]],
      },
      posthanks = {
         text = [["You have again returned to us, %p.  We have done well in your
absence, yes?  How fare you upon your quest for the Amulet?"]],
      },
   },
   Kni = {
      assignquest = {
         synopsis = "[Pass through %i to reach %n.  Destroy %ni and return with %o.]",
         output = "text",
         text = [["Ah, %p.  Thou art truly ready, as no %c before thee hath
been.  Hear now Our words:

"As thou noticed as thou approached %H, a great battle hath
been fought recently in these fields.  Know thou that Merlin himself
came to aid Us here as We battled the foul %n.  In the midst of that
battle, %n struck Merlin a great blow, felling him.  Then, as Our
forces were pressed back, %n stole %o.

"We eventually turned the tide, but lost many %cP in doing so.
Merlin was taken off by his apprentice, but hath not recovered.  We have
been told that so long as %n possesseth %o,
Merlin will not regain his health.

"We hereby charge thee with this most important of duties:

"Go forth from this place, to the fens, and there thou wilt find
%i.  From there, thou must track down %n.  Destroy the
beast, and return to Us %o.  Only then can
We restore Merlin to health."]],
      },
      badalign = {
         synopsis = "[Go and do penance.  Return when you are truly %a.]",
         output = "text",
         text = [["Thou dishonourest Us, %p!  Thou hast strayed from the path of
chivalry! Go from Our presence and do penance.  Only when thou art again
pure mayst thou return hence."]],
      },
      badlevel = {
         synopsis = "[You are not prepared to face %n.  Return when you are %Ra.]",
         output = "text",
         text = [["Verily, %p, thou hast done well.  That thou hast survived thus
far is a credit to thy valor, but thou art yet unprepared for
the demands required as Our Champion.  %rA, no matter how
pure, could never hope to defeat the foul %n.

"Journey forth from this place, and hone thy skills.  Return to
Our presence when thou hast attained the noble title of %R."]],
      },
      discourage = {
         "\"A mere %r can never withstand me!\"",
         "\"I shall kill thee now, and feast!\"",
         "\"Puny %c.  What manner of death dost thou wish?\"",
         "\"First thee, %p, then I shall feast upon %l.\"",
         "\"Hah!  Thou hast failed, %r.  Now thou shalt die.\"",
         "\"Die, %c.  Thou art as nothing against my might.\"",
         "\"I shall suck the marrow from thy bones, %c.\"",
         "\"Let's see...  Baked?  No.  Fried?  Nay.  Broiled?  Yea verily, that is the way I like my %c for dinner.\"",
         "\"Thy strength waneth, %p.  The time of thy death draweth near.\"",
         "\"Call upon thy precious %d, %p.  It shall not avail thee.\"",
      },
      encourage = {
         "\"Remember, %p, follow always the path of %d.\"",
         "\"Though %n is verily a mighty foe, We have confidence in thy victory.\"",
         "\"Beware, for %n hath surrounded %niself with hordes of foul creatures.\"",
         "\"Great treasure, 'tis said, is hoarded in the lair of %n.\"",
         "\"If thou possessest %o, %p, %ns magic shall therewith be thwarted.\"",
         "\"The gates of %i are guarded by forces unseen, %p. Go carefully.\"",
         "\"Return %o to Us quickly, %p.\"",
         "\"Destroy %n, %p, else %H shall surely fall.\"",
         "\"Call upon %d when thou art in need.\"",
         "\"To find %i, thou must keep thy heart pure.\"",
      },
      firsttime = {
         synopsis = "[Signs of battle include long gouges in the walls of %H.]",
         output = "text",
         text = [[You materialize in the shadows of %H.  Immediately, you notice
that something is wrong.  The fields around the castle are trampled and
withered, as if some great battle has been recently fought.

Exploring further, you %x long gouges in the walls of %H.
You know of only one creature that makes those kinds of marks...]],
      },
      goal_first = {
         synopsis = "[You %x the entrance to a cavern inside a hill.]",
         output = "text",
         text = [[As you exit the swamps, you %x before you a huge, gaping hole in the
side of a hill.  From within, you smell the foul stench of carrion.

The pools on either side of the entrance are fouled with blood, and
pieces of rusted metal and broken weapons show above the surface.]],
      },
      goal_next = {
         text = "Again, you stand at the entrance to %ns lair.",
      },
      gotit = {
         synopsis = "[You feel the magic of %o.]",
         output = "text",
         text = [[As you pick up %o, you feel its protective fields
form around your body.  You also feel a faint stirring in your mind, as
if you are in two places at once, and in the second, you are waking from
a long sleep.]],
      },
      guardtalk_after = {
         "\"Hail, %p!  Verily, thou lookest well.\"",
         "\"So, %p, didst thou find %n in the fens near %i?\"",
         "\"Worthy %p, hast thou proven thy right purpose on the body of %n?\"",
         "\"Verily, %l could have no better champion, %p.\"",
         "\"Hast thou indeed recovered %o?\"",
      },
      guardtalk_before = {
         "\"Hail, %p!  Verily, thou lookest well.\"",
         "\"There is word, %p, that %n hath been sighted in the fens near %i.\"",
         "\"Thou art our only hope now, %p.\"",
         "\"Verily, %l could have no better champion, %p.\"",
         "\"Many brave %cP died when %n attacked.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and deliver it to %d.]",
         output = "text",
         text = [["Thou hast succeeded, We see, %p!  Now thou art commanded to take
the Amulet to be sacrificed to %d in the Plane of the Astral.

"Merlin hath counseled Us that thou must travel always upwards through
the Planes of the Elements, to achieve this goal.

"Go with %d, %p."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you as %nh dies.]",
         output = "text",
         text = [[As %n sinks to the ground, blood gushing from %nj open mouth, %nh
defiantly curses you and %l:

    "Thou hast not won yet, %r.  By the gods, I shall return
    and dog thy steps to the grave!"

%nJ tail flailing madly, %n tries to crawl towards you, but slumps
to the ground and dies in a pool of %nj own blood.]],
      },
      leader_first = {
         synopsis = "[%lC checks whether you are ready for a great undertaking.]",
         output = "text",
         text = [["Ah, %p.  We see thou hast received Our summons.
We are in dire need of thy prowess.  But first, We must needs
decide if thou art ready for this great undertaking."]],
      },
      leader_last = {
         synopsis = "[You are a disgrace as %ca.]",
         output = "text",
         text = [["Thou disgracest this noble court with thine impure presence.  We have been
lenient with thee, but no more.  Thy name shall be spoken no more.  We
hereby strip thee of thy title, thy lands, and thy standing as %ca.
Begone from Our sight!"]],
      },
      leader_next = {
         text = "\"Welcome again, %p.  We hope thou art ready now.\"",
      },
      leader_other = {
         text = "\"Once again, thou standest before Us, %p.  Art thou ready now?\"",
      },
      locate_first = {
         synopsis = "[You have reached %i and can %x a shrine.]",
         output = "text",
         text = [[You stand at the foot of %i.  Atop, you can %x a shrine.
Strange energies seem to be focused here, and the hair on the back of
your neck stands on end.]],
      },
      locate_next = {
         text = "Again, you stand at the foot of %i.",
      },
      lowlevel = {
         synopsis = "[Art thou sure thou wish to undertake this?]",
         output = "text",
         text = [[
"Art thou serious, %p?  Wilt thou undertake this quest already,
in thine inexperienced state?"]],
      },
      nemesis_first = {
         synopsis = "[%nC taunts you and issues a threat against %H.]",
         output = "text",
         text = [["Hah!  Another puny %c seeks death.  I shall dine well tonight,
then tomorrow, %H shall fall!"]],
      },
      nemesis_next = {
         text = "\"Again, thou challengest me, %r?  So be it.  Thou wilt die here.\"",
      },
      nemesis_other = {
         text = "\"Thou art truly foolish, %r.  I shall dispatch thee anon.\"",
      },
      nemesis_wantsit = {
         text = [["So, thou darest touch MY property!  I shall have that bauble back,
puny %r.  Thou wilt die in agony!"]],
      },
      nexttime = {
         text = "Once again you stand in the shadows of %H.",
      },
      offeredit = {
         synopsis = "[%oC is yours now.  It will aid in your search for the Amulet.]",
         output = "text",
         text = [[As you approach %l, %lh beams at you and says:

    "Well done!  Thou art truly the Champion of %H.  We
    have received word that Merlin is recovering, and shall soon
    rejoin Us.

    "He hath instructed Us that thou art now to be the guardian of
    %o.  He feeleth that thou mayst have need of
    its powers in thine adventures.  It is Our wish that thou keepest
    %o with thee as thou searchest for the fabled
    Amulet of Yendor."]],
      },
      offeredit2 = {
         synopsis = "[You are the keeper of %o.  Return to %Z and find the Amulet.]",
         output = "text",
         text = [["Careful, %p!  %oC might break, and that would
be a tragic loss.  Thou art its keeper now, and the time hath come
to resume thy search for the Amulet.  %Z await thy
return through the magic portal that brought thee here."]],
      },
      othertime = {
         text = [[Again, you stand before %H.  You vaguely sense that this
may be the last time you stand before %l.]],
      },
      posthanks = {
         text = "\"Well met, %p.  How goeth thy search for the Amulet of Yendor?\"",
      },
   },
   Mon = {
      assignquest = {
         synopsis = "[Find %i, then continue to %ns lair.  Defeat %ni and return with %o.]",
         output = "text",
         text = [[
"Yes, %p.  You are truly ready now.  Listen now to the story of our
woes:

"A few nights ago, %n led a surprise assault on
%H, along with dozens of elementals.
Our %gP fought bravely, but too many of them were slaughtered,
including the one bearing %o.
The %gP that remain are barely enough to defend this hall.

"Travel through the wilderness until you reach
%i, then, from there, descend into
%ns lair.
Defeat %n and bring back %o, and with them
we will be able to reestablish %H as a center of
spiritual guidance.

"Go bravely with %d, %p."
         ]],
      },
      badalign = {
         synopsis = "[You are losing your way.  Come back when your harmony is restored.]",
         output = "text",
         text = [[
"You seem troubled, %p.  It looks as though you are letting worldly
concerns begin to turn you away from enlightenment and the %a way.

There is still some hope for you, though.  Go from here and meditate on
your transgressions, and return once you have restored harmony with
yourself and with %d."
         ]],
      },
      badlevel = {
         synopsis = "[You are not ready to face %n.  Come back when you are %Ra.]",
         output = "text",
         text = [[
"Though your progress is adequate, no %r can stand up to %n.
You must continue your martial and mental training.  Return to us when
you have attained the post of %R."
         ]],
      },
      discourage = {
         "\"Submit to my will, %c, and I shall spare you.\"",
         "\"Your puny powers are no match for me, %c.\"",
         "\"I shall have you turned into a zombie for my pleasure!\"",
         "\"Despair now, %r.  %d cannot help you.\"",
         "\"I shall feast upon your soul for many days, %c.\"",
         "\"Your death will be slow and painful.  That I promise!\"",
         "\"You cannot defeat %n, you fool.  I shall kill you now.\"",
         "\"Your precious %lt will be my next victim.\"",
         "\"I feel your powers failing you, %r.  You shall die now.\"",
         "\"With %o, nothing can stand in my way.\"",
      },
      encourage = {
         "\"Though unenlightened, %n is an incredibly dangerous martial artist.\"",
         "\"In addition to his physical threats, %n is also a master of dark magic.\"",
         "\"Keep you mind clear, my %S.\"",
         "\"The path to %i is steep, and you will face many foes there.\"",
         "\"Remember your studies, and you will prevail!\"",
         "\"Acquire and wear %o if you can.  They will aid you against %n.\"",
         "\"It will not do for you to be unfocused.  Meditate on your mission if you must.\"",
         "\"We have some small amount of supplies here; take any that you may need.\"",
         "\"%n may try to trick and distract you.  Be steadfast.\"",
         "\"I must continue to hold this hall against the elementals while you are gone.\"",
      },
      firsttime = {
         synopsis = "[You have reached %H but something is wrong.  %lC needs your aid.]",
         output = "text",
         text = [[
You find yourself standing in sight of %H.

But what is this? The great gates are sealed shut, and ominous noises
echo from the grounds around you.

You must consult with %l at once!
         ]],
      },
      goal_first = {
         synopsis = "[You are surrounded by brimstone, lava, and elementals.]",
         output = "text",
         text = [[The stench of brimstone is all about you, and the elementals close in
from all sides!

Ahead, there is a small clearing amidst the bubbling pits of lava...]],
      },
      goal_next = {
         text = "Again, you have invaded %ns domain.",
      },
      gotit = {
         synopsis = "[You feel the essence of %d and realize that you should take %o to %l.]",
         output = "text",
         text = [[As you pick up %o, you feel the essence of
%d fill your soul.  You know now why %n stole %oi from
%H, for with %oi, %ca of %d could
easily defeat his plans.

You sense a message from %d.  Though not verbal, you
get the impression that you must return to %l as soon
as possible.]],
      },
      guardtalk_after = {
         "\"Greetings, honorable %r.  It is good to see you again.\"",
         "\"%l teaches us not to focus on earthly beauty.\"",
         "\"With this test behind you, may %d bring you enlightenment.\"",
         "\"Good day, %p.  The repair efforts have been going well.\"",
         "\"Got any wands of enlightenment to spare?\"",
      },
      guardtalk_before = {
         "\"Greetings, honorable %r.  It is good to see you.\"",
         "\"Our beautiful monastery lies in shambles, %p.\"",
         "\"Welcome back, %p.  Will you join me in meditation?\"",
         "\"It will take long to repair the damage from the elementals.\"",
         "\"Got any potions of enlightenment to spare?\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and deliver it to %d.]",
         output = "text",
         text = [[
"You have prevailed, %p!  %d is surely with you.  Now,
you must bring the Amulet up out of the dungeon, and sacrifice it on
%ds altar on the Astral Plane, where you may reach true nirvana at last.
All of us here will continue to keep you in our thoughts."
         ]],
      },
      killed_nemesis = {
         synopsis = "[As %n dies, %nh threatens to return.]",
         output = "text",
         text = [[%nC gasps:

    "You have only defeated this mortal body.  Know this: my spirit
    is strong.  I shall return and reclaim what is mine!"

With that, %n expires.]],
      },
      leader_first = {
         synopsis = "[%lC checks whether you are adequately prepared to help.]",
         output = "text",
         text = [[
"Ah, %p, my %S.
Hard times have fallen on our peaceful monastery, but you may be able to
help.  But first, let us see how your training has continued while you
have been away from us."
         ]],
      },
      leader_last = {
         synopsis = "[You are unenlightened and are expelled from the monastery.]",
         output = "text",
         text = [[
"You are a disgrace, %p!  I can see that your mind has turned away from
the path of enlightenment.  We will never defeat %n if you are our
champion.

You are no longer %sa here.  Return to the dungeon from whence you came,
and never again return."
         ]],
      },
      leader_next = {
         text = "\"Again, my %S, you stand before me.  Are you ready now to help us?\"",
      },
      leader_other = {
         text = "\"Once more, %p, you stand within the sanctum.  Are you ready now?\"",
      },
      locate_first = {
         synopsis = "[You are near %i.  %nC lurks further ahead.]",
         output = "text",
         text = [[
A high mountain path stretches upwards in front of you.
Somewhere at its end, you will find %i,
and in its depths %n surely lurks.
         ]],
      },
      locate_next = {
         text = "Again, you near %i.",
      },
      lowlevel = {
         synopsis = "[Are you sure you are ready?]",
         output = "text",
         text = [[
"Oddly, %p, though you seem unready, I find myself wondering
whether the experience I have looked for in you is perhaps not the true
preparation you require.  Sending you forth now is a great risk,
however.  Are you sure you are ready?""]],
      },
      nemesis_first = {
         synopsis = "[You are no %g.  You shall never regain %o.]",
         output = "text",
         text = [["Ah, so %l has sent another %g to retrieve
%o.

"No, I see you are no %g.  Perhaps I shall have some fun today
after all.  Prepare to die, %r!  You shall never regain
%o."]],
      },
      nemesis_next = {
         text = "\"So, %r.  Again you challenge me.\"",
      },
      nemesis_other = {
         text = "\"Die now, %r.  %d has no power here to aid you.\"",
      },
      nemesis_wantsit = {
         text = "\"You shall die, %r, and I will have %o back.\"",
      },
      nexttime = {
         text = "Once again, you stand before %H.",
      },
      offeredit = {
         synopsis = "[Keep %o.  They will help you recover the Amulet of Yendor.]",
         output = "text",
         text = [[
"Amazing work, %p! You have recovered %o!

"I have meditated on the use to which %o could best be put.
The choice is clear: you must keep %o, and use them on your
quest for the Amulet of Yendor.

"I could not be more proud of you, %p.  Go with my blessing."
         ]],
      },
      offeredit2 = {
         synopsis = "[Keep %o and return to %Z to search for the Amulet.]",
         output = "text",
         text = [[%lC studies %o for a moment,
then returns his gaze to you.

"%oC must remain with you.  Use %oi
as you resume your search for the Amulet.
As you know, you will find it somewhere in %Z, back through
the magic portal.]],
      },
      othertime = {
         text = [[Again you face %H.  Your intuition hints that this
may be the final time you come here.]],
      },
      posthanks = {
         text = "\"Welcome back, %p.  Is there any small assistance we can provide you?\"",
      },
   },
   Pri = {
      assignquest = {
         synopsis = "[%nC invaded %H and captured %o.  Defeat %ni and retrieve %oh.]",
         output = "text",
         text = [["Yes, %p.  You are truly ready now.  Attend to me and I shall
tell you of what has transpired:

"At one of the Great Festivals a short time ago, %n and a legion
of undead invaded %H.  Many %gP were killed, including
the one carrying %o.

"As a final act of vengefulness, %n desecrated the altar here.
Without it, we could not mount a counter-attack.  Now, there are
barely enough %gP left to keep the undead at bay.

"We need you to find %i, then, from there, travel
to %ns lair.  If you can manage to defeat %n and return
%o here, we can then drive off the legions of
undead that befoul the land.

"Go with %d as your guide, %p."]],
      },
      badalign = {
         synopsis = "[You have deviated from the path.  Return when you have purified yourself.]",
         output = "text",
         text = [["This is terrible, %p.  You have deviated from the true path!
You know that %d requires the most strident devotion of this
order.  The %shood must stand for utmost piety.

"Go from here, atone for your sins against %d.  Return only when
you have purified yourself."]],
      },
      badlevel = {
         synopsis = "[%rA cannot withstand %n.  Come back when you are %Ra.]",
         output = "text",
         text = [["Alas, %p, it is not yet to be.  A mere %r could never
withstand the might of %n.  Go forth, again into the world, and return
when you have attained the post of %R."]],
      },
      discourage = {
         "\"Submit to my will, %c, and I shall spare you.\"",
         "\"Your puny powers are no match for me, %c.\"",
         "\"I shall have you turned into a zombie for my pleasure!\"",
         "\"Despair now, %r.  %d cannot help you.\"",
         "\"I shall feast upon your soul for many days, %c.\"",
         "\"Your death will be slow and painful.  That I promise!\"",
         "\"You cannot defeat %n, you fool.  I shall kill you now.\"",
         "\"Your precious %lt will be my next victim.\"",
         "\"I feel your powers failing you, %r.  You shall die now.\"",
         "\"With %o, nothing can stand in my way.\"",
      },
      encourage = {
         "\"You can prevail, if you rely on %d.\"",
         "\"Remember that %n has great magic at his command.\"",
         "\"Be pure, my %S.\"",
         "\"Beware, %i is surrounded by a great graveyard.\"",
         "\"You may be able to affect %n with magical cold.\"",
         "\"Acquire %o if you can.  It will aid you against %n.\"",
         "\"Call upon %d when your need is greatest.  You will be answered.\"",
         "\"The undead legions are weakest during the daylight hours.\"",
         "\"Do not lose faith, %p.  If you do so, %n will grow stronger.\"",
         "\"Use %o.  It will assist you against the undead.\"",
      },
      firsttime = {
         synopsis = "[You are at %H; the doors are closed.  %lC needs your help!]",
         output = "text",
         text = [[You find yourself standing in sight of %H.  Something
is obviously wrong here.  The doors to %H, which usually
stand open, are closed.  Strange human shapes shamble around
outside.

You realize that %l needs your assistance!]],
      },
      goal_first = {
         synopsis = "[The stench of brimstone surrounds you, the shrieks and moans are endless.]",
         output = "text",
         text = [[The stench of brimstone is all about you, and the shrieks and moans
of tortured souls assault your psyche.

Ahead, there is a small clearing amidst the bubbling pits of lava...]],
      },
      goal_next = {
         text = "Again, you have invaded %ns domain.",
      },
      gotit = {
         synopsis = "[You feel %d as you pick up %o; return %oh to %l.]",
         output = "text",
         text = [[As you pick up %o, you feel the essence of
%d fill your soul.  You know now why %n stole it from
%H, for with it, %ca of %d could
easily defeat his plans.

You sense a message from %d.  Though not verbal, you
get the impression that you must return to %l as soon
as possible.]],
      },
      guardtalk_after = {
         "\"Greetings, %r.  It is good to see you again.\"",
         "\"Ah, %p!  Our deepest gratitude for all of your help.\"",
         "\"Welcome back, %s!  With %o, no undead can stand against us.\"",
         "\"Praise be to %d, for delivering us from %n.\"",
         "\"May %d be with you, %s.\"",
      },
      guardtalk_before = {
         "\"Greetings, honored %r.  It is good to see you.\"",
         "\"Ah, %p!  Surely you can help us in our hour of need.\"",
         "\"Greetings, %s.  %lC has great need of your help.\"",
         "\"Alas, it seems as if even %d has deserted us.\"",
         "\"May %d be with you, %s.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and offer it on %ds altar.]",
         output = "text",
         text = [["You have prevailed, %p!  %d is surely with you.  Now,
you must take the amulet, and sacrifice it on %ds altar on
the Astral Plane.  I suspect that I shall never see you again in this
life, but I hope to at %ds feet."]],
      },
      killed_nemesis = {
         synopsis = "[%nC dies.  Moloch is aware of you and angry at %n.]",
         output = "text",
         text = [[You feel a wrenching shift in the ether as %ns body dissolves
into a cloud of noxious gas.

Suddenly, a voice booms out:

    "Thou hast defeated the least of my minions, %r.
    Know now that Moloch is aware of thy presence.
    As for thee, %n, I shall deal with thy failure
    at my leisure."

You then hear the voice of %n, screaming in terror...]],
      },
      leader_first = {
         synopsis = "[You have returned and we need your help.  Are you ready?]",
         output = "text",
         text = [["Ah, %p, my %S.  You have returned to us at last.
A great blow has befallen our order; perhaps you can help us.
First, however, I must determine if you are prepared for this
great challenge."]],
      },
      leader_last = {
         synopsis = "[You are a heretic who has deviated from the teachings of %d.]",
         output = "text",
         text = [["You are a heretic, %p!  How can you, %ra, deviate so from the
teachings of %d?  Begone from this temple.  You are no longer
%sa to this order.  We will pray to %d for other assistance,
as you have failed us utterly."]],
      },
      leader_next = {
         text = "\"Again, my %S, you stand before me.  Are you ready now to help us?\"",
      },
      leader_other = {
         text = "\"Once more, %p, you stand within the sanctum.  Are you ready now?\"",
      },
      locate_first = {
         synopsis = "[You have found %i.  The trail to %n lies ahead.]",
         output = "text",
         text = [[You stand facing a large graveyard.  The sky above is filled with clouds
that seem to get thicker closer to the center.  You sense the presence of
undead in larger numbers than you have ever encountered before.

You remember the descriptions of %i, given to you by
%l.  It is ahead that you will find %ns trail.]],
      },
      locate_next = {
         text = "Again, you stand before %i.",
      },
      lowlevel = {
         synopsis = "[You propose a great risk.  Are you sure?]",
         output = "text",
         text = [[
"We cannot hold back the undead here forever, %p, and if you
should fall, we have not the ability to raise you or to find another to
send in your place.  It is a great risk you propose to take.  Are you
sure you are ready for this?"]],
      },
      nemesis_first = {
         synopsis = "[%lC has sent you, but you are no %gC.  I shall destroy you.]",
         output = "text",
         text = [["Ah, so %l has sent another %gC to retrieve
%o.

"No, I see you are no %gC.  Perhaps I shall have some fun today
after all.  Prepare to die, %r!  You shall never regain
%o."]],
      },
      nemesis_next = {
         text = "\"So, %r.  Again you challenge me.\"",
      },
      nemesis_other = {
         text = "\"Die now, %r.  %d has no power here to aid you.\"",
      },
      nemesis_wantsit = {
         text = "\"You shall die, %r, and I will have %o back.\"",
      },
      nexttime = {
         text = "Once again, you stand before %H.",
      },
      offeredit = {
         synopsis = "[Congratulations, %p.  Keep %o; go and recover the Amulet.]",
         output = "text",
         text = [["You have returned, %p.  And with %o, I see.
Congratulations.

"I have been praying, and have received direction from
a minion of %d.  %d commands that you retain
%o.  With it, you must recover the Amulet
of Yendor.

"Go forth, and let %d guide your steps."]],
      },
      offeredit2 = {
         synopsis = "[%oC is yours now.  Return to %Z and find the Amulet.]",
         output = "text",
         text = [[%lC reiterates that %o is yours now.

"The time has come to resume your search for the Amulet.
%Z await your return through the magic portal
that brought you here."]],
      },
      othertime = {
         text = [[Again you face %H.  You get the feeling that %d's patience is beginning
to run out.]],
      },
      posthanks = {
         text = "\"Welcome back, %p.  How is your quest for the Amulet going?\"",
      },
   },
   Ran = {
      assignquest = {
         synopsis = "[%nC has stolen %o.  Infiltrate %i and retrieve %oh for us.]",
         output = "text",
         text = [["You are indeed ready, %p.  I shall tell you what has transpired,
and why we so desperately need your help:

"A short time ago, the mountain centaurs to the east invaded
and enslaved the plains centaurs in this area.  The local
leader is now only a figurehead, and serves %n.

"During our last gathering of worship here, we were beset by hordes of
hostile centaurs, as you witnessed.  In the first onslaught a group,
headed by %n %niself, managed to breach the grove and steal
%o.

"Since then, we have been besieged.  We do not know how much longer
we will be able to maintain our magical barriers.

"If we are to survive, you, %p, must infiltrate
%i.  There, you will find a pathway down, to the
underground cavern of %n.  He has always coveted
%o, and will surely keep it.

"Recover %o for us, %p!  Only then will %d be safe."]],
      },
      badalign = {
         synopsis = "[You are not sufficiently %a.  Come back when you have purified yourself.]",
         output = "text",
         text = [["You have strayed, %p!  You know that %d requires that
we maintain a pure devotion to things %a!

"You must go from us.  Return when you have purified yourself."]],
      },
      badlevel = {
         synopsis = "[You are too inexperienced.  Come back when you are %Ra.]",
         output = "text",
         text = [["%p, you are yet too inexperienced to withstand the demands of that
which we need you to do.  %RA might just be able to do this thing.

"Return to us when you have learned more, my %S."]],
      },
      discourage = {
         "\"Your %d is nothing, %c.  You are mine now!\"",
         "\"Run away little %c!  You can never hope to defeat %n!\"",
         "\"My servants will rip you to shreds!\"",
         "\"I shall display your head as a trophy.  What do you think about that wall?\"",
         "\"I shall break your %ls grove, and destroy all the %gP!\"",
         "\"%d has abandoned you, %c.  You are doomed.\"",
         "\"%rA?  %lC sends a mere %r against me?  Hah!\"",
         "\"%lC has failed, %c.  %oC will never leave here.\"",
         "\"You really think you can defeat me, eh %c?  You are wrong!\"",
         "\"You weaken, %c.  I shall kill you now.\"",
      },
      encourage = {
         "\"It is rumored that the Forest and Mountain Centaurs have resolved their ancient feud and now band together against us.\"",
         "\"%nC is strong, and very smart.\"",
         "\"Use %o, when you find it.  It will help you survive to reach us.\"",
         "\"Remember, let %d be your guide.\"",
         "\"Call upon %d when you face %n. The very act of doing so will infuriate him, and give you advantage.\"",
         "\"%n and his kind have always hated us.\"",
         "\"We cannot hold the grove much longer, %p.  Hurry!\"",
         "\"To infiltrate %i, you must be very stealthy.\"",
         "\"Remember that %n is a braggart.  Trust not what he says.\"",
         "\"You can triumph, %p, if you trust in %d.\"",
      },
      firsttime = {
         synopsis = "[The ancient forest grove is surrounded by centaurs.]",
         output = "text",
         text = [[You arrive in familiar surroundings.  In the distance, you %x the
ancient forest grove, the place of worship to %d.

Something is wrong, though.  Surrounding the grove are centaurs!
And they've noticed you!]],
      },
      goal_first = {
         synopsis = "[You descend into a subterranean complex.  Hooves clatter in the distance.]",
         output = "text",
         text = [[You descend into a weird place, in which roughly cut cave-like walls
join with smooth, finished ones, as if someone was in the midst of
finishing off the construction of a subterranean complex.

Off in the distance, you hear a sound like the clattering of many
hooves on rock.]],
      },
      goal_next = {
         text = "Once again, you enter the distorted castle of %n.",
      },
      gotit = {
         synopsis = "[You pick up %o and feel power.  It's time to return %oh to %l.]",
         output = "text",
         text = [[As you pick up %o, it seems to glow, and a warmth
fills you completely.  You realize that its power is what has protected
your %sp against their enemies for so long.

You must now return it to %l without delay -- their lives depend
on your speed.]],
      },
      guardtalk_after = {
         "\"%pC!  I have not seen you in many moons.  How do you fare?\"",
         "\"Birdsong has returned to the grove, surely this means you have defeated %n.\"",
         "\"%lC seems to have regained some of his strength.\"",
         "\"So, tell us how you entered %i, in case some new evil arises there.\"",
         "\"Is that truly %o that I see you carrying?\"",
      },
      guardtalk_before = {
         "\"%pC!  I have not seen you in many moons.  How do you fare?\"",
         "\"%nC continues to threaten the grove.  But we hold fast.\"",
         "\"%lC is growing weak.  The magic required to defend the grove drains us.\"",
         "\"Remember %i is hard to enter.  Beware the distraction of leatherwings.\"",
         "\"We must regain %o.  Without it we will be overrun.\"",
      },
      hasamulet = {
         synopsis = "[You have the Amulet!  Take it to the Astral Plane and offer it to %d.]",
         output = "text",
         text = [["You have it!  You have recovered the Amulet of Yendor!
Now attend to me, %p, and I will tell you what must be done:

"The Amulet has within it magic, the capability to transport you to
the Astral Plane, where the primary circle of %d resides.

"To activate this magic, you must travel upwards as far as you can.
When you reach the temple, sacrifice the Amulet to %d.

"Thus will you fulfill your destiny."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you as %nh dies.]",
         output = "text",
         text = [[%nC collapses to the ground, cursing you and %l, then says:

    "You have defeated me, %r!  But I curse you one final time, with
    my dying breath!  You shall die before you leave my castle!"]],
      },
      leader_first = {
         synopsis = "[You have returned, %p.  We need your help.  Are you ready?]",
         output = "text",
         text = [["%pC!  You have returned!  Thank %d.

"We have great need of you.  But first, I must see if you have the
required abilities to take on this responsibility."]],
      },
      leader_last = {
         synopsis = "[You are not sufficiently %a.  We renounce your %shood.]",
         output = "text",
         text = [["%pC!  You have doomed us all.  You fairly radiate %L influences
and weaken the power we have raised in this grove as a result!

"Begone!  We renounce your %shood with us!  You are an outcast now!"]],
      },
      leader_next = {
         text = "\"Once again, %p, you stand in our midst.  Are you ready now?\"",
      },
      leader_other = {
         text = "\"Ah, you are here again, %p.  Allow me to determine your readiness...\"",
      },
      locate_first = {
         synopsis = "[This is %i.  There are bats nearby.  Beware the wumpus!]",
         output = "text",
         text = [[This must be %i.

You are in a cave built of many different rooms, all interconnected
by tunnels.  Your quest is to find and shoot the evil wumpus that
resides elsewhere in the cave without running into any bottomless
pits or using up your limited supply of arrows.  Good luck.

You are in room 9 of the cave.  There are tunnels to rooms
5, 8, and 10.
*rustle* *rustle* (must be bats nearby.)
*sniff* (I can smell the evil wumpus nearby!)]],
      },
      locate_next = {
         synopsis = "[You are in %i.  There are pits.  There are bats nearby.]",
         output = "text",
         text = [[Once again, you descend into %i.

*whoosh* (I feel a draft from some pits.)
*rustle* *rustle* (must be bats nearby.)]],
      },
      lowlevel = {
         synopsis = "[You are still a %r!  Can you really do this?]",
         output = "text",
         text = [[
"You are so young!  Still a %r!  Can you fight the centaurs?  Can you
really find and infiltrate %i, take on %n, and recover
%o for us?"]],
      },
      nemesis_first = {
         synopsis = "[You have come to recover %o, but I shall keep %oh and you shall die.]",
         output = "text",
         text = [["So, %c.  %lC has sent you to recover %o.

"Well, I shall keep that bauble.  It pleases me.  You, %c, shall die."]],
      },
      nemesis_next = {
         text = "\"Back again, eh?  Well, a mere %r is no threat to me!  Die, %c!\"",
      },
      nemesis_other = {
         text = "\"You haven't learned your lesson, %c.  You can't kill me!  You shall die now.\"",
      },
      nemesis_wantsit = {
         text = [["I shall have %o from you, %r.  Then I shall
kill you."]],
      },
      nexttime = {
         text = "Once again, you stand before %H.",
      },
      offeredit = {
         synopsis = "[You have succeeded.  Take %o with you as you go to find the Amulet.]",
         output = "text",
         text = [["%pC!  You have succeeded!  I feared it was not possible!

"You have returned with %o!

"I fear, now, that the Centaurs will regroup and plot yet another raid.
This will take some time, but if you can recover the Amulet of Yendor
for %d before that happens, we will be eternally safe.

"Take %o with you.  It will aid in your quest for
the Amulet."]],
      },
      offeredit2 = {
         synopsis = "[You are the keeper of %o now.  Go and find the Amulet.]",
         output = "text",
         text = [[%l flexes %o reverently.

"With this wondrous bow, one need never run out of arrows.
You are its keeper now, and the time has come to resume your
search for the Amulet.  %Z await your return
through the magic portal that brought you here."]],
      },
      othertime = {
         text = [[You have the oddest feeling that this may be the last time you
are to enter %H.]],
      },
      posthanks = {
         text = [["Welcome, %p.  How have you fared on your quest for the Amulet
of Yendor?"]],
      },
   },
   Rog = {
      assignquest = {
         synopsis = "[Get %o from %n and bring it to %l.]",
         output = "text",
         text = [["Will everyone not going to retrieve %o from that
jerk, %n, take one step backwards.  Good choice,
%p, because I was going to send you anyway.  My other %gp
are too valuable to me.

"Here's the deal.  I want %o, %n
has %o.  You are going to get %o
and bring it back to me.  So simple an assignment even you can understand
it."]],
      },
      badalign = {
         synopsis = "[Come back when you are really %a.]",
         output = "text",
         text = [["Maybe I should chain you to my perch here for a while.  Perhaps watching
real %a men at work will bring some sense back to you.  I don't
think I could stand the sight of you for that long though.  Come back
when you can be trusted to act properly."]],
      },
      badlevel = {
         synopsis = "[%rA is not adequately trained to handle this job.]",
         output = "text",
         text = [["In the time that you've been gone you've only been able to master the
arts of %ra?  I've trained ten times again as many %Rp
in that time.  Maybe I should send one of them, no?  Where would that
leave you, %p?  Oh yeah, I remember, I was going to kill you!"]],
      },
      discourage = {
         "\"May I suggest a compromise.  Are you interested in gold or gems?\"",
         "\"Please don't force me to kill you.\"",
         "\"Grim times are upon us all.  Will you not see reason?\"",
         "\"I knew %l, and you're no %lt, thankfully.\"",
         "\"It is a shame that we are not meeting under more pleasant circumstances.\"",
         "\"I was once like you are now, %p.  Believe in me -- our way is better.\"",
         "\"Stay with me, and I will make you %os guardian.\"",
         "\"When you return, with or without %o, %l will have you killed.\"",
         "\"Do not be fooled; I am prepared to kill to defend %o.\"",
         "\"I can reunite you with the Twain.  Oh, the stories you can swap.\"",
      },
      encourage = {
         "\"You don't seem to understand, %o isn't here so neither should you be!\"",
         "\"May %d curse you with lead fingers.  Get going!\"",
         "\"We don't have all year.  GET GOING!\"",
         "\"How would you like a scar necklace?  I'm just the jeweler to do it!\"",
         "\"Lazy S.O.B.  Maybe I should call up someone else...\"",
         "\"Maybe I should open your skull and see if my instructions are inside?\"",
         "\"This is not a task you can complete in the afterlife, you know.\"",
         "\"Inside every living person is a dead person trying to get out, and I have your key!\"",
         "\"We're almost out of hell-hound chow, so why don't you just get moving!\"",
         "\"You know, %o isn't going to come when you whistle.  You must get it yourself.\"",
      },
      firsttime = {
         synopsis = "[You are in Ransmannsby, where you trained.  Find %l.]",
         output = "text",
         text = [[Unexpectedly, you find yourself back in Ransmannsby, where you trained to
be a thief.  Quickly you make the guild sign, hoping that you AND word
of your arrival reach %ls den.]],
      },
      goal_first = {
         synopsis = "[You sense %o.]",
         output = "text",
         text = [[You feel a great swelling up of courage, sensing the presence of
%o.  Or is it fear?]],
      },
      goal_next = {
         text = "The hairs on the back of your neck whisper -- it's fear.",
      },
      gotit = {
         synopsis = "[You pick up %o and know that %l should not have it.]",
         output = "text",
         text = [[As you pick up %o, the hairs on the back of your
neck fall out.  At once you realize why %n was
willing to die to keep it out of %ls hands.  Somehow
you know that you must do likewise.]],
      },
      guardtalk_after = {
         "\"I was sure wrong about Lady Tyvefelle's house; I barely got away with my life and lost my lock pick in the process.\"",
         "\"You're back?  Even the Twain don't come back anymore.\"",
         "\"Can you spare an old cutpurse a zorkmid for some grog?\"",
         "\"Fritz tried to join the other side, and now he's hell-hound chow.\"",
         "\"Be careful what you steal, I hear the boss has perfected turning rocks into worthless pieces of glass.\"",
      },
      guardtalk_before = {
         "\"I hear that Lady Tyvefelle's household is lightly guarded.\"",
         "\"You're back?  Even the Twain don't come back anymore.\"",
         "\"Can you spare an old cutpurse a zorkmid for some grog?\"",
         "\"Fritz tried to join the other side, and now he's hell-hound chow.\"",
         "\"Be careful what you steal, I hear the boss has perfected turning rocks into worthless pieces of glass.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane and find %ds temple.]",
         output = "text",
         text = [["I see that with your abilities, and my brains, we could rule this world.

"All that we would need to be all-powerful is for you to take that little
trinket you've got there up to the Astral Plane.  From there, %d will
show you what to do with it.  Once that's done, we will be invincible!"]],
      },
      killed_nemesis = {
         synopsis = "[Before dying, %n tells you to use the %o wisely.]",
         output = "text",
         text = [["I know what you are thinking, %p.  It is not too late for you
to use %o wisely.  For the sake of your guild
%sp, do what is right."

You sit and wait for death to come for %n, and then you
brace yourself for your next meeting with %l!]],
      },
      leader_first = {
         synopsis = "[You owe back dues to your guild.  You can pay them off if you're up to the job.]",
         output = "text",
         text = [["Well, look who it is boys -- %p has come home.  You seem to have
fallen behind in your dues.  I should kill you as an example to these
other worthless cutpurses, but I have a better plan.  If you are ready
maybe you could work off your back dues by performing a little job for
me.  Let us just see if you are ready..."]],
      },
      leader_last = {
         synopsis = "[You must go.]",
         output = "text",
         text = [["Well %gp, it looks like our friend has forgotten who is the boss
around here.  Our friend seems to think that %rp have been put in
charge.  Wrong.  DEAD WRONG!"

Your sudden shift in surroundings prevents you from hearing the end
of %ls curse.]],
      },
      leader_next = {
         synopsis = "[Are you stupid or are you ready?]",
         output = "text",
         text = [["Well, I didn't expect to see you back.  It shows that you are either stupid,
or you are finally ready to accept my offer.  Let us hope for your sake it
isn't stupidity that brings you back."]],
      },
      leader_other = {
         text = [["Did you perhaps mistake me for some other %lt?  You must
think me as stupid as your behavior.  I warn you not to try my patience."]],
      },
      locate_first = {
         text = "Those damn little hairs tell you that you are nearer to %o.",
      },
      locate_next = {
         text = "Not wanting to face %l without having stolen %o, you continue.",
      },
      lowlevel = {
         synopsis = "[%l laughs at you and waves you onward.]",
         output = "text",
         text = [[
%l guffaws loudly.

"This is the funniest joke I ever heard.  You actually think you're
ready to bring down %n?  You?

Know what?  Fine, moron.  Have fun dying.  We're all going to laugh at
it."]],
      },
      nemesis_first = {
         text = "\"Ah!  You must be %ls ... er, `hero'.  A pleasure to meet you.\"",
      },
      nemesis_next = {
         text = "\"We meet again.  Please reconsider your actions.\"",
      },
      nemesis_other = {
         synopsis = "[You cannot trust %l.]",
         output = "text",
         text = [["Surely, %p, you have learned that you cannot trust any bargains
that %l has made.  I can show you how to continue on
your quest without having to run into him again."]],
      },
      nemesis_wantsit = {
         synopsis = "[%lC should not have %o.]",
         output = "text",
         text = [["Please, think for a moment about what you are doing.  Do you truly
believe that %d would want %l to have
%o?"]],
      },
      nexttime = {
         text = [[Once again, you find yourself back in Ransmannsby.  Fond memories are
replaced by fear, knowing that %l is waiting for you.]],
      },
      offeredit = {
         synopsis = "[Take %o with you and go.]",
         output = "text",
         text = [["Well, I'll be damned.  You got it.  I am proud of you, a fine %r
you've turned out to be.

"While you were gone I got to thinking, you and %o
together could bring me more treasure than either of you apart, so why don't
you take it with you.  All I ask is a cut of whatever loot you come by.
That is a better deal than I offered %n.

"But, you see what happened to %n when he refused.
Don't make me find another to send after you this time."]],
      },
      offeredit2 = {
         synopsis = "[Take %o and acquire the Amulet.]",
         output = "text",
         text = [[%lC seems tempted to swap %o for
the mundane one you detect in his pocket, but noticing your alertness,
evidently chickens out.

"Go filch the Amulet before someone else beats you to it.
%Z are back the way you came, through the magic portal."]],
      },
      othertime = {
         text = [[You rub your hands through your hair, hoping that the little ones on
the back of your neck stay down, and prepare yourself for your meeting
with %l.]],
      },
      posthanks = {
         synopsis = "[How about trading %o for something?]",
         output = "text",
         text = [["Quite the little thief, aren't we, %p.  Can I interest you in a
swap for %o?  Look around, anything in the keep
is yours for the asking."]],
      },
   },
   Sam = {
      assignquest = {
         synopsis = "[You must enter %i, then regain %o from %n.]",
         output = "text",
         text = [["Domo %p-san, indeed you are ready.  I can now tell you what
it is that I require of you.

"The daimyo, %n, has betrayed us.  He has stolen from us
%o and taken it to his donjon deep within
%i.

"If I cannot show the emperor %o when he comes
for the festival he will know that I have failed in my duty, and
request that I commit seppuku.

"You must gain entrance to %i and retrieve the
emperor's property.  Be quick!  The emperor will be here for the
cha-no-you in 5 sticks.

"Wakarimasu ka?"]],
      },
      badalign = {
         synopsis = "[When you can think %a and act %a then return.]",
         output = "text",
         text = [["%p-san, you would do better to join the kyokaku.

"You have skills, but until you can call upon the bushido to know when and
how to use them you are not samurai.  When you can think %a and
act %a then return."]],
      },
      badlevel = {
         synopsis = "[\"I require %Ra to defeat %n.  Return when you are ready.\"]",
         output = "text",
         text = [["%p-san, you have learned well and honored your family.
I require the skills of %Ra in order to defeat %n.
Go and seek out teachers.  Learn what they have learned.  When you
are ready, return to me."]],
      },
      discourage = {
         "\"Ahh, I finally meet the daimyo of the kyokaku!\"",
         "\"There is no honor for me in your death.\"",
         "\"You know that I cannot resash my swords until they have killed.\"",
         "\"Your presence only compounds the dishonor of %l in not coming %liself.\"",
         "\"I will make tea with your hair and serve it to %l.\"",
         "\"Your fear shows in your eyes, coward!\"",
         "\"I have not heard of you, %p-san; has your life been that unworthy?\"",
         "\"If you will not obey me, you will die.\"",
         "\"Kneel now and make the two cuts of honor.  I will tell your %sp of your honorable death.\"",
         "\"Your master was a poor teacher.  You will pay for his mistakes in your teaching.\"",
      },
      encourage = {
         "\"To defeat %n you must overcome the seven emotions: hate, adoration, joy, anxiety, anger, grief, and fear.\"",
         "\"Remember your honor is my honor, you perform in my name.\"",
         "\"I will go to the temple and burn incense for your safe return.\"",
         "\"Sayonara.\"",
         "\"There can be honor in defeat, but no gain.\"",
         "\"Your kami must be strong in order to succeed.\"",
         "\"You are indeed a worthy %R, but now you must be a worthy samurai.\"",
         "\"If you fail, %n will be like a tai-fun on the land.\"",
         "\"If you are truly %a, %d will listen.\"",
         "\"Sharpen your swords and your wits for the task before you.\"",
      },
      firsttime = {
         synopsis = "[The banner of %n flies above town.  What has happened to %l?]",
         output = "text",
         text = [[Even before your senses adjust, you recognize the kami of
%H.

You %x the standard of your teki, %n, flying above
the town.  How could such a thing have happened?  Why are ninja
wandering freely; where are the samurai of your daimyo, %l?

You quickly say a prayer to Izanagi and Izanami and walk towards
town.]],
      },
      goal_alt = {
         text = "As you arrive once again at the home of %n.",
      },
      goal_first = {
         synopsis = "[You feel the taunts %n, but after offering a prayer to %d, you proceed.]",
         output = "text",
         text = [[In your mind, you hear the taunts of %n.

You become like the rice plant and bend to the ground, offering a
prayer to %d.  But when the wind has passed, you stand
proudly again.  Putting your kami in the hands of fate, you advance.]],
      },
      goal_next = {
         text = [[As you arrive once again at the home of %n, your thoughts
turn only to %o.]],
      },
      gotit = {
         synopsis = "[You feel the power of %o and are humbled.]",
         output = "text",
         text = [[As you pick up %o, you feel the strength of its karma.
You realize at once why so many good samurai had to die to defend it.
You are humbled knowing that you hold one of the artifacts of the
sun goddess.]],
      },
      guardtalk_after = {
         "\"Come, join us in celebrating with some sake.\"",
         "\"Ikaga desu ka?\"",
         "\"You have brought our clan and %l much honor.\"",
         "\"Please %r, sit for a while and tell us how you overcame the Ninja.\"",
         "\"%lC still lives!  You have saved us from becoming ronin.\"",
      },
      guardtalk_before = {
         "\"To succeed, you must walk like a butterfly on the wind.\"",
         "\"Ikaga desu ka?\"",
         "\"I fear for The Land of The Gods.\"",
         "\"%nC has hired the Ninja -- be careful.\"",
         "\"If %o is not returned, we will all be ronin.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to the Astral Plane to finish your task.]",
         output = "text",
         text = [["Ah, %p-sama.  You have wasted your efforts returning home.
Now that you are in possession of the Amulet, you are honor-bound to
finish the quest you have undertaken.  There will be plenty of time
for saki and stories when you have finished.

"Go now, and may our prayers be a wind at your back."]],
      },
      killed_nemesis = {
         synopsis = "[%nC dies without honor.]",
         output = "text",
         text = [[Your healing skills tell you that %ns wounds are mortal.

You know that the bushido tells you to finish him and let his kami
die with honor, but the thought of so many samurai dead due to this
man's dishonor prevents you from giving the final blow.

You order that his unwashed head be given to the crows and his body
thrown into the sea.]],
      },
      leader_first = {
         synopsis = "[%lC needs someone to lead %lj samurai against %n.  Are you ready?]",
         output = "text",
         text = [["Ah, %p-san, it is good to see you again.  I need someone who can
lead my samurai against %n.  If you are ready, you will be
that person."]],
      },
      leader_last = {
         synopsis = "[Leave and do not come back.]",
         output = "text",
         text = [["You are no longer my samurai, %p.

"Hara-kiri is denied.  You are ordered to shave your head and then to
become a monk.  Your fief and family are forfeit.  Wakarimasu ka?"]],
      },
      leader_next = {
         text = [["Once again, %p-san, you kneel before me.  Are you yet capable of
being my vassal?"]],
      },
      leader_other = {
         synopsis = "[Are you truly a samurai?]",
         output = "text",
         text = [["You begin to test my matsu, %p-san.
If you cannot determine what I want in a samurai, how can I rely on you
to figure out what I need from a samurai?"]],
      },
      locate_first = {
         text = [[You instinctively reach for your swords.  You do not recognize the
lay of this land, but you know that your teki are everywhere.]],
      },
      locate_next = {
         text = [[Thankful that your %sp at %H cannot see
your fear, you prepare again to advance.]],
      },
      lowlevel = {
         synopsis = "[Do you think your inexperience is enough to win?]",
         output = "text",
         text = [[
"Do you think you know the bushido, %p-chan?  Nantekawaii.
Do you think that will be enough to save you when you fight %n?"]],
      },
      nemesis_first = {
         text = [["Ah, so it is to be you, %p-san.  I offer you seppuku.
I will be your second if you wish."]],
      },
      nemesis_next = {
         text = [["I have offered you the honorable exit.  Now I will have your
head to send unwashed to %l."]],
      },
      nemesis_other = {
         text = "\"After I have dispatched you, I will curse your kami.\"",
      },
      nemesis_wantsit = {
         text = [["You have fought my samurai; surely you must know that you
will not be able to take %o back to
%H."]],
      },
      nexttime = {
         text = "Once again, you are back at %H.",
      },
      offeredit = {
         synopsis = "[The emperor wants you to take %o and recover the Amulet.]",
         output = "text",
         text = [[As you bow before %l, he welcomes you:

    "You have brought your family great honor, %p-sama.

    "While you have been gone the emperor's advisors have discovered in
    the ancient texts that the karma of the samurai who seeks to recover
    the Amulet and the karma of %o are joined
    as the seasons join to make a year.

    "Because you have shown such fidelity, the emperor requests
    that you take leave of other obligations and continue on the
    road that fate has set your feet upon.  I would consider it
    an honor if you would allow me to watch your household until
    you return with the Amulet."

With that, %l bows, and places his sword atop
%o.]],
      },
      offeredit2 = {
         synopsis = "[Take %o, return to %Z, and recover the Amulet.]",
         output = "text",
         text = [[%l holds %o tightly for a moment, then returns
his gaze to you.

"The time is ripe to recover the Amulet.  Return to %Z
through the magic portal that transported you here so that you may
achieve the destiny which awaits you."]],
      },
      othertime = {
         synopsis = "[%HC is threatened by %n.]",
         output = "text",
         text = [[You are back at %H.

Instantly you sense a subtle change in your karma.  You seem to know that
if you do not succeed in your quest, %n will have destroyed
the kami of %H before you return again.]],
      },
      posthanks = {
         text = "%lC bows.  \"%p-sama, tell us of your search for the Amulet.\"",
      },
   },
   Tou = {
      assignquest = {
         synopsis = "[Enter %i and recover %o from %n.]",
         output = "text",
         text = [["You have indeed proven yourself a worthy %c, %p.

"But now your kinfolk and I must ask you to put aside your travels and
help us in our time of need.  After you left us we elected a new mayor,
%n.  He proved to be a most heinous and vile creature.

"Soon after taking office he absconded with %o
and fled town, leaving behind his henchmen to rule over us.  In order
for us to regain control of our town, you must enter %i
and recover %o.

"Do not be distracted on your quest.  If you do not return quickly I fear
that all will be lost.  Let us both pray now that %d will guide you
and keep you safe."]],
      },
      badalign = {
         synopsis = "[You are not sufficiently %a.  Return when you are.]",
         output = "text",
         text = [["It would be an affront to %d to have one not true to the
%a path undertake her bidding.

"You must not return to us until you have purified yourself of these
bad influences on your actions.  Remember, only by following the %a
path can you hope to overcome the obstacles you will face."]],
      },
      badlevel = {
         synopsis = "[Return when you are %Ra.]",
         output = "text",
         text = [["There is still too much that you have to learn before you can undertake
the next step.  Return to us as a proven %R, and perhaps then
you will be ready.

"Go back now, and may the teachings of %d serve you well."]],
      },
      discourage = {
         "\"I defeated %l and I will defeat you, %p.\"",
         "\"Where is %d now!  You must realize no one can help you here.\"",
         "\"Beg for mercy now and I may be lenient on you.\"",
         "\"If you were not so %a, you might have stood a chance.\"",
         "\"Vengeance is mine at last, %p.\"",
         "\"I only wish that %l had a more worthy %r to send against me.\"",
         "\"With %o in my possession you cannot hope to defeat me.\"",
         "\"%nC has never been defeated, NEVER!\"",
         "\"Are you truly the best %H has to send against me?  I pity %l.\"",
         "\"How do you spell %p?  I want to ensure the marker on your grave is correct as a warning to your %sp.\"",
      },
      encourage = {
         "\"Do not be fooled by the false promises of %n.\"",
         "\"To enter %i you must pass many traps.\"",
         "\"If you do not return with %o, your quest will be in vain.\"",
         "\"Do not be afraid to call upon %d if you truly need help.\"",
         "\"If you do not destroy %n, he will follow you back here!\"",
         "\"Take %o from %n and you may be able to defeat him.\"",
         "\"You must hurry, %p!\"",
         "\"You are like %Sa to me, %p.  Do not let me down.\"",
         "\"If you are %a at all times you may succeed, %p.\"",
         "\"Let all who meet you on your journey know that you are on a quest for %l and grant safe passage.\"",
      },
      firsttime = {
         synopsis = "[You find yourself back at %H, but the quiet is ominous.]",
         output = "text",
         text = [[You breathe a sigh of relief as you find yourself back in the familiar
surroundings of %H.

You quickly notice that things do not appear the way they did when you
left.  The town is dark and quiet.  There are no sounds coming from
behind the town walls, and no campfires burning in the fields.  As a
matter of fact, you do not %x any movement in the fields at all, and
the crops seem as though they have been untended for many weeks.]],
      },
      goal_alt = {
         text = "You have returned to %ns lair.",
      },
      goal_first = {
         text = "You sense the presence of %o.",
      },
      goal_next = {
         text = [[You gain confidence, knowing that you may soon be united with
%o.]],
      },
      gotit = {
         synopsis = "[You pick up %o and feel relief.  Return it to %l.]",
         output = "text",
         text = [[As you pick up %o, you feel a great
weight has been lifted from your shoulders.  Your only thoughts are
to quickly return to %H and find %l.]],
      },
      guardtalk_after = {
         "\"Gehennom on 5 zorkmids a day -- more like 500 a day if you ask me.\"",
         "\"Do you know where I could find some nice postcards of The Gnomish Mines?\"",
         "\"Have you tried the weird toilets?\"",
         "\"If you stick around, I'll show you the pictures from my latest trip.\"",
         "\"Did you bring me back any souvenirs?\"",
      },
      guardtalk_before = {
         "\"Gehennom on 5 zorkmids a day -- more like 500 a day if you ask me.\"",
         "\"Do you know where I could find some nice postcards of The Gnomish Mines?\"",
         "\"Have you tried the weird toilets?\"",
         "\"Don't stay at the Inn, I hear the food is terrible and it has rats.\"",
         "\"They told me that this was the off season!\"",
      },
      hasamulet = {
         synopsis = "[You have the Amulet.  Take it to the Astral Plane to finish your task.]",
         output = "text",
         text = [["Stand back and let me look at you, %p.
Now that you have recovered the Amulet of Yendor, I'm afraid living
out your days in %H would seem pretty tame.

"You have come too far to stop now, for there are still more tasks that
our oral history foretells for you.  Forever more, though, your name shall
be spoken by the %gP with awe.  You are truly an inspiration to your
%sp!"]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses at you as %nh dies.]",
         output = "text",
         text = [[You turn in the direction of %n.  As his earthly body begins
to vanish before your eyes, you hear him curse:

    "You shall never be rid of me, %p!
    I will find you where ever you go and regain what is rightly mine."]],
      },
      leader_first = {
         synopsis = "[Someone must defeat %n.  Are your ready?]",
         output = "text",
         text = [["Is it really you, %p!  I had given up hope for your return.
As you can %x, we are desperately in need of your talents.  Someone must
defeat %n if our town is to become what it once was.

"Let me see if you are ready to be that someone."]],
      },
      leader_last = {
         synopsis = "[Leave %H and never return.]",
         output = "text",
         text = [["It is too late, %p.  You are not even worthy to die amongst us.
Leave %H and never return."]],
      },
      leader_next = {
         text = "\"Things are getting worse, %p.  I hope that this time you are ready.\"",
      },
      leader_other = {
         text = "\"I hope that for the sake of %H you have prepared yourself this time.\"",
      },
      locate_first = {
         synopsis = "[You %x the handiwork of %ns henchlings.]",
         output = "text",
         text = [[Only your faith in %d keeps you from trembling.  You %x
the handiwork of %ns henchlings everywhere.]],
      },
      locate_next = {
         text = "You know that this time you must find and destroy %n.",
      },
      lowlevel = {
         synopsis = "[You want to leave the beaten path?]",
         output = "text",
         text = [[
"So, you want to go off the beaten path, answering my summons even
though you are still a mere %r.  I can't fault you for it;
the best excursions happen to the audacious."]],
      },
      nemesis_first = {
         synopsis = "[%rA will not defeat me.]",
         output = "text",
         text = [["So, %p, %l thinks that you can wrest
%o from me!

"It only proves how desperate he has become that he sends %ra to
try and defeat me.  When this day is over, I will have you enslaved
in the mines where you will rue the day that you ever entered
%i."]],
      },
      nemesis_next = {
         text = [["I let you live the last time because it gave me pleasure.
This time I will destroy you, %p."]],
      },
      nemesis_other = {
         synopsis = "[Run away or you will suffer severely.]",
         output = "text",
         text = [["These meetings come to bore me.  You disturb my workings with
%o.

"If you do not run away now, I will inflict so much suffering on you that
%l will feel guilty for ever having sent his %S to me!"]],
      },
      nemesis_wantsit = {
         synopsis = "[\"Return %o to me and we will rule %H.\"]",
         output = "text",
         text = [["You fool.  You do not know how to call upon the powers of
%o.

"Return it to me and I will teach you how to use it, and together we
will rule %H.  But do so now, as my patience grows thin."]],
      },
      nexttime = {
         text = "Once again, you are back at %H.",
      },
      offeredit = {
         synopsis = "[Take %o and with %ds guidance, recover the Amulet.]",
         output = "text",
         text = [[As %l detects the presence of %o,
he almost smiles for the first time in many a full moon.

As he looks up from %o he says:

    "You have recovered %o.  You are its
    owner now, but not its master.  Let it work with you as you continue
    your journey.  With its help, and %d to guide you on the
    %a path, you may yet recover the Amulet of Yendor."]],
      },
      offeredit2 = {
         synopsis = "[Keep %o and return to %Z through the portal.]",
         output = "text",
         text = [["%oC is yours now.  %Z
await your return through the magic portal that brought you here."]],
      },
      othertime = {
         text = [[You are back at %H.
Things appear to have become so bad that you fear that soon
%H will not be here to return to.]],
      },
      posthanks = {
         text = [["I could not be more proud than if you were my own %S, %p!
Tell me of your adventures in quest of the Amulet of Yendor."]],
      },
   },
   Val = {
      arti_but_not_neme = {
         text = [[
%lC looks at you, confused.

"I see that you have deprived %n of %o. But %nh still
lives, does %nh not? 

"%nH does not need %o to bring about the end of the world!
Only by slaying %ni before %nh looses the wolf might we all be
spared. You must hurry back!"]],
         output = "text",
         synopsis = "[Obtaining %o is not enough, return and slay %n!]",
      },
      assignquest = {
         synopsis = "[Follow %n to Muspelheim and slay %ni.]",
         output = "text",
         text = [[
"It is not clear, %p, that anything can be done at this late hour.
But it may now be possible that you can defeat %n.

"It has not been known to many, but I now tell you clearly that the
signs of Ragnarok, and perhaps the doom of us all, are at hand.  The
winter without a summer has come and gone.  The roosters are crowing.
The great serpent has begun to writhe, and the hound of Hel is howling.

"Also aware of these signs, %n began to make %nj conquest.
%nH came from the south wielding the terrible bright sword %o, 
all the forces of Muspelheim arrayed with %ni, bringing fire and ruin
to all our lands.  Their advance opened the huge volcanic vents you %x
about the hill.  If unchecked, these flames they have unleashed will
soon consume the entire world.

"I and my hundred brave warriors defended this shrine in a great and
glorious battle, but capturing it was never %ns objective. 
Having satisfied %niself that he has destroyed any power in our land,
Midgard, that can stop %ni, %nh has since returned to Muspelheim,
there to release the terrible wolf Fenrir from his bond.

"It will not be long before the gods blow the mighty horn to call
themselves to battle, even though it will prove their own death-knell;
at that point, the fate of the world will be sealed. 

"The only thing that might forestall or prevent Ragnarok is killing
%n %niself.  And there is but a slim hope of that by now,
or so the prophecies say.  But you must try, for the sake of the world."]],
      },
      badalign = {
         synopsis = "[You have strayed from the %a path.  Return after you purify yourself.]",
         output = "text",
         text = [["NO!  This is terrible.  I see you becoming an ally of %n, and
leading his armies in the final great battles.  This must not come to
pass!  You have strayed from the %a path.  You must purge yourself,
and return here only when you have regained a state of purity."]],
      },
      badlevel = {
         synopsis = "[Come back when you are %Ra.]",
         output = "text",
         text = [["I see you and %n fighting, %p.  But you are not prepared and
shall die at %ns hand if you proceed.  No.  This will not do.
Go back out into the world, and grow more experienced at the ways of war.
Only when you have returned %Ra do you stand a chance at defeating
%n."]],
      },
      discourage = {
         "\"I am your death, %c.\"",
         "\"You cannot prevail, %r.  I have foreseen your every move.\"",
         "\"It is too late now.  Ragnarok has begun.\"",
         "\"I killed scores of %ds best at your shrine; a mere %c stands no chance!\"",
         "\"Who bears the souls of %cP to Valhalla, %r?\"",
         "\"Soon, %d will die.  All the Aesir will die!\"",
         "\"Some instrument of %d you are, %p.  You are a weakling!\"",
         "\"Never have I seen %ca so clumsy in battle.\"",
         "\"You die now, little %s.\"",
         "\"Your body I destroy now, your soul when my hordes overrun Valhalla!\"",
      },
      encourage = {
         "\"Go with the blessings of %d.\"",
         "\"Call upon %d when you are in need.\"",
         "\"Magical cold is very effective against %n.\"",
         "\"To venture into Muspelheim, you will need to be immune to fire.\"",
         "\"May %d strengthen your arm.\"",
         "\"Trust in %d.  He will not desert you.\"",
         "\"Ragnarok draws nearer with each passing moment.  You must hurry, %p.\"",
         "\"If %n looses Fenrir, the world is doomed.  This must not be!\"",
         "\"Remember your training, %p.  You can succeed.\"",
         "\"Beware %ns terrible bright sword.\"",
      },
      firsttime = {
         synopsis = "[You arrive below %H.  Something is wrong; there is lava present.]",
         output = "text",
         text = [[You materialize at the base of a snowy hill.  Atop the hill sits
a place you know well, %H.  You immediately realize
that something here is very wrong!

In places, the snow and ice have been melted into steaming pools of
water.  Fumaroles and pools of bubbling lava shooting gouts of flame
surround the hill.  The stench of sulphur is carried through the air,
and you %x creatures that should not be able to live in this
environment moving towards you.]],
      },
      goal_first = {
         synopsis = "[This is the lair of %n.]",
         output = "text",
         text = [[Through clouds of sulphurous gases, you %x a wall of great stones
surrounded with a moat of bubbling lava.  This can only be the fortress
of %n.  From somewhere ahead, you hear a great deep voice
shouting orders, and you silently hope you have not arrived too late.]],
      },
      goal_next = {
         text = "Once again, you stand in sight of %ns fortress.",
      },
      gotit = {
         -- no particular message for this, since getting the artifact isn't the
         -- goal of the quest (empty string works correctly)
         text = "", 
      },
      guardtalk_after = {
         "\"Hail, and well met, brave %c!\"",
         "\"May %d ever guide your steps, %p.\"",
         "\"%lC must be so relieved that Ragnarok will not come to pass.\"",
         "\"You defeated %n just in time, %p.\"",
         "\"Hail %d, for heralding %p as %dj champion!\"",
      },
      guardtalk_before = {
         "\"Hail, and well met, brave %c.\"",
         "\"May %d guide your steps, %p.\"",
         "\"%lH doesn't show it much, but %l is terrified.\"",
         "\"You must hurry, %p, else Ragnarok may well come.\"",
         "\"So many fallen friends...\"",
         "\"I'm glad you are still hale; we survivors are too wounded to stop %n.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to %ds temple on the Astral Plane and offer it.]",
         output = "text",
         text = [["Excellent, %p.  I see you have recovered the Amulet!

"You must take the Amulet to the Great Temple of %d, on the Astral
Plane.  There you must offer the Amulet to %d.

"Go now, my %S.  I cannot tell you your fate, as the power of the
Amulet interferes with mine.  I hope for your success."]],
      },
      killed_nemesis = {
         synopsis = "[%nC dies.]",
         output = "text",
         text = [[
%nS eyes widen in shock as he collapses to the ground,
but the shock quickly turns to utter rage.

"%rC...  You think you can prevent Ragnarok?

"I am foretold to battle your gods.  Fenrir and Jormugandr WILL be
freed.  The Aesir will be slain.  The world will be consumed.  You
cannot change this.  You cannot stop what I have begun!"

With his last breath, %n spits at you:

"I know not when I will return.  I only hope it to be while you still
live, so you can give me the pleasure of killing you."]]
      },
      leader_first = {
         synopsis = "[We need your aid.  Are you ready?]",
         output = "text",
         text = [["Ah, %p, my %S.  You have returned to %H
at last.  We are in dire need of your aid, but I must determine if you
will survive such an undertaking.

"Let me read your fate..."]],
      },
      leader_last = {
         synopsis = "[\"Begone from my presence and never return.\"]",
         output = "text",
         text = [["No, %p.  Your fate is sealed.  I must cast about for another
champion.  Begone from my presence, and never return.  Know this, that
you shall never succeed in this life, and Valhalla is denied to you."]],
      },
      leader_next = {
         text = [["Let me read the future for you now, %p, perhaps you are prepared now..."]]
      },
      leader_other = {
         text = [["Again, I shall read your fate, my %S.  Let us both hope that you have
made changes to become ready for this task..."]],
      },
      locate_first = {
         synopsis = "[This is %i.]",
         output = "text",
         text = [[
Your pursuit of %n brings you to an area you have visited only
once or twice before in your life, but there's no way you have forgotten
it -- here is the majestic rainbow bridge Bifrost, connecting this realm
to Muspelheim far below.

But what is this?  As you draw closer, you %x that it is shattered
and broken.  Cursing, you remember that this too was prophesied: that it
would be sundered by the weight of %n and %nj forces
marching across it.

Before you can think about crossing, you realize you have been spotted.
A grim giant with hatred in his eyes points at you with a gleaming sword
and says something to his compatriots.  They start moving towards you.]]
      },
      locate_next = {
         text = "Once again, you stand before %i.",
      },
      lowlevel = {
         synopsis = "[Will you abandon reason and attempt to defeat %n?]",
         output = "text",
         text = [[
"This is outrageous!  You are but a %r!  Will you turn all
reason on its head and defeat %n in such a state?"]],
      },
      nemesis_first = {
         synopsis = "[\"Fenrir can wait.  Fight us and die!\"]",
         output = "text",
         text = [[
"Who are you, %r, that approaches?  I do not recognize you
from the battle at %H.  Some last gambit of %ds,
I presume.

"I suppose Fenrir can wait a little longer, for us to have a little... 
sport.  Ah!  If your realm were not about to perish, the skalds would
sing such songs telling the fall of Midgard's last warrior.

Come, then, little %s!  We shall crush you like a gnat."]],
      },
      nemesis_next = {
         text = "\"You fight well, %r!  But not well enough to avoid death!\"",
      },
      nemesis_other = {
         text = "\"I shall pound you into oblivion, %c!\"",
      },
      nemesis_wantsit = {
         text = "\"You dare take %o from me?  May it burn you alive!\"",
      },
      nexttime = {
         text = "Once again, you are near the abode of %l.",
      },
      offeredit = {
         synopsis = "[Thank you.  Search for the Amulet.]",
         output = "text",
         text = [[
As you approach, %l rises to greet you, extremely relieved.

"%p, I never lost faith that you could break %ns power
and cast him down.  Instead of the world ending in flames, I now foresee
an era of peace.  A thousand thanks would not be enough.

"Your own future, though, is still uncertain.  You must now resume searching
for the Amulet of Yendor, in %ds name.  My thanks and blessings go
with you, my %S."]],
      },
      offeredit2 = {
         -- not printed in the regular quest, but you might still throw the
         -- artifact at the Norn
         synopsis = "[I do not need %o.  Return through the portal and find the Amulet.]",
         output = "text",
         text = [[
"So this is the sword of %n, is it?  What a magnificent weapon.

"But I have little use for this.  It will likely better serve you instead
in your search for the Amulet, even if you do not wish to wield it directly.

"%Z await your return through the magic portal that brought
you here."]],
      },
      othertime = {
         text = [[Again you materialize near %ls abode.  You have a nagging feeling
that this may be the last time you come here.]],
      },
      posthanks = {
         text = [["Greetings, %p.  I have not been able to pay as much
attention to your search for the Amulet as I have wished.  How do you fare?"]],
      },
   },
   Wiz = {
      assignquest = {
         synopsis = "[Travel to %i; overcome %n; return with %o.]",
         output = "text",
         text = [["Yes, %p, you truly are ready for this dire task.  Listen,
carefully, for what I tell you now will be of vital importance.

"Since you left us to hone your skills in the world, we unexpectedly came
under attack by the forces of %n.  As you know, we thought
%n had perished at the end of the last age, but, alas, this was
not the case.

"%nC sent an army of abominations against us.  Among them was a
minion, mindless and ensorcelled, and thus, in the confusion, it was
able to penetrate our defenses.  Alas, this creature has stolen
%o and I fear has delivered %oh to %n.

"Over the years, I had woven most of my power into this amulet, and thus,
without it, I have but a shadow of my former power, and I fear that I
shall soon perish.

"You must travel to %i, and within its dungeons,
find and overcome %n, and return %o to me.

"Go now, with %d, and complete this quest before it is too late."]],
      },
      badalign = {
         synopsis = "[Go; come back when you are worthy of %d.]",
         output = "text",
         text = [["You amaze me, %p!  How many times did I tell you that the way of a mage
is an exacting one.  One must use the world with care, lest one leave it
in ruins and simplify the task of %n.

"You must go back and show your worthiness.  Do not return until you are
truly ready for this quest.  May %d guide you in this task."]],
      },
      badlevel = {
         synopsis = "[Go; return when you are %Ra.]",
         output = "text",
         text = [["Alas, %p, you have not yet shown your proficiency as a worthy
spellcaster.  As %ra, you would surely be overcome in the challenge
ahead.  Go, now, expand your horizons, and return when you have attained
renown as %Ra."]],
      },
      discourage = {
         "\"Your puny powers are no match for me, fool!\"",
         "\"When you are defeated, your torment will last for a thousand years.\"",
         "\"After your downfall, %p, I shall devour %l for dessert!\"",
         "\"Are you ready yet to beg for mercy?  I could be lenient...\"",
         "\"Your soul shall join the enslaved multitude I command!\"",
         "\"Your lack of will is evident, and you shall die as a result.\"",
         "\"Your faith in %d is for naught!  Come, submit to me now!\"",
         "\"A mere %r is nothing compared to my skill!\"",
         "\"So, you are the best hope of %l?  How droll.\"",
         "\"Feel my power, %c!  My victory is imminent!\"",
      },
      encourage = {
         "\"Beware, for %n is immune to most magical attacks.\"",
         "\"To enter %i you must pass many traps.\"",
         "\"%nC may be vulnerable to physical attacks.\"",
         "\"%d will come to your aid when you call.\"",
         "\"You must utterly destroy %n.  He will pursue you otherwise.\"",
         "\"%oC is a mighty artifact.  With it you can destroy %n.\"",
         "\"Go forth with the blessings of %d.\"",
         "\"I will have my %gP watch for your return.\"",
         "\"Feel free to take any items in that chest that might aid you.\"",
         "\"You will know when %o is near.  Proceed with care!\"",
      },
      firsttime = {
         synopsis = "[You have arrived at %ls tower but something is very wrong.]",
         output = "text",
         text = [[You are suddenly in familiar surroundings.  You notice what appears to
be a large, squat stone structure nearby.  Wait!  That looks like the
tower of your former teacher, %l.

However, things are not the same as when you were last here.  Mists and
areas of unexplained darkness surround the tower.  There is movement in
the shadows.

Your teacher would never allow such unaesthetic forms to surround the
tower...  unless something were dreadfully wrong!]],
      },
      goal_alt = {
         text = "You have returned to %ns lair.",
      },
      goal_first = {
         text = "You feel your mentor's presence; perhaps %o is nearby.",
      },
      goal_next = {
         text = "The aura of %o tingles at the edge of your perception.",
      },
      gotit = {
         synopsis = "[You feel %os power and know you should return %oh to %l.]",
         output = "text",
         text = [[As you touch %o, its comforting power infuses you
with new energy.  You feel as if you can detect others' thoughts flowing
through it.  Although you yearn to wear %o and
attack the Wizard of Yendor, you know you must return it to its rightful
owner, %l.]],
      },
      guardtalk_after = {
         "\"I have some eye of newt to trade, do you have a spare blind-worm's sting?\"",
         "\"The magic portal now seems like it will remain stable for quite some time.\"",
         "\"Have you noticed how much stronger %l is since %o was recovered?\"",
         "\"Thank %d!  We weren't positive you would defeat %n.\"",
         "\"I, too, will venture into the world, because %n was but one of many evils to be vanquished.\"",
      },
      guardtalk_before = {
         "\"Would you happen to have some eye of newt in that overstuffed pack, %s?\"",
         "\"Ah, the spell to create the magic portal worked.  Outstanding!\"",
         "\"Hurry!  %lC may not survive that casting of the portal spell!\"",
         "\"The spells of %n were just too powerful for us to withstand.\"",
         "\"I, too, will venture into the world, because %n is but one of many evils to be vanquished.\"",
      },
      hasamulet = {
         synopsis = "[Take the Amulet to %ds altar on the Astral Plane.]",
         output = "text",
         text = [["Congratulations, %p.  I always knew that if anyone could succeed
in defeating the Wizard of Yendor and his minions, it would be you.

"Go now, and take the Amulet to the Astral Plane.  Once there, present
the Amulet on the altar of %d.  Along the way you shall pass through
the four Elemental Planes.  These planes are like nothing you have ever
experienced before, so be prepared!

"For this you were born, %s!  I am very proud of you."]],
      },
      killed_nemesis = {
         synopsis = "[%nC curses you as %nh dies.]",
         output = "text",
         text = [[%nC, whose body begins to shrivel up, croaks out:

    "I shall haunt your progress until the end of time.  A thousand
    curses on you and %l."

Then, the body bursts into a cloud of choking dust, and blows away.]],
      },
      leader_first = {
         synopsis = "[You have come a long way, but are you ready for the task I require?]",
         output = "text",
         text = [["Come closer, %p, for my voice falters in my old age.
Yes, I see that you have come a long way since you went out into the
world, leaving the safe confines of this tower.  However, I must first
determine if you have all of the skills required to take on the task
I require of you."]],
      },
      leader_last = {
         synopsis = "[\"Get out of here!\"]",
         output = "text",
         text = [["You fool, %p!  Why did I waste all of those years teaching you
the esoteric arts?  Get out of here!  I shall find another."]],
      },
      leader_next = {
         text = "\"Well, %p, you have returned.  Perhaps you are now ready...\"",
      },
      leader_other = {
         text = [["This is getting tedious, %p, but perseverance is a sign of a true mage.
I certainly hope that you are truly ready this time!"]],
      },
      locate_first = {
         text = "Wisps of fog swirl nearby.  You feel that %ns lair is close.",
      },
      locate_next = {
         text = "You believe that you may once again invade %i.",
      },
      lowlevel = {
         synopsis = "[A mere %r expects to take on %n?]",
         output = "text",
         text = [[
"Astonishing!  You show up as a mere %r and expect to take on
%n?"]],
      },
      nemesis_first = {
         synopsis = "[\"Your destruction should make for good sport.\"]",
         output = "text",
         text = [["Ah, I recognize you, %p.  So, %l has sent you to steal
%o from me, hmmm?  Well, %lh is a fool to send such
a mental weakling against me.

"Your destruction, however, should make for good sport.  In the end, you
shall beg me to kill you!"]],
      },
      nemesis_next = {
         synopsis = "[\"Your soul shall soon be mine to command.\"]",
         output = "text",
         text = [["How nice of you to return, %p!  I enjoyed our last meeting.  Are you
still hungry for more pain?

"Come!  Your soul, like %o, shall soon be mine to command."]],
      },
      nemesis_other = {
         text = [["I'm sure that your perseverance shall be the subject of innumerable
ballads, but you shall not be around to hear them, I fear!"]],
      },
      nemesis_wantsit = {
         text = [["Thief!  %oC belongs to me, now.  I shall feed
your living flesh to my minions."]],
      },
      nexttime = {
         text = "Once again, you are back at %H.",
      },
      offeredit = {
         synopsis = "[Take %o with you in your quest for the Amulet.]",
         output = "text",
         text = [[%lC notices %o in your possession,
beams at you and says:

    "I knew you could defeat %n and retrieve
    %o.  We shall never forget this
    brave service.

    "Take %oh with you in your quest for the Amulet of Yendor.
    I can sense that it has attuned %oiself to you already.

    "May %d guide you in your quest, and keep you from harm."]],
      },
      offeredit2 = {
         synopsis = "[Keep %o, return through the portal to %Z; find the other Amulet.]",
         output = "text",
         text = [["You are the keeper of %o now.  It is time to
recover the /other/ Amulet.  %Z await your return through
the magic portal which brought you here."]],
      },
      othertime = {
         text = [[You are back at %H.
You have an odd feeling this may be the last time you ever come here.]],
      },
      posthanks = {
         text = [["Come near, my %S, and share your adventures with me.
So, have you succeeded in your quest for the Amulet of Yendor?"]],
      },
   },
}
