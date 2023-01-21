-- NetHack yendor wizard2.lua	$NHDT-Date: 1652196039 2022/05/10 15:20:39 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.2 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style="mazegrid", bg ="-" });
des.level_flags("mazelevel", "nommap", "noflip")
des.map([[
            -------------            
        -----...|...|...-----        
     ----...|...---.|.....|.----     
   ----.....|.....|.|.....|....---   
 ---..|...----....|.---...|.--.|.--- 
 |....-----..----.|...|..----..|...| 
--...--...--....|.|...----....--.----
|....|.....|.----------..------..|..|
|----|.....|--...|.|........|....|..|
|....|.....|...---.---....-------|..|
|..|.--...---..|.....|----|....|.|..|
|--|..-----.------.---....|---.|.---|
|..----.|......|.|.|.--...|....|....|
--...|....------.--|..|...|.------.--
 |.--------..|.....|..|.-----....|.| 
 ---........--------.----...|....--- 
   ---..-----..|.|.---.|....|..---   
     ----.....--.|..|..--...----     
        -----.|.....|...-----        
            -------------            
]]);
des.teleport_region({ region={18,10,18,10}, dir="up" })
des.ladder("up", 13,18)
des.ladder("down", 18,10)
des.region(selection.area(00,00,35,19), 'lit')

local loot = { '!', '?', '+', '"' }
for i = 1, #loot do
   for j = 1, d(2) do
      des.object(loot[i])
   end
end

-- Non diggable/phaseable walls everywhere
des.non_diggable()
des.non_passwall()
-- Teleporter map:
-- x = landing point
-- ^ = teleportation trap
--            -------------            
--        -----^.^|^.x|^.^-----        
--     ----..^|...---.|....^|^----     
--   ----..x..|^...^|.|^.x..|...x---   
-- ---.x|^..----^...|.---..^|^--.|^--- 
-- |^...-----^.----.|..^|^.----..|..x| 
----...--^.^--...^|x|^..----^...--.----
--|^..^|^...^|x----------x.------..|.^|
--|----|..x..|--x.^|^|^......^|^..^|x.|
--|...x|^...^|^..---.---^..^-------|..|
--|^.|.--^.^---.^|^.x.^|----|^...|^|^.|
--|--|.^-----^------.---^..^|---.|.---|
--|^.----^|.....x|^|^|^--x..|...x|x..^|
----..x|^...------.--|.^|..^|^------.--
-- |^--------.^|^.x.^|..|^-----^...|^| 
-- ---^....x..--------x----^.^|...x--- 
--   ---^.-----.^|^|^---^|^...|^.---   
--     ----x....--.|..|..--.x.----     
--        -----<|^...x|^.x-----        
--            -------------            

local GOALROOM = 1
local STARTROOM = 2

local landingspots = {
   -- room 1 is goal room in bottom left
   -- room 2 is start room in center
   -- room 3-* is everything else, in an arbitrary left-right then top-down
   -- traversal
   -- room= is redundant; expectation is that every entry's room number will be
   -- identical to its index in the array. Effectively, the code can assume that
   -- landingspots[i].room == i. (This is sanity checked below, though.)
   { room= 1, x=09, y=17 },
   { room= 2, x=18, y=10 },
   { room= 3, x=17, y=06 },
   { room= 4, x=19, y=01 },
   { room= 5, x=23, y=03 },
   { room= 6, x=09, y=03 },
   { room= 7, x=30, y=03 },
   { room= 8, x=05, y=04 },
   { room= 9, x=34, y=05 },
   { room=10, x=12, y=07 },
   { room=11, x=08, y=08 },
   { room=12, x=23, y=07 },
   { room=13, x=34, y=08 },
   { room=14, x=14, y=08 },
   { room=15, x=04, y=09 },
   { room=16, x=30, y=12 },
   { room=17, x=32, y=12 },
   { room=18, x=14, y=12 },
   { room=19, x=23, y=12 },
   { room=20, x=04, y=13 },
   { room=21, x=16, y=14 },
   { room=22, x=20, y=15 },
   { room=23, x=09, y=15 },
   { room=24, x=29, y=14 },
   { room=25, x=26, y=17 },
   { room=26, x=19, y=18 },
   { room=27, x=23, y=18 }
}
local TOTALROOMS = #landingspots

