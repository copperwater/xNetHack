-- NetHack yendor wizard2.lua	$NHDT-Date: 1652196039 2022/05/10 15:20:39 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.2 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style="mazegrid", bg ="-" });
-- this level allows teleport, which does allow the player to teleport around
-- and potentially skip through the maze, but 1) teleport control will always be
-- blocked by the Wizard and normal teleports may fail; 2) this allows some more
-- interesting obstacles.
des.level_flags("mazelevel", "nommap", "noflip")
des.map([[
            -------------            
        -----...F.......-----        
      ---.......F.......|...----     
   ----.----.------------......---   
 ---.......---......|.......----.--- 
 |...........|......|-------|......| 
-----------------...|.......------.--
|......|........-----........|.|.|..|
|....------....--...--....----.|.---|
|-----....|....|.....|....|....|....|
|.......---....|.....|----|....|....|
|.......|......|.....|....|---------|
|.......---------...--....|...|.....|
------....|.....-----.....|...--...--
 |...------.......|.|...-----..|...| 
 ---......---...---.|...|...|..|.--- 
   ---......|...|...|----...|..---   
     ----...-----...|.......----     
        -----.......|...-----        
            -------------            
]]);
des.teleport_region({ region={18,09,18,11}, dir="up" })
des.ladder("up", 13,18)
des.ladder("down", 18,10)
des.region(selection.area(00,00,35,19), 'lit')
des.non_diggable()
des.non_passwall()

local loot = { '!', '?', '+', '"' }
for i = 1, #loot do
   for j = 1, d(2) do
      des.object(loot[i])
   end
end

