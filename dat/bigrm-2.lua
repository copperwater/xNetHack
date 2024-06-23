-- NetHack bigroom bigrm-2.lua	$NHDT-Date: 1652196021 2022/05/10 15:20:21 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1990 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel", "noflip");

des.map([[
---------------------------------------------------------------------------
|.........................................................................|
|.........................................................................|
|.........................................................................|
|.........................................................................|
|.........................................................................|
|.........................................................................|
|.........................................................................|
|.........................................................................|
|.........................................................................|
|.........................................................................|
|.........................................................................|
|.........................................................................|
|.........................................................................|
|.........................................................................|
|.........................................................................|
|.........................................................................|
---------------------------------------------------------------------------
]]);
-- Dungeon Description
des.region(selection.area(01,01,73,16),"lit");

local darkness;

local choice = math.random(0, 3)
if choice == 0 then
   darkness = selection.area(01,07,22,09)
      | selection.area(24,01,50,05)
      | selection.area(24,11,50,16)
      | selection.area(52,07,73,09);
elseif choice == 1 then
   darkness = selection.area(24,01,50,16);
elseif choice == 2 then
   darkness = selection.area(01,01,22,16)
      | selection.area(52,01,73,16);
elseif choice == 3 then
   -- random "spotlights"
   MIN_RADIUS = 2
   MAX_RADIUS = 4
   darkness = selection.area(01,01,73,16)
   centers = {}
   tries = 0
   for i = 5, 10 + d(6) do
      closeby = false
      radius = math.random(MIN_RADIUS,MAX_RADIUS)

      -- try to find a center for the new spotlight that won't touch already
      -- existing ones
      repeat
         cx = 1 + nh.rn2(73)
         cy = 1 + nh.rn2(16)
         closeby = false
         for j = 1, #centers do
            dx = centers[j].x - cx
            dy = centers[j].y - cy
            -- +1 to buffer will allow spotlights to touch but not overlap, +2
            -- ensures they won't touch at all
            buffer = (centers[j].r + radius) + 2
            if (dx * dx) + (dy * dy) <= buffer * buffer then
               closeby = true
            end
         end
         -- prevent trying forever to find a spot that doesn't exist
         tries = tries + 1
      until closeby == false or tries > 100

      -- and also break out of containing loop rather than adding another circle
      if tries > 100 then
         break
      end

      table.insert(centers, {x = cx, y = cy, r = radius})
      darkness = darkness - selection.circle(cx, cy, radius, 1)

   end
   -- don't light up the walls around the spotlights
   darkness = darkness + selection.rect(00,00,74,17)
end

if darkness ~= nil then
   des.region(darkness,"unlit");
   if percent(25) then
      des.replace_terrain({ selection = darkness:grow(),
                            fromterrain = ".", toterrain = "I" });
   end
end

-- Stairs
des.stair("up");
des.stair("down");
-- Non diggable walls
des.non_diggable();
-- Objects
for i = 1,15 do
   des.object();
end
-- Random traps
for i = 1,6 do
   des.trap();
end
-- Random monsters.
for i = 1,28 do
   des.monster();
end
