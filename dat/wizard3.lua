-- NetHack yendor wizard3.lua	$NHDT-Date: 1652196040 2022/05/10 15:20:40 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.4 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style="mazegrid", bg ="-" });

des.message("You enter an odd wedge-shaped chamber.")

-- Note the lack of nommap so that the player can use magic mapping to determine
-- where the gap is, if they choose to.
des.level_flags("mazelevel", "noteleport", "noflip", "hardfloor")

-- Note that this map has the top chamber open to start. (It will immediately
-- move the gap away when the player enters.)
des.map([[
            -------------            
        -----...........-----        
     ----...|...........|...----     
   ---......-+-.......-+-......---   
 ---..........|.......|..........--- 
 |............---...---............| 
------.......----...----.......------
|....--+---.----}}}}}----.---+--....|
|.........--|--}}---}}--|--.........|
|...........||}}--.--}}||...........|
|...........||}}|...|}}||...........|
|.........--||}}--.--}}||--.........|
|....--+---.|--}}---}}--|.---+--....|
------......----}}}}}----......------
 |...........-----------...........| 
 ---..........---------..........--- 
   ---........|.......|........---   
     ----...-+-.......-+-...----     
        -----...........-----        
            -------------            
]]);

-- Define some areas
local everything = selection.area(00,00,35,19)
local outsidemoat = everything - selection.area(16,07,20,13) - selection.area(15,08,21,12)
des.region(everything, 'lit')

-- The traps here are specially handled in trap.c: they have 
des.trap({ type='board', coord={18,03}, seen=1 })
des.trap({ type='board', coord={10,05}, seen=1 })
des.trap({ type='board', coord={26,05}, seen=1 })
des.trap({ type='board', coord={07,10}, seen=1 })
des.trap({ type='board', coord={29,10}, seen=1 })
des.trap({ type='board', coord={10,15}, seen=1 })
des.trap({ type='board', coord={26,15}, seen=1 })
des.trap({ type='board', coord={18,17}, seen=1 })

colors = { 'red', 'orange', 'yellow', 'green', 'blue', 'violet', 'white', 'black' }
shuffle(colors)

des.object({ id='worthless piece of '..colors[1]..' glass', coord={13,01} })
des.object({ id='worthless piece of '..colors[1]..' glass', coord={15,01} })
des.object({ id='worthless piece of '..colors[1]..' glass', coord={23,01} })
des.object({ id='worthless piece of '..colors[1]..' glass', coord={21,01} })

des.object({ id='worthless piece of '..colors[2]..' glass', coord={27,02} })
des.object({ id='worthless piece of '..colors[2]..' glass', coord={30,03} })
des.object({ id='worthless piece of '..colors[2]..' glass', coord={32,04} })
des.object({ id='worthless piece of '..colors[2]..' glass', coord={34,05} })

des.object({ id='worthless piece of '..colors[3]..' glass', coord={35,07} })
des.object({ id='worthless piece of '..colors[3]..' glass', coord={35,09} })
des.object({ id='worthless piece of '..colors[3]..' glass', coord={35,10} })
des.object({ id='worthless piece of '..colors[3]..' glass', coord={35,12} })

des.object({ id='worthless piece of '..colors[4]..' glass', coord={34,14} })
des.object({ id='worthless piece of '..colors[4]..' glass', coord={32,15} })
des.object({ id='worthless piece of '..colors[4]..' glass', coord={30,16} })
des.object({ id='worthless piece of '..colors[4]..' glass', coord={27,17} })

des.object({ id='worthless piece of '..colors[5]..' glass', coord={13,18} })
des.object({ id='worthless piece of '..colors[5]..' glass', coord={15,18} })
des.object({ id='worthless piece of '..colors[5]..' glass', coord={23,18} })
des.object({ id='worthless piece of '..colors[5]..' glass', coord={21,18} })

