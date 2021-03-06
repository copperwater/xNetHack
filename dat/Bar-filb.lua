-- NetHack 3.7	Barb.des	$NHDT-Date: 1432512784 2015/05/25 00:13:04 $  $NHDT-Branch: master $:$NHDT-Revision: 1.9 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.

des.level_flags("mazelevel", "outdoors", "hardfloor", "inaccessibles", "noflip");

-- TODO: Why does mines-style open up x=1 and x=79 whereas solidfill doesn't?
-- And then, specifying e.g. x=0 in commands in this file actually comes out to x=1 in game...
des.level_init({ style = "mines", fg = ".", bg=".", lit=0, walled=false });

-- let's make some mesa-like rock promontories
local mesa_centers = selection.fillrect(08,00,72,20)
for i=1, 12+d(4) do
   local cx, cy = mesa_centers:rndcoord()
   local rock = selection.gradient({ type="radial", mindist=0, maxdist=3, limited=true, x=cx, y=cy })
   -- gradients are non-invertible so have to do some selection magic to flip it
   rock = rock:negate()
   rock = rock & selection.circle(cx, cy, 3);

   if percent(50) then
      rock = rock:grow("north")
   else
      rock = rock:grow("east")
   end
   rock = rock:grow()
   des.terrain({ selection = rock, typ = ' ' })
end

-- guarantee a way across the level
local leftstair = { selection.line(0,0, 0,20):rndcoord() }
local rightstair = { selection.line(78,1, 78,20):rndcoord() }
des.stair({ dir = "up", coord = leftstair })
des.stair({ dir = "down", coord = rightstair })

local lstairy = leftstair[2]
local rstairy = rightstair[2]
local path = selection.randline(0, lstairy, 78, rstairy, 10)
des.terrain({ selection = path:grow("north"):grow("south"), typ = ".", lit = 0 })

for i=1,11 do
  des.object()
end
for i=1,4 do
  des.trap()
end
for i=1,7 do
  des.monster({ id="ogre", peaceful=0 })
end
for i=1,7 do
  des.monster({ id="rock troll", peaceful=0 })
end
des.monster({ class="O", peaceful=0 })
des.monster({ class="T", peaceful=0 })
des.monster({ class="o", peaceful=0 })

-- wallify the rock promontories
-- we don't want to wallify the stone left and right map edges, so need to limit the range
des.wallify()
