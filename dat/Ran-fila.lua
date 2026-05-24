-- NetHack Ranger Ran-fila.lua	$NHDT-Date: 1652196009 2022/05/10 15:20:09 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.2 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " ", lit = 1 });

des.level_flags("mazelevel", "hardfloor", "outdoors", "arboreal", "noflipy");

-- boundary syms (B) indicate "trail junctions", which will be connected by
-- tree-less paths; other blobs of them are places that we deliberately want to
-- avoid generating trees on.
-- The map extends the full available height (i.e. no layer of stone at any
-- point) in order to prevent the randline paths from wandering outside the
-- intended area.
des.map([[
TTTTTTTTTBTTTTTTTTTTTTTTTTTTTTTTTTBBBBBBBBTTTTTTTTTTTTTTTTTTTBTTTTTTTTTTTTT
T.........................................................................T
T..................B......................................................T
T.........................................................................T
T.................................B..........B.......................B....T
B.........................................................................T
T.........................................................................T
T.......................B..........................B......................T
T.........................................................................T
T...............................................................B.........T
T...........B.............................................................T
T.........................................................................B
T.......................................B.................................T
T...................................................................-------
T...BBB.......................................................BBB----      
----BBB..........B............................................BBB|   ####  
   -B--.......................................................--B- ###  #  
    # ---.......................B..................B..........| ####    ## 
 ####   |.....................................................|          # 
 #      -----.................................................----       ##
##          |TBTTTTTTTTTTTTTTTTTTTTTTTTTTBTTTTTTTTTTTTTTTTTTTBBBB|        #
]]);

-- In graph form, from which a spanning tree will be taken:
--          1                            2                 .....3..           
--       .............  ......          . ...         .....    ..  ....       
--     ..   .         4      ........ ..     ... .....      ... .      .      
--  ...      .       . ...           5          6         ..     .      7     
-- 8          .    ..     . ......... .        . .....  ..        .    . .... 
-- ......      .  .        9           .       .      10           . ..      .
-- .     ....... .        . .           .     .    ...  ...........11         .
--  .          12         .  .           .   .  ...     .         .  ........ .
--   .        .  .       .    .           .  ...        .         .          13
--    .    ...    .     .      .           14           .         .     .....  
--     .  .        .   .        .         . ...         .         ......      
--      15 .......... .          .       .  .  ...      .        16           
--  (goal 1)        17            .     .   .     ...   .    ....(goal 2)          
--                 .  .............. ...     .       .  .....   .             
--                 .               18        .        19        .             
--                 .        .......  ...     .   .....  ....    .             
--                 .........            .... ....           .....             
--               20                         21                  22          

-- Light up the level
des.region(selection.area(00,00,74,20), "lit")

-- Upstair back to the home level is near the middle of the top edge.
local ustairx = 34 + nh.rn2(8)
des.stair("up", ustairx, 00)

-- Downstair is in one of the random caves.
if percent(50) then
   des.stair("down", 00,20)
else
   des.stair("down", 74,20)
end

-- Forest the level.
des.replace_terrain({ region={00,00,74,20}, fromterrain='.', toterrain='T', chance=85 })

junctions = {
   { x=09, y=00 },
   { x=ustairx, y=00 },
   { x=61, y=00, },
   { x=19, y=02, },
   { x=34, y=04, },
   { x=45, y=04, },
   { x=69, y=04, },
   { x=00, y=05, },
   { x=24, y=08, },
   { x=51, y=08, },
   { x=64, y=09, },
   { x=12, y=10, },
   { x=74, y=11, },
   { x=40, y=12, },
   { x=06, y=14, },
   { x=62, y=14, },
   { x=17, y=15, },
   { x=27, y=17, },
   { x=51, y=17, },
   { x=14, y=20, },
   { x=41, y=20, },
   { x=61, y=20, }
}
local NUM_JUNCTIONS = 22

local all_edges = {
   { 1,  4}, { 1,  8}, { 1, 12},
   { 2,  5}, { 2,  6},
   { 3,  6}, { 3,  7}, { 3, 10}, { 3, 11},
   { 4,  5}, { 4,  9}, { 4, 12},
   { 5,  6}, { 5,  9}, { 5, 14},
   { 6, 10}, { 6, 14},
   { 7, 11}, { 7, 13},
   { 8, 12}, { 8, 15},
   { 9, 12}, { 9, 14}, { 9, 17}, { 9, 18},
   {10, 11}, {10, 14},
   {11, 13}, {11, 16},
   {12, 15}, {12, 17},
   {13, 16},
   {14, 18}, {14, 19}, {14, 21},
   {15, 17},
   {16, 19}, {16, 22},
   {17, 18}, {17, 20},
   {18, 20}, {18, 21},
   {19, 21}, {19, 22}
   -- 20 thru 22 don't have any higher-numbered nodes to connect to
}

-- Given an edge, carve a trail between its two endpoints.
function cut_trail(edge)
   local x1 = junctions[edge[1]].x
   local y1 = junctions[edge[1]].y
   local x2 = junctions[edge[2]].x
   local y2 = junctions[edge[2]].y
   des.terrain({ selection=selection.randline(x1,y1, x2, y2, 3), typ='.' })
end

-- Connect up all the junctions.
make_spanning_tree(all_edges, NUM_JUNCTIONS, 2, cut_trail)

-- Add a few extra paths.
-- It's fine if this picks a path that was already cut; such doubled trails
-- happen in real life sometimes.
for i = 1,5 do
   local edge = all_edges[d(#all_edges)]
   cut_trail(edge)
end

-- convert boundary syms into floor (basically running remove_boundary_syms
-- before level has finished loading) so that floodfill won't stop on them and
-- remove_tree_chokepoints won't ignore them
des.replace_terrain({ region={00,00,74,20}, fromterrain='B', toterrain='.' })

-- Correct for potential diagonal gaps left by randline (or holes from
-- replace_terrain next to the lines) that the hero would have to squeeze
-- through.
remove_tree_chokepoints()

-- the replace_terrain could have left inaccessible holes in the middle of tree
-- clusters; rather than setting the inaccessibles flags on this level (which
-- would provide a wealth of escape items), remove any such holes.
-- 1. obtain selection of all floor
local everything = selection.area(00,00,74,20)
local floor = everything:filter_mapchar('.')
-- 2. flood fill from area immediately surrounding the stairs
local nearstairs = selection.area(33,00,42,01):filter_mapchar('.')
local reachfromstairs = selection.new()
nearstairs:iterate(function(x, y)
   if reachfromstairs:get(x, y) == 0 then
      reachfromstairs = reachfromstairs + selection.floodfill(x, y)
   end
end)

des.terrain(floor - reachfromstairs, 'T')

-- Traps, objects and monsters.
for i = 1,9 do
   des.object()
end
for i = 1,4 do
   des.object("boulder")
end

for i = 1,6 do
   des.trap()
end

for i = 1,2 do
   des.monster({ id = "mountain centaur", peaceful=0 })
   des.monster({ id = "forest centaur", peaceful=0 })
   des.monster({ class = "C", peaceful=0 })
   des.monster({ id = "scorpion", peaceful=0 })
end
des.monster({ id = "giant scorpion", peaceful=0, waiting=1 })
