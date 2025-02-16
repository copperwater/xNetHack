-- NetHack Monk Mon-goal.lua	$NHDT-Date: 1652196007 2022/05/10 15:20:07 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
-- des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "nommap-boss", "noflip");

des.level_init({ style = "solidfill", fg = " " });

des.map([[
           ----------             
           |........|             
           --.......|             
            |.......|             
            |.......|             
           --.......--   ----     
    --------.........-----..|     
    |.......................|     
    |.......................|     
    |.......................|     
    --------.........--------     
           --.......--            
            |.......|             
            |.......|             
            |.......|         ----
-------------.......-----------..|
|................................|
|................................|
|................................|
----------------------------------
]])
-- Dungeon Description
local place = { {14,04},{13,07} }
local placeidx = math.random(1, #place);

des.region(selection.area(00,00,33,19), "unlit")

-- Stairs and arrival area
des.stair("up", 16,18)

-- There is deliberately no teleport region constraining arrival to near the
-- stairs - since the hero cannot levelport down in the Quest while the nemesis
-- lives, the only way to arrive here is by falling from above. That can land
-- them anywhere on this map, and guaranteeing a landing close to the stairs
-- allows for a quick escape.

-- Traps
for i = 1,2 do
   des.trap("fire")
end
for i = 1,6 do
   des.trap("pit")
end
for i = 1,3 do
   des.trap("falling rock")
end
for i = 1,3 do
   des.trap()
end

-- Objects
for i = 1,15 do
   des.object("boulder")
   des.object('*')
end

des.monster({ id="Master Kaen", x=16,y=08, inventory=function()
   des.object({ id = "lenses", buc="blessed", name="The Eyes of the Overworld" })
end, keep_default_invent=true })
local core = selection.area(12,06,20,10)
for i = 1,6 do
   des.monster({ id="earth elemental", coord=core:rndcoord(), peaceful=0 })
end
for i = 1,9 do
   des.monster({ id="xorn", coord=core:rndcoord(), peaceful=0 })
end
-- water, wood, fire, earth, metal - not all are elementals so golems substitute
local points = { {16,01}, {05,08}, {27,08}, {01,17}, {32,17} }
shuffle(points)
des.monster({ id="water elemental", coord=points[1], peaceful=0 })
des.monster({ id="wood golem", coord=points[2], peaceful=0 })
des.monster({ id="fire elemental", coord=points[3], peaceful=0 })
des.monster({ id="earth elemental", coord=points[4], peaceful=0 })
des.monster({ id="iron golem", coord=points[5], peaceful=0 })
