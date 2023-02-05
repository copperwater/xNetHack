-- The Gate of Hell - entrance to Gehennom proper, beneath the Valley

des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel", "noflip", "noteleport", "hardfloor", "solidify");

des.map([[
............................................LLLLLLLLLLL....LLLLLL............
.................----------------------.LLLLLLLLLLLLLLLLLLLLLLL..............
.......................LLLLLL........LLLLLLLLLLLLLLLLLLLLLLL.................
.....................LLLLLLLLLLLLLLLLLLLLLLLLLL.LLL...LLL....................
.....................LLLLLLLLLLLLLLLLLLLLLLLL.........LLLL...................
...................LLLLLLLLLLL.LLLLLLL..................LLLLL..............LL
.................LLLLLLLLLL..LLLLLL.......................LLLLLLLL.....LLLLLL
.................LLLLLLL.LL..LLLLLLL.......................LLLLLLLLLLLLLLLLLL
.................LLLLL.LLLLLLLLLLLLLL.......................LLLLL........LLLL
......----...LLLLLLLLLL..LLLLLLLLLLLLLLLL....................LLLLL.......LLLL
-------LLLLLLLLLLLLLLLLLLLLLLLLLLL.LL..L.....................LLLLLLL...LLLLLL
LLLLLLLLLLLLLLLLLLLLLLLLLLL...LLLLLLLL.L................LLLLLLLLLLLLLLLLLLLLL
LLLLLLLLL..LL.LLLLLLL..........LLLLL.LLLL...........LLLLLLLLLLLLLLLLLLLLLLLLL
...------...LLLLLLL.---LLLLL---L.L.LLLLLLLLLLLL...LLLLLLLLLLLLLLLLLLLLLLLLLLL
..........LLLLLLLL..-----------LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL...LLLLL
.LLLLLLLLLLLLLLLLLLLL|.......|..LLLLLLLLLLLLLLLLLLLLLLL..LLL.................
LLLLLLLLLLLLLLLLLLLLL|FFFFFFF|....LLLL...-----...-----...-----...-----...----
-------..LLLLLLLLL...|.......-------------...-----...-----...-----...-----...
......----------------.......................................................
.............................................................................
]])

-- undiggable and unphaseable, except the iron bars
local allbutironbars = selection.line(22,16,28,16):negate()
des.non_diggable(allbutironbars)
des.non_passwall(allbutironbars)

-- some terrain variation
local variations = {
   { x=22, y=02, mapchar='L' },
   { x=20, y=04, mapchar='L' },
   { x=30, y=05, mapchar='L' },
   { x=37, y=05, mapchar='.' },
   { x=24, y=07, mapchar='L' },
   { x=22, y=08, mapchar='L' },
   { x=37, y=08, mapchar='L' },
   { x=13, y=09, mapchar='L' },
   { x=34, y=10, mapchar='L' },
   { x=37, y=10, mapchar='L' },
   { x=38, y=10, mapchar='L' },
   { x=61, y=10, mapchar='L' },
   { x=38, y=11, mapchar='L' },
   { x=56, y=11, mapchar='.' },
   { x=13, y=12, mapchar='L' },
   { x=20, y=12, mapchar='.' },
   { x=30, y=12, mapchar='L' },
   { x=36, y=12, mapchar='L' },
   { x=36, y=12, mapchar='L' },
   { x=31, y=13, mapchar='.' },
   { x=34, y=13, mapchar='L' },
   { x=10, y=14, mapchar='.' },
   { x=56, y=15, mapchar='L' },
   { x=60, y=15, mapchar='L' },
   { x=09, y=17, mapchar='.' }
}
for i = 1, #variations do
   if percent(20) then
      local islit = 0
      des.terrain({ x=variations[i].x, y=variations[i].y,
                    typ=variations[i].mapchar, lit=0 })
   end
end

-- Vlad's Tower entrance is in the top left, amid some areas where the ceiling
-- has collapsed and rubble piled up from the Chasm
local vladstairarea = selection.area(00,00,16,09)
for i = 09,16 do
   -- this slices off a triangle from that rectangle that we don't want
   vladstairarea = vladstairarea - selection.line(i,09,i+8,01)
end
-- Impassable pillars / cave-ins
for i=1,18 do
   des.terrain({ typ=' ', coord=vladstairarea:rndcoord(1) })
end
vladstairarea = vladstairarea:filter_mapchar('.')
-- Line of boulders
selection.line(10,08,16,02):iterate(function(x,y)
   des.object('boulder', x, y)
end)
-- A few boulders block the other route
local topboulders = selection.line(19,00,38,00)
for i = 1, d(3) do
   des.object({ id='boulder', coord=topboulders:rndcoord() })
end
-- Random boulders and rocks scattered in the remaining area
for i = 1, 10 + d(10) do
   des.object({ id='boulder', coord=vladstairarea:rndcoord() })
   des.object({ id='rock', quan = d(5), coord=vladstairarea:rndcoord() })
end
-- And finally the staircase
vstairs = vladstairarea:rndcoord()
des.levregion({ region={vstairs.x,vstairs.y,vstairs.x,vstairs.y}, type='branch' })

local upstairarea = selection.area(57,00,76,06):filter_mapchar('.')
des.stair({ dir = 'up', coord = upstairarea:rndcoord() })

local dnstairarea = selection.area(00,18,18,19) + selection.area(32,17,76,19)
dnstairarea = dnstairarea:filter_mapchar('.')
des.stair({ dir = 'down', coord = dnstairarea:rndcoord() })

-- Set up teleport regions so that hero cannot randomly bypass parts of the
-- level
des.teleport_region({ region={65,00,76,04}, dir="down" })
des.teleport_region({ region={00,18,76,19}, dir="up" })

-- The (main) Gate
des.drawbridge({ dir="south", state="closed", x=25, y=13 })
-- and its primary guardian
des.monster('Cerberus', 48,08)
-- and its decoration (which might hint to anyone who circumvents Cerberus that
-- there's a reason the drawbridge is not opening)
-- these are intentionally embedded in the wall
des.object({ id = 'statue', x=21, y=13, montype='hell hound' })
des.object({ id = 'statue', x=29, y=13, montype='hell hound' })

-- Various traps
for i = 1, 16 + d(4) do
   des.trap()
end

-- Various objects
for i = 1, 8 + d(4) do
   des.object()
end

-- Demons on guard duty
for i = 1, 13 + d(3) do
   des.monster('&')
end
for i = 1, d(3) do
   des.monster('i')
end

-- And some non-demonic monsters
for i = 1, 2 do
   des.monster('red dragon')
   des.monster('D')
end
for i = 1, d(3) do
   des.monster('ghost')
   des.monster('hell hound')
end

