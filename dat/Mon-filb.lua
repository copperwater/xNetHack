-- NetHack Monk Mon-filb.lua	$NHDT-Date: 1652196006 2022/05/10 15:20:06 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-2 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.

des.level_flags("mazelevel", "nommap-boss")

des.map([[
--------  --------                    --------       -------- --------      
|......----......|          -----------......------- |......| |......|      
|................|          |......................---.----.---......----   
|......----......|    -------......----......-----.......||.............|   
|......|  |......|    |............|  |......|   --......|-----......--.|   
-----.--  -----.--  ---.-----......|  -----.--    |......|    --------|.|   
    |.|       |.-----......|-----.--     --.----- |......|          ---.----
  ---.----    |............| ----.---    |......| ----.----------   |......|
  |......|    |.-----......| |......|    |......------.---......|   |......|
---......|    |.|   |......| |......------......................|   |......|
|........| ----.--- -------- |..................----.-----......|   |......|
|.|......| |......|          |......----------------.|   |......|   |......|
|.|------- |......---------- |.------        |.......|   ----.---   ---.----
|.------   |...............| |.| --------    |......--   ----.---     |.|   
|......|   |......---......---.| |......|    |......|    |......----- |.|   
|......-------.---- |..........---......|    |......|    |..........---.|   
|..............|    |......---..........--------.----    |......---.....|   
|......---------    -------- -----...............|       |......| -------   
--------                         -----------------       --------           
]]);

upstairloc = selection.area(01,01,06,04)
             + selection.area(03,08,08,11)
             + selection.area(01,14,06,17)

dnstairloc = selection.area(69,07,74,11)

des.stair({ coord=upstairloc:rndcoord(), dir="up" })
des.stair({ coord=dnstairloc:rndcoord(), dir="down" })

-- Non diggable walls
des.non_diggable(selection.area(00,00,75,20))

-- locked doors on the downstair room
des.door({ coord={71,06}, iron=true, state="locked" })
des.door({ coord={71,12}, iron=true, state="locked" })

-- Obstructions
local obs = { {09,02}, {15,05}, {15,10}, {20,07}, {01,13}, {14,15}, {05,06},
              {20,13}, {28,15}, {26,04}, {36,02}, {33,06}, {33,16}, {37,10},
              {40,10}, {43,17}, {52,10}, {50,09}, {54,07}, {56,09}, {43,05},
              {47,02}, {55,01}, {59,02}, {62,03}, {69,03}, {61,12}, {64,15} }

for i = 1,#obs do
   local x = obs[i][1]
   local y = obs[i][2]
   local pick = math.random(0,99)
   if pick < 15 then
      des.door({ coord=obs[i], state="secret" })
   elseif pick < 25 then
      des.door({ coord=obs[i], iron=true, locked=true })
   elseif pick < 35 then
      des.door({ coord=obs[i], trapped=true })
   elseif pick < 55 then
      for j = 1,1+d(3) do
         des.object("boulder", x, y)
      end
   elseif pick < 65 then
      des.monster({ id="earth elemental", coord=obs[i], asleep=true, peaceful=false })
   end
end

-- Traps
for i = 1,5 do
   des.trap('pit')
   des.trap('trap door')
   des.trap()
end

-- Monsters
for i = 1,3 do
   des.monster({ id="xorn", peaceful=0 })
end
for i = 1,2 do
   des.monster({ class="E", peaceful=0 })
   des.monster({ id="earth elemental", peaceful=0 })
end

-- Objects
for i = 1, 2 + d(2) do
  des.object()
end
for i = 1, 10 + d(10) do
   des.object('*')
end
des.object("+")
des.object("scroll of earth")
des.object("scroll of earth")
des.object("")
