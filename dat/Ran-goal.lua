-- NetHack Ranger Ran-goal.lua	$NHDT-Date: 1652196010 2022/05/10 15:20:10 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " ", lit = 0 });

des.level_flags("mazelevel", "outdoors", "noflipx");

-- As in the filler levels, B indicates a trail junction, but it is also used to
-- enforce some clearings by the riverside and define an area on the right side
-- that Scorpius inhabits to be a clearing without trees in it.

des.map([[
TTTTTTTBTTTTTTTTBTTTTTTT}}}}}}}TTTTTTTTTTTTTTTTTBTTTTTTTTTTTTTTTTTTTBTTTTTT
T......................}}}}}}}............................................T
B......................}}}}}}}.............B...................B..........T
T...........B......BB}}}}}}}}......B..................B...................T
T..................B}}.}}}}}..............................................T
T..................}}}}}BB}}...................B......................B...T
T....B............}}}}}B..............................BB..BBBBB...........T
T.................}}}}}B......B......................B..BB.....BB.........T
T.................B}}}}}BB...............B...........B...........BBB......T
B..................}}}}}}}............................B.............B.....T
T.......B.......B...}}.}}}}...........................B.............B.....B
T....................}}}}}}}..........................B............B......T
T....................BB}}}}}..........................B............B......T
T......................B}}}}}......B.................B...B..........B.....T
T...........B..........B}}}}}B.................B......B.B.BBBBB.B..B...B..T
B..................}}BB}.}.}BB.........................B.......B.BB.......T
T..................}}}}}}}}BB...............................B.............T
T.....B..........B}}}}}}}}...............B...........B....................T
T................}}}}}}}BB................................................T
T................}}}}}}}...............................................B..T
TTTBTTTTTTTBTTTT}}}}}}}TTTTTTTTTBTTTTTTTTTTTTTTBTTTTTTTTTTTTBTTTTTTTTTTTTTT
]]);

-- This one's actually going to have two separate spanning trees, on account of
-- the stream. In graph form:
--       A        B       }}}}}}}                 a                   b      
-- ...... ..     . ..    }}}}}}}              .... ...            .... .     
--C     .   .. ..    .   }}}}}}}      .......c        .. ........d     .     
-- ..   .     D.......E}}}}}}}}      e        .         f       .      .     
--   .  .  ... .      }} }}}}}     ....        .. ...... ...    .      .     
--    . ...     .    }}}}}g }}    .  . ..        h          ............i    
--     F         .  }}}}}  ..... .   .   .     ..               j        .   
--   .. ............}}}}}       k    .    . ...                          .   
-- ..   .           G}}}}}       .   .     l                              .  
--H      .         . }}}}}}}      .  .    . .                              . 
-- .......I.......J   }} }}}}     .  .  ..   .                              m
--       . .     . ..  }}}}}}}     . . .      .                            . 
--     ..   .   .    ..  }}}}}      ...        .                          .  
--   ..      . .       .. }}}}}      n          .                         .  
-- ..         K..........L}}}}}   ... ..         o                   ....p   
--M        ... ..    }}  } } }....  .   ..    ......           .....q    .   
-- ..... ..      ..  }}}}}}}}r      .     . ..   .  ... ......s      .   .   
-- .    N          O}}}}}}}}  ..   .       t     .     u      .       .  .   
--  .  . ..     ...}}}}}}}v     .  .   .... ..   .   .. ..... .        . .   
--  . .    .. ..   }}}}}}} ....... ....       .......        ............w   
--   P       Q    }}}}}}}         x              y            z              

local left_junctions = {
   {x=07, y=00},
   {x=16, y=00},
   {x=00, y=02},
   {x=12, y=03},
   {x=20, y=03},
   {x=05, y=06},
   {x=18, y=08},
   {x=00, y=09},
   {x=08, y=10},
   {x=16, y=10},
   {x=12, y=14},
   {x=23, y=14},
   {x=00, y=15},
   {x=06, y=17},
   {x=17, y=17},
   {x=03, y=20},
   {x=11, y=20}
}

