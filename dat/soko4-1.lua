-- NetHack sokoban soko4-1.lua	$NHDT-Date: 1652196036 2022/05/10 15:20:36 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.2 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
--
-- In case you haven't played the game Sokoban, you'll learn
-- quickly.  This branch isn't particularly difficult, just time
-- consuming.  Some players may wish to skip this branch.
--
-- The following actions are currently permitted without penalty:
--   Carrying or throwing a boulder already in inventory
--     (player or nonplayer).
--   Teleporting boulders.
--   Digging in the floor.
--   Jumping.
--   Being pulled by a thrown iron ball.
--   Hurtling through the air from Newton's 3rd law.
-- The following actions are permitted, but with a luck penalty:
--   Breaking boulders.
--   Stone-to-fleshing boulders.
--   Creating new boulders (e.g., with a scroll of earth).
--   Squeezing past boulders when naked or as a giant.
-- These actions are not permitted:
--   Moving diagonally between two boulders and/or walls.
--   Pushing a boulder diagonally.
--   Picking up boulders (player or nonplayer).
--   Digging or walking through walls.
--   Teleporting within levels or between levels of this branch.
--   Using cursed potions of gain level.
--   Escaping a pit/hole (e.g., by flying, levitation, or
--     passing a dexterity check).
--   Bones files are not permitted.


--## Bottom (first) level of Sokoban ###
des.level_init({ style = "solidfill", fg = " " });

des.message("The floor here is covered in deep perpendicular grooves.")

des.level_flags("mazelevel", "noteleport", "hardfloor", "premapped", "sokoban", "solidify", "noflip");

des.map([[
------  ----- 
|....|  |...| 
|....----...| 
|...........| 
|..|-|.|-|..| 
---------|.---
|......|.....|
|..----|.....|
--.|   |.....|
 |.|---|.....|
 |...........|
 |..|---------
 ----         
]]);
des.levregion({ region = {06,04,06,04}, type = "branch" })
des.stair("up", 06,06)
des.region(selection.area(00,00,13,12), "lit")
des.non_diggable(selection.area(00,00,13,12))
des.non_passwall(selection.area(00,00,13,12))

-- Boulders
des.object("boulder",02,02)
des.object("boulder",02,03)
--
des.object("boulder",10,02)
des.object("boulder",09,03)
des.object("boulder",10,04)
--
des.object("boulder",08,07)
des.object("boulder",09,08)
des.object("boulder",09,09)
des.object("boulder",08,10)
des.object("boulder",10,10)

-- prevent monster generation over the (filled) pits
des.exclusion({ type = "monster-generation", region = { 01,06, 07,11 } });
-- Traps
des.trap("pit",03,06)
des.trap("pit",04,06)
des.trap("pit",05,06)
des.trap("pit",02,08)
des.trap("pit",02,09)
des.trap("pit",04,10)
des.trap("pit",05,10)
des.trap("pit",06,10)
des.trap("pit",07,10)

-- A little help
des.object("scroll of earth",02,11)
des.object("scroll of earth",03,11)

-- Random objects
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "=" });
des.object({ class = "/" });

