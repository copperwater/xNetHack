-- NetHack sokoban soko4-2.lua	$NHDT-Date: 1652196036 2022/05/10 15:20:36 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.2 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.message("The floor here is covered in deep perpendicular grooves.")

des.level_flags("mazelevel", "noteleport", "hardfloor", "premapped", "sokoban", "solidify", "noflip");

des.map([[
-------- ------
|.|....|-|....|
|.|-..........|
|.||....|.....|
|.||....|.....|
|.|-----|.-----
|.|    |......|
|.-----|......|
|.............|
|..|---|......|
----   --------
]]);
des.levregion({ region = {03,01,03,01}, type = "branch" })
des.stair("up", 01,01)
des.region(selection.area(00,00,14,10),"lit")
des.non_diggable(selection.area(00,00,14,10))
des.non_passwall(selection.area(00,00,14,10))

-- Boulders
des.object("boulder",05,02)
des.object("boulder",06,02)
des.object("boulder",06,03)
des.object("boulder",07,03)
--
des.object("boulder",09,05)
des.object("boulder",10,03)
des.object("boulder",11,02)
des.object("boulder",12,03)
--
des.object("boulder",07,08)
des.object("boulder",08,08)
des.object("boulder",09,08)
des.object("boulder",10,08)

-- prevent monster generation over the (filled) pits
des.exclusion({ type = "monster-generation", region = { 01,01, 01,09 } });
des.exclusion({ type = "monster-generation", region = { 01,08, 07,09 } });
-- Traps
des.trap("pit",01,02)
des.trap("pit",01,03)
des.trap("pit",01,04)
des.trap("pit",01,05)
des.trap("pit",01,06)
des.trap("pit",01,07)
des.trap("pit",03,08)
des.trap("pit",04,08)
des.trap("pit",05,08)
des.trap("pit",06,08)

-- A little help
des.object("scroll of earth",01,09)
des.object("scroll of earth",02,09)

-- Random objects
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "=" });
des.object({ class = "/" });


