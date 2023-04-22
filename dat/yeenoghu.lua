-- Yeenoghu's lair

des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel", "noteleport", "nommap-boss");

des.message("You come to a dark, stinking wasteland.")

des.map([[
}}}}}}}}}}}}}}}}}....----.......}}}}}}}}}}}}}}}}}}}}}}}....|...............
---}}}}}}}}}}}.......|..............}}}}}}}}}}}}}}}}}}}.............---....
..}}}}}}}}|}.........|..............}}}}}}}}}}}}----}...............|......
.....}}}..|............................}}}}}}}}}|}}}}.............---......
....}}}}.--...............................}}}-----.........................
....}}}}}|.......................---..........}}}........|.................
.......}}..............................................---...............--
.....................|..................................|..................
...................----.................................|..................
....................|...................................|..................
....---..................}}}}}}.................|.............|............
......|..................}}}}}}}}}}.............|.............|............
......................}}}}}}}}}}}|}.............|.............---..........
..................}}}}}}}}}}}}}}}|}}..........-------...........|..........
..................}}}}}}}}}}------}........................................
........-------..}}}}...}}}}}}}}}}}....................................---.
..........|.............}}}}}}}}...........................--..............
..........|.................}}............|................................
]]);


-- Randomly placed iron bars
des.replace_terrain({ fromterrain='.', toterrain='F', chance=3 })

-- Define areas
local everything = selection.area(00,00,74,17)
local allfloor = everything:filter_mapchar('.')
local yeen_area = selection.circle(67,09, 5, 0)

-- Teleport region to constrain arrival to near the stairs
des.teleport_region({ region={00,00,05,17} })

-- This branch is the bottom of either the Abyss or Shedaklah, so no downstairs
des.levregion({ region={00,00,00,17}, type="stair-up" });

local Yx = 63 + nh.rn2(8)
local Yy = 6 + nh.rn2(6)
des.monster({ id='Yeenoghu', coord={Yx,Yy}, waiting=1, inventory=function()
   des.object({ id='flail', spe=9, name='Butcher', material='iron', eroded=1 })
   -- the sack is necessary to prevent players from using object detection to
   -- figure out if he has a wand or not
   des.object({ id='sack', contents=function()
      if nh.is_wish_dlord('Yeenoghu') then
         nh.pline('got a wish')
         des.object({ class='/', id='wishing', spe=1 })
      else
         des.object({ class='/', id='magic missile' })
      end
   end })
   -- he is specially coded not to flee or retreat so compensate for this
   des.object({ class='!', id='full healing', quan=2 })
end })

-- Corpses... a lot of corpses
for i = 1, 100 do
   des.object('corpse')
end

-- Ghouls (for the corpses)
for i = 1, 10 + d(5) do
   des.monster('ghoul')
end

-- High level d and f beasts
for i = 1, 3 do
   des.monster({ class='d', peaceful=false })
   des.monster('hell hound')
   des.monster({ class='f', peaceful=false })
end

-- Major and minor demons
for i = 1, 3 + d(3) do
   des.monster('i')
end
for i = 1, 10 do
   des.monster({ class='&', align=(percent(60) and 'chaos' or 'random'), coord=yeen_area:rndcoord() })
end
for i = 1, d(6) do
   des.monster({ class='&', align=(percent(60) and 'chaos' or 'random') })
end

-- some random objects
for i = 1, 5 do
   des.object()
end

-- Traps
for i = 1, 30 do
   des.trap()
end
