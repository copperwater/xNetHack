-- NetHack gehennom asmodeus.lua	$NHDT-Date: 1652196020 2022/05/10 15:20:20 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.2 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style="solidfill", fg =" " });

des.level_flags("mazelevel", "noteleport", "nommap-boss", "icedpools", "solidify", "noflipx")

des.map([[
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
IIIIIIIIIIIIIIIII---F--F------IIIIIIIIIIIIIIIII--F-F--IIIIIIIIIIII-------II
IIIIIIIIIIIIIII---......F....|IIIIIIIIIIIIIIIII|.....--------------..|..|II
.IIIIIIIIIII----........F.----IIIIIIIIIIIIIIIIIF.....|.........|.....--S|II
IIIIIIIIII---...........---IIIIII--F-F-F--IIIII|.....|..----...+........|II
IIIIIIIIIIF..........----IIIIIIII|.......|IIIII----------..|...------..--II
IIIIIIIIII|.......----IIIIIIIIIII|.......|IIIIII|.......-S------.|.....|III
IIIIIII----+----------------------.......-------|.......|IIIIIIS.|.....|III
IIIII..|.......|................................|-----+-|..IIII|--.....|III
IIII...+.......+................................+.......|..IIII+.......|III
IIII...+.......+................................+.......|..IIII+.......|III
IIIII..|.......|................................|-----+-|..IIII|--.....|III
IIIIIII----+----------------------.......-------|.......|IIIIIIS.|.....|III
IIIIIIIIII|.......----IIIIIIIIIII|.......|IIIIII|.......|S------.|.....|III
IIIIIIIIIIF..........----IIIIIIII|.......|IIIII---------|..|...---..-----II
IIIIIIIIII---...........---IIIIII--F-F-F--IIIII|.....|..----.....|......|II
.IIIIIIIIIII----........F.----IIIIIIIIIIIIIIIIIF.....|...........+......|II
IIIIIIIIIIIIIII---......F....|IIIIIIIIIIIIIIIII|.....--------------.....|II
IIIIIIIIIIIIIIIII---F--F------IIIIIIIIIIIIIIIII--F-F--IIIIIIIIIIII-------II
IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
]] )
-- the two spaces on the far left guarantee there will always be valid spots to
-- place the stairs on (since it won't place them on ice, and we can't count on
-- the replace_terrain to always put at least one spot of floor there)

-- Define areas
local everything = selection.area(00,00,74,19)
local wing1 = selection.floodfill(18,04)
local wing2 = selection.floodfill(18,15)
local recess1 = selection.area(34,05,40,07)
local recess2 = selection.area(34,12,40,14)
local antechamber = selection.floodfill(11,09)
local hallway = selection.area(16,08,47,11)
local fortress = selection.area(48,02,71,17):filter_mapchar('.')
local sanctum = selection.area(57,07,62,12)
fortress = fortress - sanctum -- fortress does NOT include sanctum
local allindoors = wing1 + wing2 + antechamber + hallway + recess1 + recess2 + fortress + sanctum

-- Teleport region so people don't fall into the fortress
des.teleport_region({ region={00,00,74,19}, exclude={08,02,71,17} })

-- This isn't a lake
des.replace_terrain({ selection=(everything - sanctum),
                      fromterrain='I', toterrain='.', chance=20 })

-- Some conditional doors
local doorlocs = { {50,05}, {53,03}, {55,05} }
shuffle(doorlocs)
des.door("random", doorlocs[1][1], doorlocs[1][2])
des.door("random", doorlocs[2][1], doorlocs[2][2])
doorlocs = { {50,14}, {53,16}, {55,14} }
shuffle(doorlocs)
des.door("random", doorlocs[1][1], doorlocs[1][2])
des.door("random", doorlocs[2][1], doorlocs[2][2])

-- Two conditional walls
if percent(50) then
   des.terrain(selection.line(69,05,70,05), '-')
else
   des.terrain(selection.line(66,14,67,14), '-')
end

