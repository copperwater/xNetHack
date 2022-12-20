-- NetHack yendor wizard2.lua	$NHDT-Date: 1652196039 2022/05/10 15:20:39 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.2 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style="mazegrid", bg ="-" });
des.level_flags("mazelevel", "noteleport", "nommap", "noflip")
des.map([[
            -------------            
        -----...........-----        
     ----...+...........+...----     
   ---....---...........---....---   
 ---...----.|...........|.|---...--- 
 |...---.|..|--------------..---...| 
--+---...|..|..|...............---+--
|....|.-+-------+-------------..|...|
|....|.|.........|...|.......--.|...|
|....|.|.........|.|.|........|.|...|
|....|.|.........|.|.|........|.|...|
|....|+-------------.-+---+----.|...|
|....|........+......|...|.---..|...|
--+---........|------|...|.-...---+--
 |...---..----|......+...|...---...| 
 ---...----...-+--------------...--- 
   ---....----|.........|--....---   
     ----.....|.........+...----     
        -----.|.........-----        
            -------------            
]]);
des.teleport_region({ region={18,08,20,10}, dir="up" })
des.region({ region={01,01, 26,11}, lit=0, type="ordinary", arrival_room=true })
des.ladder("up", 13,18)
des.ladder("down", 18,10)

-- Non diggable/phaseable walls everywhere
des.non_diggable()
des.non_passwall()

-- Doors
function maybelocked()
   return percent(30) and 'locked' or 'closed'
end
des.door(maybelocked(), 12,02)
des.door(maybelocked(), 24,02)
des.door(maybelocked(), 02,06)
des.door(maybelocked(), 34,06)
des.door(maybelocked(), 08,07)
des.door(maybelocked(), 16,07)
des.door(maybelocked(), 06,11)
des.door(maybelocked(), 22,11)
des.door(maybelocked(), 26,11)
des.door(maybelocked(), 14,12)
des.door(maybelocked(), 02,13)
des.door(maybelocked(), 34,13)
des.door(maybelocked(), 21,14)
des.door(maybelocked(), 15,15)
des.door(maybelocked(), 24,17)

-- Large rooms
local largerooms = { {13,01,23,04}, {01,07,05,12}, {15,16,23,18} }
shuffle(largerooms)
-- large room 1: zoo
des.region({ region=largerooms[1], lit=0, type="zoo", filled=1 })
-- large room 2: wolves
local wolves = { 'wolf', 'warg', 'hell hound', 'winter wolf' }
selection.area(largerooms[2][1],largerooms[2][2],
               largerooms[2][3],largerooms[2][4]):iterate(function(x,y)
                  if percent(30) then
                     des.monster(wolves[d(#wolves)], x, y)
                  end
                  if percent(2) then
                     des.object('tripe ration', x, y)
                  end
               end)
-- third large room is empty

-- Medium rooms
local mediumrooms = { {x=11,y=09}, {x=26,y=09}, {x=34,y=10}, {x=09,y=13} }
shuffle(mediumrooms)
-- medium room 1: elementals
selection.floodfill(mediumrooms[1].x, mediumrooms[1].y):iterate(function(x,y)
   if percent(40) then
      des.monster('E', x, y)
   end
end)
-- medium room 2: beehive
des.region({ region={mediumrooms[2].x,mediumrooms[2].y, mediumrooms[2].x,mediumrooms[2].y},
             irregular=1, lit=0, type="beehive", filled=1 })
-- medium room 3: demons
selection.floodfill(mediumrooms[3].x, mediumrooms[3].y):iterate(function(x,y)
   if percent(40) then
      des.monster('&', x, y)
   end
end)
-- medium room 4 has a couple trap doors
selection.floodfill(mediumrooms[4].x, mediumrooms[4].y):iterate(function(x,y)
   if percent(10) then
      des.trap('trap door', x, y)
   end
end)

-- Small room
des.monster('L', 23,13)

-- Tiny secret loot rooms
local lootspots = { {11,05}, {14,06}, {25,04}, {12,15} }
local loot = { '!', '?', '+', '"' }
local doorspots = {
   -- possible doors for each room; 1 from each row will be picked
   { {11,03}, {09,05}, {12,04}, {11,07} },
   { {12,06}, {14,05}, {13,07}, {15,06} },
   { {24,04}, {25,03}, {25,05} },
   { {13,16}, {11,14}, {13,14} }
}
shuffle(lootspots)
shuffle(loot)
for i = 1, 4 do
   des.object(loot[i], lootspots[i])
   shuffle(doorspots[i])
   des.door({ state='secret', x=doorspots[i][1][1], y=doorspots[i][1][2] })
end

-- Random traps.
des.trap("spiked pit")
des.trap("sleep gas")
des.trap("anti magic")
des.trap("magic")
des.trap('trap door')
des.trap('trap door')
for i = 1, 4 do
   des.trap()
end

