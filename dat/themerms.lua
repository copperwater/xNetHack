
-- themerooms is an array of tables and/or functions.
-- the tables define "frequency", "contents", "mindiff" and "maxdiff".
-- frequency is optional; if omitted, 1 is assumed.
-- mindiff and maxdiff are optional and independent; if omitted, the room is
-- not constrained by level difficulty.
-- a plain function has frequency of 1, and no difficulty constraints.
-- des.room({ type = "ordinary", filled = 1 })
--   - ordinary rooms can be converted to shops or any other special rooms.
--   - filled = 1 means the room gets random room contents, even if it
--     doesn't get converted into a special room. Without filled,
--     the room only gets what you define in here.
--   - use type = "themed" to force a room that's never converted
--     to a special room, such as a shop or a temple. As a rule of thumb, any
--     room that can have non-regular-floor terrain should use this.
-- core calls themerooms_generate() multiple times per level
-- to generate a single themed room.

themerooms = {
   {
     -- the "default" room
      frequency = 1000,
      contents = function()
         des.room({ type = "ordinary", filled = 1 });
         end
   },

   -- Fake Delphi
   function()
      des.room({ type = "themed", w = 11,h = 9, filled = 1,
                 contents = function()
                    des.room({ type = "themed", x = 4,y = 3, w = 3,h = 3, filled = 1,
                               contents = function()
                                  des.door({ state="random", wall="all" });
                                  des.feature("fountain", 1, 1);
                               end
                    });
                 end
      });
   end,

   -- Room in a room
   -- FIXME: subroom location is too often left/top?
   function()
      des.room({ type = "ordinary", filled = 1,
                 contents = function()
                    des.room({ type = "ordinary",
                               contents = function()
                                  des.door({ state="random", wall="all" });
                               end
                    });
                 end
      });
   end,

   -- Huge room, with another room inside (90%)
   function()
      des.room({ type = "ordinary", w = nh.rn2(10)+11,h = nh.rn2(5)+8, filled = 1,
                 contents = function()
                    if (percent(90)) then
                    des.room({ type = "ordinary", filled = 1,
                               contents = function()
                                  des.door({ state="random", wall="all" });
                                  if (percent(50)) then
                                     des.door({ state="random", wall="all" });
                                  end
                               end
                    });
                    end
                 end
      });
   end,

   -- Ice room
   function()
      des.room({ type = "themed", filled = 1,
                 contents = function()
                    des.terrain(selection.floodfill(1,1), "I");
                 end
      });
   end,

   -- Boulder room
   {
      -- rolling boulder traps only generate on DL2 and below; bump the difficulty up a bit 
      mindiff = 4,
      contents = function()
         des.room({ type = "themed",
                  contents = function(rm)
                     for x = 0, rm.width - 1 do
                        for y = 0, rm.height - 1 do
                           if (percent(30)) then
                              if (percent(50)) then
                                 des.object("boulder");
                              else
                                 des.trap("rolling boulder");
                              end
                           end
                        end
                     end
                  end
         });
      end
   },

   -- Spider nest
   {
      mindiff = 10,
      contents = function()
         des.room({ type = "themed",
                  contents = function(rm)
                     for x = 0, rm.width - 1 do
                        for y = 0, rm.height - 1 do
                           if (percent(30)) then
                              des.trap("web", x, y);
                           end
                        end
                     end
                  end
         });
      end
   },

   -- Trap room
   function()
      des.room({ type = "themed", filled = 0,
                 contents = function(rm)
                    local traps = { "arrow", "dart", "falling rock", "bear",
                                    "land mine", "sleep gas", "rust",
                                    "anti magic" };
                    shuffle(traps);
                    for x = 0, rm.width - 1 do
                       for y = 0, rm.height - 1 do
                          if (percent(30)) then
                             des.trap(traps[1], x, y);
                          end
                       end
                    end
                 end
      });
   end,

   -- Buried treasure
   function()
      des.room({ type = "themed", filled = 1,
                 contents = function()
                    des.object({ id = "chest", buried = true, contents = function()
                                    for i = 1, d(3,4) do
                                       des.object();
                                    end
                    end });
                 end
      });
   end,

   -- Massacre
   function()
      des.room({ type = "themed",
                 contents = function()
                    local mon = { "apprentice", "warrior", "ninja", "thug",
                                  "hunter", "acolyte", "abbot", "page",
                                  "attendant", "neanderthal", "chieftain",
                                  "student", "wizard", "valkyrie", "tourist",
                                  "samurai", "rogue", "ranger", "priestess",
                                  "priest", "monk", "knight", "healer",
                                  "cavewoman", "caveman", "barbarian",
                                  "archeologist" };
                    shuffle(mon);
                    for i = 1, d(5,5) do
                       if (percent(10)) then shuffle(mon); end
                       des.object({ id = "corpse", montype = mon[1] });
                    end
                 end
      });
   end,

   -- Pillars
   function()
      des.room({ type = "themed", w = 10, h = 10,
                 contents = function(rm)
                    local terr = { "-", "-", "-", "-", "L", "P", "T" };
                    shuffle(terr);
                    for x = 0, (rm.width / 4) - 1 do
                       for y = 0, (rm.height / 4) - 1 do
                          des.terrain({ x = x * 4 + 2, y = y * 4 + 2, typ = terr[1], lit = -2 });
                          des.terrain({ x = x * 4 + 3, y = y * 4 + 2, typ = terr[1], lit = -2 });
                          des.terrain({ x = x * 4 + 2, y = y * 4 + 3, typ = terr[1], lit = -2 });
                          des.terrain({ x = x * 4 + 3, y = y * 4 + 3, typ = terr[1], lit = -2 });
                       end
                    end
                 end
      });
   end,

   -- Statuary
   function()
      des.room({ type = "themed",
                 contents = function()
                    for i = 1, d(5,5) do
                       des.object({ id = "statue" });
                    end
                    for i = 1, d(3) do
                       des.trap("statue");
                    end
                 end
      });
   end,

   -- Light source
   function()
      des.room({ type = "themed", lit = 0,
                 contents = function()
                    des.object({ id = "oil lamp", lit = true });
                 end
      });
   end,

   -- Temple of the gods
   function()
      des.room({ type = "themed",
                 contents = function()
                    des.altar({ align = align[1] });
                    des.altar({ align = align[2] });
                    des.altar({ align = align[3] });
                 end
      });
   end,

   -- Mausoleum
   function()
      des.room({ type = "themed", w = 5 + nh.rn2(3)*2, h = 5 + nh.rn2(3)*2,
                 contents = function(rm)
                    des.room({ type = "themed",
			       x = (rm.width - 1) / 2, y = (rm.height - 1) / 2,
			       w = 1, h = 1, joined = false,
                               contents = function()
                                  if (percent(50)) then
                                     local mons = { "M", "V", "L", "Z" };
                                     shuffle(mons);
                                     des.monster(mons[1], 0,0);
                                  else
                                     des.object({ id = "corpse", montype = "@", coord = {0,0} });
                                  end
                                  if (percent(20)) then
                                     des.door({ state="secret", wall="all" });
                                  end
                               end
                    });
                 end
      });
   end,

   -- Random dungeon feature in the middle of a odd-sized room
   function()
      local wid = 3 + (nh.rn2(3) * 2);
      local hei = 3 + (nh.rn2(3) * 2);
      des.room({ type = "themed", filled = 1, w = wid, h = hei,
                 contents = function(rm)
                    local feature = { "C", "L", "I", "P", "T" };
                    shuffle(feature);
                    des.terrain((rm.width - 1) / 2, (rm.height - 1) / 2,
				feature[1]);
                 end
      });
   end,

   -- L-shaped
   function()
      des.map({ map = [[
-----xxx
|...|xxx
|...|xxx
|...----
|......|
|......|
|......|
--------]], contents = function(m) des.region({ region={1,1,3,3}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- L-shaped, rot 1
   function()
      des.map({ map = [[
xxx-----
xxx|...|
xxx|...|
----...|
|......|
|......|
|......|
--------]], contents = function(m) des.region({ region={5,1,5,3}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- L-shaped, rot 2
   function()
      des.map({ map = [[
--------
|......|
|......|
|......|
----...|
xxx|...|
xxx|...|
xxx-----]], contents = function(m) des.region({ region={1,1,2,2}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- L-shaped, rot 3
   function()
      des.map({ map = [[
--------
|......|
|......|
|......|
|...----
|...|xxx
|...|xxx
-----xxx]], contents = function(m) des.region({ region={1,1,2,2}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- Blocked center
   function()
      des.map({ map = [[
-----------
|.........|
|.........|
|.........|
|...LLL...|
|...LLL...|
|...LLL...|
|.........|
|.........|
|.........|
-----------]], contents = function(m)
if (percent(30)) then
   local terr = { "-", "P" };
   shuffle(terr);
   des.replace_terrain({ region = {1,1, 9,9}, fromterrain = "L", toterrain = terr[1] });
end
des.region({ region={1,1,2,2}, type="themed", irregular=true, filled=1 });
end });
   end,

   -- Circular, small
   function()
      des.map({ map = [[
xx---xx
x--.--x
--...--
|.....|
--...--
x--.--x
xx---xx]], contents = function(m) des.region({ region={3,3,3,3}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- Circular, medium
   function()
      des.map({ map = [[
xx-----xx
x--...--x
--.....--
|.......|
|.......|
|.......|
--.....--
x--...--x
xx-----xx]], contents = function(m) des.region({ region={4,4,4,4}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- Circular, big
   function()
      des.map({ map = [[
xxx-----xxx
x---...---x
x-.......-x
--.......--
|.........|
|.........|
|.........|
--.......--
x-.......-x
x---...---x
xxx-----xxx]], contents = function(m) des.region({ region={5,5,5,5}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- T-shaped
   function()
      des.map({ map = [[
xxx-----xxx
xxx|...|xxx
xxx|...|xxx
----...----
|.........|
|.........|
|.........|
-----------]], contents = function(m) des.region({ region={5,5,5,5}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- T-shaped, rot 1
   function()
      des.map({ map = [[
-----xxx
|...|xxx
|...|xxx
|...----
|......|
|......|
|......|
|...----
|...|xxx
|...|xxx
-----xxx]], contents = function(m) des.region({ region={2,2,2,2}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- T-shaped, rot 2
   function()
      des.map({ map = [[
-----------
|.........|
|.........|
|.........|
----...----
xxx|...|xxx
xxx|...|xxx
xxx-----xxx]], contents = function(m) des.region({ region={2,2,2,2}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- T-shaped, rot 3
   function()
      des.map({ map = [[
xxx-----
xxx|...|
xxx|...|
----...|
|......|
|......|
|......|
----...|
xxx|...|
xxx|...|
xxx-----]], contents = function(m) des.region({ region={5,5,5,5}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- S-shaped
   function()
      des.map({ map = [[
-----xxx
|...|xxx
|...|xxx
|...----
|......|
|......|
|......|
----...|
xxx|...|
xxx|...|
xxx-----]], contents = function(m) des.region({ region={2,2,2,2}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- S-shaped, rot 1
   function()
      des.map({ map = [[
xxx--------
xxx|......|
xxx|......|
----......|
|......----
|......|xxx
|......|xxx
--------xxx]], contents = function(m) des.region({ region={5,5,5,5}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- Z-shaped
   function()
      des.map({ map = [[
xxx-----
xxx|...|
xxx|...|
----...|
|......|
|......|
|......|
|...----
|...|xxx
|...|xxx
-----xxx]], contents = function(m) des.region({ region={5,5,5,5}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- Z-shaped, rot 1
   function()
      des.map({ map = [[
--------xxx
|......|xxx
|......|xxx
|......----
----......|
xxx|......|
xxx|......|
xxx--------]], contents = function(m) des.region({ region={2,2,2,2}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- Cross
   function()
      des.map({ map = [[
xxx-----xxx
xxx|...|xxx
xxx|...|xxx
----...----
|.........|
|.........|
|.........|
----...----
xxx|...|xxx
xxx|...|xxx
xxx-----xxx]], contents = function(m) des.region({ region={6,6,6,6}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- Four-leaf clover
   function()
      des.map({ map = [[
-----x-----
|...|x|...|
|...---...|
|.........|
---.....---
xx|.....|xx
---.....---
|.........|
|...---...|
|...|x|...|
-----x-----]], contents = function(m) des.region({ region={6,6,6,6}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- Water-surrounded vault
   function()
      des.map({ map = [[
}}}}}}
}----}
}|..|}
}|..|}
}----}
}}}}}}]], contents = function(m) des.region({ region={3,3,3,3}, type="themed", irregular=true, filled=0, joined=false });
     local nasty_undead = { "giant zombie", "ettin zombie", "vampire lord" };
     des.object("chest", 2, 2);
     des.object("chest", 3, 2);
     des.object("chest", 2, 3);
     des.object("chest", 3, 3);
     shuffle(nasty_undead);
     des.monster(nasty_undead[1], 2, 2);
end });
   end,

   -- Four connected rooms
   -- Note: they're all independent, meaning each one generates monsters,
   -- objects, furniture etc separately.
   function()
      des.map({ map = [[
-----x-----
|...|x|...|
|...+#+...|
|...|x|...|
--+--x--+--
xx#xxxxx#xx
--+--x--+--
|...|x|...|
|...+#+...|
|...|x|...|
-----x-----]],
      contents = function(m)
         des.region({ region={1,1,3,3}, type="ordinary", filled=1 });
         des.region({ region={1,7,3,9}, type="ordinary", filled=1 });
         des.region({ region={7,1,9,3}, type="ordinary", filled=1 });
         des.region({ region={7,7,9,9}, type="ordinary", filled=1 });
      end });
   end,

   -- Barbell-shaped room, horizontal
   function()
      des.map({ map = [[
-----xxx-----
|...-----...|
|...........|
|...-----...|
-----xxx-----]],
      contents = function(m)
         des.region({ region={2,2,2,2}, type="themed", irregular=true, filled=1 });
         des.door("random", 04, 02)
         des.door("random", 08, 02)
      end });
   end,

   -- Barbell-shaped room, vertical
   function()
      des.map({ map = [[
-----
|...|
|...|
|...|
--.--
x|.|x
x|.|x
x|.|x
--.--
|...|
|...|
|...|
-----]],
      contents = function(m)
         des.region({ region={2,2,2,2}, type="themed", irregular=true, filled=1 });
         des.door("random", 02, 04)
         des.door("random", 02, 08)
      end });
   end,

   -- Graffiti room
   function()
      des.room({ type = "themed", filled = 1,
                 contents = function(rm)
                     for x = 0, rm.width - 1 do
                        for y = 0, rm.height - 1 do
                           if percent(30) then
                              des.engraving({ coord={x,y}, type="mark", text="" })
                           end
                        end
                     end
                 end
      });
   end,

   -- Room with small pillars (also, possibly wood nymph room)
   function()
      des.room({ type = "themed", w = 5 + nh.rn2(3)*2, h = 5 + nh.rn2(3)*2, filled = 1,
                 contents = function(rm)
                    local grove = percent(20)
                    if nh.level_difficulty() < nh.mon_difficulty("wood nymph") then
                       grove = false
                    end
                    local mapchr = (grove and 'T') or '-'
                    for i = 0, (rm.width - 3) / 2 do
                       for j = 0, (rm.height - 3) / 2 do
                          des.terrain(i*2 + 1, j*2 + 1, mapchr)
                       end
                    end
                    if grove then
                        -- TODO: When object materials are added to the parser,
                        -- these objects should be made of wood.
                       if percent(50) then
                          des.object({ id = "statue", montype = "wood nymph" })
                       end
                       if percent(20) then
                          if percent(50) then
                             des.object({ id = "statue", montype = "wood nymph" })
                          else
                             des.object({ id = "figurine", montype = "wood nymph" })
                          end
                       end
                       local nnymphs = math.floor(d(nh.level_difficulty()) / 4)
                       for i=1,nnymphs do
                          des.monster("wood nymph")
                       end
                    end
                 end
      });
   end,

   -- Boomerang-shaped, rot 1
   function()
      des.map({ map = [[
-----xxxxx
|...---xxx
|.....--xx
--.....--x
x----...--
xxxx--...|
xxxxx|...|
xxxx--...|
x----...--
--.....--x
|.....--xx
|...---xxx
-----xxxxx]], contents = function(m) des.region({ region={2,2,2,2}, type="ordinary", irregular=true, filled=1 }); end });
   end,

   -- Boomerang-shaped, rot 2
   function()
      des.map({ map = [[
xxxxx-----
xxx---...|
xx--.....|
x--.....--
--...----x
|...--xxxx
|...|xxxxx
|...--xxxx
--...----x
x--.....--
xx--.....|
xxx---...|
xxxxx-----]], contents = function(rm)
                  des.region({ region={7,1,7,1}, type="ordinary", irregular=true, filled=1 });
              end
      });
   end,

   -- Rectangular walled corridor
   -- Note: a 5x5 version of this room could be confused for Mausoleum
   function()
      des.room({ type = "themed", filled = 1,
                 contents = function(rm)
                    for x = 1, rm.width - 2 do
                       for y = 1, rm.height - 2 do
                          des.terrain(x, y, "-");
                       end
                    end
                 end
       });
    end,

   -- Tiny cage, big monster
   function()
      des.map({ map = [[
-------
|.....|
|.FFF.|
|.F.F.|
|.FFF.|
|.....|
-------]], contents = function(m)
              des.region({ region={1,1,5,5}, type="themed", irregular=false, filled=1 });
              if percent(80) then
                 local mons = { 'M', 'Z', 'O', 'T' }
                 if nh.level_difficulty() > 6 then
                    table.insert(mons, 'H');
                    if nh.level_difficulty() > 12 then
                       table.insert(mons, 'D');
                    end
                 end
                 shuffle(mons);
                 des.monster(mons[1], 3,3);
              else
                 des.trap({ x = 3, y = 3});
              end
           end
      });
   end,

   -- Split room
   function()
      des.room({ type = "ordinary", filled = 1, w = 3 + nh.rn2(10), h = 3 + nh.rn2(4),
                 contents = function(rm)
                    local doorstates = { "open", "closed", "locked" }
                    if nh.level_difficulty() >= 5 then
                       table.insert(doorstates, "secret")
                    end
                    shuffle(doorstates)

                    local rndx = nh.rn2(rm.width - 2) + 1
                    local rndy = nh.rn2(rm.height - 2) + 1

                    if percent(50) then
                       des.room({ type = "ordinary", x = 0, y = 0, w = rndx, h = rm.height, filled = 1,
                                  contents = function()
                                     des.door({ state = doorstates[1], wall = "east" })
                                  end
                       });
                    else
                       des.room({ type = "ordinary", x = 0, y = 0, w = rm.width, h = rndy, filled = 1,
                                  contents = function()
                                     des.door({ state = doorstates[1], wall = "south" })
                                  end
                       });
                    end
                 end
      });
   end,

   -- Corner 2x2 subroom
   function()
      des.room({ type = "themed", filled = 1, w = 4 + nh.rn2(9), h = 4 + nh.rn2(3),
                 contents = function(rm)
                    local doorstates = { "open", "closed", "locked" }
                    if nh.level_difficulty() >= 5 then
                       table.insert(doorstates, "secret")
                    end
                    shuffle(doorstates)

                    local rmx = 0
                    local rmy = 0
                    if percent(50) then
                       rmx = rm.width - 2
                    end
                    if percent(50) then
                       rmy = rm.height - 2
                    end
                    des.room({ type = "themed", x = rmx, y = rmy, w = 2, h = 2, filled = 1,
                                contents = function()
                                   des.door({ state = doorstates[1], wall = "random" })
                                end
                    });
                 end
      });
   end,

   -- Storeroom vault
   {
      maxdiff = 14,
      contents = function()
         des.room({ type = "themed", filled = 0, w = 2, h = 2, joined = false,
                    contents = function(rm)
                       for i=1,d(2) do
                          des.object("chest")
                       end
                    end
         });
      end
   },

   -- Crossed X of water
   -- FIXME: This breaks the rule that the space in front of a door should
   -- always be free of traps or dangerous terrain, but we can't address that
   -- here since no doors exist yet.
   function()
      des.room({ type = "themed", filled = 1, w = 5 + nh.rn2(9), h = 5 + nh.rn2(3),
                 contents = function(rm)
                    des.terrain(selection.line(0, 0, rm.width - 1, rm.height - 1), "}");
                    des.terrain(selection.line(0, rm.height - 1, rm.width - 1, 0), "}");
                 end
      });

   end,

   -- Mini maze
   -- Warning: x and y must be odd for mazewalk not to break out of the walls.
   -- The formulas for obtaining them assume ROWNO=21 and COLNO=80.
   {
      mindiff = 6,
      contents = function()
         des.map({ map = [[
-----------
-----------
||.- - - ||
||-------||
|| - - - ||
||-------||
|| - - - ||
||-------||
|| - - - ||
-----------
-----------]], x = 3 + nh.rn2(32)*2, y = 3 + nh.rn2(4)*2, contents = function(m)
            des.mazewalk({ x = 2, y = 2, dir = "east", stocked = false });
            des.region({ region={2,2,2,2}, type="themed", irregular=true, filled=1, joined=true, lit=1 });
         end

         });
      end
   },

   -- Bunch of closets
   {
      mindiff = 5,
      contents = function()
         des.room({ type = "themed", filled = 1, joined = true, w = 3 + nh.rn2(6)*2, h = 5,
                    contents = function(rm)
                       for x = 0, rm.width - 1 do
                          for y = 0, 4 do
                             if y == 0 or y == 4 then
                                -- closet row
                                if x % 2 == 0 then
                                   -- closet column
                                   if percent(10) then
                                      des.monster({ x = x, y = y })
                                   end
                                   if percent(10) then
                                      des.object({ x = x, y = y })
                                   end
                                else
                                   -- wall column
                                   des.terrain(x, y, '|')
                                end
                             elseif y == 1 or y == 3 then
                                -- wall row
                                if x % 2 == 0 then
                                   -- closet column
                                   local doiron = percent(nh.level_difficulty()*4) and 1 or 0
                                   des.door({ x = x, y = y, iron = doiron });
                                else
                                   -- wall column
                                   des.terrain(x, y, '-')
                                end
                             end
                          end
                       end
                    end
         });
      end
   },

   -- Beehives
   {
      mindiff = 10, -- minimum beehive depth in rand_roomtype()
      contents = function()
         local nhives = d(3)
         des.room({ type = "themed", w = 1 + 5*nhives, h = 6,
                    contents = function()
                       for i=1,nhives do
                          des.room({ type = "beehive", w = 2, h = 2, x = (i*5) - 3, y = 2, filled = 1,
                                     contents = function()
                                        des.door({ wall = "random" })
                                     end
                          });
                       end
                    end
         });
      end
   },

   -- Super Honeycomb
   {
      mindiff = 13,
      contents = function()
         des.map({ map = [[
xxxx----xx----xxxx
xxx--..----..--xxx
xxx|....||....|xxx
x----..----..----x
--..----..----..--
|....||....||....|
--..----..----..--
x----..----..----x
xxx|....||....|xxx
xxx--..----..--xxx
xxxx----xx----xxxx]], contents=function(m)
               -- The goal: connect adjacent cells until a spanning tree is formed.
               -- The following tables {a,b,w,x,y,z} denote pairs of cells with
               -- ids a and b that would become connected if positions (w,x)
               -- and (y,z) became floor. Cell ids are 1-7 left to right then
               -- top to bottom.
               local conns = {
                  {1,2, 08,02,09,02},
                  {1,3, 04,03,04,04},
                  {1,4, 07,03,07,04},
                  {2,4, 10,03,10,04},
                  {2,5, 13,03,13,04},
                  {3,4, 05,05,06,05},
                  {3,6, 04,06,04,07},
                  {4,5, 11,05,12,05},
                  {4,6, 07,06,07,07},
                  {4,7, 10,06,10,07},
                  {5,7, 13,06,13,07},
                  {6,7, 08,08,09,08}
               }
               local reached = { false, false, false, false, false, false, false }
               reached[d(7)] = true -- initial cell
               local nreached = 1
               while nreached < 7 do
                  -- pick a random element of conns that adds a new cell
                  local pick = d(#conns)
                  while reached[conns[pick][1]] == reached[conns[pick][2]] do
                     pick = d(#conns)
                  end

                  -- change those walls to floor
                  des.terrain(conns[pick][3], conns[pick][4], '.')
                  des.terrain(conns[pick][5], conns[pick][6], '.')

                  -- update reached; one of the two must have been true so set both to true
                  reached[conns[pick][1]] = true
                  reached[conns[pick][2]] = true

                  -- recompute allconnected
                  nreached = nreached + 1
               end
               des.region({ region={05,01,05,01}, type="beehive", irregular=true, joined=true, filled=1 })
            end
         });
      end
   },

   -- Random terrain room, except randomly applied to any square, not just the center
   function()
      des.room({ type="themed", filled=1, contents=function(rm)
            local picks = selection.floodfill(1,1):percentage(30)
            local feature = { "C", "I", 'g' };
            -- These features can spawn in such a way that traversal is
            -- impossible across the room (e.g. doors on the left and right, and
            -- a line of trees stretches vertically across the room). Cope by
            -- ensuring none of the real blockers can be created before the Mines.
            if nh.level_difficulty() > 4 then
               table.insert(feature, 'P')
               table.insert(feature, 'T')
               if nh.level_difficulty() > 10 then
                  table.insert(feature, 'L')
               end
            end
            shuffle(feature)
            -- Prevent the features from blocking off an entire wall, which will
            -- cause impossibles if a door tries to generate on that wall.
            -- The room being filled = 1 shouldn't randomly generate an extra
            -- tree in one of the spaces deliberately left blank here, unless no
            -- door generated next to it; there's a bydoor() check in that bit
            -- of code.
            if feature[1] == 'P' or feature[1] == 'T' or feature[1] == 'L' then
               picks:set(nh.rn2(rm.width), 0, 0)             -- top
               picks:set(nh.rn2(rm.width), rm.height - 1, 0) -- bottom
               picks:set(0, nh.rn2(rm.height), 0)            -- left
               picks:set(rm.width - 1, nh.rn2(rm.height), 0) -- right
            end
            des.terrain(picks, feature[1])
         end
      });
   end,

   -- Swimming pool
   {
      mindiff = 5,
      contents = function()
         des.room({ type="themed", filled=0, contents=function(rm)
               local poolarea = selection.fillrect(1,1,rm.width-2,rm.height-2)
               des.terrain(poolarea, '}');
               -- spice it up with some sea monsters
               local waterarea = (rm.width-2)*(rm.height-2)
               local nmonsters = math.min(d(5), waterarea/2);
               for i=1,nmonsters do
                  des.monster(';')
               end
               -- sunken treasure
               if percent(50) then
                  des.object({ id='chest', coord={poolarea:rndcoord(1)}, contents=function()
                        for i=1,d(2,2) do
                           des.object('*')
                        end
                        des.object({ id = "gold piece", quantity = d(80, 5) })
                     end
                  });
               end
            end
         });
      end
   },

   -- Thin long horizontal room
   function()
      local width = 14 + d(6)
      des.room({ type="ordinary", filled=1, w=14+d(6), h=d(2) });
   end,

   -- Scummy moldy room
   {
      mindiff = 6,
      contents = function()
         des.room({ type="themed", filled=0, contents=function(rm)
               mons = { 'gas spore', 'F', 'b', 'j', 'P' }
               local nummons = math.min(rm.width * rm.height, d(4,3))
               for i=1, nummons do
                  -- Note: this is a bit different from the original UnNetHack room;
                  -- that one picked one member from mons and filled the room with it,
                  -- whereas this randomizes the member of mons each time.
                  des.monster(mons[d(#mons)])
               end
            end
         });
      end
   },

   -- Ozymandias' Tomb
   {
      mindiff = 18,
      contents = function()
         des.room({ type="themed", filled=0, w=7, h=7, contents=function()
               des.feature("throne", 3, 3)
               -- FIXME: It should be possible to have webs appear without
               -- giant spiders. Until then, no webs.
               -- des.trap("web")
               -- des.trap("web")
               for i=1,4 do
                  if percent(75) then
                     des.trap('falling rock')
                  end
               end
               for i=1,d(3)+1 do
                  des.trap('hole')
               end
               des.object({ id = "chest", trapped = 1 });
               for i=1,2 do
                  des.trap('statue')
               end
               -- no statue of Ozymandias; it shouldn't be possible to reanimate it
               local x=3
               local y=3
               if percent(50) then
                  x = x + ((2 * nh.rn2(2)) - 1)
               else
                  y = y + ((2 * nh.rn2(2)) - 1)
               end
               des.engraving({ coord={x,y}, type="engrave", text="My name is Ozymandias, king of kings: Look on my works, ye Mighty, and despair!" })
            end
         });
      end
   },

   -- Gas spore den
   -- Tread carefully...
   {
      mindiff = 5,
      contents = function()
         des.room({ type="themed", filled=0, contents=function(rm)
               for x=0,rm.width-1 do
                  for y=0,rm.width-1 do
                     if percent(math.min(100, 75 + nh.level_difficulty())) then
                        des.monster({ id='gas spore', asleep=1 })
                     end
                  end
               end
            end
         });
      end
   },

   -- Pool room: a homage to the /dev/null pool challenge
   function()
      des.room({ type="themed", filled=0, w=9, h=9, contents = function(rm)
            des.trap('hole', 0, 0)
            des.trap('hole', 0, 4)
            des.trap('hole', 0, 8)
            des.trap('hole', 8, 0)
            des.trap('hole', 8, 4)
            des.trap('hole', 8, 8)
            for i=1,9 do
               -- pick x and y to avoid placing the boulders on the pockets
               bx = 0
               by = 0
               while ((bx == 0 or bx == 8) and (by % 4 == 0)) do
                  bx = nh.rn2(9)
                  by = nh.rn2(9)
               end
               des.object({ id = 'boulder', x = bx, y = by, name = tostring(i) })
            end
         end
      });
   end,

   -- Twin businesses
   {
      mindiff = 4; -- arbitrary
      contents = function()
         -- Due to the way room connections work in mklev.c, we must guarantee
         -- that the "aisle" between the shops touches all four walls of the
         -- larger room. Thus it has an extra width and height.
         des.room({ type="themed", w=9, h=5, contents = function()
               -- There are eight possible placements of the two shops, four of
               -- which have the vertical aisle in the center.
               southeast = function() return percent(50) and "south" or "east" end
               northeast = function() return percent(50) and "north" or "east" end
               northwest = function() return percent(50) and "north" or "west" end
               southwest = function() return percent(50) and "south" or "west" end
               placements = {
                  { lx = 1, ly = 1, rx = 4, ry = 1, lwall = "south", rwall = southeast() },
                  { lx = 1, ly = 2, rx = 4, ry = 2, lwall = "north", rwall = northeast() },
                  { lx = 1, ly = 1, rx = 5, ry = 1, lwall = southeast(), rwall = southwest() },
                  { lx = 1, ly = 1, rx = 5, ry = 2, lwall = southeast(), rwall = northwest() },
                  { lx = 1, ly = 2, rx = 5, ry = 1, lwall = northeast(), rwall = southwest() },
                  { lx = 1, ly = 2, rx = 5, ry = 2, lwall = northeast(), rwall = northwest() },
                  { lx = 2, ly = 1, rx = 5, ry = 1, lwall = southwest(), rwall = "south" },
                  { lx = 2, ly = 2, rx = 5, ry = 2, lwall = northwest(), rwall = "north" }
               }
               ltype,rtype = "weapon shop","armor shop"
               if percent(50) then
                  ltype,rtype = rtype,ltype
               end
               shopdoorstate = function()
                  if percent(1) then
                     return "locked"
                  elseif percent(50) then
                     return "closed"
                  else
                     return "open"
                  end
               end
               p = placements[d(#placements)]
               des.room({ type=ltype, x=p["lx"], y=p["ly"], w=3, h=3, filled=1, joined=false,
                           contents = function()
                     des.door({ state=shopdoorstate(), wall=p["lwall"] })
                  end
               });
               des.room({ type=rtype, x=p["rx"], y=p["ry"], w=3, h=3, filled=1, joined=false,
                           contents = function()
                     des.door({ state=shopdoorstate(), wall=p["rwall"] })
                  end
               });
            end 
         });
      end
   },

   -- Four-way circle-and-cross room
   function()
      des.map({ map = [[
xxxx---xxxx
xxxx|.|xxxx
xxx--.--xxx
xx--...--xx
---.....---
|.........|
---.....---
xx--...--xx
xxx--.--xxx
xxxx|.|xxxx
xxxx---xxxx]], contents = function(m)
         centerfeature = percent(15)
         des.region({ region = {5,1,5,1}, type=centerfeature and "themed" or "ordinary", irregular=1 })
         if centerfeature then
            des.terrain(5, 5, percent(20) and 'T' or '{')
         end
      end })
   end,

   -- Four 3x3 rooms, directly adjacent
   -- Like the other four-room cluster, each room generates its own monsters,
   -- items and features.
   function()
      des.room({ type="ordinary", w=7, h=7, filled=1, contents=function()
         des.room({ type="ordinary", x=1, y=1, w=3, h=3, filled=1 })
         des.room({ type="ordinary", x=4, y=1, w=3, h=3, filled=1, contents=function()
               des.door({ state="random", wall = "west" })
               des.door({ state="random", wall = "south" })
            end
         })
         des.room({ type="ordinary", x=1, y=4, w=3, h=3, filled=1, contents=function()
               des.door({ state="random", wall = "north" })
               des.door({ state="random", wall = "east" })
            end
         })
         -- the southeast room is just the parent room and doesn't need to
         -- be defined specially; in fact, if it is, the level generator may
         -- stumble on trying to place a feature in the parent room and not
         -- finding any open spaces for it.
      end })
   end,

   -- Prison cell
   {
      mindiff = 8,
      contents = function()
         des.map({ map = [[
--------
|......|
|......|
|FFFF..|
|...F..|
|...+..|
--------]], contents = function()
            des.door({ state = "locked", x=4, y=5, iron=1 })
            if percent(70) then
               des.monster({ id="prisoner", x=d(3), y=3+d(2), peaceful=1, asleep=1 })
               -- and a rat for company
               rats = {"sewer rat", "giant rat", "rabid rat"} -- no 'r' = rock moles to break them out!
               des.monster(rats[d(#rats)])
            end
            des.region({ region={01,01,01,01}, type="themed", irregular=true, filled=1, joined=true })
            des.region({ region={01,04,03,05}, type="themed", irregular=true, filled=0, joined=false })
         end })
      end
   },

   -- Mirrored obstacles, sort of like a Rorschasch figure
   {
      mindiff = 5, -- obstacles can impede stairways in unlucky cases; put this after Mines
      contents = function()
         width = 5 + nh.rn2(10)
         height = 5 + nh.rn2(4)
         des.room({ type="themed", w=width, h=height, contents=function(rm)
            obstacles = { 'T', '}', 'F', 'L', 'C' } -- no grass/ice; not obstacles
            terrain = obstacles[d(#obstacles)]
            for x = 1,rm.width/2 do
               for y = 1,rm.height-2 do
                  if percent(40) then
                     des.terrain(x, y, terrain)
                     des.terrain(rm.width - x - 1, y, terrain)
                  end
               end
            end
         end })
      end
   },

};

function is_eligible(room)
   local t = type(room);
   local diff = nh.level_difficulty();
   if (t == "table") then
      if (room.mindiff ~= nil and diff < room.mindiff) then
         return false
      elseif (room.maxdiff ~= nil and diff > room.maxdiff) then
         return false
      end
   elseif (t == "function") then
      -- functions currently have no constraints
   end
   return true
end

function themerooms_generate()
   local pick = 1;
   local total_frequency = 0;
   for i = 1, #themerooms do
      -- Reservoir sampling: select one room from the set of eligible rooms,
      -- which may change on different levels because of level difficulty.
      if is_eligible(themerooms[i]) then
         local this_frequency;
         if (type(themerooms[i]) == "table" and themerooms[i].frequency ~= nil) then
            this_frequency = themerooms[i].frequency;
         else
            this_frequency = 1;
         end
         total_frequency = total_frequency + this_frequency;
         -- avoid rn2(0) if a room has freq 0
         if this_frequency > 0 and nh.rn2(total_frequency) < this_frequency then
            pick = i;
         end
      end
   end

   local t = type(themerooms[pick]);
   if (t == "table") then
      themerooms[pick].contents();
   elseif (t == "function") then
      themerooms[pick]();
   end
end
