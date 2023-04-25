-- Demogorgon's lair

des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel", "noflip", "noteleport", "nommap-boss");

--2345678901234567890123456789012345678901234567890123456789012345678901234567890
des.map([[
---......-------......---------......--------.....---------......------   
|.........|...|.........|...|.........|...|.........|...|.........|...|   
|.........|...|.........|...|.........|...|.........|...|.........|...|   
|.........|...|.........|...|.........|...|.........|...|.........|...|   
---......-------......--------......--------......---------......-------- 
  --------.....--------......--------......--------.......--------......--
   |...|.........|...|.........|...|.........|...|.........|...|.........|
   |...|.........|...|.........|...|.........|...|.........|...|.........|
   |...|.........|...|.........|...|.........|...|.........|...|.........|
 --------......--------......---------......--------.....---------.....---
--......--------......--------.......--------......-------.......-------  
|.........|...|.........|...|.........|...|.........|...|.........|...|   
|.........|...|.........|...|.........|...|.........|...|.........|...|   
|.........|...|.........|...|.........|...|.........|...|.........|...|   
--......---------......-------......---------.....--------......--------- 
 --------.......--------.....--------.......-------......--------.......--
   |...|.........|...|.........|...|.........|...|.........|...|.........|
   |...|.........|...|.........|...|.........|...|.........|...|.........|
   |...|.........|...|.........|...|.........|...|.........|...|.........|
   ------......---------.....---------......--------.....---------......--
]]);

-- Rooms are: 1-10 first row, 11-20 second, 21-30 third, 31-40 fourth
-- Other fields that are used below but left nil now:
-- group = after room connections are set up, this is the id of the
--         lowest-numbered room that can be reached from this room without using
--         any keys.
local rooms = {
   { id= 1, big=true,  cx=05, cy=02 }, -- top row
   { id= 2, big=false, cx=12, cy=02 },
   { id= 3, big=true,  cx=19, cy=02 },
   { id= 4, big=false, cx=26, cy=02 },
   { id= 5, big=true,  cx=33, cy=02 },
   { id= 6, big=false, cx=40, cy=02 },
   { id= 7, big=true,  cx=47, cy=02 },
   { id= 8, big=false, cx=54, cy=02 },
   { id= 9, big=true,  cx=61, cy=02 },
   { id=10, big=false, cx=68, cy=02 },
   { id=11, big=false, cx=05, cy=07 }, -- upper middle row
   { id=12, big=true , cx=12, cy=07 },
   { id=13, big=false, cx=19, cy=07 },
   { id=14, big=true , cx=26, cy=07 },
   { id=15, big=false, cx=33, cy=07 },
   { id=16, big=true , cx=40, cy=07 },
   { id=17, big=false, cx=47, cy=07 },
   { id=18, big=true , cx=54, cy=07 },
   { id=19, big=false, cx=61, cy=07 },
   { id=20, big=true , cx=68, cy=07 },
   { id=21, big=true,  cx=05, cy=12 }, -- lower middle row
   { id=22, big=false, cx=12, cy=12 },
   { id=23, big=true,  cx=19, cy=12 },
   { id=24, big=false, cx=26, cy=12 },
   { id=25, big=true,  cx=33, cy=12 },
   { id=26, big=false, cx=40, cy=12 },
   { id=27, big=true,  cx=47, cy=12 },
   { id=28, big=false, cx=54, cy=12 },
   { id=29, big=true,  cx=61, cy=12 },
   { id=30, big=false, cx=68, cy=12 },
   { id=31, big=false, cx=05, cy=17 }, -- bottom row
   { id=32, big=true , cx=12, cy=17 },
   { id=33, big=false, cx=19, cy=17 },
   { id=34, big=true , cx=26, cy=17 },
   { id=35, big=false, cx=33, cy=17 },
   { id=36, big=true , cx=40, cy=17 },
   { id=37, big=false, cx=47, cy=17 },
   { id=38, big=true , cx=54, cy=17 },
   { id=39, big=false, cx=61, cy=17 },
   { id=40, big=true , cx=68, cy=17 }
}
local TOTAL_ROOMS = #rooms

