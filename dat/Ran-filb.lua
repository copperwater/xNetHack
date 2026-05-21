-- NetHack Ranger Ran-filb.lua	$NHDT-Date: 1652196010 2022/05/10 15:20:10 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.2 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " ", lit = 0 });

des.message("You emerge into a foggy valley with close, dark trees.")

des.level_flags("mazelevel", "hardfloor", "outdoors", "noflipx");

-- Generation for this level works along much the same lines as in Ran-fila. See
-- that file for descriptions.
-- We can't hardcode a "cave exit" for the upstair in this one since there may
-- be 2 filler levels.

des.map([[
TTTTTTTTTTTTTTTBTTTTTTTTTTTTBTTTTTTTTTTTTTTTTTTTBTTTTTTTTTTTTTTTTTBTTTTTTTT
T.........................................................................T
B......................................B..................................T
T.........................................................................T
T........B................................................................B
T.....................B..................................B................T
T.................................B.......................................T
T..................................................................B......T
T...........................................B.............................T
B.........................................................................T
T.........................B...............................................B
T.........................................................B...............T
T.........B............................B..................................T
T.........................................................................T
T.........................................................................T
B...................B.............................B.................B.....T
T...............................B.........................................T
T.........................................................................T
T.........................................................................B
T.........................................................................T
TTTBTTTTTTTTTTTBTTTTTTTTTTTTTTTTTTTBTTTTTTTTTTTTTTTTBTTTTTTTTTTTTTTBTTTTTTT
]]);

-- In graph form, from which a spanning tree will be taken:
--               1            2                   3                 4        
-- .............. .         .. .........  ........ .              .. ..      
--5           .    ..      .    .        6      .   ..          ..   . ...   
-- ........ ..       ..   .      .      . .     .     ..      ..     .    .. 
--         7......     . .        .   ..  .     .       ..  ..       .      8
--       ...     .......9         .  .     .    .         10         .   ... 
--      .  .           . .         11       .   .          ......... ....    
--    ..   .          .   .      ..  .      .  .           .        12       
--  ..      .      ...    .     .     .      13            .     ...  ...    
--14        .     .        .  ..      .     .  ......      .    .        ..  
--  ....     .   .          15         .   .   .     ....... ...           16
--      .... ....          .  ..........  .     .          17             .  
--          18            .   .         19      .        ..  ..          .   
--      ..... ....       .     .      ..         .     ..      ....     .    
--  ....    .     ....  .      .     .            .  ..            ..  .     
--20       .          21        .  ..              22                23      
--  .     .          .  .........24               .. .....           . ..    
--  .    .          .           .  .          .... .      ....       .   ..  
--  .   .           .     ......   .      ....      .         ....   .     25
--  .  .           .......         .  ....          .             .. ......  
--   26          27                 28               29             30       
-- Upstair is at 5, 14, or 20; downstair is at 4, 8, 16, 25, or 30.

-- Forest the level.
des.replace_terrain({ region={00,00,74,20}, fromterrain='.', toterrain='T', chance=85 })

junctions = {
   {x=15, y=00},
   {x=28, y=00},
   {x=48, y=00},
   {x=66, y=00},
   {x=00, y=02},
   {x=39, y=02},
   {x=09, y=04},
   {x=74, y=04},
   {x=22, y=05},
   {x=57, y=05},
   {x=34, y=06},
   {x=67, y=07},
   {x=44, y=08},
   {x=00, y=09},
   {x=26, y=10},
   {x=74, y=10},
   {x=58, y=11},
   {x=10, y=12},
   {x=39, y=12},
   {x=00, y=15},
   {x=20, y=15},
   {x=50, y=15},
   {x=68, y=15},
   {x=32, y=16},
   {x=74, y=18},
   {x=03, y=20},
   {x=15, y=20},
   {x=35, y=20},
   {x=52, y=20},
   {x=67, y=20}
}
local NUM_JUNCTIONS = 30
local all_edges = {
   { 1,  5}, { 1,  7}, { 1,  9},
   { 2,  6}, { 2,  9}, { 2, 11},
   { 3,  6}, { 3, 10}, { 3, 13},
   { 4,  8}, { 4, 10}, { 4, 12},
   { 5,  7},
   { 6, 11}, { 6, 13},
   { 7,  9}, { 7, 14}, { 7, 18},
   { 8, 12},
   { 9, 15}, { 9, 18},
   {10, 12}, {10, 17},
   {11, 15}, {11, 19},
   {12, 16}, {12, 17},
   {13, 17}, {13, 19}, {13, 22},
   {14, 18},
   {15, 19}, {15, 21}, {15, 24},
   {16, 23},
   {17, 22}, {17, 23},
   {18, 20}, {18, 21}, {18, 26},
   {19, 24},
   {20, 26},
   {21, 24}, {21, 27},
   {22, 28}, {22, 29}, {22, 30},
   {23, 25}, {23, 30},
   {24, 27}, {24, 28},
   {25, 30}
}

-- Given an edge, carve a trail between its two endpoints.
function cut_trail(edge)
   local x1 = junctions[edge[1]].x
   local y1 = junctions[edge[1]].y
   local x2 = junctions[edge[2]].x
   local y2 = junctions[edge[2]].y
   des.terrain({ selection=selection.randline(x1,y1, x2, y2, 3), typ='.' })
end

make_spanning_tree(all_edges, NUM_JUNCTIONS, 1, cut_trail)

-- as in Ran-fila, add a couple extra paths
for i = 1,5 do
   local edge = all_edges[d(#all_edges)]
   cut_trail(edge)
end

-- convert boundary syms into floor (basically running remove_boundary_syms
-- before level has finished loading) so that floodfill won't stop on them and
-- remove_tree_chokepoints won't ignore them
des.replace_terrain({ region={00,00,74,20}, fromterrain='B', toterrain='.' })

remove_tree_chokepoints()

-- strip inaccessible holes from middle of tree clusters, like on Ran-fila,
-- except here we haven't assigned the stairs yet:
-- 1. obtain selection of all floor
local everything = selection.area(00,00,74,20)
local floor = everything:filter_mapchar('.')
-- 2. flood fill from area immediately surrounding one known floor point
local reachfromstairs = selection.floodfill(00,02)
-- 3. turn everything that's not that back into a tree
des.terrain(floor - reachfromstairs, 'T')

ustair_locs = { 5, 14, 20 }
dstair_locs = { 4, 8, 16, 25, 30 }
shuffle(ustair_locs)
shuffle(dstair_locs)
des.stair("up", junctions[ustair_locs[1]].x, junctions[ustair_locs[1]].y)
des.stair("down", junctions[dstair_locs[1]].x, junctions[dstair_locs[1]].y)

-- Fog
des.replace_terrain({ region={00,00,74,20}, fromterrain='.', toterrain='C', chance=15 })

-- Objects
-- note that scorpions and spiders generate random items when they spawn, so
-- reduce this amount
for i = 1,5 do
   des.object()
end
for i = 1,5 do
   des.object("boulder")
end

-- Traps
for i = 1, 6 do
   des.trap()
end

-- Monsters:
-- note that centaurs are giving way to scorpions and other s-class monsters by
-- this point on the quest.
-- There are still some centaurs, and more can spawn post-generation, but the
-- place the hero is approaching is not a place that centaurs particularly want
-- to hang out in.
for i = 1, 2 do
   des.monster({ id = "mountain centaur", peaceful=0 })
   des.monster({ class = "s", peaceful=0 })
end
for i = 1, 4 do
   des.monster({ id = "scorpion", peaceful=0 })
   des.monster({ id = "giant scorpion", peaceful=0 })
end