local teleporters = {
   -- A teleporter that has a "dest" field is considered locked-in to go to that
   -- room. The code that picks an unused teleporter will skip it. They all
   -- initialize with dest=nil because none are spoken for yet, except the one
   -- in the goal room that goes back to start.
   { room= 1, x=14, y=16, dest=2 },

   { room= 2, x=18, y=08 },
   { room= 2, x=16, y=10 },
   { room= 2, x=20, y=10 },
   { room= 2, x=18, y=12 },

   { room= 3, x=13, y=01 },
   { room= 3, x=15, y=01 },
   { room= 3, x=13, y=03 },
   { room= 3, x=17, y=03 },
   { room= 3, x=14, y=04 },

   { room= 4, x=17, y=01 },
   { room= 4, x=19, y=06 },
   { room= 4, x=21, y=05 },

   { room= 5, x=21, y=01 },
   { room= 5, x=23, y=01 },
   { room= 5, x=21, y=03 },
   { room= 5, x=25, y=04 },
   { room= 5, x=23, y=05 },

   { room= 6, x=11, y=02 },
   { room= 6, x=07, y=04 },

   { room= 7, x=27, y=02 },
   { room= 7, x=27, y=04 },
   { room= 7, x=26, y=06 },

   { room= 8, x=02, y=05 },
   { room= 8, x=01, y=07 },
   { room= 8, x=04, y=07 },

   { room= 9, x=32, y=04 },
   { room= 9, x=29, y=08 },
   { room= 9, x=32, y=08 },

   { room=10, x=11, y=05 },
   { room=10, x=15, y=06 },

   { room=11, x=07, y=06 },
   { room=11, x=09, y=06 },
   { room=11, x=06, y=07 },
   { room=11, x=10, y=07 },
   { room=11, x=06, y=09 },
   { room=11, x=10, y=09 },
   { room=11, x=07, y=10 },
   { room=11, x=09, y=10 },

   { room=12, x=20, y=08 },
   { room=12, x=27, y=08 },
   { room=12, x=22, y=09 },
   { room=12, x=25, y=09 },

   { room=13, x=35, y=07 },
   { room=13, x=34, y=10 },

   { room=14, x=12, y=09 },
   { room=14, x=14, y=10 },
   { room=14, x=16, y=08 },

   { room=15, x=01, y=10 },
   { room=15, x=05, y=11 },

   { room=16, x=27, y=10 },
   { room=16, x=27, y=13 },

   { room=17, x=32, y=10 },
   { room=17, x=35, y=12 },
   { room=17, x=34, y=14 },

   { room=18, x=11, y=11 },
   { room=18, x=07, y=12 },
   { room=18, x=06, y=13 },

   { room=19, x=22, y=11 },
   { room=19, x=25, y=11 },
   { room=19, x=25, y=13 },
   { room=19, x=23, y=14 },

   { room=20, x=01, y=12 },
   { room=20, x=02, y=14 },

   { room=21, x=16, y=12 },
   { room=21, x=14, y=14 },
   { room=21, x=18, y=14 },

   { room=22, x=20, y=12 },
   { room=22, x=21, y=13 },

   { room=23, x=04, y=15 },
   { room=23, x=06, y=16 },
   { room=23, x=12, y=14 },

   { room=24, x=32, y=14 },
   { room=24, x=29, y=16 },
   
   { room=25, x=24, y=16 },
   { room=25, x=25, y=15 },
   { room=25, x=27, y=15 },

   { room=26, x=18, y=16 },
   { room=26, x=16, y=16 },
   { room=26, x=15, y=18 },

   { room=27, x=21, y=18 },
   { room=27, x=22, y=16 }
}

for r = 1,#landingspots do
   if landingspots[r].room ~= r then
      nh.impossible('landingspots['..r..'] has wrong room index')
   end
   -- des.terrain(landingspots[i].x, landingspots[i].y, '\\')
end