-- Connections between each room in the form
-- rm1,rm2 = indexes of connected rooms
-- x,y = coords of a door that will join these rooms if placed
-- Other fields that are used below but left nil now:
-- isdoor = boolean, whether this conn actually exists as a door
-- needkey = boolean, whether this conn is a special door that requires a key
local conns = {
   -- top row
   { rm1= 1, rm2= 2, x=10, y=02 },
   { rm1= 2, rm2= 3, x=14, y=02 },
   { rm1= 3, rm2= 4, x=24, y=02 },
   { rm1= 4, rm2= 5, x=28, y=02 },
   { rm1= 5, rm2= 6, x=38, y=02 },
   { rm1= 6, rm2= 7, x=42, y=02 },
   { rm1= 7, rm2= 8, x=52, y=02 },
   { rm1= 8, rm2= 9, x=56, y=02 },
   { rm1= 9, rm2=10, x=66, y=02 },
   -- top row to upper middle row
   { rm1= 1, rm2=11, x=05, y=05 },
   { rm1= 3, rm2=13, x=19, y=05 },
   { rm1= 5, rm2=15, x=33, y=05 },
   { rm1= 7, rm2=17, x=47, y=05 },
   { rm1= 9, rm2=19, x=61, y=05 },
   { rm1= 2, rm2=12, x=12, y=04 },
   { rm1= 4, rm2=14, x=26, y=04 },
   { rm1= 6, rm2=16, x=40, y=04 },
   { rm1= 8, rm2=18, x=54, y=04 },
   { rm1=10, rm2=20, x=68, y=04 },
   { rm1= 1, rm2=12, x=08, y=05 }, -- diagonal connections start here
   { rm1= 3, rm2=12, x=16, y=05 },
   { rm1= 3, rm2=14, x=23, y=04 },
   { rm1= 5, rm2=14, x=30, y=05 },
   { rm1= 5, rm2=16, x=37, y=04 },
   { rm1= 7, rm2=16, x=44, y=05 },
   { rm1= 7, rm2=18, x=51, y=04 },
   { rm1= 9, rm2=18, x=57, y=04 },
   { rm1= 9, rm2=20, x=64, y=05 },
   -- upper middle row
   { rm1=11, rm2=12, x=07, y=07 },
   { rm1=12, rm2=13, x=17, y=07 },
   { rm1=13, rm2=14, x=21, y=07 },
   { rm1=14, rm2=15, x=31, y=07 },
   { rm1=15, rm2=16, x=35, y=07 },
   { rm1=16, rm2=17, x=45, y=07 },
   { rm1=17, rm2=18, x=49, y=07 },
   { rm1=18, rm2=19, x=59, y=07 },
   { rm1=19, rm2=20, x=63, y=07 },
   -- upper middle row to lower middle row
   { rm1=11, rm2=21, x=05, y=09 },
   { rm1=13, rm2=23, x=19, y=09 },
   { rm1=15, rm2=25, x=33, y=09 },
   { rm1=17, rm2=27, x=47, y=09 },
   { rm1=19, rm2=29, x=61, y=09 },
   { rm1=12, rm2=22, x=12, y=10 },
   { rm1=14, rm2=24, x=26, y=10 },
   { rm1=16, rm2=26, x=40, y=10 },
   { rm1=18, rm2=28, x=54, y=10 },
   { rm1=20, rm2=30, x=68, y=10 },
   { rm1=12, rm2=21, x=09, y=10 }, -- diagonal connections start here
   { rm1=12, rm2=23, x=16, y=09 },
   { rm1=14, rm2=23, x=23, y=10 },
   { rm1=14, rm2=25, x=30, y=09 },
   { rm1=16, rm2=25, x=36, y=09 },
   { rm1=16, rm2=27, x=43, y=10 },
   { rm1=18, rm2=27, x=50, y=09 },
   { rm1=18, rm2=29, x=58, y=09 },
   { rm1=20, rm2=29, x=64, y=09 },
   -- lower middle row
   { rm1=21, rm2=22, x=10, y=12 },
   { rm1=22, rm2=23, x=14, y=12 },
   { rm1=23, rm2=24, x=24, y=12 },
   { rm1=24, rm2=25, x=28, y=12 },
   { rm1=25, rm2=26, x=38, y=12 },
   { rm1=26, rm2=27, x=42, y=12 },
   { rm1=27, rm2=28, x=52, y=12 },
   { rm1=28, rm2=29, x=56, y=12 },
   { rm1=29, rm2=30, x=66, y=12 },
   -- lower middle row to bottom row
   { rm1=21, rm2=31, x=05, y=15 },
   { rm1=23, rm2=33, x=19, y=15 },
   { rm1=25, rm2=35, x=33, y=15 },
   { rm1=27, rm2=37, x=47, y=15 },
   { rm1=29, rm2=39, x=61, y=15 },
   { rm1=22, rm2=32, x=12, y=14 },
   { rm1=24, rm2=34, x=26, y=14 },
   { rm1=26, rm2=36, x=40, y=14 },
   { rm1=28, rm2=38, x=54, y=14 },
   { rm1=30, rm2=40, x=68, y=14 },
   { rm1=21, rm2=32, x=09, y=14 }, -- diagonal connections start here
   { rm1=23, rm2=32, x=15, y=14 },
   { rm1=23, rm2=34, x=22, y=15 },
   { rm1=25, rm2=34, x=30, y=15 },
   { rm1=25, rm2=36, x=37, y=14 },
   { rm1=27, rm2=36, x=43, y=14 },
   { rm1=27, rm2=38, x=51, y=14 },
   { rm1=29, rm2=38, x=58, y=15 },
   { rm1=29, rm2=40, x=65, y=14 },
   -- bottom row
   { rm1=31, rm2=32, x=07, y=17 },
   { rm1=32, rm2=33, x=17, y=17 },
   { rm1=33, rm2=34, x=21, y=17 },
   { rm1=34, rm2=35, x=31, y=17 },
   { rm1=35, rm2=36, x=35, y=17 },
   { rm1=36, rm2=37, x=45, y=17 },
   { rm1=37, rm2=38, x=49, y=17 },
   { rm1=38, rm2=39, x=59, y=17 },
   { rm1=39, rm2=40, x=63, y=17 }
}
local TOTAL_CONNS = #conns

