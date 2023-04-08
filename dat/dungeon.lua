-- NetHack 3.6	dungeon dungeon.lua	$NHDT-Date: 1652196135 2022/05/10 15:22:15 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.4 $
-- Copyright (c) 1990-95 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
-- The dungeon description file.

-- Randomize a couple of demon lair placements. These strings are the special
-- level identifiers.
local lawdemon1 = 'baalz'
local lawdemon2 = 'geryon'
if percent(50) then
   lawdemon1, lawdemon2 = lawdemon2, lawdemon1
end

local chaosdemon1 = 'orcus'
local chaosdemon2 = 'yeenoghu'
if percent(50) then
   chaosdemon1, chaosdemon2 = chaosdemon2, chaosdemon1
end

dungeon = {
   {
      name = "The Dungeons of Doom",
      bonetag = "D",
      base = 25,
      range = 5,
      alignment = "unaligned",
      themerooms = "themerms.lua",
      branches = {
         {
            name = "The Gnomish Mines",
            base = 2,
            range = 3
         },
         {
            name = "Sokoban",
            chainlevel = "oracle",
            base = 1,
            direction = "up"
         },
         {
            name = "The Quest",
            chainlevel = "oracle",
            base = 6,
            range = 2,
            branchtype = "portal"
         },
         {
            name = "Fort Ludios",
            base = 18,
            range = 4,
            branchtype = "portal"
         },
         {
            name = "Gehennom",
            chainlevel = "castle",
            base = 0,
            branchtype = "no_down"
         },
         {
            name = "The Elemental Planes",
            base = 1,
            branchtype = "no_down",
            direction = "up"
         }
      },
      levels = {
         {
            name = "oracle",
            bonetag = "O",
            base = 5,
            range = 5,
            alignment = "neutral",
            nlevels = 3
         },
         {
            name = "bigrm",
            bonetag = "B",
            base = 13,
            range = 3,
            chance = 40,
            nlevels = 13
         },
         {
            name = "medusa",
            base = -5,
            range = 4,
            nlevels = 4,
            alignment = "chaotic"
         },
         {
            name = "castle",
            base = -1
         }
      }
   },
   {
      name = "Gehennom",
      bonetag = "G",
      base = 10,
      flags = { "mazelike", "hellish" },
      alignment = "noalign",
      lvlfill = "hellfill",
      branches = {
         {
            name = "Vlad's Tower",
            base = 2,
            direction = "up"
         },
         {
            name = "Cocytus",
            base = 3,
            direction = "down"
         },
         {
            name = "Asphodel",
            base = 4,
            direction = "down"
         },
         {
            name = "Shedaklah",
            base = 5,
            direction = "down"
         }, 
         {
            name = "The Citadel of Dis",
            base = 6,
            direction = "up"
         },
         {
            name = "The Abyss",
            base = 7,
            direction = "down"
         },
         {
            name = "The Wizard's Tower",
            base = 8,
            direction = "up"
         }
      },
      levels = {
         {
            name = "valley",
            bonetag = "V",
            base = 1
         },
         {
            name = "hellgate",
            bonetag = "G",
            base = 2
         },
         {
            name = "styxmarsh",
            bonetag = "S",
            base = 5
         },
         {
            name = "dis",
            bonetag = "D",
            base = 6
         },
         {
            name = "invocation",
            base = -2
         },
         {
            name = "sanctum",
            base = -1
         },
      }
   },
   {
      name = "The Gnomish Mines",
      bonetag = "M",
      base = 8,
      range = 2,
      alignment = "lawful",
      flags = { "mazelike" },
      lvlfill = "minefill",
      levels = {
         {
            name = "minetn",
            bonetag = "T",
            base = 3,
            range = 2,
            nlevels = 8,
            flags = "town"
         },
         {
            name = "minend",
--          3.7.0: minend changed to no-bones to simplify achievement tracking
--          bonetag = "E"
            base = -1,
            nlevels = 4
         },
      }
   },
   {
      name = "The Quest",
      bonetag = "Q",
      base = 5,
      range = 2,
      levels = {
         {
            name = "x-strt",
            base = 1,
            range = 1
         },
         {
            name = "x-loca",
            bonetag = "L",
            base = 3,
            range = 1
         },
         {
            name = "x-goal",
            base = -1
         },
      }
   },
   {
      name = "Sokoban",
      base = 4,
      alignment = "neutral",
      flags = { "mazelike" },
      entry = -1,
      levels = {
         {
            name = "soko1",
            base = 1,
            nlevels = 2
         },
         {
            name = "soko2",
            base = 2,
            nlevels = 2
         },
         {
            name = "soko3",
            base = 3,
            nlevels = 2
         },
         {
            name = "soko4",
            base = 4,
            nlevels = 2
         },
      }
   },
   {
      name = "Fort Ludios",
      base = 1,
      bonetag = "K",
      flags = { "mazelike" },
      alignment = "unaligned",
      levels = {
         {
            name = "knox",
            bonetag = "K",
            base = -1
         }
      }
   },
   {
      name = "Vlad's Tower",
      base = 4,
      bonetag = "V",
      protofile = "tower",
      alignment = "chaotic",
      flags = { "mazelike" },
      entry = -1,
      levels = {
         {
            name = "tower1",
            base = 1
         },
         {
            name = "tower2",
            base = 2
         },
         {
            name = "tower3",
            base = 3
         },
         {
            name = "tower4",
            base = 4
         },
      }
   },
   {
      name = "Cocytus",
      bonetag = "C",
      base = 3,
      flags = { "mazelike", "hellish" },
      alignment = "lawful",
      lvlfill = "cocytusfill",
      entry = 1,
      levels = {
         {
            name = lawdemon1,
            bonetag = "H",
            base = 2
         },
         {
            name = "asmodeus",
            bonetag = "A",
            base = 3
         }
      }
   },
   {
      name = "Asphodel",
      bonetag = "P",
      base = 1,
      flags = { "mazelike", "hellish" },
      alignment = "lawful",
      levels = {
         {
            name = lawdemon2,
            bonetag = "P",
            base = 1
         }
      }
   },
   {
      name = "Shedaklah",
      bonetag = "S",
      base = 2,
      flags = { "mazelike", "hellish" },
      alignment = "chaotic",
      levels = {
         {
            name = "juiblex",
            bonetag = "J",
            base = 1
         },
         {
            name = chaosdemon1,
            bonetag = "P",
            base = 2
         }
      }
   },
   {
      name = "The Citadel of Dis",
      bonetag = "I",
      base = 1,
      flags = { "mazelike", "hellish" },
      kkalignment = "lawful",
      levels = {
         {
            name = "dispater",
            bonetag = "D",
            base = 1
         }
      }
   },
   {
      name = "The Abyss",
      bonetag = "A",
      base = 3,
      flags = { "mazelike", "hellish" },
      alignment = "chaotic",
      lvlfill = "abyssfill",
      branches = {
         {
            name = "Tartarus",
            base = 2,
            direction = "down"
         }
      },
      levels = {
         {
            name = chaosdemon2,
            bonetag = "Q",
            base = 3
         }
      }
   },
   {
      name = "Tartarus",
      bonetag = "T",
      base = 1,
      flags = { "mazelike", "hellish" },
      alignment = "chaotic",
      levels = {
         {
            name = "demogorgon",
            bonetag = "D",
            base = 1
         }
      }
   },
   {
      name = "The Wizard's Tower",
      bonetag = "W",
      base = 3,
      flags = { "mazelike" },
      alignment = "unaligned",
      entry = -1,
      levels = {
         {
            name = "wizard1",
            base = 1
         },
         {
            name = "wizard2",
            bonetag = "X",
            base = 2
         },
         {
            name = "wizard3",
            bonetag = "Y",
            base = 3
         },
      }
   },
   {
      name = "The Elemental Planes",
      bonetag = "E",
      base = 6,
      alignment = "unaligned",
      flags = { "mazelike" },
      entry = -2,
      levels = {
         {
            name = "astral",
            base = 1
         },
         {
            name = "water",
            base = 2
         },
         {
            name = "fire",
            base = 3
         },
         {
            name = "air",
            base = 4
         },
         {
            name = "earth",
            base = 5
         },
         {
            name = "dummy",
            base = 6
         },
      }
   },
}