-- port_per_room = storage of how many unassigned teleportation traps there are
-- in each room.
local port_per_room = {}
for t = 1, #teleporters do
   local rm = teleporters[t].room
   if port_per_room[rm] == nil then
      port_per_room[rm] = 0
   end
   if teleporters[t].dest == nil then
      -- originally an exception for the goal room was hardcoded below, but
      -- doing it this way allows for more flexibility (like if someone decides
      -- in the future to specify other teleporters' dest fields)
      port_per_room[rm] = port_per_room[rm] + 1
   end
end
   
-- Maze generation algorithm assumptions:
--    1: The room containing the ladder up (#1) is special and its sole
--       teleporter returns you to the start.
--    2: Every other room has at least 2 teleporters.

-- First, generate the sole solution by random-walking from the solution room to
-- the start room and assigning teleporters to go to those spots. No loops are
-- allowed in this step so every room will have at least 1 unused teleporter
-- remaining.
local solpath = { GOALROOM }
local curroom = nil
local MIN_SOL_LEN = 7
local MAX_SOL_LEN = 11

while (curroom ~= STARTROOM) and (#solpath < MAX_SOL_LEN) do
   curroom = d(TOTALROOMS)

   if (curroom == STARTROOM) and (#solpath < MIN_SOL_LEN) then
      -- do not allow picking STARTROOM and ending the path if it is still too
      -- short
      curroom = nil
   else
      -- is curroom already in the path?
      for s = 1, #solpath do
         if solpath[s] == curroom then
            curroom = nil
            break
         end
      end
   end

   if curroom ~= nil then
      table.insert(solpath, curroom)
   end
end
-- if we did not randomly end on STARTROOM, put it in now
if solpath[#solpath] ~= STARTROOM then
   table.insert(solpath, STARTROOM)
end

-- Select a random teleporter in room <src> that does not yet have a
-- destination set, and set its destination to room <dest>.
-- If there are no more unused teleporters in <src>, error.
function assign_unused_teleporter(src, dest)
   if port_per_room[src] < 1 then 
      nh.impossible('using up more teleporters than exist')
      return
   end
   local nfound = 0
   local indexfound = -1
   for t = 1, #teleporters do
      if teleporters[t].room == src and teleporters[t].dest == nil then
         nfound = nfound + 1
         if nh.rn2(nfound) == 0 then
            indexfound = t
         end
      end
   end
   if nfound == 0 or indexfound < 0 then
      nh.impossible('no more teleporters in room '..src)
      return
   end
   port_per_room[src] = port_per_room[src] - 1
   teleporters[indexfound].dest = dest
end

-- Trace the solution path backwards and assign "rank" to every room in the
-- solution path. STARTROOM is rank 0, the first room you get to from it
-- along the solution path is rank 1, etc. 
local rank = {}
for s = 1, #solpath do
   rank[solpath[s]] = #solpath - s
   -- While doing this, also set the teleporters along the path.
   if solpath[s] ~= GOALROOM then -- ... but do set one for STARTROOM
      assign_unused_teleporter(solpath[s], solpath[s-1])
   end
end
-- Rooms not in the solution path are rank 0.
for r = 1, TOTALROOMS do
   if rank[r] == nil then
      rank[r] = 0
   end
end

-- Next, ensure the start room is reachable from every room by doing the
-- following: 
--    Let roomSet = set containing just the start room.
--    For every room except the start and finish, in random order:
--       Assign a teleporter to go to a room in roomSet of lower or equal rank.
--       Add this room to roomSet.
local roomOrder = {}
for r = STARTROOM, TOTALROOMS do
   if r ~= STARTROOM then
      table.insert(roomOrder, r)
   end
end
shuffle(roomOrder)

local roomSet = { STARTROOM }
for r = 1, #roomOrder do
   local src = roomOrder[r]
   local dest = -1 
   repeat
      dest = roomSet[d(#roomSet)]
   until rank[dest] <= rank[src]
   assign_unused_teleporter(src, dest)
   table.insert(roomSet, src)
end

-- Next, ensure that all rooms are reachable from the start, so there are no
-- unreachable rooms. Do this by looking for any remaining rooms with in-degree
-- 0 and randomly picking a teleporter to go to them from the set that is
-- reachable from the start.
-- We should never run out of teleporters to do this, because even if the entire
-- solution path consists of rooms with out-degree of only 2 (so they have no
-- remaining teleporters after the one that goes towards the goal and the one
-- that goes towards the start), the start room still has 4 teleporters, and
-- even if those 4 teleporters are all used to go to rooms that only have an
-- out-degree of 2, only one of those 2 will be occupied by the back-to-start
-- teleporter. 
reachable = { }

function dfs_find_reachable_from(src)
   -- Given room <src> that is (newly) reachable from the start, do a depth
   -- first search to find other rooms that are reachable from it, and add them
   -- to reachable.
   if reachable[src] then
      return
   end
   reachable[src] = true
   for t = 1, #teleporters do
      if teleporters[t].room == src and teleporters[t].dest ~= nil then
         dfs_find_reachable_from(teleporters[t].dest)
      end
   end
end

-- Init it to ones that are reachable from the start. (This should include the
-- entire solution path.)
dfs_find_reachable_from(STARTROOM)

-- Until every room is reachable from the start...
while #reachable < TOTALROOMS do
   -- take the first non-reachable room...
   local dest
   for r = STARTROOM, TOTALROOMS do
      if reachable[r] ~= true then
         dest = r
         break
      end
   end

   -- and pick a random reachable room that has available teleporters...
   local src
   repeat
      src = d(TOTALROOMS)
   until port_per_room[src] > 0 and reachable[src] == true
      -- should this also check rank[src] >= rank[dest]? I don't think so; if
      -- dest is unreachable from start, it is not in the solution path and
      -- therefore ought to have rank = 0, so it shouldn't matter

   -- and connect them...
   assign_unused_teleporter(src, dest)

   -- and mark it and its descendants reachable.
   dfs_find_reachable_from(dest)
end

-- Now we just need to obtain targets for the remaining unassigned teleporters.
-- For this, assign teleporters to random rooms with the following parameters:
--    1. Never allow a teleporter to go to a room of higher rank (since this
--       could provide multiple solution paths).
--    2. Never allow a teleporter to go to its own room.
--    2. If possible, don't allow 2 teleporters in the same room to end up in
--       the same room, though do if there is no other option. (Unimplemented
--       for now.)
for t = 1, #teleporters do
   if teleporters[t].dest == nil then
      local src = teleporters[t].room
      local dest

      repeat
         dest = d(TOTALROOMS)
      until rank[dest] <= rank[src] and src ~= dest

      teleporters[t].dest = dest
   end
end

-- Finally, construct the actual teleporters.
for t = 1, #teleporters do
   local dx = landingspots[teleporters[t].dest].x
   local dy = landingspots[teleporters[t].dest].y
   des.trap({ type = "teleport", x = teleporters[t].x, y = teleporters[t].y,
              seen = true, teledest = { dx, dy } })
end

