-- Stygian Marsh

des.level_init({ style = "swamp", lit=0 })
des.level_flags("mazelevel", "shortsighted", "hardfloor", "noflipx")

-- Left side of the map is 2 fixed and 3 interchangeable pieces of "cavern
-- entrance"
des.map({ x=01, y=00, map=[[
 ----xxxxx
 |..----xx
 --....--x
 --..---xx
 |...|xxxx
]] })
des.map({ x=01, y=19, map=[[
 |....|xxx
 --...---x
  --....|x
   ------x
]] })

local othermap_y = { 05, 09, 13 }
shuffle(othermap_y)
des.map({ x=01, y=othermap_y[1], map=[[
--....|xxx
|..-..--.x
|..--..B..
--....--.x
]] })
des.map({ x=01, y=othermap_y[2], map=[[
 --...|xxx
  --..----
   |.....|
 ---..----
]] })
des.map({ x=01, y=othermap_y[3], map=[[
--...--xxx
.....|xxxx
--..--xxxx
 |...--xxx
]], contents=function()
   des.stair('up', 00, 01)
end })

-- Add some grass to the map.
des.reset_coordinate_system() -- clear the relative coords from the last map
local everything = selection.area(09,00,77,20)
des.replace_terrain({ selection=everything, fromterrain='.', toterrain='g', chance=40 })

-- Now add in random ruins akin to medusa-1 in aesthetic
-- note that no ruin can be wider than 11 (if it chooses x=67 to generate) or
-- taller than 6 (if it chooses y=15 to generate)
randmapcoords = { {12,03}, {16,13}, {34,07}, {30,15}, {39,02}, {45,11}, {56,14},
                  {59,03}, {67,10} }
shuffle(randmapcoords)

des.map({ coord=randmapcoords[1], map=[[
xx|.x--.xxx
..|..|....x
x-----.---.
xx.....|.xx
]], contents=function()
   des.object('statue')
end })
des.map({ coord=randmapcoords[2], map=[[
x..xxx
..|..|
x.----
x..|.x
----..
x....x
]] })
des.map({ coord=randmapcoords[3], map=[[
xxxxxx...xx
..-----....
x.....----.
xxxxxx...xx
]] })
des.map({ coord=randmapcoords[4], map=[[
x}|}}|}x
}------}
x}|..|}x
x}|..|}x
}---x|-}
x}|xx|}x
]], contents = function()
   des.monster({ x=04, y=02 })
end })
-- and one non-ruin outpost
des.map({ coord=randmapcoords[5], map=[[
--x-F-x-F-x
|---.---.--
+.........F
|---.---.--
--x-F-x-F-x
]], contents = function()
   des.door("open", 00,02)
   des.monster('&', 02,02)
   des.monster('&', 04,02)
   des.monster('minotaur', 06,02)
   des.monster('&', 08,02)
end})

-- clear relative coords again
des.reset_coordinate_system()

-- downstairs is on right edge, avoiding the corners
des.levregion({ region={78,04,78,16}, region_islev=1, type="stair-down" })

-- make a (very) few trees
local allgrass = selection.area(09,00,78,20):filter_mapchar('g')
for i = 1, 2 + d(2) do
   des.terrain({ coord=allgrass:rndcoord(), typ='T' })
end

-- branch to Shedaklah
-- this must come AFTER the grass/trees, because otherwise the grass may replace
-- the point for the stairways and then be replaced by a tree, while the staircase
-- doesn't try to generate until the very end, and will not be able to replace a
-- tree.
local branchx = math.random(44,58)
des.map({ x=branchx, y=00, map=[[
.....
x...x
]], contents=function()
   des.levregion({ region={02,00,02,00}, region_islev=0, type='branch' })
end })

-- clear relative coords AGAIN
des.reset_coordinate_system()

-- Randomly placed monsters
for i = 1, 5 + d(3) do
   des.monster('F')
   des.monster(';')
   des.monster('&')
end
for i = 1, 2 do
   des.monster('j')
   des.monster('P')
end
for i = 1, 3 do
   des.monster('piranha')
   des.monster('water demon')
end

-- Random traps, with a bias towards gas and rust traps
for i = 1, 6 do
   des.trap('sleep gas')
   des.trap('rust')
   des.trap('fire') -- because this is still Gehennom...
end
for i = 1, 8 do
   des.trap()
end

-- Corpses... very few on the surface, more underwater.
-- They never rot. Some have treasure.
local walkable = everything:filter_mapchar('.') + everything:filter_mapchar('g')
local water = everything:filter_mapchar('}')
-- uneven distribution among zombifiable monsters
local corpsetypes = { 'human', 'human', 'human', 'human', 'giant', 'giant', 'giant',
                      'ettin', 'orc', 'elf', 'elf', 'gnome', 'dwarf' }
for i = 1, 80 do
   local crd
   if percent(99) then
      crd = water:rndcoord()
   else
      crd = walkable:rndcoord()
      if percent(4) then
         des.object({ class='"', buc="cursed", coord=crd })
      end
      if percent(4) then
         des.object({ class='=', buc="cursed", coord=crd })
      end
      if percent(4) then
         des.object({ class='(', buc="cursed", coord=crd })
      end
      if percent(4) then
         des.object({ class='/', buc="cursed", coord=crd })
      end
      if percent(4) then
         des.object({ class=')', buc="cursed", spe=(-8 + nh.rn2(17)), coord=crd })
      end
      if percent(4) then
         des.object({ class='[', buc="cursed", spe=(-8 + nh.rn2(17)), coord=crd })
      end
      if percent(4) then
         des.gold({ coord=crd })
      end
   end
   local corpse = des.object({ coord=crd, id='corpse', montype=corpsetypes[d(#corpsetypes)] })
   corpse:stop_timer('rot-corpse')
end

-- Candles to light up a few parts of the swamp
for i = 1, 1 + d(7) do
   des.object({ id = 'tallow candle', lit = 1 })
end

des.non_diggable()

