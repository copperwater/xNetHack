-- Gehennom "fill" level template for when no specific level occupies the space
-- There are a few possible concepts for this, one is a simple cavern fill with
-- lava and lots of demons and traps, with demon dens burrowed into the walls;
-- another is to take a heck^2 approach and have a variety of specific
-- structures which can generate in the level, selected at random.

des.level_flags("mazelevel", "noflip");

-- Don't place any prefab over top of another feature. To do this, ensure that
-- rnd_halign will never return the same value twice:
used_haligns = {}
function rnd_halign()
   local aligns = { "half-left", "center", "half-right" };

   local all_used = true
   for i = 1, #aligns do
      if used_haligns[aligns[i]] == nil then
         all_used = false
         break
      end
   end
   if all_used then
      nh.impossible('All haligns are used! Defaulting to center...')
      return 'center'
   end

   repeat
      selected = aligns[math.random(1, #aligns)];
   until used_haligns[selected] == nil

   used_haligns[selected] = true
   return selected
end

function rnd_valign()
   local aligns = { "top", "center", "bottom" };
   return aligns[math.random(1, #aligns)];
end

-- sort of like themed rooms, but in hell!
-- for compatibility with vanilla practices (preventing the coordinate system
-- from going out of whack for things after the last defined map), all des.maps
-- here must have a contents function even if it's empty
-- all haligns should be rnd_halign()
local hell_prefabs = {
   -- spiral chamber
   function()
      des.map({ halign = rnd_halign(), valign = rnd_valign(), map=[[
xxx...........x
xx..---------..
...--.......--.
.---..-----..|.
.|...--...--.|.
.|.---.....|.|.
.|.|.......|.|.
.|.|.--...--.|.
.|.|..-----..|.
.|.--.......--.
.|..---------..
.--...........x
..------------x]], contents = function() end })
   end,
   -- large open area
   function ()
      des.map({ halign = rnd_halign(), valign = "center", map = [[
xxxxxx.....xxxxxx
xxxx.........xxxx
xx.............xx
xx.............xx
x...............x
x...............x
.................
........F........
.......FFF.......
........F........
.................
x...............x
x...............x
xx.............xx
xx.............xx
xxxx.........xxxx
xxxxxx.....xxxxxx
]], contents = function() end });
   end,
   function ()
      des.map({ halign = rnd_halign(), valign = rnd_valign(), map = [[
xxxxxx.xxxxxx
xLLLLLLLLLLLx
xL---------Lx
xL|.......|Lx
xL|.......|Lx
.L|.......|L.
xL|.......|Lx
xL|.......|Lx
xL---------Lx
xLLLLLLLLLLLx
xxxxxx.xxxxxx
]], contents = function()
   des.non_diggable(selection.area(2,2, 10,8));
   des.region(selection.area(4,4, 8,6), "lit");
   local dblocs = {
      { x = 1, y = 5, dir="east", state="closed" },
      { x = 11, y = 5, dir="west", state="closed" },
      { x = 6, y = 1, dir="south", state="closed" },
      { x = 6, y = 9, dir="north", state="closed" }
   }
   shuffle(dblocs);
   for i = 1, math.random(1, #dblocs) do
      des.drawbridge(dblocs[i]);
   end
   local mons = { "H", "T", "@" };
   shuffle(mons);
   for i = 1, 3 + math.random(1, 5) do
      des.monster(mons[1], 6, 5);
   end
      end });
   end,
   -- small circular area with demons
   function ()
      des.map({ halign = rnd_halign(), valign = rnd_valign(), lit = true, map = [[
x.....x
.......
.......
...FF..
.......
.......
x.....x]], contents = function()
         for i = 1, 9 do
            if percent(65) then
               des.monster('&')
            else
               des.monster('i')
            end
         end
      end  });
   end,
   -- temple/altar to moloch
   function ()
      des.map({ halign = rnd_halign(), valign = rnd_valign(), map = [[
BBBBBBB
B.....B
B.....B
B.....B
B.....B
B.....B
BBBBBBB]], contents = function()
         des.region({ region={2,2, 2,2}, type="temple", filled=1, irregular=1 });
         des.altar({ x=3, y=3, align="noalign", type=percent(75) and "altar" or "shrine" });
      end  });
   end,
   -- monster in a cage
   function ()
      des.map({ halign = rnd_halign(), valign = rnd_valign(), map = [[
xxxx..xxxx
xx......xx
x........x
...FFFF...
...F..F...
...F..F...
...FFFF...
x........x
xx......xx
xxxx..xxxx]], contents = function()
         local mons = { "Angel", "D", "H", "L" };
         des.monster(mons[math.random(1, #mons)], 4,4);
      end });
   end,
   -- fake fake wizard's tower
   function ()
      des.map({ halign = rnd_halign(), valign = rnd_valign(), map = [[
x.......x
..}}}}}..
.}}---}}.
.}--.--}.
.}|...|}.
.}--.--}.
.}}---}}.
..}}}}}..
x.......x
]], contents = function(rm)
         des.monster("L",04,04)
         des.object('"',04,04)
      end })
   end,
   -- lava pool
   function ()
      des.map({ halign = rnd_halign(), valign = rnd_valign(), map = [[
...........
...........
...........
...........
...........
...........
...........
...........
...........]], contents = function()
         local inrect = selection.area(01,01,08,07)
         des.replace_terrain({ selection=inrect, chance=50,
                               fromterrain='.', toterrain='L' })
         local fillins = selection.match('L.L') + selection.match('L\n.\nL')
         fillins = fillins & inrect
         des.terrain(fillins, 'L')
      end })
   end,
   -- lava/water river
   function ()
      des.map({ halign = rnd_halign(), valign = "center", map = [[
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx
xxxxxxxxxxxxxxx]], contents = function()
         for i = 1, d(3) do
            local river = selection.randline(07,00,07,20, 60):grow('east'):grow('west')
            des.terrain(river, percent(80) and 'L' or '}')
         end
      end })
   end,
   -- The Kraken
   function ()
      des.map({ halign = rnd_halign(), valign = "center", map = [[
xxxxxxxx}}xxxxxxxx
xxxxx.}}}}}}.xxxxx
xxxx..}}}}}}..xxxx
xxxx....}}....xxxx
xxxx...}}}}...xxxx
xxxx.}..}}..}.xxxx
xxxx..}}}}}}..xxxx
xxx....}}}}....xxx
xxx...}}}}}}...xxx
xx...}}}}}}}}...xx
x}..}}}}..}}}}..}x
}..}}}.}..}.}}}..}
}..}.}.}..}.}.}..}
}..}.}.}..}.}.}..}
}.}}.}.}..}.}.}}.}
x}}..}.}..}.}..}}x
x...}}.}..}.}}...x
}.}}}..}..}..}}}.}
}}}xxx}}xx}}xxx}}}]], contents = function()
         -- TODO: replace with lava with some chance, add likes_lava monsters if
         -- so, add krakens and other sea monsters and maybe some sunken
         -- treasure in the head (or treasure carried by monsters) if not
         if percent(40) then
            -- replace with lava
            des.replace_terrain({ selection=selection.area(00,00,19,18),
                                  fromterrain='}', toterrain='L' })
            for i = 1, 2 + d(3) do
               des.monster('salamander')
               des.monster('&')
            end
            for i = 1, d(4) do
               des.monster('fire elemental')
            end
         else
            -- add sea monsters and treasure
            for i = 1, 2 + d(3) do
               des.monster('kraken')
            end
            for i = 1, d(4) do
               des.monster('shark')
            end
            for i = 3, 3 + d(3) do
               des.monster('piranha')
            end
            local water = selection.area(00,00,19,18):filter_mapchar('}')
            des.object({ class = '=', coord = water:rndcoord() })
            des.object({ id = 'chest', coord = water:rndcoord() })
            for i = 1, 10 do
               des.object({ class = '*', coord = water:rndcoord() })
            end
         end
      end })
   end,
   -- pit fiend hall of pits
   function ()
      des.map({ halign = rnd_halign(), valign = rnd_valign(), map = [[
xxx|.--xxx
xx--..-.--
-.-......|
|.........
|........|
.........|
|......---
--......xx
x--....|xx
xx--.---xx]], contents = function()
         for i = 1, 8 + d(3) do
            des.trap('pit')
            if percent(70) then
               des.monster('pit fiend')
            end
         end
      end });
   end,
   -- from vanilla: several columns of water/lava flowing down into a pool
   function ()
      local mapstr = percent(30) and [[
.....
.LLL.
.LZL.
.LLL.
.....]] or [[
.....
.PPP.
.PWP.
.PPP.
.....]];
      des.map({ halign = rnd_halign(), valign = rnd_valign(),
                map = mapstr, contents = function() end })
   end,

   -- TODO: add some more
   -- lava lake (to be done after gradient behavior is inverted with vanilla
   -- merge, then look at implementing lava pool with smaller gradient)
}

