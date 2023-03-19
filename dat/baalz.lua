-- NetHack gehennom baalz.lua	$NHDT-Date: 1652196020 2022/05/10 15:20:20 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.4 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "nommap-boss", "noteleport")

--        1         2         3         4         5         6         7
--2345678901234567890123456789012345678901234567890123456789012345678901234567890
des.map([[
    --.............--                   -----------------------            
    |..---.---.---..|             -------.....................--------     
   --....--- ---....--        -----..................................------
   |.......| |.......|     ----...............---------....................
-- --..----| |----..--   ---.....--------.....|..................--------..
.-- |......| |......|   --..............|.....|.....-----------..|.........
..----...-------...-- ---...------......|..---------|.........|..|.........
...--|.---..|..---.|---.....F....|..-------|...........--------------......
....--.....F-F.....--.....---....|--|..................S............|----..
..........................+...--....S..----------------|............S...|.>
....--.....F-F.....--.....---....|--|..................|............|----..
...--|.---..|..---.|---.....F....|..-------|...........-----S--------......
..----...-------...-- ---...------......|..---------|.........|..|.........
.-- |......| |......|   --..............|.....|.....-----------..|.........
-- --..----| |----..--   ---.....--------.....|..................--------..
   |.......| |.......|     ----...............---------....................
   --....--- ---....--        -----..................................------
    |..---.---.---..|             -------.....................--------     
    --.............--                   -----------------------            
]]);
-- the iron bars are eyes

-- define areas
local outside = selection.floodfill(00,09)
local head = selection.floodfill(29,09)
local thorax = selection.floodfill(38,09)
local abdomen = selection.area(56,08,67,10) -- doesn't include treasure chamber
local smallbugzone = selection.area(03,00,19,19):filter_mapchar('.')

function rndbugclass()
   return percent(25) and 's' or 'a'
end

-- this level can appear in cocytus, or outside it
local cocytus = (nh.dnum_name(u.dnum) == 'Cocytus')

-- stairs
des.levregion({ region = {00,05,00,13}, region_islev=0, type="stair-up" })
des.stair('down', 74,09) -- does nothing if this is the bottom level of the branch

-- levelport or falling into the level
des.teleport_region({ region={00,00,03,19}, dir="up" })

-- non-diggable everywhere
des.non_diggable()

-- set up the outdoor area
if cocytus then
   des.replace_terrain({ selection=outside, fromterrain='.', toterrain='I', chance=50 })
   -- guarantee one space for upstair
   des.terrain(00,06, '.')
end
local traps = { 'spiked pit', 'polymorph', 'magic' }
shuffle(traps)
des.trap(traps[1], 11,09)
des.trap(traps[2], 12,09)
des.trap(traps[3], 13,09)
local tmpbugzone = smallbugzone:clone()
for i = 1, 30 do
   if percent(15) then
      des.trap({ coord=tmpbugzone:rndcoord(1) })
   else
      des.monster({ class=rndbugclass(), coord=tmpbugzone:rndcoord(1) })
   end
end
-- now the rest of outside
local icymons = get_icymon_list()
for i = 1, 15 do
   des.trap({ coord = outside:rndcoord() })
   if percent(30) then
      des.monster({ class = '&', coord = outside:rndcoord() })
   elseif percent(20) then
      des.monster({ class = 'i', coord = outside:rndcoord() })
   elseif percent(30) then
      des.monster({ class = rndbugclass(), coord = outside:rndcoord() })
   elseif cocytus then
      des.monster({ id = icymons[d(#icymons)], coord = outside:rndcoord() })
   else
      des.monster({ coord = outside:rndcoord() })
   end
end

-- inside: head
des.door({ state="locked", iron=1, x=26, y=09 })
for i = 1, 2 do
   des.monster({ id='barbed devil', coord=head:rndcoord() })
   des.monster({ class = rndbugclass(), coord=head:rndcoord() })
   des.trap({ coord = head:rndcoord() })
end
des.monster({ class='i', coord=head:rndcoord() })

-- inside: thorax
for i = 1, 3 do
   des.monster({ class = '&', coord = thorax:rndcoord() })
   des.trap({ coord = thorax:rndcoord() })
   des.object()
end
des.monster({ class='i', coord=thorax:rndcoord() })
des.monster({ class='V', coord=thorax:rndcoord() })

-- inside: abdomen
-- Unfilled throne room so Baalz can spawn when hero enters
des.region({ region={56,08,67,10}, type='throne', filled=0, lit = 1 })
des.monster({ id='horned devil', coord=abdomen:rndcoord() })
des.monster({ id='barbed devil', coord=abdomen:rndcoord() })
des.monster({ class='V', coord=abdomen:rndcoord() })
for i = 1, 3 do
   des.trap({ coord = abdomen:rndcoord() })
   des.object()
end

-- the treasure
des.door({ state='secret', locked=1, iron=1, coord = {68,09} })
des.object({ id='chest', locked=1, coord={71,09}, material='iron', contents=function()
   if nh.is_wish_dlord('Baalzebub') then
      des.object({ class='/', id='wishing', spe=1 })
   else
      for i = 1, 3 do
         des.object({ class='?', id='create monster' })
      end
   end
end })

-- level wide stuff
for i = 1, 2 do
   des.object("[")
   des.object(")")
   des.object("*")
   des.object("!")
   des.object("?")
   des.monster("ghost")
end
des.monster("L")

for i = 1, 8 do
   des.trap(cocytus and 'cold' or 'fire')
end
