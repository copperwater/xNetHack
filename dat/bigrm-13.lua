-- "The Great Bridge" bigroom

des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel", "noflipx");

-- Originally the river was narrower and just passed under the center of the
-- bridge with space to either side, but this has the unwanted effect of
-- monsters on the right side trying to beeline to the player and getting stuck.
-- So instead, it's shaped to help funnel them towards the bridge.
-- 
-- If better AI is introduced at some point so that monsters will intelligently
-- pathfind across the bridge, you can probably change it back.
des.map([[
--------------------------------}}}}}}}}}}}}}}}}}}}------------------------
|...............................}}}}}}}}}}}}}}}}}}}.......................|
|..............................}}}}}}}}}}}}}}}}}}}........................|
|.............................}}}}}}}}}}}}}}}}}}}}........................|
|.............................}}}}}}}}}}}}}}}}}}}.........................|
|............................}}}}}}}}}}}}}}}}}}}}.........................|
|.........................----}}}}}}}}}}}}}}}----.........................|
|............................-----------------............................|
|.........................................................................|
|.........................................................................|
|............................-----------------............................|
|.........................----}}}}}}}}}}}}}}}----.........................|
|............................}}}}}}}}}}}}}}}}}}}}.........................|
|............................}}}}}}}}}}}}}}}}}}}}}........................|
|...........................}}}}}}}}}}}}}}}}}}}}}}........................|
|...........................}}}}}}}}}}}}}}}}}}}}}}}.......................|
|...........................}}}}}}}}}}}}}}}}}}}}}}}}......................|
----------------------------}}}}}}}}}}}}}}}}}}}}}}}}-----------------------
]]);

des.region(selection.area(01,01, 73, 16), "lit");
des.non_diggable();
des.teleport_region({ region = {01,01,25,16}, dir="down" })

-- are you playing Horatius, or Gandalf?
if percent(10) then
   des.replace_terrain({ region={00,00,74,17}, fromterrain='}', toterrain='A' })
end

leftside = selection.area(01,01,25,16)
rightside = selection.area(49,01,73,16):filter_mapchar('.')

des.stair({ dir='up', coord=leftside:rndcoord() });
des.stair({ dir='down', coord=rightside:rndcoord() });

for i = 1,8 do
   des.trap();
end

-- there's fewer things on the left side
leftobjs = nh.rn2(6)
for i = 1, leftobjs do
   des.object({ coord = leftside:rndcoord() });
end
for i = 1, 15 - leftobjs do
   des.object({ coord = rightside:rndcoord() });
end

leftmons = nh.rn2(4)
for i = 1, leftmons do
   des.monster({ coord = leftside:rndcoord(1) })
end
for i = 1, 28 - leftmons do
   des.monster({ coord = rightside:rndcoord(1) })
end
