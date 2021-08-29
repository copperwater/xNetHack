-- NetHack 3.6	oracle.des	$NHDT-Date: 1524689580 2018/04/25 20:53:00 $  $NHDT-Branch: NetHack-3.6.0 $:$NHDT-Revision: 1.10 $
-- Copyright (c) 2015 by Pasi Kallinen
-- NetHack may be freely redistributed.  See license for details.

des.level_flags("noflip");

des.room({ type ="delphi", lit = 1, x=39, y=10, xalign="center", yalign="center", w=11, h=9,
           contents = function()

               -- Make a square ring of pools around the Oracle, 2 spaces out
               local ring = selection.new()
               ring = selection.rect(3,2,7,6)
               -- Remove one pool orthogonal to the Oracle so she is reachable
               notpool = { {5,2},{3,4},{7,4},{5,6} }
               shuffle(notpool)
               ring:set(notpool[1][1], notpool[1][2], 0)
               des.terrain({ selection=ring, typ="}", lit=1 })
               
               -- The 4 fountains are on the outside corners
               des.feature("fountain", 2,1)
               des.feature("fountain", 8,1)
               des.feature("fountain", 2,7)
               des.feature("fountain", 8,7)
   
               -- statues of the 4 elements - appropriately neutral
               statuelocs = { {5,0},{5,8},{0,4},{10,4} }
               shuffle(statuelocs)
               des.object({ id = "statue", coord = statuelocs[1], montype = "water elemental", historic = 1 })
               des.object({ id = "statue", coord = statuelocs[2], montype = "earth elemental", historic = 1 })
               des.object({ id = "statue", coord = statuelocs[3], montype = "fire elemental", historic = 1 })
               des.object({ id = "statue", coord = statuelocs[4], montype = "air elemental", historic = 1 })
               
               des.monster("Oracle", 5, 4)
   
               des.monster()
               des.monster()
            end
})

des.room({ contents = function()
                 des.stair("up")
                 des.object()
              end
})

des.room({ contents = function()
                 des.stair("down")
                 des.object()
                 des.trap()
                 des.monster()
                 des.monster()
              end
})

des.room({ contents = function()
                 des.object()
                 des.object()
                 des.monster()
              end
})

des.room({ contents = function()
                 des.object()
                 des.trap()
                 des.monster()
              end
})

des.room({ contents = function()
                 des.object()
                 des.trap()
                 des.monster()
              end
})

des.random_corridors()