-- sanity checks
for i = 1, TOTAL_ROOMS do
   if rooms[i].id ~= i then
      nh.impossible('Room number '..i..' has a wrong id')
   end
end
for j = 1, TOTAL_CONNS do
   if conns[j].rm1 >= conns[j].rm2 then
      -- As set up, connections always have the lower room number first.
      -- There's no particular reason why this has to be obeyed, but might as
      -- well enforce the assumption.
      nh.impossible('Bad ordering in connection '..j..': '..conns[j].rm1..' to '..conns[j].rm2)
   end
   if ((conns[j].rm2 - 1) // 10) - ((conns[j].rm1 - 1) // 10) > 1 then
      nh.impossible('Rooms connected to completely separate rows '..conns[j].rm1..' to '..conns[j].rm2)
   end
   if rooms[conns[j].rm1].big == false and rooms[conns[j].rm2].big == false then
      nh.impossible('Connection between two small rooms '..conns[j].rm1..' '..conns[j].rm2)
   end
end

-- Possible evil addition: add 5 connections that work by way of 2
-- specific-coord teleport traps between two randomly chosen 3x3 rooms,
-- bidirectional to each other, thereby making traversing the level not a
-- Euclidean plane anymore. The hardest room to access could be right outside
-- the starting room.

-- Step 1: generate spanning tree of rooms
rooms[1].reached = true
local nreached = 1
while nreached < TOTAL_ROOMS do
   -- since we don't init reached to false, this will check true/true, true/nil
   -- or nil/nil; should work anyways
   local pick
   repeat
      pick = d(TOTAL_CONNS)
   until rooms[conns[pick].rm1].reached ~= rooms[conns[pick].rm2].reached
   -- now mark both rooms as reached
   rooms[conns[pick].rm1].reached = true
   rooms[conns[pick].rm2].reached = true
   -- and mark the connection
   conns[pick].isdoor = true
   nreached = nreached + 1
end

-- Step 2: add a bunch of connections that weren't in the spanning tree
for k = 1, 20 do
   local pick
   repeat
      pick = d(TOTAL_CONNS)
   until conns[pick].isdoor == nil
   -- mark the connection
   conns[pick].isdoor = true
end

-- Step 3: randomly select doors to be "demogorgon special" (iron; see
-- demogorgon_special_door() ) which will require keys to open. Doors can be
-- created now
for j = 1, TOTAL_CONNS do
   -- Doors that require special keys to unlock should always be iron; doors that
   -- don't should always be wood. This helps the player visually read the level
   -- better.
   local doiron = false
   conns[j].needkey = false
   if conns[j].rm1 ~= 1 and percent(75) then
      conns[j].needkey = true
   end
   if conns[j].isdoor ~= nil then
      des.door({ state='locked', coord={conns[j].x,conns[j].y},
                 iron=(conns[j].needkey), secret=false })
   end
end

-- Step 4: do a search and assign rooms to groups which are all mutually
-- reachable without any use of keys.
function create_room_group(curr_rm, groupid)
   rooms[curr_rm].group = groupid
   for j = 1, TOTAL_CONNS do
      if conns[j].isdoor and not conns[j].needkey then
         if conns[j].rm1 == curr_rm and rooms[conns[j].rm2].group == nil then
            create_room_group(conns[j].rm2, groupid)
         end
         if conns[j].rm2 == curr_rm and rooms[conns[j].rm1].group == nil then
            create_room_group(conns[j].rm1, groupid)
         end
      end
   end
end
local somedonthavegroup = false
repeat
   somedonthavegroup = false
   for i = 1, TOTAL_ROOMS do
      if rooms[i].group == nil then
         somedonthavegroup = true -- here's one with no group!
         create_room_group(rooms[i].id, rooms[i].id)
      end
   end
until somedonthavegroup == false
-- another sanity check: enforce the assumption that a room's group id is
-- always that of a room lower or equal to it, never higher
for i = 1, TOTAL_ROOMS do
   if rooms[i].group == nil then
      nh.impossible('Room '..i..' has nil group')
   end
   if rooms[i].group > rooms[i].id then
      nh.impossible('Room '..i..' has higher group id '..rooms[i].group)
   end
end

-- Step 5: randomly choose Demogorgon's room from a preset list
local demoroom = nil
local demorooms = { 7, 9, 18, 20, 27, 29, 36, 38, 40 }
demoroom = demorooms[d(#demorooms)]
des.region({ region={rooms[demoroom].cx, rooms[demoroom].cy, rooms[demoroom].cx, rooms[demoroom].cy},
             irregular=true, type="throne", filled=0, lit=1 })

-- Step 6: fill other rooms with stuff
for i = 1, TOTAL_ROOMS do
   if i ~= 1 and i ~= demoroom then -- skip these two special rooms
      local choice = d(14)
      local cx = rooms[i].cx
      local cy = rooms[i].cy
      local room = selection.floodfill(cx, cy)
      if choice == 1 then
         -- hole in the middle (big rooms only)
         if rooms[i].big then
            local air = selection.area(cx-2, cy-1, cx+2, cy+1)
            air:set(cx-3, cy)
            air:set(cx+3, cy)
            des.terrain(air, 'A')
         end
      elseif choice == 2 then
         -- lava in the middle
         local lava = selection.new()
         if rooms[i].big and percent(80) then
            lava = selection.area(cx-2, cy-1, cx+2, cy+1)
            lava:set(cx-3, cy)
            lava:set(cx+3, cy)
            des.terrain(lava, 'L')
         else
            lava:set(cx, cy)
            des.terrain(lava, 'L')
         end
         for i = 1, 1 + (rooms[i].big and d(3) or 0) do
            des.monster({ id='salamander', coord=lava:rndcoord() })
         end
      elseif choice == 3 then
         if percent(30) then
            -- mixed lava, water, and steam
            local sel = room:clone():percentage(60)
            des.terrain(sel, '}')
            sel = sel:percentage(50)
            des.terrain(sel, 'L')
            for i = 1, 2 + d(3) do
               des.monster({ id='steam vortex', coord=room:rndcoord(1) })
            end
         end
      elseif choice == 4 then
         -- wall in the middle
         if rooms[i].big then
            des.terrain(selection.line(cx-2,cy, cx+2,cy), '-')
            des.terrain(selection.line(cx,cy-1, cx,cy+1), '-')
         else
            des.terrain(cx, cy, '-')
         end
      elseif choice == 5 then
         -- boulders
         local bould = room:clone():percentage(70)
         room:iterate(function(x,y)
            des.object('boulder', x, y)
         end)
      elseif choice == 6 then
         -- demons
         for i = 1, d(5) do
            des.monster({ class='&', align=percent(50) and 'chaos' or 'random', coord=room:rndcoord() })
         end
      elseif choice == 7 then
         -- ghosts/undead
         local undead = { {id='ghost'}, {id='shade'}, {class='Z'}, {class='M'},
                         {class='L'}, {class='V'}, {class='W'} }
         for i = 1, d(8) do
            local tmplt = undead[d(#undead)]
            tmplt['coord'] = room:rndcoord()
            des.monster(tmplt)
         end
      elseif choice == 8 or choice == 9 then
         -- fake key chest
         des.object({ id='chest', locked=true, trapped=percent(80), coord=room:rndcoord(),
                      contents=function() end })
      end
   end
end

-- Step 7: place on average 1.2 bone keys (minimum 1, maximum 2) somewhere
-- within every group
-- (this should come after terrain alterations so that we know where the good
-- floor spaces are)
for i = 1, TOTAL_ROOMS do
   if rooms[i].group == rooms[i].id then
      local currgrp = rooms[i].group
      local floor_in_grp = selection.new()
      -- loop over all rooms and construct a selection with all the . spaces in
      -- this group
      for m = 1, TOTAL_ROOMS do
         if rooms[m].group == currgrp then
            if rooms[m].big then
               -- this only works because we can make assumptions about the
               -- construction and interlocking of rooms; if rooms are ever
               -- given weirder shapes this won't work
               floor_in_grp = floor_in_grp + selection.area(rooms[m].cx-4, rooms[m].cy-2,
                                                            rooms[m].cx+4, rooms[m].cy+2):filter_mapchar('.')
            else
               floor_in_grp = floor_in_grp + selection.area(rooms[m].cx-1, rooms[m].cy-1,
                                                            rooms[m].cx+1, rooms[m].cy+1):filter_mapchar('.')
            end
         end
      end
      local makekey = function()
         des.object({ id='key', material='bone' })
      end
      for k = 1, (percent(20) and 2 or 1) do
         if percent(10) then
            -- have a monster carry the key
            des.monster({ class='&', align='chaos', coord=floor_in_grp:rndcoord(),
                          inventory=function()
               makekey()
            end })
         else
            des.object({ id='chest', locked=true, trapped=percent(80),
                         coord=floor_in_grp:rndcoord(), contents=function()
               makekey()
            end })
         end
      end
   end
end

-- then the wand of wishing, if any
local wisharea = (selection.area(64,05,72,19) + selection.area(57,10,65,19)
                  + selection.area(50,15,58,19)):filter_mapchar('.')
des.object({ id='chest', material='gold', coord=wisharea:rndcoord(), contents=function()
   if nh.is_wish_dlord('Demogorgon') then
      des.object({ class='/', id='wishing', spe=1 })
   else
      if percent(10) then
         des.object('magic lamp') -- even if not a wish dlord
      elseif percent(30) then
         des.object('magic marker')
      elseif percent(30) then
         des.object({ class='/', id='death' })
      else
         des.object({ id='gold piece', quan=1 })
      end
   end
end })

-- now random level wide stuff
-- monsters:
for i = 1, 12 do
   des.monster({ class='&', align=percent(80) and 'chaos' or 'random' })
end
for i = 1, 5 do
   des.monster('i')
   des.monster('Y')
   des.monster()
end
for i = 1, 3 do
   des.monster('W')
   des.monster('Z')
end

-- objects:
for i = 1, 20 do
   des.gold()
end
for i = 1, 10 do
   des.object()
end

-- traps:
for i = 1, 30 do
   des.trap()
end

des.levregion({ region={05,02,05,02}, type="branch" })

des.non_diggable()
des.non_passwall()