local right_junctions = {
   {x=48, y=00},
   {x=68, y=00},
   {x=43, y=02},
   {x=63, y=02},
   {x=35, y=03},
   {x=54, y=03},
   {x=24, y=05},
   {x=47, y=05},
   {x=70, y=05},
   {x=62, y=06},
   {x=30, y=07},
   {x=41, y=08},
   {x=74, y=10},
   {x=35, y=13},
   {x=47, y=14},
   {x=71, y=14},
   {x=66, y=15},
   {x=27, y=16},
   {x=60, y=16},
   {x=41, y=17},
   {x=53, y=17},
   {x=24, y=18},
   {x=71, y=19},
   {x=32, y=20},
   {x=47, y=20},
   {x=60, y=20}
}

local left_edges = {
   { 1,  3}, { 1,  4}, { 1,  6},
   { 2,  4}, { 2,  5},
   { 3,  6},
   { 4,  5}, { 4,  6}, { 4,  7},
   { 6,  8}, { 6,  9},
   { 7, 10},
   { 8,  9},
   { 9, 10}, { 9, 11}, { 9, 13},
   {10, 11}, {10, 12},
   {11, 12}, {11, 14}, {11, 15},
   {13, 14}, {13, 16},
   {14, 16}, {14, 17},
   {15, 17}
}

local right_edges = {
   { 1,  3}, { 1,  6},
   { 2,  4}, { 2,  9},
   { 3,  5}, { 3,  8},
   { 4,  6}, { 4, 10},
   { 5, 11}, { 5, 12}, { 5, 14},
   { 6,  8}, { 6, 10},
   { 7, 11},
   { 8, 12},
   { 9, 10}, { 9, 13},
   {11, 14},
   {12, 14}, {12, 15},
   {13, 16},
   {14, 18}, {14, 20}, {14, 24},
   {15, 20}, {15, 21}, {15, 25},
   {16, 17}, {16, 23},
   {17, 19}, {17, 23},
   {18, 24},
   {19, 21}, {19, 26},
   {20, 24}, {20, 25},
   {21, 25}, {21, 26},
   {22, 24},
   {23, 26}
}

local scorp_x = 62
local scorp_y = 10
local scorp_clearing = selection.floodfill(scorp_x, scorp_y)

-- Forest the level.
-- Note that the floodfills deliberately exclude Scorpius's area and the
-- stepping stones in the river.
local forestable = selection.floodfill(01,01) + selection.floodfill(51,01)
des.replace_terrain({ selection=forestable, fromterrain='.', toterrain='T', chance=85 })

-- global variable to switch between junction arrays without injecting into
-- cut_trail
using_left = true

-- Given an edge, carve a trail between its two endpoints.
-- This uses replace_terrain so it doesn't overwrite the river at any point.
-- It may create an unexpected entrance to Scorpius's clearing; that's fine if
-- it happens.
function cut_trail(edge)
   local junctions = using_left and left_junctions or right_junctions
   local x1 = junctions[edge[1]].x
   local y1 = junctions[edge[1]].y
   local x2 = junctions[edge[2]].x
   local y2 = junctions[edge[2]].y
   des.replace_terrain({ selection=selection.randline(x1,y1, x2, y2, 3), 
                         fromterrain='T', toterrain='.' })
end