des.object({ id='worthless piece of '..colors[6]..' glass', coord={02,14} })
des.object({ id='worthless piece of '..colors[6]..' glass', coord={04,15} })
des.object({ id='worthless piece of '..colors[6]..' glass', coord={06,16} })
des.object({ id='worthless piece of '..colors[6]..' glass', coord={09,17} })

des.object({ id='worthless piece of '..colors[7]..' glass', coord={01,07} })
des.object({ id='worthless piece of '..colors[7]..' glass', coord={01,09} })
des.object({ id='worthless piece of '..colors[7]..' glass', coord={01,10} })
des.object({ id='worthless piece of '..colors[7]..' glass', coord={01,12} })

des.object({ id='worthless piece of '..colors[8]..' glass', coord={02,05} })
des.object({ id='worthless piece of '..colors[8]..' glass', coord={04,04} })
des.object({ id='worthless piece of '..colors[8]..' glass', coord={06,03} })
des.object({ id='worthless piece of '..colors[8]..' glass', coord={09,02} })


-- Stairs to main Gehennom
des.levregion({ region = {18,01,18,01}, type = "branch" })
-- Ladder up
des.ladder("up", 18,10)

-- Doors between chambers
des.door({ state="random", coord={13,03}, iron=0, locked=0 })
des.door({ state="random", coord={23,03}, iron=0, locked=0 })
des.door({ state="random", coord={07,07}, iron=0, locked=0 })
des.door({ state="random", coord={29,07}, iron=0, locked=0 })
des.door({ state="random", coord={07,12}, iron=0, locked=0 })
des.door({ state="random", coord={29,12}, iron=0, locked=0 })
des.door({ state="random", coord={13,17}, iron=0, locked=0 })
des.door({ state="random", coord={23,17}, iron=0, locked=0 })

-- Define regions for each chamber so that we can later hook into entering them
-- and triggering effects
-- IMPORTANT: The wizard puzzle code makes several assumptions about these
-- chambers:
--    1) The chambers are the first 8 rooms defined on this level so that they
--       occupy indexes 0-7 in g.rooms. Do not define other rooms before these.
--    2) The chambers are defined in a specific order (clockwise from top).
--       Don't mess with the order.
--    3) The chambers have specific shapes, with the gaps that open and close
--       expressed in terms of hardcoded offsets (gap_spaces[]) from their top
--       left corners.
-- make the entry chamber a real room; it affects monster arrival
des.region({ region={18,02,18,02}, lit=1, irregular=1, type='ordinary', arrival_room=true })
des.region({ region={28,04,28,04}, lit=1, irregular=1, type='ordinary' })
des.region({ region={31,10,31,10}, lit=1, irregular=1, type='ordinary' })
des.region({ region={28,16,28,16}, lit=1, irregular=1, type='ordinary' })
des.region({ region={18,18,18,18}, lit=1, irregular=1, type='ordinary' })
des.region({ region={08,16,08,16}, lit=1, irregular=1, type='ordinary' })
des.region({ region={05,10,05,10}, lit=1, irregular=1, type='ordinary' })
des.region({ region={08,04,08,04}, lit=1, irregular=1, type='ordinary' })

-- FIXME: This restricts branchports in, but also restricts falling down from
-- another level. Adding dir="up" allows falling down to put you anywhere, but
-- also allows branchporting in to put you anywhere.
-- Maybe it isn't a huge problem, because then if you get to a higher level
-- without solving the puzzle and fall back down, you won't wind up inside the
-- middle of the level.
des.teleport_region({ region={17,01,19,02} })

-- Non diggable walls
-- Walls inside the moat stay diggable
des.non_diggable(outsidemoat)
des.non_passwall(outsidemoat)
--
des.monster("L", 17, 10)
des.monster("vampire lord", 19, 10)
-- Some surrounding horrors
des.monster("kraken")
des.monster("giant eel")
des.monster("kraken")
des.monster("giant eel")

-- Some loot
if percent(40) then des.object(")") end
if percent(40) then des.object("!") end
if percent(40) then des.object("?") end
if percent(40) then des.object("?") end
if percent(40) then des.object("(") end
des.object('"', 18, 10)
