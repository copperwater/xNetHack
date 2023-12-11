-- NetHack gehennom orcus.lua	$NHDT-Date: 1652196033 2022/05/10 15:20:33 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.3 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style="solidfill", fg =" " });

des.level_flags("mazelevel", "shortsighted", "noteleport", "noflip", "nommap-boss")
-- The Necropolis, or the Dead End
des.map([[
.-------------------------------------------------------------------------   
..--....|....|....|..............|....|.........|...|....................--  
..--....|....|....|..............|....|.........|...|.....------------....---
...|....|....|....|--...-+-------|............---.---...---..........----...|
...|....|....|....|..............+............|...|....--...............--..|
...|.........|....|..............|....|.......|...+...--..    .....    ..--.|
...|-+-...-+----+--....-------...--------.-+--|...|...|..      ...      ..|.|
...|...................|.....|................|---|...|..      ...      ..|.|
...|...................|.....|................|...|...|...    .....    ...|.|
...|-+----....-+---....|.....|...----------+---...|...---....... .......---.|
...|....|....|....|....---+---...|......|....|....|.....|......   ......|...|
...|.........|....|..............|......|....|...--...---......   ......---.|
...|---...---------.....-----....+......|....--+-|....|...................|.|
...|........................|....|......|........|....----.............----.|
...|---------+-...--+--|....|....----------+----.|.......|.|.|.|.|.|.|.|....|
...|....|..............|....+....|.............----......|.|.|.|.|.|.|.|..---
...|....+.......|......|....|....|.........-----.....|...---------------..|  
..--....|.......|......|....|....|.........|.........|...................--  
.-------------------------------------------------------------------------   
]]);

des.non_diggable()
des.non_passwall(selection.area(54,02,74,16))

-- Entire main area
des.region(selection.area(00,00,74,18),"unlit")

-- This branch is the bottom of either the Abyss or Shedaklah, so no downstairs
des.levregion({ region={00,00,00,18}, type="stair-up" });

-- Wall "ruins"
local boulders = selection.new()
boulders = boulders + selection.line(07,06,09,06)
boulders:set(08,05)
boulders = boulders + selection.line(07,12,09,12)
boulders:set(08,11)
boulders = boulders + selection.line(10,09,13,09)
boulders = boulders + selection.line(15,14,17,14)
boulders:set(16,15)
boulders = boulders + selection.line(23,12,23,13)
boulders = boulders + selection.line(21,03,23,03)
boulders = boulders + selection.line(38,03,38,04)
boulders:set(41,06)
boulders = boulders + selection.line(51,15,53,15)
boulders:iterate(function(x,y)
   if percent(90) then
      des.object('boulder', x, y)
      if percent(10) then
         des.object('boulder', x, y)
      end
   end
end)

local leftwall = selection.line(03,03,03,16)
leftwall:set(03,06, 0);
leftwall:set(03,09, 0);
leftwall:set(03,12, 0);
leftwall:set(03,14, 0);
local madegap = false
leftwall:iterate(function(x,y)
   if percent(20) then
      des.terrain(x, y, '.')
      if percent(50) then
         des.object('boulder', x, y)
      end
      madegap = true
   end
end)
if not madegap then
   des.terrain(03,10, '.') -- failsafe
end

-- Doors
des.door("closed",25,03)
des.door("open",33,04)
des.door("nodoor",05,06)
des.door("closed",11,06)
des.door("closed",16,06)
des.door("closed",43,06)
des.door("open",05,09)
des.door("nodoor",15,09)
des.door("open",43,09)
des.door("closed",26,10)
des.door("closed",33,12)
des.door("open",13,14)
des.door("closed",20,14)
des.door("closed",43,14)
des.door("open",28,15)
des.door("closed",08,16)
des.door("closed",50,05)
des.door("closed",47,12)
local choice = d(4)
if choice == 1 then
   des.door("secret", 48,01)
elseif choice == 2 then
   des.door("secret", 48,07)
elseif choice == 3 then
   des.door("secret", 50,09)
elseif choice == 4 then
   des.door("secret", 45,16)
end

-- Special rooms
des.altar({ x=26,y=08,align="noalign",type="sanctum" })
des.region({ region={24,13,27,17},lit=0,type="morgue",filled=1 })
des.region({ region={34,10,39,13},lit=1,type="shop",filled=1 })
des.region({ region={14,01,17,05},lit=1,type="shop",filled=1 })

-- The Skull
local teeth = selection.area(58,14,70,15):filter_mapchar('.')
local skull = selection.floodfill(64,08) - teeth
des.door({ state='secret', locked=1, iron=1, x = 58 + (nh.rn2(7) * 2), y=16 })
teeth:iterate(function(x,y)
   if percent(60) then
      des.object('boulder', x, y)
   end
end)
for i = 1, 3 do
   des.monster({ id='shade', coord=skull:rndcoord(), waiting=1 })
   des.monster({ id='vampire', coord=skull:rndcoord() })
   des.monster({ id='vampire lord', coord=skull:rndcoord() })
end
for i = 1, 2 do
   des.monster({ class='L', coord=skull:rndcoord(), waiting=1 })
   des.monster({ class='Z', coord=skull:rndcoord() })
   des.monster({ class='&', align='chaos', coord=skull:rndcoord() })
end
des.region({ region={59,03,68,05}, type="throne", filled=0, lit=0 })
des.object({ id='chest', x=64, y=03, locked=true, trapped=true, contents=function()
   if nh.is_wish_dlord('Orcus') then
      des.object({ class='/', id='wishing', spe=1 })
   else
      local blackgems = { 'jet', 'obsidian', 'black opal', 'worthless piece of black glass' }
      for i = 1, 8 + d(6) do
         des.object({ id=blackgems[d(#blackgems)] })
      end
   end
end })
if percent(5) then
   des.object("diamond", 60,07)
   des.object("diamond", 68,07)
end

---------------------------------------
-- Random level-wide things
---------------------------------------
local restoftown = (selection.area(04,01,75,17) - skull - teeth):filter_mapchar('.')

-- Graves (outside graveyards)
local notemple = restoftown - selection.area(24,07,28,09)
for i = 1, 5 + d(3) do
   des.grave({ coord=notemple:rndcoord() })
end

-- A few scattered corpses
for i = 1, 5 + d(3) do
   des.object('corpse')
end

-- Monsters
des.monster('L', 16,04)
des.monster('L', 35,12)
for i = 1, 5 do
   des.monster('skeleton')
   des.monster('shade')
   des.monster('&')
   des.monster()
end
for i = 1, 3 do
   des.monster('giant zombie')
   des.monster('ettin zombie')
   des.monster('human zombie')
   des.monster('Z')
   des.monster('V')
end

-- Traps
for i = 1, 10 + d(3) do
   des.trap({ coord=restoftown:rndcoord() })
end
des.trap('spiked pit', 55,12)
des.trap('spiked pit', 73,12)

-- Objects (note that the shops and graveyards are stocked with their normal
-- items)
for i = 1, 8 + d(4) do
   des.object()
end