-- "Throne" room (no throne or court, but define this as a special room so
-- entering can trigger Asmodeus's appearance). Asmodeus is not generated when
-- the level is created.
des.region({ region={57,07,62,12}, type="throne", filled=0, lit=1 })
des.object('ruby', 57, 08)
des.object('ruby', 57, 11)
des.object('sapphire', 59, 07)
des.object('sapphire', 59, 12)
des.object('ruby', 61, 07)
des.object('ruby', 61, 12)
des.trap('fire', 57,09)
des.trap('fire', 57,10)

-- Entry stairs
des.levregion({ region={00,00,00,19}, type="stair-up" });

-- Non diggable/phaseable walls
des.non_diggable()
des.non_passwall()

-- Lit areas
des.region(fortress,"lit")

-- Doors
des.door({ state='locked', iron=1, x=07, y=09 })
des.door({ state='locked', iron=1, x=07, y=10 })
des.door('open', 11,07)
des.door('open', 11,12)
-- we don't want any of the double doors to be mismatched iron and non iron
local doiron = percent(80) and 1 or 0
des.door({ state='locked', x=15, y=09, iron=doiron })
des.door({ state='locked', x=15, y=10, iron=doiron })
doiron = percent(80) and 1 or 0
des.door({ state='closed', x=48, y=09, iron=doiron })
des.door({ state='closed', x=48, y=10, iron=doiron })
doiron = percent(80) and 1 or 0
des.door({ state='closed', x=63, y=09, iron=doiron })
des.door({ state='closed', x=63, y=10, iron=doiron })
-- non double doors
des.door('random', 54,08)
des.door('random', 54,11)
des.door('random', 63,04)
des.door('random', 65,16)

-- The wings:
if percent(25) then
   des.monster({ id='prisoner', x=28, y=2 })
end
if percent(25) then
   des.monster({ id='prisoner', x=28, y=17 })
end
for i = 1, 4 do
   des.monster({ class='&', align = percent(70) and 'law' or 'random', coord=wing1:rndcoord() })
   des.monster({ class='&', align = percent(70) and 'law' or 'random', coord=wing2:rndcoord() })
end
for i = 1, 2 do
   des.monster({ class='i', coord=wing1:rndcoord() })
   des.monster({ class='i', coord=wing2:rndcoord() })
   des.object({ class=')', coord=wing1:rndcoord() })
   des.object({ class=')', coord=wing2:rndcoord() })
end

-- The hallway:
-- kind of annoying that we have to hardcode this list because montype requires
-- an exact species. Foocubi are chaotic, but appear on this list because
-- Asmodeus is associated with lust
local devils = { 'horned devil', 'barbed devil', 'bone devil', 'ice devil', 'amorous demon' }
function rnddevil()
   return devils[d(#devils)]
end
function demonstatue(xx, yy)
   des.object({ id='statue', x=xx, y=yy, montype=rnddevil(), trapped=percent(10) and 1 or 0,
                  contents=function()
      des.object({ id='gold piece', quantity=d(5,10) })
   end })
end
for x = 17,32,3 do
   for y = 08,11,3 do
      demonstatue(x, y)
   end
end
for x = 42,45,3 do
   for y = 08,11,3 do
      demonstatue(x, y)
   end
end
-- waiting devils in the recesses of the hallway
local bothrecesses = (recess1 + recess2) - selection.area(34,07,40,12)
for i = 1, 3 + d(4) do
   des.monster({ class = '&', align = percent(80) and 'law' or 'random', coord = bothrecesses:rndcoord(1), waiting = 1 })
end

-- The fortress:
fortcopy = fortress:clone() -- so we can rndcoord(1) it
for i = 1, 10 do
   des.trap({ type='cold', coord = fortcopy:rndcoord(1) })
   des.monster({ class = '&', align = 'law', coord = fortcopy:rndcoord(1), waiting = 1 })
end
des.monster({ class='V', coord = fortcopy:rndcoord(1) })
des.monster({ class='V', coord = fortcopy:rndcoord(1) })
des.monster({ class='L', coord = fortcopy:rndcoord(1) })
des.monster({ id='ghost', coord = fortcopy:rndcoord(1) })

des.trap({ type="spiked pit", coord = fortcopy:rndcoord(1) })
des.trap({ type="fire", coord = fortcopy:rndcoord(1) })
des.trap({ type="sleep gas", coord = fortcopy:rndcoord(1) })
des.trap({ type="anti magic", coord = fortcopy:rndcoord(1) })
des.trap({ type="fire", coord = fortcopy:rndcoord(1) })
des.trap({ type="magic", coord = fortcopy:rndcoord(1) })
des.trap({ type="magic", coord = fortcopy:rndcoord(1) })
des.object("chest", 70,02)
for i = 1,3 do
   des.object({ class='?', coord=fortress:rndcoord() })
   des.object({ class='+', coord=fortress:rndcoord() })
end
local lootspots = { {58,05}, {58,14}, {64,06}, {64,13} }
shuffle(lootspots)
for i = 1, 3 do
   des.object({ id='chest', locked=1, coord=lootspots[i], material='iron', contents=function()
      des.object({ id='gold piece', quantity=d(6,50) })
      for n = 1, 3 do
         des.object('*')
         des.object()
      end
   end })
end
des.object({ id='chest', locked=1, coord=lootspots[4], material='iron', contents=function()
   if nh.is_wish_dlord('Asmodeus') then
      des.object({ class='/', id='wishing', spe=1 })
   else
      des.object({ class='/', id = percent(30) and 'fire' or 'cold' })
   end
end })

-- general level-wide stuff:
for i = 1,3 do
   des.object('[')
   des.object(')')
   des.object('*')
end
local icymons = get_icymon_list()
for i = 1, #icymons do
   for j = 1, d(4)-1 do
      des.monster(icymons[i])
   end
end
for i = 1, 30 do
   des.trap('cold')
end
