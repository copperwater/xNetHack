
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel", "noflip");

des.map([[
|                                    .                                    |
|                                ----+----                                |
|                                |.......|                                |
|                                |.......|                                |
|      .                         +.......+                         .      |
|  ----+----                     |.......|                     ----+----  |
|  |.......|                     |.......|                     |.......|  |
|  |.......|                     ----+----                     |.......|  |
|  +.......+.                        .                         +.......+  |
|  |.......|                                                   |.......|  |
|  |.......|                                                   |.......|  |
|  ----+----                                                   ----+----  |
|      .                   .                   .                   .      |
|                        --+--               --+--                        |
|                        |...|               |...|                        |
|                        |...|               |...|                        |
|                        +...+               +...+                        |
|                        -----               -----                        |
]]);
-- note mazewalk quirks in the map: . squares needed at x=07,37,67 to prevent
-- walls from generating there in front of the door

des.mazewalk(12, 08, "east")
des.wallify()

-- left room
des.door({ coord={03,08}, iron=1 })
des.door({ coord={07,05}, iron=1 })
des.door({ coord={07,11}, iron=1 })
des.door({ coord={11,08}, iron=1 })
-- top room
des.door({ coord={33,04}, iron=1 })
des.door({ coord={37,01}, iron=1 })
des.door({ coord={37,07}, iron=1 })
des.door({ coord={41,04}, iron=1 })
-- right room
des.door({ coord={63,08}, iron=1 })
des.door({ coord={67,05}, iron=1 })
des.door({ coord={67,11}, iron=1 })
des.door({ coord={71,08}, iron=1 })
-- bottom left room
des.door({ coord={25,16}, iron=1 })
des.door({ coord={27,13}, iron=1 })
des.door({ coord={29,16}, iron=1 })
-- bottom right room
des.door({ coord={45,16}, iron=1 })
des.door({ coord={47,13}, iron=1 })
des.door({ coord={49,16}, iron=1 })

-- stairs up
staircoords = {{27,15},{47,15}}
des.stair({ coord = staircoords[d(#staircoords)], dir = "up" })

-- the Square itself
vscoords={{07,08}, {37,04}, {67,08}}
des.trap({ coord = vscoords[d(#vscoords)], type = "vibrating square" })

-- put some mazey stock in (mostly a direct copy of mkmaze.c logic)
for i = 1, d(8)+10 do
   if percent(50) then
      des.object()
   else
      des.object('*')
   end
end

for i = 1, d(10)+1 do
   des.object('boulder')
end

for i = 1, nh.rn2(3) do
   des.monster('minotaur')
end

for i = 1, d(10)+20 do
   des.monster()
end

for i = 1, d(6)+6 do
   des.gold()
end

for i = 1, d(6)+6 do
   des.trap()
end
