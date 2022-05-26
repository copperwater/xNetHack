-- NetHack 3.7	mines minetn-1.lua	$NHDT-Date: 1652196030 2022/05/10 15:20:30 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.8 $
--	Copyright (c) 1989-95 by Jean-Christophe Collet
--	Copyright (c) 1991-95 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
-- A tragic accident has occurred in Frontier Town....
--
-- Minetown variant 1
-- Orcish Town - a variant of Frontier Town that has been
-- overrun by orcs.  Note the barricades (iron bars).

des.level_flags("inaccessibles")

des.level_init({ style="mines", fg=".", bg=" ", smoothed=true, joined=true, walled=true })

des.map([[
.....................................
.----------------F------------------.
.|.................................|.
.|.-------------......------------.|.
.|.|...|...|...|......|..|...|...|.|.
.F.|...|...|...|......|..|...|...|.|.
.|.|...|...|...|......|..|...|...|.F.
.|.|...|...|----......------------.|.
.|.---------.......................|.
.|.................................|.
.|.---------.....--...--...........|.
.|.|...|...|----.|.....|.---------.|.
.|.|...|...|...|.|.....|.|..|....|.|.
.|.|...|...|...|.|.....|.|..|....|.|.
.|.|...|...|...|.|.....|.|..|....|.|.
.|.-------------.-------.---------.|.
.|.................................F.
.-----------F------------F----------.
.....................................
]]);

-- Don't let the player fall into his likely death; used to explicitly exclude
-- the town, but that meant that you couldn't teleport out as well as not in.
des.teleport_region({ region={01,01,20,19}, region_islev=1 })
des.region(selection.area(01,01,35,17), "lit")
des.levregion({ type="stair-up", region={01,03,20,19}, region_islev=1,
		exclude={00,01,36,17} });
des.levregion({ type="stair-down", region={61,03,75,19}, region_islev=1,
		exclude={00,01,36,17} })

-- Define areas of the map:
-- inside the walls
local inside = selection.floodfill(18,8)
-- near the temple
local near_temple = selection.area(17,8, 23,14) & inside

-- Izchak managed to barricade his shop with boulders, and he lives!
des.region({ region={26,4,28,6}, lit=1, type="candle shop", filled=1 })
des.door("locked", 27, 7)
des.object("boulder", 26, 8)
des.object("boulder", 27, 8)
des.object("boulder", 28, 8)

-- shame we can't make polluted fountains
des.feature("fountain",16,09)
des.feature("fountain",25,09)

-- the altar's defiled; useful for BUC but never coaligned
des.altar({ x=20,y=13,align="noalign", type="shrine" })

-- set up the shop doors; could be broken down
des.door("random",5,8)
des.door("random",9,8)
des.door("random",13,7)
des.door("random",22,5)
des.door("random",31,7)
des.door("random",5,10)
des.door("random",9,10)
des.door("random",15,13)
des.door("random",25,13)
des.door("random",31,11)

-- knock a few holes in the shop interior walls
des.replace_terrain({ region={07,04,11,06}, fromterrain="|", toterrain=".", chance=18 })
des.replace_terrain({ region={07,12,11,14}, fromterrain="|", toterrain=".", chance=18 })
des.replace_terrain({ region={28,12,28,14}, fromterrain="|", toterrain=".", chance=33 })

-- Rubble!
for i=1,9 + d(2,5) do
  if percent(90) then
    des.object("boulder")
  end
  des.object("rock")
end

-- Shopkeeper "bones" piles. One spot each in most shops...
-- there were 4 shops in Frontier Town; Izchak is still present
-- so there should only be three dead shks here
local shk_demise = { {05,07},{09,05},{13,04},{31,05},{30,14},{05,14},{10,13},{26,14},{27,13} }
shuffle(shk_demise)
des.monster({ id = "shopkeeper", coord = shk_demise[1], dead = 1 })
des.monster({ id = "shopkeeper", coord = shk_demise[2], dead = 1 })
des.monster({ id = "shopkeeper", coord = shk_demise[3], dead = 1 })
-- Orcish Town is hard enough without the orcs picking up and using the
-- shopkeepers' wands and potions.
des.object({ id = "boulder", coord = shk_demise[1] })
des.object({ id = "boulder", coord = shk_demise[2] })
des.object({ id = "boulder", coord = shk_demise[3] })

-- shop inventories... 9 items from a tool shop, 4 from a delicatessen
-- Ideally we could just call some routine to fill the shop objects with the
-- appropriate probabilities for their shops and not put in the shopkeepers,
-- but that can't be made to work without a refactor of room-filling semantics
-- (i.e. separating the notion of "filled" into "filled with monsters" and
-- "filled with objects")
for i=1,9 do
  des.object("(", inside:rndcoord(1))
end
-- For now, only FOOD_CLASS items from deli.
for i=1,4 do
  des.object("%", inside:rndcoord(1))
end

-- priest spoils
-- have to manually provide the spellbooks because those are given to a priest
-- when initialized as a shrine attendant - which this priest is not.
des.monster({ id = "aligned cleric", x = 20, y = 12, dead = 1 })
for i = 1, 1 + math.random(1,3) do
  des.object({ class = "+", x = 20, y = 12 })
end

-- the remains of the Watch
for i = 1,4 do
  des.monster({ id = "watchman", dead = 1 })
end
des.monster({ id = "watch captain", dead = 1 })

-- the Orcish Army

for i=1,5 + d(10) do
   if percent(50) then
      des.monster({ id = "orc-captain", coord = inside:rndcoord(1), peaceful=0 });
   else
      if percent(80) then
         des.monster({ id = "Uruk-hai", coord = inside:rndcoord(1), peaceful=0 })
      else
         des.monster({ id = "Mordor orc", coord = inside:rndcoord(1), peaceful=0 })
      end
   end
end
-- shamans can be hanging out in/near the temple
for i=1,d(2,3) do
   des.monster({ id = "orc shaman", coord = near_temple:rndcoord(0), peaceful=0 });
end
-- these are not such a big deal to run into outside the bars
for i=1,9 + d(2,5) do
   if percent(90) then
      des.monster({ id = "hill orc", peaceful = 0 })
   else
      des.monster({ id = "goblin", peaceful = 0 })
   end
end

-- Hack to force full-level wallification

des.wallify()
