-- Minetown variant 8 (formerly 1)
-- "Frontier Town" - not yet pillaged by orcs

des.level_flags("mazelevel", "inaccessibles")

des.level_init({ style="mines", fg=".", bg=" ", smoothed=true, joined=true,lit=1,walled=true })

local gnomelord = percent(50)

des.map({ map=[[
x..............xxx.............xx
.................................
..------------.....------------..
..|...|...|..|.....|..|...|...|..
..|...|...|..|.....|..|...|...|..
..|...|...|..|.....|..|...|...|..
..|...|...|---.....------------..
..---------......................
..............{......{...........
..---------....------............
..|...|...|---.|....|.---------..
..|...|...|..|.|....|.|..|....|..
..|...|...|..|.|....|.|..|....|..
..|...|...|..|.|....|.|..|....|..
..------------.------.---------..
x................................
xx............................xxx]], x=25, y=2, contents=function()

   local townrect = selection.area(00,00,33,16)
   des.region(townrect,"lit")

   local anyshops = false -- have any non-guaranteed shops generated yet?

   -- temple (bottom center)
   des.region({ region={16,10,19,13}, lit=1, type="temple", x=16,y=10, w=4,h=4,
                contents = function()
                   des.door({ state="closed", wall="north" })
                   des.altar({ x=2, y=2, align=align[1],type="shrine" })
                   des.monster("gnomish wizard")
                end
   })

   -- top left block
   if percent(90) then
      des.region({ region={03,03,05,06}, type="shop", filled=1, contents=function()
         des.door({ state="closed", wall="south" })
      end })
      anyshops = true
   else
      des.terrain(selection.area(02,02,05,07), '.')
   end

   des.door({ x = 7 + nh.rn2(3), y = 7, state = "closed" })

   if percent(50) then
      anyshops = true
      des.region({ region={11,03,12,05}, type="food shop", filled=1, contents=function()
         des.door({ state="closed", wall="south" })
      end })
   else
      des.terrain(selection.area(11,02,13,06), '.')
   end

   -- bottom left block
   des.region({ region={03,10,05,13}, type="tool shop", filled=1, contents=function()
      des.door({ state="closed", wall="north" })
   end })

   des.door({ x = 07 + nh.rn2(3), y = 09, state = "closed" })

   des.door({ x = 13, y = 11 + nh.rn2(3), state = "locked" })
   des.monster("gnome", 11, 12)

   -- top right block
   des.door({ x = 19, y = 3 + nh.rn2(3), state = "locked" })
   des.monster("gnome", 21, 04)

   des.region({ region={23,03,25,05}, type="candle shop", filled=1, contents=function()
      des.door({ state="closed", wall="south" })
   end })

   des.door({ x = 27 + nh.rn2(3), y = 06, state = "closed" })
   if gnomelord then
      des.monster("gnome lord", 28, 04)
   end

   -- bottom right block
   if true then --not anyshops or percent(20) then
      des.region({ region={23,11,24,13}, type="potion shop", filled=1, contents=function()
         des.door({ state="closed", wall="north" })
      end })
   else
      des.door({ x = 22, y = 11 + nh.rn2(3), state = "locked" })
   end

   des.door({ x = 26 + nh.rn2(4), y = 10, state = "closed" })
   des.monster("gnome", 27, 12)

   -- in the town in general
   -- since some rooms may not exist, we can't do this floodfill until after
   -- deciding whether or not to destroy them
   local intown = selection.floodfill(1,1) & townrect

   des.monster({ id = "watchman", peaceful = 1, coord = intown:rndcoord(1) })
   des.monster({ id = "watchman", peaceful = 1, coord = intown:rndcoord(1) })
   des.monster({ id = "watch captain", peaceful = 1, coord = intown:rndcoord(1) })

   -- inside dwellings
   local nonshop_houses = selection.area(08,03,10,06) + selection.area(08,10,10,13)
                          + selection.area(12,11,13,13) + selection.area(21,03,22,05)
                          + selection.area(28,03,30,05) + selection.area(27,11,30,13)

   if percent(50) then
      des.object({ id = 'touchstone', buc = 'cursed', coord = nonshop_houses:rndcoord() })
   else
      des.object({ id = 'oil lamp', coord = nonshop_houses:rndcoord() })
   end

end });

-- monsters which could be anywhere on the level
des.monster('dwarf')
des.monster('dwarf')
des.monster('dwarf')
if not gnomelord then
   des.monster('dwarf lord')
end

-- stairs
des.levregion({ type="stair-up", region={01,03,20,19}, region_islev=1, exclude={0,0,39,15} })
des.levregion({ type="stair-down", region={61,03,75,19}, region_islev=1, exclude={0,0,39,15} })

des.wallify()
des.mineralize()