-- Now start constructing the actual level.
-- smoothed=false: parts of the level could get very jagged.
-- joined=false: blobs of walkable space could get disconnected, and it may not
-- be possible to move between stairs without digging.
-- Both are intentional. The player can get stuck in an area they teleported or
-- fell into if they didn't bring a way to get out; but at this stage of the
-- game, having a way to get out is expected.
-- walled=false just simplifies constructing the level; we will call wallify later
des.level_init({ style="mines", fg=".", bg=" ", lit=0,
                 smoothed=false ,joined=false, walled=false })

-- lava features
if percent(20) then
   local transverse_river = selection.randline(00, 2 + d(15), 78, 2 + d(15), 40)
   for i = 1, 1 + d(2) do
      transverse_river = transverse_river:grow(percent(50) and 'north' or 'south')
   end
   des.terrain(transverse_river, 'L')
end

for i = 1, 3 + d(5) do
   local x = 4 + d(68)
   local y = 2 + d(16)
   local pool = selection.ellipse(x, y, 2 + d(2), 1 + d(2), 1)
   des.replace_terrain({ selection=pool, fromterrain='.', toterrain='L' })
end

local prefab1 = d(#hell_prefabs)
local prefab2 = d(#hell_prefabs)
local prefab3 = d(#hell_prefabs)

if percent(60) then
   hell_prefabs[prefab1]()
end
-- Don't allow multiple of the same prefab on the same level.
if percent(50) and prefab2 ~= prefab1 then
   hell_prefabs[prefab2]()
end
if percent(40) and prefab3 ~= prefab2 and prefab3 ~= prefab1 then
   hell_prefabs[prefab3]()
end

des.stair("up")
des.stair("down")
-- branch stair, if any, is added by default

-- default monsters
for i = 1, d(3) do
   des.monster('minotaur') -- not a maze, but still...
end
for i = 1, 6 do
   des.monster('&')
end
for i = 1, 3 + d(5) do
   des.monster('i')
end
for i = 1, nh.rn2(4) do
   des.monster('Z')
end
if percent(50) then
   des.monster('shade')
else
   des.monster('W')
end
if percent(30) then
   des.monster('L')
else
   des.monster('ghost')
end
for i = 1, 6 + d(6) do
   des.monster()
end

-- default traps
for i = 1, 6 do
   des.trap('fire')
end
for i = 1, 7 + d(6) do
   des.trap()
end

-- default objects
for i = 1, 4 + d(4) do
   des.object('boulder')
end
for i = 1, 7 + d(6) do
   des.gold()
end
for i = 1, 7 + d(5) do
   if percent(50) then
      des.object('*')
   else
      des.object()
   end
end

des.wallify()