-- Connect up all the junctions on the left...
make_spanning_tree(left_edges, #left_junctions, 8, cut_trail)

-- ... and on the right
using_left = false
make_spanning_tree(right_edges, #right_junctions, 14, cut_trail)

-- A few extra paths
for i = 1,2 do
   local edge = left_edges[d(#left_edges)]
   cut_trail(edge)
   edge = right_edges[d(#right_edges)]
   cut_trail(edge)
end

-- convert boundary syms into floor (basically running remove_boundary_syms
-- before level has finished loading) so that floodfill won't stop on them and
-- remove_tree_chokepoints won't ignore them
des.replace_terrain({ region={00,00,74,20}, fromterrain='B', toterrain='.' })

-- Destroy trees that create choke points, especially since this level is
-- non-diggable
remove_tree_chokepoints()

-- strip inaccessible holes from middle of tree clusters
-- 1. obtain selection of all floor
local everything = selection.area(00,00,74,20)
local floor = everything:filter_mapchar('.')
-- 2. flood fill from area immediately surrounding one known floor point on
-- either side of the river. note 3rd parameter is true because there can be
-- diagonal adjacencies in the river-edge clearings
local realpaths = selection.floodfill(00,02,true)
   + selection.floodfill(scorp_x,scorp_y, true)
   -- river edge clearings relevant to stepping stones may STILL be disconnected
   -- due to the randline walking through the river instead of trees
   -- so add them
   + selection.area(23,05,25,08):filter_mapchar('.')
   + selection.area(21,12,23,15):filter_mapchar('.')
-- 3. correct for stepping stones in river
local stepstones = selection.new()
stepstones:set(22,04, 1)
stepstones:set(22,10, 1)
stepstones:set(26,15, 1)
stepstones:set(28,15, 1)
-- 4. turn everything that's not that back into a tree
des.terrain((floor - realpaths) - stepstones, 'T')

-- Stairs
ustair_locs = { 1, 3, 8, 13, 16 }
shuffle(ustair_locs)
local usx = left_junctions[ustair_locs[1]].x
local usy = left_junctions[ustair_locs[1]].y
des.stair("up", usx, usy)
-- Scorpion next to the stairs since the quest text mentions one appearing
local next2stairs = selection.rect(usx-1, usy-1, usx+1, usy+1):filter_mapchar('.')
des.monster({ id = 'scorpion', coord=next2stairs:rndcoord(), noitem=true })

-- shouldn't be possible to levelport or fall down into this level (except in
-- wizmode) but just in case...
des.teleport_region({ region={00,00,22,20}, dir="down" })

-- Non diggable trees
des.non_diggable(selection.area(00,00,74,20))

-- Possibly easier path across the river
if percent(20) then
   des.terrain(25,14, '.')
end
if percent(40) then
   des.terrain(21,04, '.')
end
if percent(40) then
   des.terrain(23,04, '.')
end

-- Fog
-- uses the realpaths selection from above
des.replace_terrain({ selection=(realpaths - scorp_clearing) - stepstones,
                      fromterrain='.', toterrain='C', chance=25 })

-- Some boulders
for i = 1, 15 do
   des.object("boulder")
end

-- Objects
-- note that scorpions and spiders generate random items when they spawn, so
-- reduce this amount
for i = 1, 4 do
   des.object()
end
for i = 1, 3 do
   des.object({ id = "egg", montype = "scorpion" })
end

-- Random traps
for i = 1, 6 do
   des.trap()
end

-- Monsters:
des.monster({ id = "Scorpius", x=scorp_x, y=scorp_y, peaceful=0 })

-- a few terrified centaurs
for i = 1, 2 do
   des.monster({ id = "forest centaur", coord=scorp_clearing:rndcoord(1), peaceful=0 })
   des.monster({ id = "mountain centaur", coord=scorp_clearing:rndcoord(1), peaceful=0 })
   des.monster({ class = "C", peaceful=0 }) -- not necessarily in clearing
end

-- the brethren of Scorpius
for i = 1, 5 do
   des.monster({ id = "scorpion", coord=scorp_clearing:rndcoord(1), peaceful=0 })
   des.monster({ id = "scorpion", peaceful=0 }) -- not in clearing
end
for i = 1, 3 do
   des.monster({ id = "giant scorpion", coord=scorp_clearing:rndcoord(1), peaceful=0 })
   des.monster({ id = "giant scorpion", peaceful=0 }) -- not in clearing
   des.monster({ class = "s", peaceful=0 })
end
