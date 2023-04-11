-- Geryon's lair

des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel", "nommap-boss", "noteleport", "noflipx");

des.map([[
  --.|......||.................|........|.....|...........||.....|....|.-- 
  |..F......++......--------+--|........|--+--|...........++.....F....F..--
---..F......||-------...|......+........+.....--+---------||.....F....F...-
.....|.......|......|...+......|........|..........|......|......+....|...-
.....F.......|......+...|...-------++-------.......+......|......F....F....
.....F.......|......---------..............---------......|......F....F....
.....|.......--------.......|..............|.......--------......-----|....
-....F......................-------++-------..........................F....
--...F...........................F....F...............................F....
-....|...........................|....|...............................|....
.....F...........................F....F...............................F....
.....F...........................F....F...............................F....
.....|...........................|....|...............................|....
.....F...........................F....F...............................F....
--...F...........................F....F...............................F....
 |...-FFF-FFF-FFF-FFF-FFF-FFF-FFF-....-FFF-FFF-FFF-FFF-FFF-FFF-FFF-FFF-...-
 --......................................................................--
  --...................................................................--- 
]]);

-- define areas
local pasture1 = selection.floodfill(08,00)
local pasture2 = selection.floodfill(50,10)
local horsepasture = selection.area(66,00,69,05)
local outside = selection.floodfill(02,04)
local inside = selection.floodfill(20,00) + selection.floodfill(22,02) + selection.floodfill(26,02)
               + selection.floodfill(35,00) + selection.floodfill(35,05) + selection.floodfill(42,00)
               + selection.floodfill(42,02) + selection.floodfill(50,00)

-- this level can appear in cocytus, or outside it
local cocytus = (nh.dnum_name(u.dnum) == 'Cocytus')

-- stairs
-- ugly hack here in which assumptions are made about the law demon branches:
-- if in cocytus we are not on the level which connects to gehennom, and if not
-- in cocytus we are on that level; place an upstair or branch accordingly
des.levregion({ region = {00,00,00,18}, region_islev=0, type=(cocytus and "stair-up" or "branch") })
des.levregion({ region = {74,00,74,10}, region_islev=0, type='stair-down' })

-- levelport or falling into the level
des.teleport_region({ region={00,00,04,19}, dir="up" })

-- non diggable everywhere
des.non_diggable()

-- set up the outside
if not cocytus then
   -- grass in pastures
   des.terrain(pasture1, 'g')
   des.terrain(pasture2, 'g')
   des.terrain(horsepasture, 'g')
   -- a few trees: geryon goes to great trouble to maintain them in gehennom
   des.terrain(13,11, 'T')
   des.terrain(25,11, 'T')
   des.terrain(43,11, 'T')
   des.terrain(54,11, 'T')
   des.terrain(65,11, 'T')
else
   des.replace_terrain({ selection=outside, fromterrain='.', toterrain='I', chance=50 })
   -- guarantee one space for upstair and downstair
   des.terrain(00,04, '.')
   des.terrain(74,05, '.')
end
for i = 1, 12 do
   des.trap({ coord=outside:rndcoord() })
end
for i = 1, 4 do
   des.monster({ class='&', coord=outside:rndcoord() })
end
for i = 1, 3 do
   des.monster({ coord=outside:rndcoord() })
end
for i = 1, 3 + d(3) do
   des.object({ coord=outside:rndcoord() })
end

-- pastures: Geryon's herds of cattle (and other quadrupeds)
for i = 1, 20 do
   if percent(35) then
      des.monster({ id='rothe', coord=pasture1:rndcoord(), peaceful=1 })
   else
      des.monster({ class='q', coord=pasture1:rndcoord(), peaceful=1 })
   end
   if percent(35) then
      des.monster({ id='rothe', coord=pasture2:rndcoord(), peaceful=1 })
   else
      des.monster({ class='q', coord=pasture2:rndcoord(), peaceful=1 })
   end
end
for i = 1, d(2) do
   des.monster({ class = '&', coord=pasture1:rndcoord() })
   des.monster({ class = '&', coord=pasture2:rndcoord() })
end
for i = 1, 2 do
   des.monster({ id='warhorse', coord=horsepasture:rndcoord() })
end
des.monster({ class = 'u', coord=horsepasture:rndcoord() })
for i = 1, 2 + d(2) do
   des.object({ coord=(pasture1 + pasture2):rndcoord() })
end

-- doors
des.door({ state='open', iron=0, x=35, y=04 })
des.door({ state='open', iron=0, x=36, y=04 })
des.door({ state='closed', iron=0, x=35, y=07 })
des.door({ state='closed', iron=0, x=36, y=07 })
des.door({ state='closed', iron=1, x=12, y=01 })
des.door({ state='open', iron=0, x=13, y=01 })
des.door({ state='open', iron=0, x=58, y=01 })
des.door({ state='closed', iron=1, x=59, y=01 })
des.door({ state='random', iron=0, x=28, y=01 })
des.door({ state='random', iron=0, x=43, y=01 })
des.door({ state='random', iron=0, x=31, y=02 })
des.door({ state='random', iron=0, x=40, y=02 })
des.door({ state='random', iron=0, x=20, y=04 })
des.door({ state='random', iron=0, x=24, y=03 })
des.door({ state='random', iron=0, x=48, y=02 })
des.door({ state='random', iron=0, x=51, y=04 })
des.door({ state='locked', iron=0, x=65, y=03 })

-- interior of the ranch house
des.object({ id='statue', x=32, y=00, material='gold', montype='rothe' })
des.object({ id='statue', x=39, y=00, material='gold', montype='rothe' })

-- throne room, where geryon will appear; the non geryon room should be
-- indistinguishable from it based on monster or object detection
local coordsets = {
   { thronereg={14,03,19,05}, wishchest={14,04}, regchest={57,04} },
   { thronereg={52,03,57,05}, wishchest={57,04}, regchest={14,04} }
}
local w = d(2)
des.region({ region=coordsets[w].thronereg, type='throne', filled=0, lit = 1 })
des.object({ id='chest', locked=true, coord=coordsets[w].wishchest, material='wooden',
             contents=function()
   if nh.is_wish_dlord('Geryon') then
      des.object({ class='/', id='wishing', spe=1 })
   else
      for i = 1, 1 + d(3) do
         des.object({ id='figurine', montype='rothe' })
      end
   end
end })
des.object({ id='chest', locked=true, coord=coordsets[w].regchest, material='wooden' })

-- remainder of the interior
for i = 1, 10 + d(5) do
   des.monster({ class='&', coord=inside:rndcoord() })
end
for i = 1, 3 do
   des.monster({ class='i', coord=inside:rndcoord() })
   des.monster({ coord=inside:rndcoord() })
end
for i = 1, 10 do
   des.object({ coord=inside:rndcoord() })
end
