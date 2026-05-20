-- NetHack Ranger Ran-loca.lua	$NHDT-Date: 1652196010 2022/05/10 15:20:10 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "hardfloor")
des.map([[
                       ................#############                   
       ...      #######... ........  #             ########  ....      
     .....#######                   ...   ########        # ........   
  ........             ...##########...   #      #        ..........  .
 ..... #      ..     .....           ......    .......    ...........F.
.....  #    ......   ....   .....        ..   #........       # .....  
....   #### ......   #  ####.......####       #  ......       #  .. #  
 ...      ........####         ....   ......###  ...  #    .. #     #  
  ....    ....          ...    # ..   ......    ..   ##   ...##     #  
   ....     ....    .  .......##        .. .         #  ...         #  
  .....     ...    ..    .....        ###            .......        #  
##...         #   .... ##.....  ..   ..              .......       ##  
#  ..         ###.....##     #  ..##....##    ..      # ...       ##   
#                .....       .....  .... ####  ....####          ##    
##       #####    #          ......  ..     .......           ..##     
 ##...  ..   # .....  ##..   #          ####.......     ###.....   ... 
   .......  ......... # ....##  ##..... #   .....     ###  ........... 
     ....   ..........# ....    #  ......          ####  ......  ...   
   ......                  ######  .......##########     ......  ...   
  ..... ############                                     ......        
  .....            ####################################### ...         
]]);
--1234567890123456789012345678901234567890123456789012345678901234567890

-- Dungeon Description
des.region(selection.area(00,00,70,20), "unlit")

-- Non diggable walls (but leave the iron bars diggable)
local nondig = selection.area(00,00,70,20)
nondig:set(69,04, 0)
des.non_diggable(nondig)

-- Stairs
-- "Centers" for the caverns which are where stairs, pits, etc might spawn
local centers = {
   {31,00},
   {38,03},
   {63,03},
   {03,04},
   {23,04},
   {51,05},
   {14,06},
   {31,06},
   {41,08},
   {27,10},
   {57,11},
   {19,12},
   {31,13},
   {38,13},
   {47,15},
   {17,16},
   {26,16},
   {36,17},
   {60,18},
   {04,19}
}
shuffle(centers)

-- centers[1]: upstairs
des.stair("up", centers[1])
-- shouldn't be possible to levelport or fall down into this level (except in
-- wizmode) but just in case...
local usx = centers[1][1]
local usy = centers[1][2]
des.teleport_region({ region={usx - 1, usy - 1, usx + 1, usy + 1,},
                      dir="down" })

-- centers[2]: downstairs
des.stair("down", centers[2])
-- the Wumpus guards the downstairs
des.monster({ id = "wumpus", x=centers[2][1], y=centers[2][2], peaceful=0, waiting=1 })

-- centers[3..4]: bottomless pits
for c = 3,4 do
   des.terrain(selection.floodfill(centers[c][1],centers[c][2]), 'A')
end

-- centers[5..6]: lost bands of centaurs
for c = 5,6 do
   for i = 1, 3 + d(4) do
      local mon = percent(60) and 'mountain centaur' or 'forest centaur'
      des.monster({ id = mon, peaceful = 0, x = centers[c][1], y = centers[c][2] })
   end
end

-- centers[7..8]: super bats (that come in pairs)
for c = 7,8 do
   for i = 1,2 do
      des.monster({ id = "giant bat", peaceful = 0, x = centers[c][1], y = centers[c][2] })
   end
end

-- fixed level location: Sirius, presumably captured by centaurs
des.monster({ id = "large dog", x=70, y=04, female=0, peaceful=1, name="Sirius" })

-- now stuff that could be anywhere on the level
local floor = selection.area(00,00,70,20):filter_mapchar('.')

-- Random traps
for i = 1,2 do
   des.trap("spiked pit", floor:rndcoord(1))
   des.trap("teleport", floor:rndcoord(1))
   des.trap("arrow", floor:rndcoord(1))
end

-- Random objects
-- note that scorpions and spiders generate random items when they spawn, so
-- reduce this amount
for i = 1,6 do
   des.object(floor:rndcoord())
end

-- Roaming monsters
for i = 1, 4 do
   des.monster({ id = "scorpion", peaceful=0 })
end
for i = 1, 2 do
   des.monster({ class = "s", peaceful = 0 })
end
des.monster("giant scorpion")