-- Room map: (# = iron bars)
--             -------------            
--         -----   #       -----        
--       ---   1   #   2   |   ----     
--    ---- ---- ------------ 5    ---   
--  ---       ---      |       ---- --- 
--  |     3     |   4  |-------|   9  | 
-- -----------------   |       ------ --
-- |  6   |        -----  8     | | |  |
-- |    ------ 7  --   --    ---- | ---|
-- |-----    |    | 11  |    | 12 | 13 |
-- |       ---    |  >  |----|    |    |
-- |  10   |      |     |    |---------|
-- |       ---------   --    | 16|  17 |
-- ------    |     -----  15 |   --   --
--  |   ------  14   | |   -----  |   |
--  ---      ---   --- |   |   |  | --- 
--    --- 18   |   |   |---- 20|  ---   
--      ----   -----   |       ----     
--         -----<   19 |   -----        
--             -------------            
--
-- Teleporter map:
-- ^ = teleportation trap (bidirectional with some other trap)
-- # = iron bars
-- B = one-way teleporter back to start
-- H = hole
-- 0123456789012345678901234567890123456
--0            -------------            
--1        ----- ^ # ^ ^ ^ -----        
--2      ---^      #       |^  ----     
--3   ----^----^------------     ^---   
--4 ---       ---     ^|^      ----^--- 
--5 |^         ^|^     |-------|^     | 
--6-----------------  ^|^      ------ --
--7|     ^|^      ^-----       ^|^|^| ^|
--8|^  ^------    -- ^ --    ---- | ---|
--9|-----   ^|^   |     |^  ^|^   |   ^|
--0|^      ---    |^   ^|----|   ^|^   |
--1|       |^    ^|     |^  ^|---------|
--2|^      --------- ^ --    |  ^|^   ^|
--3------   ^|^    -----H   ^|^  --   --
--4 |^  ------      ^|^|   -----  |   |
--5 ---     ^---   --- |^  |^  |  |^--- 
--6   ---     ^| ^ |B  |----   |^ ---   
--7     ----   -----   |      ^----     
--8        -----<      |^  -----        
--             -------------            

-- The original implementation of this level contained 27 rooms and 80
-- single-direction teleporters with a single correct solution path of between
-- 7 and 11 different steps. There was no possibility of generating an
-- accidental shortcut that shortened the path. This made the level very
-- unpopular, so this is take 2 at it: 20 rooms and 32 pairs of teleporters that
-- each allow backtracking by virtue of being bidirectional.
--
-- In this fixed-maze algorithm, the topological graph of connections between
-- the rooms is always the same, but the actual rooms are randomized.

-- return true if telemaze was set up correctly, false if there was some error
function set_up_telemaze()
   -- These are the PHYSICAL rooms, containing:
   -- id: room id number
   -- region: a selection of the points in the room
   -- teleporters: table of teleporters, which contain the x,y location of the
   --              teleporter and a "dest" field.
   -- When "dest" is set on a teleporter later, it's considered fixed; when dest
   -- is nil, it's available to be paired with another teleporter.
   local physical_rooms = {
      { id = 1, region = selection.floodfill(12,02), teleporters = {
            { x=09, y=02, dest=nil },
            { x=12, y=03, dest=nil },
            { x=14, y=01, dest=nil } } },

      { id = 2, region = selection.floodfill(20,02), teleporters = {
            { x=18, y=01, dest=nil },
            { x=20, y=01, dest=nil },
            { x=22, y=01, dest=nil } } },

      { id = 3, region = selection.floodfill(07,05), teleporters = {
            { x=07, y=03, dest=nil },
            { x=02, y=05, dest=nil },
            { x=12, y=05, dest=nil } } },

      { id = 4, region = selection.floodfill(17,05), teleporters = {
            { x=14, y=05, dest=nil },
            { x=19, y=04, dest=nil },
            { x=19, y=06, dest=nil } } },

      { id = 5, region = selection.floodfill(26,03), teleporters = {
            { x=21, y=04, dest=nil },
            { x=25, y=02, dest=nil },
            { x=30, y=03, dest=nil } } },

      { id = 6, region = selection.floodfill(03,07), teleporters = {
            { x=01, y=08, dest=nil },
            { x=04, y=08, dest=nil },
            { x=06, y=07, dest=nil } } },

      { id = 7, region = selection.floodfill(12,09), teleporters = {
            { x=08, y=07, dest=nil },
            { x=15, y=07, dest=nil },
            { x=11, y=09, dest=nil },
            { x=09, y=11, dest=nil },
            { x=14, y=11, dest=nil } } },

      { id = 8, region = selection.floodfill(23,07), teleporters = {
            { x=21, y=06, dest=nil },
            { x=28, y=07, dest=nil },
            { x=22, y=09, dest=nil },
            { x=25, y=09, dest=nil } } },

      { id = 9, region = selection.floodfill(32,05), teleporters = {
            { x=29, y=05, dest=nil },
            { x=32, y=04, dest=nil },
            { x=35, y=07, dest=nil } } },

      { id = 10, region = selection.floodfill(04,11), teleporters = {
            { x=09, y=09, dest=nil },
            { x=01, y=10, dest=nil },
            { x=01, y=12, dest=nil },
            { x=09, y=13, dest=nil } } },

      { id = 11, region = selection.floodfill(18,09), teleporters = {
            { x=18, y=08, dest=nil },
            { x=16, y=10, dest=nil },
            { x=20, y=10, dest=nil },
            { x=18, y=12, dest=nil } } },

      { id = 12, region = selection.floodfill(29,09), teleporters = {
            { x=30, y=07, dest=nil },
            { x=27, y=09, dest=nil },
            { x=30, y=10, dest=nil } } },

      { id = 13, region = selection.floodfill(33,09), teleporters = {
            { x=32, y=07, dest=nil },
            { x=35, y=09, dest=nil },
            { x=32, y=10, dest=nil } } },

      { id = 14, region = selection.floodfill(14,14), teleporters = {
            { x=11, y=13, dest=nil },
            { x=14, y=16, dest=nil },
            { x=17, y=14, dest=nil } } },

      { id = 15, region = selection.floodfill(23,13), teleporters = {
            { x=22, y=11, dest=nil },
            { x=25, y=11, dest=nil },
            { x=25, y=13, dest=nil },
            { x=21, y=15, dest=nil } } },

      { id = 16, region = selection.floodfill(28,13), teleporters = {
            { x=27, y=13, dest=nil },
            { x=29, y=12, dest=nil },
            { x=29, y=16, dest=nil } } },

      { id = 17, region = selection.floodfill(33,13), teleporters = {
            { x=31, y=12, dest=nil },
            { x=35, y=12, dest=nil },
            { x=32, y=15, dest=nil } } },

      { id = 18, region = selection.floodfill(08,16), teleporters = {
            { x=02, y=14, dest=nil },
            { x=09, y=15, dest=nil },
            { x=11, y=16, dest=nil } } },

      { id = 19, region = selection.floodfill(19,16), teleporters = {
            { x=19, y=14, dest=nil } } },

      { id = 20, region = selection.floodfill(25,17), teleporters = {
            { x=25, y=15, dest=nil },
            { x=27, y=17, dest=nil },
            { x=21, y=18, dest=nil } } },
   }
   local STARTROOM = 11

   -- For debug use; not called.
   function dump_rooms(do_teledest)
      for i = 1, #physical_rooms do
         nh.pline('i = '..i)
         nh.pline('rm '..physical_rooms[i].id)
         nh.pline('tele '..#(physical_rooms[i].teleporters))
         for j = 1, #(physical_rooms[i].teleporters) do
            nh.pline('('..physical_rooms[i].teleporters[j].x..','
                     ..physical_rooms[i].teleporters[j].y..')')
            if (do_teledest) then
               if physical_rooms[i].teleporters[j].dest == nil then
                  nh.pline('d(nil)')
               else
                  nh.pline('d('..physical_rooms[i].teleporters[j].dest[1]
                           ..','..physical_rooms[i].teleporters[j].dest[2]..')')
               end
            end
         end
      end
   end

   -- These are the LOGICAL rooms, in a fixed map.
   -- 'x' = { 'y', 'z' } means room x connects to rooms y and z.
   local logical_rooms = {
      ['a'] = { 'b', 'c', 'd', 'e' }, -- fixed: start room
      ['b'] = { 'a', 'c', 'f' },
      ['c'] = { 'a', 'b', 'i' },
      ['d'] = { 'a', 'e', 'i' },
      ['e'] = { 'a', 'd', 'g' },
      ['f'] = { 'b', 'g', 'h' },
      ['g'] = { 'e', 'f', 'j' },
      ['h'] = { 'f', 'i', 'p' },
      ['i'] = { 'c', 'd', 'h', 'j' },
      ['j'] = { 'g', 'i', 'p' },
      ['k'] = { 'l', 'm', 'n' },
      ['l'] = { 'k', 'm', 'n' },
      ['m'] = { 'k', 'l', 'n', 'o' },
      ['n'] = { 'k', 'l', 'm', 'p' },
      ['o'] = { 'm', 'p', 'q' },
      ['p'] = { 'h', 'j', 'n', 'o', 'r' },
      ['q'] = { 'o', 'r', 's' },
      ['r'] = { 'p', 'q', 's' },
      ['s'] = { 'q', 'r', 't' },
      ['t'] = { 's' }, -- goal room, but not actually fixed; it is the only
                       -- 1-teleporter room; if other such rooms were added, any
                       -- of them could be the goal room
   }

   -- sanity check 1: that both maps have the same number of rooms
   local logicrm_ct = 0
   for dummy1, dummy2 in pairs(logical_rooms) do
      logicrm_ct = logicrm_ct + 1
   end
   if #physical_rooms ~= logicrm_ct then
      nh.impossible('different number of physical and logical rooms')
      return false
   end

   -- sanity check 2: that each physical room has the correct id number
   for i = 1, #physical_rooms do
      if physical_rooms[i].id ~= i then
         nh.impossible('room '..i..' has wrong id number')
         return false
      end
   end

   -- sanity check 3: the integrity of the logical room map, by confirming that
   -- every declared edge (x -> y) has a corresponding (y -> x)
   for rm1, connects in pairs(logical_rooms) do
      for dummy1, rm2 in ipairs(connects) do
         -- rm1 has an edge to rm2 -- does rm2 have an edge to rm1?
         local foundit = false
         if logical_rooms[rm2] == nil then
            nh.impossible('connection to nonexistent logical room '..rm2)
            return false
         end
         for dummy2, rm_tmp in ipairs(logical_rooms[rm2]) do
            if rm_tmp == rm1 then
               foundit = true
               break
            end
         end
         if not foundit then
            nh.impossible('logical rm '..rm1..' has edge to '..rm2
                          ..', but '..rm2..' does not have one back')
            return false
         end
         -- this doesn't handle the potential case of "double edges":
         -- rooms with multiple connections to some other room. Currently those
         -- don't exist in this maze.
      end
   end

   -- now put both rooms into separate arrays of IDs-by-degree
   local physrooms_by_deg = {}
   for i = 1, #physical_rooms do
      ntele = #(physical_rooms[i].teleporters)
      if (physrooms_by_deg[ntele] == nil) then
         physrooms_by_deg[ntele] = { physical_rooms[i].id }
      else
         table.insert(physrooms_by_deg[ntele], physical_rooms[i].id)
      end
   end
   local logicrooms_by_deg = {}
   for letter, connects in pairs(logical_rooms) do
      nconn = #connects
      if (logicrooms_by_deg[nconn] == nil) then
         logicrooms_by_deg[nconn] = { letter }
      else
         table.insert(logicrooms_by_deg[nconn], letter)
      end
   end

   -- sanity check 4: that physical and logical have the same amount of rooms
   -- with the same degrees
   for degree, dummy in pairs(physrooms_by_deg) do
      if #(physrooms_by_deg[degree]) ~= #(logicrooms_by_deg[degree]) then
         nh.impossible('degree mismatch: phys='..#(physrooms_by_deg[degree])
                       ..', logic='..#(logicrooms_by_deg[degree]))
         return false
      end
   end
   -- this combined with the previous sanity check ensuring an equal number of
   -- rooms ought to guard against logicrooms_by_deg having a key that
   -- physrooms_by_deg doesn't have (i.e. some room of degree 8), so we don't
   -- need to check the other way round.

   -- now start actually assigning logical rooms to physical ones
   -- log_to_phys['x'] stores the physical id of the room mapped to logical x
   local log_to_phys = {}
   for degree, connects in pairs(logicrooms_by_deg) do
      shuffle(connects)
      for i = 1, #connects do
         id_phys = physrooms_by_deg[degree][i]
         id_log = connects[i]
         log_to_phys[id_log] = id_phys
         physical_rooms[id_phys]['letter'] = id_log
      end
   end

   -- end room is treated the same as any other 1-degree room; but the logical
   -- start room 'a' must be room 11, so swap it if it's not
   if physical_rooms[STARTROOM].letter ~= 'a' then
      other_logid = physical_rooms[STARTROOM].letter
      other_physid = log_to_phys['a']

      log_to_phys['a'] = STARTROOM
      log_to_phys[other_logid] = other_physid

      physical_rooms[STARTROOM].letter = 'a'
      physical_rooms[other_physid].letter = other_logid
   end

   -- first shuffle the teleporters to avoid any bias in simple 1..n iteration
   for i = 1, #physical_rooms do
      shuffle(physical_rooms[i].teleporters)
   end

   -- now start assigning teleporters
   for i = 1, #physical_rooms do
      for j = 1, #(physical_rooms[i].teleporters) do
         local letter1 = physical_rooms[i].letter
         local tele1 = physical_rooms[i].teleporters[j]
         -- if nil, it has not yet been claimed
         if tele1.dest == nil then
            local logical_conns = logical_rooms[letter1]
            -- note: this is mutating (and eventually cleaning out)
            -- the elements of logical_rooms, eventually leaving all its table
            -- values as empty.
            local conn = table.remove(logical_conns)
            if conn == nil then
               nh.impossible('removing more elements from logical room '
                             ..letter1..' than exist')
               return false
            end
            local other_phys = log_to_phys[conn]
            for k = 1, #(physical_rooms[other_phys].teleporters) do
               local tele2 = physical_rooms[other_phys].teleporters[k]
               if tele2.dest == nil then
                  -- found a nil-dest teleporter in both rooms, assign it
                  tele1.dest = { tele2.x, tele2.y }
                  tele2.dest = { tele1.x, tele1.y }
                  -- also have to clean up the other half of this edge in
                  -- logical_rooms so that doesn't later attempt to make a
                  -- second teleporter into this room
                  local letter2 = physical_rooms[other_phys].letter
                  for idx, let in ipairs(logical_rooms[letter2]) do
                     if let == letter1 then
                        table.remove(logical_rooms[letter2], idx)
                        break
                     end
                  end
                  break
               end
            end
         end
      end
   end

   -- sanity check 5: we have used up every connection in logical_rooms
   for letter, connects in pairs(logical_rooms) do
      if #connects > 0 then
         nh.impossible('logical_rooms['..letter..'] still has connections')
         return false
      end
   end

   -- sanity check 6: the integrity of the final teleporter network
   local sanity_telemap = {}
   for i = 1, #physical_rooms do
      for j = 1, #(physical_rooms[i].teleporters) do
         local tele = physical_rooms[i].teleporters[j]
         if tele.dest == nil then
            nh.impossible('unassigned teleporter (rm='..i..', tele='..j..')')
            return false
         end
         sanity_telemap[(tele.x)..','..(tele.y)] = (tele.dest[1])..','..(tele.dest[2])
      end
   end
   for src, dest in pairs(sanity_telemap) do
      if sanity_telemap[dest] ~= src then
         nh.impossible('teleporter integrity: '..src..' points to '..dest
                       ..' but that one points to '..sanity_telemap[dest])
         return false
      end
   end

   -- finally, MAKE the teleporters
   for i = 1, #physical_rooms do
     for j = 1, #(physical_rooms[i].teleporters) do
        local tele = physical_rooms[i].teleporters[j]
        des.trap({ type = "teleport", x = tele.x, y = tele.y, seen = true,
                   teledest = tele.dest })
     end
   end

   return true
end

if not set_up_telemaze() then
   -- this level will not be passable; escalate to an uncaught impossible to
   -- invoke a default, hopefully passable, level
   nh.impossible("couldn't make teleport maze")
end

-- Insert level features unrelated to the teleporters and specific to particular
-- physical rooms after here.

-- The hole in room 15 is in case a player gets into this level without
-- completing the puzzle in the previous level - if they had only the ladder and
-- no means with which to make their own hole, they would only be able to get
-- into the small center chamber on wizard-3 and be otherwise trapped. With the
-- hole, they can fall into the outer area.
des.trap("hole", 21, 13)

-- the one-way "back to start" teleporter is special and doesn't get set
-- algorithmically
des.trap({ type = "teleport", coord = {17,16}, seen = true,
           teledest = { 18, 10 } })

-- some randomly placed traps (do these first so subsequent things that check
-- for has_trap will find them)
for i = 1,3 do
   des.trap('magic')
   des.trap('anti magic')
   des.trap(percent(50) and 'fire' or 'cold')
   -- polymorph traps would be thematic, but hero is likely to be immune to
   -- those at this point, and they would likely weaken rather than strengthen
   -- any monster in the maze that runs into one
   -- no trap doors either; those are nice at sending the hero "back to start"
   -- but they're more likely to hit the monsters than the hero. Potential
   -- future improvement is properly implementing mtrapseen and making sure
   -- every monster created knows about the trap doors (or just all traps on the
   -- level).
end

-- lich in room 17
local lichtemplate = { x = 33, y = 13, waiting = true, inventory = function()
   des.object('+')
   des.object('+')
   des.object('?')
end }
if not nh.is_genocided('master lich') then
   lichtemplate['id'] = 'master lich'
else
   lichtemplate['class'] = 'L'
end
des.monster(lichtemplate)

-- dragon and dragon statues in room 3
local dragontemplate = { x = 07, y = 05, waiting = true }
if not nh.is_genocided('black dragon') then
   dragontemplate['id'] = 'black dragon'
else
   dragontemplate['class'] = 'D'
end
des.monster(dragontemplate)
local dragoncolors = { 'red', 'blue', 'green', 'white', 'black' } -- chaotics
function dragon_statue(x, y)
   des.object({ id = 'statue', coord = {x, y},
                montype = dragoncolors[d(#dragoncolors)]..' dragon',
                trapped = percent(50),
                material = 'gold' })
end
dragon_statue(05, 05)
dragon_statue(09, 05)

-- minotaurs in room 14
des.monster({ id = 'minotaur', coord = {13,14}, waiting = true,
              keep_default_invent = false})
des.monster({ id = 'minotaur', coord = {14,14}, waiting = true,
              keep_default_invent = false})
des.monster({ id = 'minotaur', coord = {15,14}, waiting = true,
              keep_default_invent = false})

-- gargoyle statues in room 10
function garg_statue(x, y)
   des.object({ id = 'statue', coord = {x,y}, material = 'stone',
                montype = 'winged gargoyle',
                trapped = percent(50) })
end
garg_statue(01,11)
garg_statue(07,11)
garg_statue(04,10)
garg_statue(04,12)
garg_statue(06,09)
garg_statue(06,13)
des.monster({ id = 'winged gargoyle', coord = {05,11}, waiting = true })

-- spheres in room 4
des.monster({ class = 'e', coord = {17,04}, waiting = true })
des.monster({ class = 'e', coord = {17,05}, waiting = true })
des.monster({ class = 'e', coord = {17,06}, waiting = true })
des.monster({ class = 'e', coord = {16,04}, waiting = true })
des.monster({ class = 'e', coord = {16,05}, waiting = true })

-- webs/spiders in room 20
selection.floodfill(24,17):iterate(function(x, y)
   local loc = nh.getmap(x, y)
   if not loc.has_trap then
      if percent(40) then
         des.monster({ id = 'giant spider', coord = {x, y}, waiting = true })
      elseif percent(40) then
         des.trap({ type = 'web', coord = {x, y}, spider_on_web = false })
      end
   end
end)

-- nymphs in room 8
selection.floodfill(24,07):iterate(function(x, y)
   local loc = nh.getmap(x, y)
   if not loc.has_trap then
      if percent(25) then
         des.monster({ class = 'n', coord = {x, y}, sleeping = false,
                       waiting = true, invisible = true })
      end
   end
end)

-- demons in room 7
des.monster({ class='&', coord = {12,07}, waiting = true })
des.monster({ class='&', coord = {13,08}, waiting = true })
des.monster({ class='&', coord = {13,09}, waiting = true })
des.monster({ class='&', coord = {13,10}, waiting = true })
des.monster({ class='&', coord = {12,11}, waiting = true })

-- mirrors in room 18
des.object('mirror', 04,16)
des.object('mirror', 07,17)
des.object('mirror', 10,18)
if percent(20) then
   des.monster({ id = 'silver dragon', coord = {05,15}, waiting = true })
end

-- potential randomly placed graffiti
if percent(100) then -- TODO: make 20
   des.engraving({ type = 'engrave', degrade = true,
                   text = 'You are in a maze of twisty little passages... '
                          ..'all leading to doom!' })
end

-- Rejected teleport maze algorithms:
-- 1. Make 3-4 groupings of random rooms not including start or goal, which have
--    high interconnectivity within the group but limited interconnectivity
--    outside it (and only directly connect to the next and prev groups). Player
--    must proceed from one group to the next and only the final group has the
--    teleporter to the goal.
-- 2. Make a rectangular graph of some size where each node connects to its
--    immediate neighbors (and optionally the graph represents a torus, so edge
--    nodes connect to their neighbors on the other side too). The original size
--    is 5x5, 6x5, 7x7 - depends on how many rooms there will be. Define one
--    node to be the start and another node a certain minimum distance away to
--    be the goal. Remove rooms randomly, so long as they are not the start/goal
--    and do not disconnect the graph.
-- 3. As above, but use a hexagonal graph. (Drawback: any room must be large
--    enough to support 6 teleporters in it.)
-- 4. Start and end rooms must have only 2 teleporters. Generate a completely
--    random graph without those 2 nodes, where every other node is of degree
--    3-5. Find the pair of edges with the highest distance between them, and
--    insert the start and end room into the "middle" of each.
