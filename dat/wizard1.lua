-- NetHack yendor wizard1.lua	$NHDT-Date: 1652196039 2022/05/10 15:20:39 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.3 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
--
-- The top (real) wizard level.
des.level_init({ style="solidfill", fg=" " });

des.level_flags("mazelevel", "noteleport", "nommap", "noflip")
des.map([[
            -------------            
        -----LL.......LL-----        
     ----.|LLL.........LLL|.----     
   ---....|--...........---..|.---   
 ---......|.-------------....|...--- 
 |.|.....--....|.....|..|....|.....| 
--.------------|.....|..|------....--
|.........|....|.....|..|.....|.....|
|---...-----...|.....|..|.....|-----|
|..----|...|...|.....|--|...---.|...|
|...|..|...-----------..|...|...|...|
|...|..|.....|.........-----|.------|
|----..|..--------------....|.|.....|
--..-------......|.|..|.....|.|....--
 |.....|..|......|.|..|..-------...| 
 ---...|..-------|.---|..|.....|.--- 
   ---.|.........|....-----....---   
     ----...-----------...|.----     
        -----...........-----        
            -------------            
]]);

-- This uses an algorithm similar to the "Super Honeycomb" themed room to form a
-- randomized spanning tree of rooms (except connected by secret doors, rather
-- than deleting walls). First we define all possible "edges" in terms of the
-- rooms; each entry {a,b, p,q,r,s} is:
-- a,b = room IDs of the two rooms sharing this edge. (This is not the internal
--       room number tracked by the game; it is just something to keep track of
--       the blob of floor spaces. The arbitrary room numbering system is
--       obtained by traversing the level left-to-right and then down, so the
--       Wizard's chamber at the top is 0, the room in the top left is 1, etc.
--                   -------------            
--               -----...........-----        
--            ----.|.......0.......|.----     
--          ---....|--...........---..|.---   
--        ---..1...|.-------------..2.|...--- 
--        |.|.....--.5..|.....|..|....|..3..| 
--       --.------------|.....|7.|------....--
--       |....4....|....|..6..|..|.....|.....|
--       |---...-----.8.|.....|..|.9...|-----|
--       |..----|...|...|.....|--|...---.|...|
--       |10.|..|.11-----------..|...|.12|.13|
--       |...|14|.....|....15...-----|.------|
--       |----..|..--------------....|.|.....|
--       --..-------..19..|.|21|.16..|.|.17.--
--        |..18.|..|......|.|..|..-------...| 
--        ---...|..-------|.---|..|..23.|.--- 
--          ---.|..22.....|20..-----....---   
--            ----...-----------...|.----     
--               -----.....24....-----        
--                   -------------            
-- p,q,r,s = Coordinates that define a line (x1,y1,x2,y2, as in
-- selection.line()) that represents the set of possible locations for a secret
-- door connecting room a with room b. If p == r and q == s there is only one
-- possible spot to place the door. The expectation is that either p == q or
-- r == s for every connection (ensuring an orthogonal line).
--
-- Room 0, the Wizard's chamber, is left out by default, since Lua table
-- operations usually start at 1, and things like d(# of rooms) will not select
-- it. It should always be a dead end on the spanning tree (i.e. having only 1
-- entrance), so it will get added afterwards.
local conns = {
   { 1, 5, 10,04,10,04},
   { 2, 3, 29,03,29,05},
   { 1, 4, 03,05,03,05},
   { 2, 7, 24,05,24,05},
   { 5, 8, 11,06,14,06},
   { 2, 9, 25,06,28,06},
   { 4, 8, 10,07,10,07},
   { 6, 8, 15,07,15,09},
   { 6, 7, 21,05,21,08},
   { 7, 9, 24,07,24,08},
   { 4,10, 01,08,02,08},
   { 4,11, 08,08,09,08},
   { 3,12, 31,08,31,08},
   { 3,13, 33,08,35,08},
   { 7,15, 22,09,23,09},
   { 9,12, 29,09,29,09},
   {10,14, 04,10,04,10},
   {11,14, 07,10,07,12},
   { 8,15, 14,10,14,10},
   {11,15, 13,11,13,11},
   {13,17, 33,11,35,11},
   {11,19, 11,12,12,12},
   {15,21, 20,12,21,12},
   {12,16, 28,12,28,13},
   {12,17, 30,12,30,13},
   {11,22, 08,13,09,13},
   {19,20, 17,13,17,14},
   {16,21, 22,13,22,14},
   {18,22, 07,14,07,16},
   {19,22, 11,15,16,15},
   {23,17, 31,15,31,15},
   {22,24, 16,17,16,17},
   {20,24, 21,17,21,17},
   {23,24, 26,17,26,17}
}

local TOTAL_ROOMS = 24 -- still not counting Wizard's
local reached = { }
for i = 1,TOTAL_ROOMS do
   reached[i] = false
end

-- pick 1 initial room to build spanning tree from
local initroom = d(TOTAL_ROOMS)
reached[initroom] = true
local nreached = 1
while nreached < TOTAL_ROOMS do
   local pick = d(#conns)
   while reached[conns[pick][1]] == reached[conns[pick][2]] do
      pick = d(#conns)
   end

   -- take a random point from the line and make it a secret door
   local cc = selection.line(conns[pick][3],conns[pick][4],
                             conns[pick][5],conns[pick][6]):rndcoord()
   des.door({ state='secret', coord=cc })

   -- update reached; both rooms are now reachable (one already was)
   reached[conns[pick][1]] = true
   reached[conns[pick][2]] = true
   nreached = nreached + 1
end

-- Now connect the Wizard's chamber.
-- Note that these doors are placed such that the player cannot hit the Wizard
-- with a ray while standing in or behind it.
wizdoors = { {14,04},{16,04},{20,04},{22,04} }
des.door({ state="secret", coord=wizdoors[d(#wizdoors)] })

-- Define a few areas
-- All the walkable floor
local allfloor = selection.area(00,00,36,19):filter_mapchar('.')
-- All the walkable floor minus Wizard chamber and chamber with downstairs ladder
local mostfloor = allfloor - selection.area(13,01,23,03) - selection.floodfill(14,18)
-- That minus 1-wide hallways
local interooms = mostfloor - selection.floodfill(11,04) - selection.floodfill(14,11)
                            - selection.floodfill(30,10) - selection.floodfill(18,14)
-- the Wizard chamber
local wizchamber = selection.floodfill(14,18)

des.non_diggable()
des.non_passwall()

-- entry point and region to constrain monster arrival
des.ladder("down", 13,18)
des.region({ region={13,18, 23,18}, type="ordinary", arrival_room=true })
des.teleport_region({ region={13,18, 23,18} })

-- the Wizard himself
des.monster({ id = "Wizard of Yendor", x=18, y=01, waiting=1 })
des.monster({ class = 'V', x=16, y=01, waiting=1 })
des.monster({ id = 'hell hound', x=20, y=01, waiting = 1 })
-- his treasure
des.object("Book of the Dead", 18, 01)

-- Back when the Wizard was still "asleep", these guys often cast aggravate
-- monsters and woke him up nearly immediately, but now that he's awake and
-- waiting he doesn't seem to be affected.
local strongcasters= { 'L', 'L', 'titan', 'golden naga', 'nalfeshnee', 'barrow-wight' }

-- Define a number of irregular regions. They and their contents may be placed
-- in any of the interior rooms, on or off the correct path to the Wizard.
-- These tables also accept an "filltyp" key whose value is a function(x,y)
-- which will be iterated over the same area of the region via selection.
--
-- Level-wide things (like a monster where we don't care what room it's in)
-- don't need to go here. This is for rooms that should contain multiple
-- thematic contents.
local regions = {
   { lit=0, type='morgue', filled=2,
      contents=function()
         for i = 1, d(2) + 2 do
            des.monster('M')
         end
      end
   },
   { lit=0, type='morgue', filled=2 },
   -- Three water areas; sea monsters added later
   { iterate = function(x,y) des.terrain(x,y,'}') end },
   { iterate = function(x,y) des.terrain(x,y,'}') end },
   { iterate = function(x,y) des.terrain(x,y,'}') end },
   { lit=0,
      contents=function()
         for i=1,2 do
            des.monster(strongcasters[d(#strongcasters)])
         end
      end
   },
   { lit=0,
      contents=function()
         for i=1,1 + d(2) do
            des.monster(strongcasters[d(#strongcasters)])
         end
         des.object('+')
      end
   },
   { lit=0,
      contents=function()
         for i=1,3 do
            des.trap()
         end
         des.trap('trap door')
      end
   },
   { lit=0,
      contents=function()
         for i=1,4 do
            des.trap(percent(50) and 'magic' or 'anti magic')
         end
      end
   },
   { lit=1,
      contents=function()
         des.object('?')
         des.object('!')
         if percent(50) then
            des.object('ruby')
         end
      end
   },
   { lit=1,
      contents=function()
         des.object('+')
         if percent(50) then
            des.object('?')
         else
            des.object('+')
         end
      end
   },
   { lit=0,
      contents = function()
         des.object('chest')
      end
   },
   { lit=0,
      contents = function()
         des.object({ id='chest', contents=function()
            des.object({ class="/", id = "wishing", spe=1 })
         end })
      end
   },
   { lit=0,
      contents = function()
         des.monster({ id='giant mimic', appear_as='obj:chest' })
      end
   },
   { lit=1,
      contents = function()
         des.monster({ class='m', appear_as='obj:identify' })
         des.monster({ class='m', appear_as='obj:force bolt' })
      end
   }
}

for i = 1, #regions do
   local cc = interooms:rndcoord()
   local currarea = selection.floodfill(cc.x, cc.y)
   interooms = interooms - currarea
   local reg = regions[i]
   reg['irregular'] = 1
   reg['region'] = {cc.x,cc.y, cc.x,cc.y}
   if reg['iterate'] ~= nil then
      currarea:iterate(reg['iterate'])
   end
   des.region(reg)
end

-- set up a few squeaky boards in the inner chamber
-- local boards = selection.area(14,02,22,03)
--
-- for i = 1, 5 + d(2) do
--    des.trap({ type='board', coord=boards:rndcoord(1) })
-- end

----------------------------------------------
-- Random level-wide stuff goes below here. --
----------------------------------------------

des.monster({ class='D', coord=mostfloor:filter_mapchar('.'):rndcoord() })
for i = 1, 3 + d(3) do
   des.monster({ class='&', coord=mostfloor:filter_mapchar('.'):rndcoord() })
end

-- minimum amount of water space: 4 + 6 + 6 = 16
for i = 1, 8 do
   des.monster('piranha')
end
for i = 1, 3 do
   des.monster('kraken')
end
for i = 1, 5 do
   des.monster('giant eel')
end

-- a guilty pleasure for the Wizard or one of his fellow spellcasters?
local rincewind_titles = {
   'The Colour of Magic', 'The Light Fantastic', 'Sourcery', 'Eric',
   'Interesting Times', 'The Last Continent', 'The Last Hero'
}
if percent(10) then
   des.object({ id='novel', name=rincewind_titles(d(#rincewind_titles)) })
end
