-- Cocytus fill -- icy, cold branch of Gehennom where Asmodeus lives. However,
-- this is just the filler level.
-- This will be a cold-themed branch, like Sheol (in fact, this was very close
-- to being named Sheol, but then I couldn't find any references to it being a
-- specifically cold place in mythology other than the person who first created
-- the Sheol branch in UnNetHack.) The main reason it is associated with cold is
-- because Asmodeus is at the bottom of it.

-- Most of this level is like hellfill, but without prefabs (there will only be
-- 1 of these filler levels in the game, so not much point) and with some more
-- ice themes.
des.level_init({ style="mines", fg=".", bg=" ", lit=0,
                 smoothed=false ,joined=false, walled=false })

des.replace_terrain({ fromterrain='.', toterrain='I', chance=50 })

des.stair("up");
des.stair("down");
-- branch stair, if any, is added by default

-- Monsters:
for i = 1, 6 + d(3) do
   if percent(60) then
      des.monster('ice devil')
   else
      des.monster('&')
   end
end

for i = 1, 3 do
   des.monster('i')
end

if percent(50) then
   des.monster('shade')
else
   des.monster('W')
end

if percent(40) then
   des.monster('L')
else
   des.monster('ghost')
end

local icymons = get_icymon_list()
for i = 1, 4 + d(3) do
   des.monster(icymons[d(#icymons)])
end

-- Traps:
for i = 1, 20 do
   des.trap('cold')
end
for i = 1, 8 do
   des.trap()
end

-- Objects:
for i = 1, 5 + d(2) do
   des.object('boulder')
end
for i = 1, 7 + d(6) do
   des.gold()
end
for i = 1, 7 + d(5) do
   if percent(50) then
      des.object('sapphire')
   else
      des.object()
   end
end

des.wallify();
