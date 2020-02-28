-- NetHack 3.6	Barb.des	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: master $:$NHDT-Revision: 1.9 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.

des.level_flags("mazelevel", "outdoors", "hardfloor", "inaccessibles", "noflip");

-- unlit, unlike the upper filler
des.level_init({ style = "solidfill", fg = ".", lit=0 });

-- let's make some mesa-like rock promontories
-- TODO: gradients not implemented yet
--local mesa_centers = selection.fillrect(08,00,72,20)
--LOOP [12 + 1d4] {
--  -- gradients are non-invertible so have to do some selection magic to flip it
--  $center = rndcoord($mesa_centers)
--  $rock = selection: complement gradient(radial, (0 , 3, limited), $center)
--  $overlap = selection: circle($center,3,unfilled)
--  IF [50%] {
--      $rock = selection: grow(north,filter($rock, $overlap))
--  }
--  ELSE {
--      $rock = selection: grow(east,filter($rock, $overlap))
--  }
--  $rock = selection: grow($rock)
--  TERRAIN:$rock, ' '
--}

-- guarantee a way across the level
local leftstair = { selection.line(1,1, 1,20):rndcoord() }
local rightstair = { selection.line(77,1, 77,20):rndcoord() }
local lstairy = leftstair[2]
local rstairy = rightstair[2]

local path = selection.randline(1, lstairy, 77, rstairy, 10)
des.terrain(path:grow("north"):grow("south"), ".")

des.stair({ dir = "up", coord = leftstair })
des.stair({ dir = "down", coord = rightstair })

for i=1,11 do
  des.object()
end
for i=1,4 do
  des.trap()
end
for i=1,7 do
  des.monster("ogre")
end
for i=1,4 do
  des.monster("rock troll")
end
des.monster({ class="O" })
des.monster({ class="T" })
des.monster({ class="o" })

-- wallify the rock promontories
-- TODO: when there actually are rock promontories...
-- TODO: this may wallify the left and right edges, consider using the x,y range form
-- des.wallify()
