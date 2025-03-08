-- NetHack themerms.lua	$NHDT-Date: 1652196294 2022/05/10 15:24:54 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.16 $
--	Copyright (c) 2020 by Pasi Kallinen
-- NetHack may be freely redistributed.  See license for details.
--
-- themerooms is an array of tables.
-- the tables define "name", "frequency", "contents", "mindiff" and "maxdiff".
-- * "name" is not shown in-game; it is so that developers can specify a certain
--   room to generate by using the THEMERM or THEMERMFILL environment variable.
--   While technically optional, it should be provided on all the rooms; if it
--   isn't, the room can't be directly specified.
-- * "frequency" is optional; if omitted, 1 is assumed.
-- * "contents" is a function describing what gets put into the room.
-- * "mindiff" and "maxdiff" are optional and independent; if omitted, the room
--    is not constrained by level difficulty.
--
-- themeroom_fills is an array of tables with the exact same structure as
-- themerooms. It is used for contents of a room that are independent of its
-- shape, so that interestingly-shaped themerooms can be filled with a variety
-- of contents.
-- * The "contents" functions in themeroom_fills take the room they are filling as
--   an argument.
-- * Frequency of themeroom_fills is a separate pool from themerooms, and has no
--   effect on how likely it is that any given room will receive a themeroom_fill.
--
-- des.room({ type = "ordinary", filled = 1 })
--   - ordinary rooms can be converted to shops or any other special rooms.
--   - filled = 1 means the room gets random room contents, even if it
--     doesn't get converted into a special room. Without filled,
--     the room only gets what you define in here.
--   - use type = "themed" to force a room that's never converted
--     to a special room, such as a shop or a temple. As a rule of thumb, any
--     room that can have non-regular-floor terrain should use this.
--
-- for each level, the core first calls pre_themerooms_generate(),
-- then it calls themerooms_generate() multiple times until it decides
-- enough rooms have been generated, and then it calls
-- post_themerooms_generate(). When the level has been generated, with
-- joining corridors and rooms filled, the core calls post_level_generate().
-- The lua state is persistent through the gameplay, but not across saves,
-- so remember to reset any variables.

local postprocess = { };

-- Themed room fills, which can be applied to rooms of various shape and size
-- that use filler_region() to provide a potential fill.
-- This allows for a lot of variety in rooms, since otherwise there would have
-- to be (#empty room shapes x #fills) different themed rooms to accomplish the
-- same effect.
-- Generally expected is that the region is in fact empty, which is a safe
-- assumption if calling filler_region on an empty room space.
themeroom_fills = {

   {
      name = "Ice room",
      contents = function(rm)
         local ice = selection.room();
         des.terrain(ice, "I");
         if (percent(25)) then
            local mintime = 1000 - (nh.level_difficulty() * 100);
            local ice_melter = function(x,y)
               nh.start_timer_at(x,y, "melt-ice", mintime + nh.rn2(1000));
            end;
            ice:iterate(ice_melter);
         end
      end,
   },

   {
      name = "Cloud room",
      contents = function(rm)
         local fog = selection.room();
         for i = 1, (fog:numpoints() / 4) do
            des.monster({ id = "fog cloud", asleep = true });
         end
         des.gas_cloud({ selection = fog });
      end,
   },

   {
      name = "Boulder room",
      -- rolling boulder traps only generate on DL2 and below; bump the difficulty up a bit 
      mindiff = 4,
      contents = function(rm)
         local locs = selection.room():percentage(30);
         local func = function(x,y)
            if (percent(50)) then
               des.object("boulder", x, y);
            else
               des.trap("rolling boulder", x, y);
            end
         end;
         locs:iterate(func);
      end,
   },

   {
      name = "Spider nest",
      contents = function(rm)
         local spooders = nh.level_difficulty() > 8;
         local locs = selection.room():percentage(30);
         local func = function(x,y)
            des.trap({ type = "web", x = x, y = y,
                       spider_on_web = spooders and percent(80) });
         end
         locs:iterate(func);
      end,
   },

   {
      name = "Trap room",
      contents = function(rm)
         local traps = { "arrow", "dart", "falling rock", "bear",
                        "land mine", "sleep gas", "rust",
                        "anti magic" };
         shuffle(traps);
         local locs = selection.room():percentage(30);
         local func = function(x,y)
            des.trap(traps[1], x, y);
         end
         locs:iterate(func);
      end,
   },

   {
      name = "Garden",
      mindiff = nh.mon_difficulty('wood nymph') + 2;
      eligible = function(rm) return rm.lit == true; end,
      contents = function(rm)
         local s = selection.room();
         local npts = (s:numpoints() / 6);
         for i = 1, npts do
            des.monster({ id = "wood nymph", asleep = true });
            if (percent(30)) then
               des.feature("fountain");
            end
            if percent(30) then
               des.object({ id = 'statue', montype = 'gnome', material = "stone" })
            end
         end
         table.insert(postprocess, { handler = make_garden_walls, data = { sel = selection.room() } });
      end
   },

   {
      name = "Buried treasure",
      contents = function(rm)
         des.object({ id = "chest", buried = true, contents = function(otmp)
            local xobj = otmp:totable();
            -- keep track of the last buried treasure
            if (xobj.NO_OBJ == nil) then
               table.insert(postprocess, { handler = make_dig_engraving, data = { x = xobj.ox, y = xobj.oy }});
            end
            for i = 1, d(3,4) do
               des.object();
            end
         end });
      end,
   },

   {
      name = "Buried zombies",
      contents = function(rm)
         local diff = nh.level_difficulty()
         -- start with [1..4] for low difficulty
         local zombifiable = { "kobold", "gnome", "orc", "dwarf" };
         if diff > 3 then          -- medium difficulty
            zombifiable[5], zombifiable[6] = "elf", "human";
            if diff > 6 then       -- high difficulty (relatively speaking)
               zombifiable[7], zombifiable[8] = "ettin", "giant";
            end
         end
         for i = 1, (rm.width * rm.height) / 2 do
            shuffle(zombifiable);
            local o = des.object({ id = "corpse", montype = zombifiable[1],
                                 buried = true });
            o:stop_timer("rot-corpse");
            o:start_timer("zombify-mon", math.random(990, 1010));
         end
      end,
   },

   {
      name = "Massacre",
      contents = function(rm)
         local mon = { "apprentice", "warrior", "ninja", "thug",
                     "hunter", "acolyte", "abbot", "page",
                     "attendant", "neanderthal", "chieftain",
                     "student", "wizard", "valkyrie", "tourist",
                     "samurai", "rogue", "ranger", "priestess",
                     "priest", "monk", "knight", "healer",
                     "cavewoman", "caveman", "barbarian",
                     "archeologist" };
         local idx = math.random(#mon);
         for i = 1, d(5,5) do
            if (percent(10)) then idx = math.random(#mon); end
            des.object({ id = "corpse", montype = mon[idx] });
         end
      end,
   },

   {
      name = "Statuary",
      contents = function(rm)
         for i = 1, d(5,5) do
            des.object({ id = "statue" });
         end
         for i = 1, d(3) do
            des.trap("statue");
         end
      end,
   },


   {
      name = "Light source",
      eligible = function(rm) return rm.lit == false; end,
      contents = function(rm)
         des.object({ id = "oil lamp", lit = true });
      end
   },

   {
      name = "Temple of the gods",
      contents = function(rm)
         des.altar({ align = align[1] });
         des.altar({ align = align[2] });
         des.altar({ align = align[3] });
      end,
   },

   {
      name = "Ghost of an Adventurer",
      contents = function(rm)
         local loc = selection.room():rndcoord(0);
         des.monster({ id = "ghost", asleep = true, waiting = true, coord = loc });
         if percent(65) then
            des.object({ id = "dagger", coord = loc, buc = "not-blessed" });
         end
         if percent(55) then
            des.object({ class = ")", coord = loc, buc = "not-blessed" });
         end
         if percent(45) then
            des.object({ id = "bow", coord = loc, buc = "not-blessed" });
            des.object({ id = "arrow", coord = loc, buc = "not-blessed" });
         end
         if percent(65) then
            des.object({ class = "[", coord = loc, buc = "not-blessed" });
         end
         if percent(20) then
            des.object({ class = "=", coord = loc, buc = "not-blessed" });
         end
         if percent(20) then
            des.object({ class = "?", coord = loc, buc = "not-blessed" });
         end
      end,
   },

   {
      name = "Storeroom",
      contents = function(rm)
         local locs = selection.room():percentage(30);
         local func = function(x,y)
            if (percent(25)) then
               des.object("chest");
            else
               des.monster({ class = "m", appear_as = "obj:chest" });
            end
         end;
         locs:iterate(func);
      end,
   },

   {
      name = "Teleportation hub",
      contents = function(rm)
         local locs = selection.room():filter_mapchar(".");
         for i = 1, 2 + nh.rn2(3) do
            local pos = locs:rndcoord(1);
            if (pos.x > 0) then
               pos.x = pos.x + rm.region.x1 - 1;
               pos.y = pos.y + rm.region.y1;
               table.insert(postprocess, { handler = make_a_trap, data = { type = "teleport", seen = true, coord = pos, teledest = 1 } });
            end
         end
      end,
   },

   -- XNETHACK THEMED ROOM FILLS START HERE

   {
      name = 'Graffiti room',
      contents = function(rm)
         local locs = selection.room():percentage(30);
         local graffiti_it = function(x, y)
            -- text="" means make a random engraving
            des.engraving({ coord={x,y}, type="mark", text="" })
         end;
         locs:iterate(graffiti_it)
      end,
   },
   {
      name = 'Scummy moldy room',
      mindiff = 6,
      contents = function(rm)
         mons = { 'gas spore', 'F', 'b', 'j', 'P' }
         local nummons = math.min(selection.room():numpoints(), d(4,3))
         for i=1, nummons do
            -- Note: this is a bit different from the original UnNetHack
            -- room; that one picked one member from mons and filled the room
            -- with it, whereas this randomizes the member of mons each time.
            des.monster(mons[d(#mons)])
         end
      end
   },
   {
      -- Tread carefully...
      name = 'Gas spore den',
      mindiff = 5,
      contents = function()
         local make_spore = function(x, y)
            if percent(math.min(100, 75 + nh.level_difficulty())) then
               des.monster({ id="gas spore", coord={x,y}, asleep=1 })
            end
         end
         selection.room():iterate(make_spore)
      end
   },
   {
      name = 'Water temple', -- (not a real temple)
      mindiff = nh.mon_difficulty('water nymph') + 1,
      contents = function()
         local totsiz = selection.room():numpoints()
         for i = 1, math.min(d(6)+6, math.floor(totsiz / 4)) do
            des.feature({ type='pool' })
         end
         for i = 1, math.min(d(3), math.floor(totsiz / 4)) do
            des.feature({ type='fountain' })
         end
         if percent(30) then
            des.feature({ type='sink' })
         end
         for i = 1, math.min(d(4)+1, math.floor(totsiz / 4)) do
            des.monster('water nymph')
         end
      end
   },
   {
      name = 'Meadow',
      eligible = function(rm) return rm.lit == true; end,
      contents = function()
         local interior = selection.room()
         des.terrain(interior, 'g')
         if interior:numpoints() > 8 then
            -- in the original implementation this tried to pick points not against
            -- a wall so as not to block doors. now that this is a fill, that's too
            -- hard to do.
            for i = 1, d(4)-2 do
               des.terrain(interior:rndcoord(), 'T')
            end
         end
      end,
   },

   {
      name = 'Minesweeper',
      mindiff = 7,
      eligible = function(rm)
         return selection.room():filter_mapchar('.'):numpoints() >= 40
      end,
      contents = function(rm)
         local pts = selection.room():filter_mapchar('.')
         local ptscopy = pts:clone()
         local mines = {}
         -- first, lay down the mines
         -- mine percentage in beginner is roughly 10%, intermediate 15%, and
         -- expert 20%. Beginner might make only a couple mines and be
         -- trivial; expert will probably make too many mines and leave the
         -- whole room covered in markers. Use intermediate.
         local nummines = math.floor(ptscopy:numpoints() * 15 / 100)
         for i = 1, nummines do
            local coord = ptscopy:rndcoord(1)
            mines[coord.x..','..coord.y] = true
            des.trap({ type = 'land mine', coord = coord })
         end
         -- second pass: for all spaces, calculate number of adjacent mines and
         -- engrave a number (no engraving for 0s). Mines get an X engraving;
         -- they need some engraving otherwise they would be obvious.
         -- If the hero ever becomes capable of reading adjacent engravings this
         -- will sadly stop working...
         pts:iterate(function(x, y)
            if mines[x..','..y] then
               des.engraving({ type = 'burn', coord = {x, y}, text = 'X' })
               return
            end
            local adj = 0
            for xx = x-1, x+1 do
               for yy = y-1, y+1 do
                  if mines[xx..','..yy] then
                     adj = adj + 1
                  end
               end
            end
            if adj > 0 then
               des.engraving({ type='burn', coord = {x, y}, text = tostring(adj) })
            end
         end)
      end
   },
   {
      name = 'Monster sauna',
      mindiff = nh.mon_difficulty('steam vortex'),
      -- no large saunas
      eligible = function(rm) return selection.room():numpoints() < 20; end,
      contents = function(rm)
         local npts = selection.room():numpoints()
         for i = 1, npts / 4 do
            des.monster({ id = 'steam vortex', waiting = 1 })
         end
         for i = 1, npts / 3 do
            des.monster({ id = 'fog cloud', waiting = 1 })
         end
         for i = 1, d(2) do
            des.monster()
         end
         for i = 1, d(3) do
            des.object('towel')
         end
         -- steam
         selection.room():iterate(function(x, y)
            if percent(40) then
               des.terrain(x, y, 'C')
            end
         end)
         -- cooling pool
         des.terrain(selection.room():rndcoord(), 'P')
      end,
   },
   {
      name = 'Scattered gems',
      contents = function(rm)
         local room = selection.room():filter_mapchar('.')
         local ngems = room:numpoints() / 3
         for i = 1, ngems do
            -- rule out gray stones so we only get gems and glass
            local gem
            repeat
               gem = obj.new("*")
            until obj.class(gem)['material'] ~= 'stone'
            -- use rndcoord(1) to avoid stacks
            local coord = room:rndcoord(1)
            gem:placeobj(coord.x, coord.y)
         end
      end,
   },
};

------------------------------- HELPER FUNCTIONS -------------------------------
function way_out_method(trap_ok)
   -- Lua analogue to generate_way_out_method() for non-joined, inaccessible
   -- areas the player might teleport into
   -- intended to be called inside the contents() of such an area
   if trap_ok and percent(60) then
      if percent(60) then
         des.trap('hole')
      else
         des.trap('teleport')
      end
   else
      items = { { id = 'pick-axe' },
                { id = 'dwarvish mattock' },
                { class = '/', id = 'digging' },
                { class = '?', id = 'teleportation' },
                { class = '=', id = 'teleportation' } } -- no wand of teleportation
      des.object(items[d(#items)])
   end
end

themerooms = {
   {
      name = "default",
      frequency = 1000,
      contents = function()
         des.room({ type = "ordinary", filled = 1 });
         end
   },

   {
      name = "Fake Delphi",
      contents = function()
         des.room({ type = "ordinary", w = 11,h = 9, filled = 1,
                  contents = function()
                     des.room({ type = "ordinary", x = 4,y = 3, w = 3,h = 3, filled = 1,
                                 contents = function()
                                    des.door({ state="random", wall="all" });
                                    des.feature("fountain", 1, 1);
                                 end
                     });
                     for i = 1, d(2) do
                        des.object({ id = "statue", montype = "C" })
                     end
                  end
         });
      end,
   },

   {
      name = "Room in a room",
      contents = function()
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
   },

   {
      name = "Huge room with another room inside",
      contents = function()
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
   },

   {
      name = "Nesting rooms",
      contents = function()
         des.room({ type = "ordinary", w = 9 + nh.rn2(4), h = 9 + nh.rn2(4), filled = 1,
                  contents = function(rm)
                     local wid = math.random(math.floor(rm.width / 2), rm.width - 2);
                     local hei = math.random(math.floor(rm.height / 2), rm.height - 2);
                     des.room({ type = "ordinary", w = wid,h = hei, filled = 1,
                                 contents = function()
                                    if (percent(90)) then
                                       des.room({ type = "ordinary", filled = 1,
                                                   contents = function()
                                                      des.door({ state="random", wall="all" });
                                                      if (percent(15)) then
                                                         des.door({ state="random", wall="all" });
                                                      end
                                                   end
                                       });
                                    end
                                    des.door({ state="random", wall="all" });
                                    if (percent(15)) then
                                       des.door({ state="random", wall="all" });
                                    end
                                 end
                     });
                  end
         });
      end,
   },

   {
      name = "Default room with themed fill",
      frequency = 6,
      contents = function()
         des.room({ type = "themed", contents = themeroom_fill });
      end
   },

   {
      name = "Unlit room with themed fill",
      frequency = 2,
      contents = function()
         des.room({ type = "themed", lit = 0, contents = themeroom_fill });
      end
   },

   {
      name = "Room with both normal contents and themed fill",
      frequency = 2,
      contents = function()
         des.room({ type = "themed", filled = 1, contents = themeroom_fill });
      end
   },

   {
      name = 'Pillars',
      contents = function()
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
   },

   {
      name = 'Mausoleum',
      contents = function()
         des.room({ type = "themed", w = 5 + nh.rn2(3)*2, h = 5 + nh.rn2(3)*2,
                  contents = function(rm)
                     des.room({ type = "themed",
                                 x = (rm.width - 1) / 2, y = (rm.height - 1) / 2,
                                 w = 1, h = 1, joined = false,
                                 contents = function()
                                    if (percent(50)) then
                                       local mons = { "M", "V", "L", "Z" };
                                       shuffle(mons);
                                       des.monster({ class = mons[1], x=0,y=0, waiting = 1 });
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
   },

   {
      name = 'Random dungeon feature in the middle of an odd-sized room',
      contents = function()
         local wid = 3 + (nh.rn2(3) * 2);
         local hei = 3 + (nh.rn2(3) * 2);
         des.room({ type = "ordinary", filled = 1, w = wid, h = hei,
                  contents = function(rm)
                     local feature = { "C", "L", "I", "P", "T" };
                     shuffle(feature);
                     des.terrain((rm.width - 1) / 2, (rm.height - 1) / 2,
                                 feature[1]);
                  end
         });
      end,
   },

   {
      name = 'L-shaped',
      contents = function()
         des.map({ map = [[
-----xxx
|...|xxx
|...|xxx
|...----
|......|
|......|
|......|
--------]], contents = function(m) filler_region(1,1); end });
      end,
   },

   {
      name = 'L-shaped, rot 1',
      contents = function()
         des.map({ map = [[
xxx-----
xxx|...|
xxx|...|
----...|
|......|
|......|
|......|
--------]], contents = function(m) filler_region(5,1); end });
      end,
   },

   {
      name = 'L-shaped, rot 2',
      contents = function()
         des.map({ map = [[
--------
|......|
|......|
|......|
----...|
xxx|...|
xxx|...|
xxx-----]], contents = function(m) filler_region(1,1); end });
      end,
   },

   {
      name = 'L-shaped, rot 3',
      contents = function()
         des.map({ map = [[
--------
|......|
|......|
|......|
|...----
|...|xxx
|...|xxx
-----xxx]], contents = function(m) filler_region(1,1); end });
      end,
   },

   {
      name = 'Blocked center',
      contents = function()
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
            filler_region(1,1);
         end });
      end,
   },

   {
      name = 'Circular, small',
      contents = function()
         des.map({ map = [[
xx---xx
x--.--x
--...--
|.....|
--...--
x--.--x
xx---xx]], contents = function(m) filler_region(3,3); end });
      end,
   },

   {
      name = 'Circular, medium',
      contents = function()
         des.map({ map = [[
xx-----xx
x--...--x
--.....--
|.......|
|.......|
|.......|
--.....--
x--...--x
xx-----xx]], contents = function(m) filler_region(4,4); end });
      end,
   },

   {
      name = 'Circular, big',
      contents = function()
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
xxx-----xxx]], contents = function(m) filler_region(5,5); end });
      end,
   },

   {
      name = 'T-shaped',
      contents = function()
         des.map({ map = [[
xxx-----xxx
xxx|...|xxx
xxx|...|xxx
----...----
|.........|
|.........|
|.........|
-----------]], contents = function(m) filler_region(5,5); end });
      end,
   },

   {
      name = 'T-shaped, rot 1',
      contents = function()
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
-----xxx]], contents = function(m) filler_region(2,2); end });
      end,
   },

   {
      name = 'T-shaped, rot 2',
      contents = function()
         des.map({ map = [[
-----------
|.........|
|.........|
|.........|
----...----
xxx|...|xxx
xxx|...|xxx
xxx-----xxx]], contents = function(m) filler_region(2,2); end });
      end,
   },

   {
      name = 'T-shaped, rot 3',
      contents = function()
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
xxx-----]], contents = function(m) filler_region(5,5); end });
      end,
   },

   {
      name = 'S-shaped',
      contents = function()
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
xxx-----]], contents = function(m) filler_region(2,2); end });
      end,
   },

   {
      name = 'S-shaped, rot 1',
      contents = function()
         des.map({ map = [[
xxx--------
xxx|......|
xxx|......|
----......|
|......----
|......|xxx
|......|xxx
--------xxx]], contents = function(m) filler_region(5,5); end });
      end,
   },

   {
      name = 'Z-shaped',
      contents = function()
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
-----xxx]], contents = function(m) filler_region(5,5); end });
      end,
   },

   {
      name = 'Z-shaped, rot 1',
      contents = function()
         des.map({ map = [[
--------xxx
|......|xxx
|......|xxx
|......----
----......|
xxx|......|
xxx|......|
xxx--------]], contents = function(m) filler_region(2,2); end });
      end,
   },

   {
      name = 'Cross',
      contents = function()
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
xxx-----xxx]], contents = function(m) filler_region(6,6); end });
      end,
   },

   {
      name = 'Four-leaf clover',
      contents = function()
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
-----x-----]], contents = function(m) filler_region(6,6); end });
      end,
   },

   {
      name = 'Water-surrounded vault',
      contents = function()
         des.map({ map = [[
}}}}}}
}----}
}|..|}
}|..|}
}----}
}}}}}}]], contents = function(m) des.region({ region={3,3,3,3}, type="themed", irregular=true, filled=0, joined=false });
            local nasty_undead = { "giant zombie", "ettin zombie", "vampire lord" };
            local chest_spots = { { 2, 2 }, { 3, 2 }, { 2, 3 }, { 3, 3 } };

            shuffle(chest_spots)
            -- Guarantee an escape item inside one of the chests, to prevent the
            -- hero falling in from above and becoming permanently stuck
            -- [cf. generate_way_out_method(sp_lev.c)].
            -- If the escape item is made of glass or crystal, make sure that
            -- the chest isn't locked so that kicking it to gain access to its
            -- contents won't be necessary; otherwise retain lock state from
            -- random creation.
            -- "pick-axe", "dwarvish mattock" could be included in the list of
            -- escape items but don't normally generate in containers.
            local escape_items = {
               "scroll of teleportation", "ring of teleportation",
               "wand of teleportation", "wand of digging"
            };
            local itm = obj.new(escape_items[math.random(#escape_items)]);
            local itmcls = itm:class()
            local box
            if itmcls[ "material" ] == "glass" then
               -- explicitly force chest to be unlocked
               box = des.object({ id = "chest", coord = chest_spots[1],
                                  olocked = "no" });
            else
               -- accept random locked/unlocked state
               box = des.object({ id = "chest", coord = chest_spots[1] });
            end;
            box:addcontent(itm);

            for i = 2, #chest_spots do
                  des.object({ id = "chest", coord = chest_spots[i] });
            end

            shuffle(nasty_undead);
            des.monster(nasty_undead[1], 2, 2);
            des.exclusion({ type = "teleport", region = { 2,2, 3,3 } });
         end });
      end,
   },

   {
      name = 'Twin businesses',
      mindiff = 4, -- arbitrary
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
            des.room({ type=ltype, x=p["lx"], y=p["ly"], w=3, h=3, filled=1,
                       joined=false, contents = function()
               des.door({ state=shopdoorstate(), wall=p["lwall"] })
            end })
            des.room({ type=rtype, x=p["rx"], y=p["ry"], w=3, h=3, filled=1,
                       joined=false, contents = function()
               des.door({ state=shopdoorstate(), wall=p["rwall"] })
            end })
         end })
      end
   },

   -- XNETHACK THEMED ROOMS START HERE

   {
      name = "Four connected rooms",
      -- Note: they're all independent, meaning each one generates monsters,
      -- objects, furniture etc separately.
      contents = function()
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
-----x-----]], contents = function(m)
            des.region({ region={1,1,3,3}, type="ordinary", filled=1 })
            des.region({ region={1,7,3,9}, type="ordinary", filled=1 })
            des.region({ region={7,1,9,3}, type="ordinary", filled=1 })
            des.region({ region={7,7,9,9}, type="ordinary", filled=1 })
         end })
      end,
   },
   {
      name = "Barbell-shaped room, horizontal",
      contents = function()
         des.map({ map = [[
-----xxx-----
|...-----...|
|...........|
|...-----...|
-----xxx-----]], contents = function(m)
            -- we can only use filler_region if there is no special terrain, such
            -- as doors. so, only do one or the other.
            if percent(50) then
               filler_region(2,2)
            else
               des.region({ region={2,2,2,2}, type="themed", irregular=true,
                           filled=1 })
               des.door("random", 04, 02)
               des.door("random", 08, 02)
            end
         end })
      end,
   },
   {
      name = "Barbell-shaped room, vertical",
      contents = function()
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
-----]], contents = function(m)
            -- as above, doors are mutually exclusive with filler_region
            if percent(50) then
               filler_region(2,2)
            else
               des.region({ region={2,2,2,2}, type="themed", irregular=true,
                           filled=1 })
               des.door("random", 02, 04)
               des.door("random", 02, 08)
            end
         end })
      end,
   },
   {
      name = "Room with small pillars", -- (also, possibly wood nymph room)
      contents = function()
         des.room({ type = "themed", w = 5 + nh.rn2(3)*2, h = 5 + nh.rn2(3)*2,
                  filled = 1, contents = function(rm)
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
               des.replace_terrain({ selection = selection.area(0,0,rm.width-1,rm.height-1),
                                    fromterrain = '.', toterrain='g' })
               if percent(50) then
                  des.object({ id = "statue", montype = "wood nymph" })
               end
               if percent(20) then
                  if percent(50) then
                     des.object({ id = "statue", montype = "wood nymph" })
                  else
                     des.object({ id = "figurine", montype = "wood nymph",
                                 material = "wooden" })
                  end
               end
               local nnymphs = math.floor(d(nh.level_difficulty()) / 4)
               for i=1,nnymphs do
                  des.monster("wood nymph")
               end
            end
         end })
      end,
   },
   {
      name = "Boomerang-shaped, rot 1",
      contents = function()
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
-----xxxxx]], contents = function(m) filler_region(2,2) end })
      end,
   },
   {
      name = "Boomerang-shaped, rot 2",
      contents = function()
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
xxxxx-----]], contents = function(rm) filler_region(7,1) end })
      end,
   },
   {
      -- Note: a 5x5 version of this room could be confused for Mausoleum
      name = "Rectangular 1-wide ring around a blocked center",
      contents = function()
         des.room({ type = "themed", filled = 1,
                  contents = function(rm)
            for x = 1, rm.width - 2 do
               for y = 1, rm.height - 2 do
                  des.terrain(x, y, "-")
               end
            end
         end })
      end,
   },
   {
      name = "Tiny cage, big monster",
      contents = function()
         des.map({ map = [[
-------
|.....|
|.FFF.|
|.F.F.|
|.FFF.|
|.....|
-------]], contents = function(m)
            des.region({ region={1,1,5,5}, type="themed", irregular=false,
                        filled=1 })
            if percent(80) then
               local mons = { 'M', 'Z', 'O', 'T' }
               if nh.level_difficulty() > 6 then
                  table.insert(mons, 'H')
                  if nh.level_difficulty() > 12 then
                     table.insert(mons, 'D')
                  end
               end
               shuffle(mons)
               des.monster(mons[1], 3,3)
            else
               des.trap({ x = 3, y = 3})
            end
         end })
      end,
   },
   {
      name = "Split room",
      contents = function()
         des.room({ type = "ordinary", filled = 1,
                  w = 3 + nh.rn2(10), h = 3 + nh.rn2(4),
                  contents = function(rm)
            local doorstates = { "open", "closed", "locked" }
            local doiron = nh.level_difficulty() > 10 and percent(30)
            if nh.level_difficulty() >= 5 and not doiron then
               -- a secret door in a wall of iron bars doesn't make
               -- sense...
               table.insert(doorstates, "secret")
            end
            shuffle(doorstates)

            local rndx = nh.rn2(rm.width - 2) + 1
            local rndy = nh.rn2(rm.height - 2) + 1
            local repltyp -- TODO: this is dumb, replace_terrain with
                        -- fromterrain='w' doesn't work

            if percent(50) then
               des.room({ type = "ordinary", x = 0, y = 0,
                        w = rndx, h = rm.height, filled = 1,
                        contents = function()
                  des.door({ state = doorstates[1], wall = "east",
                           iron = doiron })
               end })
               repltyp = '|'
            else
               des.room({ type = "ordinary", x = 0, y = 0,
                        w = rm.width, h = rndy, filled = 1,
                        contents = function()
                  des.door({ state = doorstates[1], wall = "south",
                           iron = doiron })
               end })
               repltyp = '-'
            end
            if doiron then
               des.replace_terrain({ selection = selection.area(0,0,rm.width-1,rm.height-1),
                                    fromterrain = repltyp, toterrain = 'F' })
            end
         end })
      end,
   },
   {
      name = "Corner 2x2 subroom",
      contents = function()
         des.room({ type = "themed", filled = 1,
                  w = 4 + nh.rn2(9), h = 4 + nh.rn2(3),
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
            des.room({ type = "themed", x = rmx, y = rmy, w = 2, h = 2,
                     filled = 1, contents = function()
               des.door({ state = doorstates[1], wall = "random" })
            end })
         end })
      end,
   },
   {
      name = "Storeroom vault",
      maxdiff = 14,
      contents = function()
         des.room({ type = "themed", filled = 0, w = 2, h = 2, joined = false,
                    contents = function(rm)
            for i=1,d(3) do
               des.object("chest")
            end
            way_out_method(true)
         end })
      end
   },
   {
      name = "Storeroom vault v2",
      contents = function()
         -- TODO: the nut of figuring out how to have a room that is only joinable
         -- at certain points/sides still hasn't been cracked. Thus the part of
         -- this room that wraps up and to the left. In the initial proposal, this
         -- was supposed to be a room connected only on one side, with the
         -- storeroom component in the dead end.
         des.map({ map=[[
--------
|......|
|-----.|
|..|...|
|..|...|
--------]], contents = function()
            des.region({ region = {01,01,01,01}, type = 'themed', filled = 0,
                        irregular = 1 })
            des.region({ region = {01,03,02,04}, type = 'themed', filled = 0,
                        joined = false, contents = function()
               for i = 1, d(2) do
                  local boxtype = percent(4) and 'ice box'
                                 or percent(50) and 'chest' or 'large box'
                  des.object(boxtype)
               end
               for i = 1, d(4) do
                  des.monster('r')
               end
               if percent(20) then
                  des.door({ wall="east", locked = 1, iron = 1,
                           state = percent(50) and 'secret' or 'closed' })
               end
               way_out_method(true)
            end })
         end })
      end,
   },
   {
      name = "Crossed X of water",
      contents = function()
         -- FIXME: This breaks the rule that the space in front of a door should
         -- always be free of traps or dangerous terrain, but we can't address that
         -- here since no doors exist yet.
         des.room({ type = "themed", filled = 1,
                  w = 5 + nh.rn2(9), h = 5 + nh.rn2(3),
                  contents = function(rm)
            des.terrain(selection.line(0, 0, rm.width - 1, rm.height - 1), "}")
            des.terrain(selection.line(0, rm.height - 1, rm.width - 1, 0), "}")
         end })
      end,
   },
   {
      name = "Mini maze",
      mindiff = 6,
      contents = function()
         des.map({ map = [[
-----------
|.|.|.|.|.|
|---------|
|.|.|.|.|.|
|---------|
|.|.|.|.|.|
|---------|
|.|.|.|.|.|
-----------]], contents = function(map)
            -- des.mazewalk is not intended for only a small area in the level
            -- to be a maze, and has a number of pitfalls and weird breakages
            -- that make it more trouble than it's worth to use here.
            -- Instead, do a bare reimplementation of a mazewalk and simply
            -- break down walls until every maze space is connected.
            local connected = {}
            mazewalk_step = function(x, y)
               local nsew = { { 0, -1 }, { 0, 1 }, { 1, 0 }, { -1, 0 } }
               shuffle(nsew)
               for i = 1, #nsew do
                  local dx = nsew[i][1]
                  local dy = nsew[i][2]
                  local cx = x + (nsew[i][1] * 2)
                  local cy = y + (nsew[i][2] * 2)
                  if (cx > 0 and cy > 0 and cx < map.width and cy < map.height
                      and not connected[cx..','..cy]) then
                     des.terrain(x + dx, y + dy, '.')
                     connected[cx..','..cy] = true
                     mazewalk_step(cx, cy)
                  end
               end
            end
            connected['5,3'] = true
            mazewalk_step(5, 3)
            des.region({ region={3,3,3,3}, type="themed", irregular=true,
                         filled=1, joined=true, lit=1 })
         end })
      end
   },
   {
      name = "Bunch of closets",
      mindiff = 5,
      contents = function()
         des.room({ type = "themed", filled = 1, joined = true,
                    w = 3 + nh.rn2(6)*2, h = 5, contents = function(rm)
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
                        local doiron = percent(nh.level_difficulty() * 4)
                        des.door({ x = x, y = y, iron = doiron })
                     else
                        -- wall column
                        des.terrain(x, y, '-')
                     end
                  end
               end
            end
         end })
      end
   },
   {
      name = "Beehives",
      mindiff = 10, -- minimum beehive depth in rand_roomtype()
      contents = function()
         local nhives = d(3)
         des.room({ type = "themed", w = 1 + 5*nhives, h = 6,
                    contents = function()
            for i=1,nhives do
               des.room({ type = "beehive", w = 2, h = 2, x = (i*5) - 3, y = 2,
                          filled = 1, contents = function()
                  des.door({ wall = "random" })
               end })
            end
         end })
      end
   },
   {
      name = "Super Honeycomb",
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

               -- update reached; one of the two must have been true so set both
               -- to true
               reached[conns[pick][1]] = true
               reached[conns[pick][2]] = true

               -- recompute allconnected
               nreached = nreached + 1
            end
            des.region({ region={05,01,05,01}, type="beehive", irregular=true,
                         joined=true, filled=1 })
         end })
      end
   },
   {
      name = "Random terrain room v2",
      -- (terrain is randomly applied to any square, not just the center)
      contents = function()
         des.room({ type="themed", filled=1, contents=function(rm)
            local picks = selection.floodfill(1,1):percentage(30)
            local feature = { "C", "I", 'g' }
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
         end })
      end,
   },
   {
      name = "Swimming pool", -- (ring of dry land surrounding water)
      mindiff = 5,
      contents = function()
         des.room({ type="themed", filled=0, contents=function(rm)
            local poolarea = selection.fillrect(1,1,rm.width-2,rm.height-2)
            des.terrain(poolarea, '}')
            -- spice it up with some sea monsters
            local waterarea = (rm.width-2)*(rm.height-2)
            local nmonsters = math.min(d(5), waterarea/2)
            for i=1,nmonsters do
               des.monster(';')
            end
            -- sunken treasure
            if percent(50) then
               des.object({ id='chest', coord=poolarea:rndcoord(1),
                            contents=function()
                  for i=1,d(2,2) do
                     des.object('*')
                  end
                  des.object({ id = "gold piece", quantity = d(80, 5) })
               end })
            end
         end })
      end
   },
   {
      name = "Anti swimming pool", -- (ring of water surrounding a dry platform)
      mindiff = 14,
      contents = function()
         des.room({ type="themed", filled=0, contents = function(rm)
            local water = selection.rect(0, 0, rm.width-1, rm.height-1)
            des.terrain(water, '}')
            for i = 1, d(3) do
               des.monster(';')
            end
         end })
      end
   },
   {
      name = "Thin long horizontal room",
      contents = function()
         local width = 14 + d(6)
         local height = d(2)
         -- only make an ordinary room if it's 2 spaces high; otherwise it might
         -- turn into a shop with its door on the top/bottom and have nowhere to
         -- put items
         local typ = (height == 1) and "themed" or "ordinary"
         des.room({ type=typ, filled=1, w=width, h=height })
      end,
   },
   {
      name = "Ozymandias' Tomb",
      mindiff = 18,
      contents = function()
         des.room({ type="themed", filled=0, w=7, h=7, contents=function()
            des.feature("throne", 3, 3)
            for i = 1, 2+d(4) do
               des.trap({ type="web", spider_on_web = false })
            end
            for i=1,4 do
               if percent(75) then
                  des.trap('falling rock')
               end
            end
            for i=1,d(3)+1 do
               des.trap('hole')
            end
            des.object({ id = "chest", trapped = 1 })
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
            des.engraving({ coord={x,y}, type="engrave",
                            text="My name is Ozymandias, king of kings: Look on my works, ye Mighty, and despair!" })
         end })
      end
   },
   {
      -- a homage to the /dev/null pool challenge, named "Billiards" instead of
      -- "Pool room" to avoid confusion with the swimming pool room
      name = "Billiards room",
      contents = function()
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
            des.object("quarterstaff") -- cue stick
         end })
      end,
   },
   {
      name = "Four-way circle-and-cross room",
      contents = function()
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
            des.region({ region = {5,1,5,1}, irregular=1,
                        type=centerfeature and "themed" or "ordinary" })
            if centerfeature then
               des.terrain(5, 5, percent(20) and 'T' or '{')
            end
         end })
      end,
   },
   {
      -- Like the other four-room cluster, each room here generates its own
      -- monsters, items and features.
      name = "Four 3x3 rooms, directly adjacent",
      contents = function()
         des.room({ type="ordinary", w=7, h=7, filled=1, contents=function()
            des.room({ type="ordinary", x=1, y=1, w=3, h=3, filled=1 })
            des.room({ type="ordinary", x=4, y=1, w=3, h=3, filled=1,
                     contents=function()
               des.door({ state="random", wall = "west" })
               des.door({ state="random", wall = "south" })
            end })
            des.room({ type="ordinary", x=1, y=4, w=3, h=3, filled=1,
                     contents=function()
               des.door({ state="random", wall = "north" })
               des.door({ state="random", wall = "east" })
            end })
            -- the southeast room is just the parent room and doesn't need to
            -- be defined specially; in fact, if it is, the level generator may
            -- stumble on trying to place a feature in the parent room and not
            -- finding any open spaces for it.
         end })
      end,
   },
   {
      name = "Prison cell",
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
               des.monster({ id="prisoner", x=d(3), y=3+d(2), peaceful=1,
                             asleep=1 })
               -- and a rat for company
               -- no 'r' = rock moles to break them out!
               rats = {"sewer rat", "giant rat", "rabid rat"} 
               des.monster(rats[d(#rats)])
            end
            des.region({ region={01,01,01,01}, type="themed", irregular=true,
                         filled=1, joined=true })
            des.region({ region={01,04,03,05}, type="themed", irregular=true,
                         filled=0, joined=false })
         end })
      end
   },
   {
      name = "Mirrored obstacles, sort of like a Rorschasch figure",
      -- obstacles can impede stairways in unlucky cases; put this after Mines
      mindiff = 5,
      contents = function()
         width = 5 + nh.rn2(10)
         height = 5 + nh.rn2(4)
         des.room({ type="themed", w=width, h=height, contents=function(rm)
            -- no grass/ice; not obstacles
            obstacles = { 'T', '}', 'F', 'L', 'C' } 
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
   {
      name = "Dragon hall",
      mindiff = nh.mon_difficulty('black dragon') + 1,
      contents = function()
         des.map({ map = [[
xxxxxx----xx----xxx
xxxx---..----..--xx
xxx--...........--x
x---.............--
--................|
|................--
|...............--x
----..........---xx
xxx--........--xxxx
xxxx----....--xxxxx
xxxxxxx------xxxxxx]], contents = function()
            des.region({ region = {04,04,04,04}, irregular = true, filled = 0,
                         type = "themed", joined = true })
            local floor = selection.floodfill(04, 04)
            local hoardctr = floor:rndcoord()

            function loot(x, y)
               local choice = d(25)
               if choice == 1 then
                  des.object('(', x, y)
               elseif choice == 2 then
                  des.object({ class=')', x=x, y=y, spe=2+nh.rn2(3) })
               elseif choice == 3 then
                  des.object({ class='[', x=x, y=y, spe=2+nh.rn2(3) })
               elseif choice == 4 then
                  des.object('chest', x, y)
               elseif choice >= 5 and choice <= 7 then
                  des.object('=', x, y)
               elseif choice == 8 then
                  des.object(percent(50) and '?' or '+', x, y)
               else
                  des.object('*', x, y)
               end
               -- recursive 10% chance for more loot
               if percent(10) then
                  loot(x, y)
               end
            end
            local goldpile = selection.circle(hoardctr.x, hoardctr.y, 5, 1)
            goldpile:filter_mapchar('.'):iterate(function(x,y)
               local dist2 = (x-hoardctr.x) * (x-hoardctr.x) + (y-hoardctr.y) * (y-hoardctr.y)
               if (dist2 >= 20 and percent(20)) -- radius 4-5
                  or (dist2 >= 12 and dist2 < 20 and percent(50)) -- radius 3-4
                  or dist2 < 12 then -- radius 0-3
                  des.object({ id = 'gold piece', x = x, y = y,
                               quantity = 200 - dist2*5 + d(50) })
               end
               -- given circle radius of 5, practical max for dist2 is 29
               if percent(40 - (dist2 * 2)) then
                  loot(x, y)
               end
               -- dragon eggs
               if dist2 < 3 and percent(80) then
                  des.object({ id = 'egg', x = x, y = y, montype = 'D' })
               end
            end)

            -- put some pits and traps down
            local nonpile = (floor ~ goldpile) & floor
            for i = 1, 2+d(2) do
               des.trap("pit", nonpile:rndcoord())
               des.trap(nonpile:rndcoord())
            end

            -- no way to specifically force a baby or adult dragon
            -- so we have to do it manually
            colors = { 'gray', 'silver', 'red', 'white', 'orange', 'black',
                       'blue', 'green', 'yellow', 'gold' }
            for i = 1, 3+d(3) do
               des.monster({ id='baby '..colors[d(#colors)]..' dragon',
                             coord=goldpile:rndcoord(), waiting=1 })
            end
            for i = 1, 5 + d(5) do
               -- would be neat if the adult dragons here could be buffed
               des.monster({ id=colors[d(#colors)]..' dragon',
                             coord=goldpile:rndcoord(), waiting=1 })
            end
            -- TODO: problem with this room is that the dragons start picking
            -- up the hoard. Something needs to tell their AI that it's a
            -- dragon hoard and shouldn't be picked up.
         end })
      end
   },
   {
      name = "Triple rhombus",
      contents = function()
         des.map({ map = [[
-------xxx-------
|.....--x--.....|
--.....---.....--
x--.....|.....--x
xx--.........--xx
xxx-----.-----xxx
xxxxxx--.--xxxxxx
xxxxx--...--xxxxx
xxxxx|.....|xxxxx
xxxxx|.....|xxxxx
xxxxx--...--xxxxx
xxxxxx--.--xxxxxx
xxxxxxx---xxxxxxx]], contents = function() filler_region(1,1) end })
      end,
   },
   {
      name = "Spiral",
      contents = function()
         des.map({ map = [[
x-----------xxx
--.........----
|..-------....|
|.--.....----.|
|.|..---....|.|
|.|.--.----.|.|
|.|.|...+.|.|.|
|.|.--.--.|.|.|
|.|..---..|.|.|
|.--.....--.|.|
|..-------..|--
--.........--xx
x------------xx]], contents = function()
            des.region({ region = {02,01,02,01}, type = 'themed', irregular = true,
                        filled = 1 })
            des.region({ region = {06,06,06,06}, type = 'themed', irregular = true,
                        joined = false, contents = function()
               local choice = d(5)
               if choice == 1 then
                  des.feature({ type='tree', x=01, y=01 })
               elseif choice == 2 then
                  des.feature({ type='fountain', x=01, y=01 })
               elseif choice == 3 then
                  if percent(50) then
                     des.altar({ x=01, y=01 })
                  else
                     des.feature({ type='throne', x=01, y=01 })
                  end
               elseif choice == 4 then
                  for i=1,3 do
                     des.object()
                  end
               end -- choice 5 = nothing
               des.monster()
            end })
            des.door("random", 08, 06)
         end })
      end,
   },
   {
      name = "Kitchen", -- idea by spicycebolla
      contents = function()
         des.room({ type = 'themed', w = d(4)+8, h = 2, filled = 0,
                  contents = function()
            -- furniture...
            for i = 1, d(2) do
               des.feature('sink')
            end
            if percent(50) then
               des.object('ice box')
            end
            -- cookware and other useful things...
            for i = 1, d(3)-1 do
               des.object('dented pot')
            end
            for i = 1, d(3) do
               des.object('knife')
            end
            if percent(70) then
               des.object('tin opener')
            end
            if percent(20) then
               des.object('tinning kit')
            end
            if percent(20) then
               des.object('towel')
            end
            -- sauces and stocks...
            for i = 1, d(3)-1 do
               des.object({ class='!', id='oil' })
            end
            for i = 1, d(3)-1 do
               des.object({ class='!', id='fruit juice' })
            end
            if percent(50) then
               des.object({ class='!', id='booze' })
            end
            -- raw ingredients...
            for i = 1, d(3) do
               des.object('tin')
            end
            local ingr = { 'meatball', 'egg', 'apple', 'orange', 'pear', 'melon',
                           'banana', 'carrot', 'clove of garlic', 'fruit',
                           'meat stick' }
            for i = 1, d(4) + 4 do
               local ingrdt = ingr[d(#ingr)]
               des.object({ class='%', id=ingr[d(#ingr)] })
            end
         end })
      end,
   },
   {
      name = "Abandoned shop",
      mindiff = 16,
      contents = function()
         des.room({ type = "shop", filled = 2, contents = function(rm)
            local size = rm.width * rm.height
            for i = 1, math.floor(size / 5) + d(3) do
               des.monster('m')
               if percent(35) then
                  des.object()
               end
            end
         end })
      end
   },
   {
      name = "Irregular anthole",
      mindiff = nh.mon_difficulty('soldier ant') + 4,
      contents = function()
         des.map({ map = [[
...............
...............
...............
...............
...............
...............
...............]], contents = function()
            local room = selection.area(00, 00, 14, 06)
            local origroom = room:clone()
            local center = selection.ellipse(07, 03, 7, 3, 1)
            room = room ~ center -- outermost edge
            des.replace_terrain({ selection=room, fromterrain='.',
                                  toterrain=' ', chance=80 })
            room = center
            center = selection.ellipse(07, 03, 5, 2, 1)
            room = room ~ center -- a bit further in...
            des.replace_terrain({ selection=room, fromterrain='.',
                                  toterrain=' ', chance=60 })
            room = center
            center = selection.ellipse(07, 03, 3, 1, 1)
            room = room ~ center -- even further in...
            des.replace_terrain({ selection=room, fromterrain='.',
                                  toterrain=' ', chance=40 })
            des.replace_terrain({ selection=center, fromterrain='.', 
                                  toterrain=' ', chance=20 })
            des.terrain(07, 03, '.')
            -- now clear out any orphaned disconnected spaces not accessible
            -- from the center
            local orphans = origroom:filter_mapchar('.')
                            ~ selection.floodfill(07, 03, true)
            des.terrain(orphans, ' ')
            -- finally, mark it up as an anthole
            des.region({ region={07,03,07,03}, type = "anthole", filled = 1,
                         joined = 1, irregular = true })
            des.wallify()
         end })
      end
   },
   {
      name = 'The Casque of Amontillado',
      mindiff = 8, -- arbitrary
      contents = function()
         des.map({ map = [[
----------xx
|........|xx
|........|--
|........|.|
------------]], contents = function()
            des.region({ region = {01,01,08,03}, type="themed", filled=1 })
            -- FIXME: Despite leaving the right room edge open for joining,
            -- corridors can sometimes still just fail to connect, leaving
            -- either this room isolated or the level somehow unconnected
            des.region({ region = {10,03,10,03}, type="themed", filled=0, joined = false })
            des.engraving({10,03}, "engrave", "Amontillado!");
            des.object({ id = "corpse", montype = "human", coord = {10,03} })
            des.object({ class = "!", id = "booze", coord = {10,03} })
         end })
      end
   },
   {
      name = 'Triple Rhombus v2',
      contents = function()
         des.map({ map = [[
xxx---xxxxx---xxxxx---xxx
xxx|.|xxxxx|.|xxxxx|.|xxx
xx--.--xxx--.--xxx--.--xx
xx|...|xxx|...|xxx|...|xx
---...-----...-----...---
|.......|.......|.......|
|.......................|
|.......|.......|.......|
---...-----...-----...---
xx|...|xxx|...|xxx|...|xx
xx--.--xxx--.--xxx--.--xx
xxx|.|xxxxx|.|xxxxx|.|xxx
xxx---xxxxx---xxxxx---xxx]], contents = function()
            -- originally this had boundary syms in the gaps and did 3 separate
            -- filler_regions, but that causes some very strange things to
            -- happen when the rooms attempt to join. So just make it 1 big
            -- region.
            filler_region(04,06);
         end })
      end,
   },
   {
      name = 'Spikes',
      contents = function()
         des.map({ map = [[
xx---xxx---xxx---xx
xx|.|xxx|.|xxx|.|xx
x--.--x--.--x--.--x
x|...|x|...|x|...|x
x|...|x|...|x|...|x
--...---...---...--
|.....|.....|.....|
|.................|
-------------------]], contents = function()
            if percent(90) then
               filler_region(03,03);
            else
               -- not an "ordinary" region because we don't want it to get
               -- spiked pits and then become a shop
               des.region({ region={03,03,03,03}, irregular = true,
                            type="themed", filled=1 })
               for i = 1, 5 do
                  des.trap('spiked pit')
               end
            end
         end })
      end,
   },
   {
      name = 'Pennants',
      contents = function()
         des.map({ map = [[
-------------------
|.................|
|.....|.....|.....|
--...---...---...--
x|...|x|...|x|...|x
x|...|x|...|x|...|x
x--.--x--.--x--.--x
xx|.|xxx|.|xxx|.|xx
xx---xxx---xxx---xx]], contents = function()
            if percent(10) then
               local poss_x = {3, 9, 15}
               shuffle(poss_x)
               local cloth = obj.new('piece of cloth')
               cloth:placeobj(poss_x[1], 7)
            end
            filler_region(03,03);
         end })
      end,
   },
   {
      name = 'Wizard study',
      mindiff = 14, -- arbitrary but should be fairly deep
      contents = function()
         des.room({ type = 'themed', w = 3, h = 3, filled = 0, joined = false,
                    contents = function()
            local sel = selection.room()
            sel:set(01, 01, 0) -- leave the center free
            des.trap({ type = 'teleport', coord = sel:rndcoord(1) })
            if percent(10) then
               if percent(50) then
                  des.object({ id = 'magic marker', coord = sel:rndcoord(1) })
               else
                  des.object({ id = 'cloak of magic resistance',
                               coord = sel:rndcoord(1) })
               end
            end
            -- the remaining items don't use rndcoord(1); they can be stacked on
            -- top of each other, but won't appear on top of the other items
            -- which do use rndcoord(1)
            for i = 1, 3 do
               des.object({ class = '+', coord = sel:rndcoord() })
            end
            des.object({ class = '?', coord = sel:rndcoord() })
            for i = 1, 3 do
               local choice = d(4)
               if choice == 1 then
                  des.object({ class = '?', coord = sel:rndcoord() })
               elseif choice == 2 then
                  des.object({ class = '=', coord = sel:rndcoord() })
               elseif choice == 3 then
                  des.object({ class = '/', coord = sel:rndcoord() })
               elseif choice == 4 then
                  des.object({ class = '"', coord = sel:rndcoord() })
               end
            end
         end })
      end,
   },
   -- there is no "Ring, small" room because it would only block 1 single tile
   -- in the center
   {
      name = 'Ring, medium',
      contents = function()
         des.map({ map = [[
xx-----xx
x--...--x
--..|..--
|..---..|
|.-| |-.|
|..---..|
--..|..--
x--...--x
xx-----xx]], contents = function(m) filler_region(04,01); end });
      end,
   },
   {
      name = 'Ring, big',
      contents = function()
         des.map({ map = [[
xxx-----xxx
x---...---x
x-...|...-x
--.-----.--
|..|   |..|
|.-|   |-.|
|..|   |..|
--.-----.--
x-...|...-x
x---...---x
xxx-----xxx]], contents = function(m) filler_region(05,01); end });
      end,
   },
   {
      -- ported from a gehennom special room (originally named just "statuary"
      -- but that exists already as a themeroom fill name): room eerily lined
      -- with empty statues of the player
      name = "Player statuary",
      mindiff = 15, -- arbitrary, but this shouldn't appear early
      contents = function()
         -- artificially increase the rarity of this fill even more than the
         -- default frequency = 1; originally this room had only a 1% chance
         -- among Gehennom special rooms
         local donothing = percent(75)
         des.room({ type = "themed", filled = donothing and 1 or 0,
                    contents = function(rm)
            if donothing then
               return
            end
            local mkstatue = function(x, y)
               des.object({ id = 'statue', montype = u.role, name = u.name,
                            contents = function() end, coord = {x, y} })
            end
            -- pick the longer dimension to place statues (vertical if equal)
            if rm.width > rm.height then
               for x = 0, rm.width - 1 do
                  mkstatue(x, 0)
                  mkstatue(x, rm.height - 1)
               end
            else
               for y = 0, rm.height - 1 do
                  mkstatue(0, y)
                  mkstatue(rm.width-1, y)
               end
            end
         end })
      end
   }
};

-- store these at global scope, they will be reinitialized in
-- pre_themerooms_generate
debug_rm_idx = nil
debug_fill_idx = nil

-- Given a point in a themed room, ensure that themed room is stocked with
-- regular room contents.
-- With 30% chance, also give it a random themed fill.
function filler_region(x, y)
   local rmtyp = "ordinary";
   local func = nil;
   if (percent(30)) then
      rmtyp = "themed";
      func = themeroom_fill;
   end
   des.region({ region={x,y,x,y}, type=rmtyp, irregular=true, filled=1, contents = func });
end

-- Is the themed room template or themed room filler teplate "room" eligible to
-- generate where it's being considered?
-- "mkrm" is either nil or the actual des.room being considered.
function is_eligible(room, mkrm)
   local t = type(room);
   local diff = nh.level_difficulty();
   if (room.mindiff ~= nil and diff < room.mindiff) then
      return false
   elseif (room.maxdiff ~= nil and diff > room.maxdiff) then
      return false
   end
   if (mkrm ~= nil and room.eligible ~= nil) then
      return room.eligible(mkrm);
   end
   return true
end

-- given the name of a themed room or fill, return its index in that array
function lookup_by_name(name, checkfills)
   if name == nil then
      return nil
   end
   if checkfills then
      for i = 1, #themeroom_fills do
         if themeroom_fills[i].name == name then
            return i
         end
      end
   else
      for i = 1, #themerooms do
         if themerooms[i].name == name then
            return i
         end
      end
   end
   return nil
end

-- called repeatedly until the core decides there are enough rooms
function themerooms_generate()
   if debug_rm_idx ~= nil then
      -- room may not be suitable for stairs/portals, so create the "default"
      -- room half of the time
      -- (if the user specified BOTH a room and a fill, presumably they are
      -- interested in what happens when that room gets that fill, so don't
      -- bother generating default-with-fill rooms as happens below)
      local actualrm = lookup_by_name("default", false);
      if percent(50) then
         if is_eligible(themerooms[debug_rm_idx]) then
            actualrm = debug_rm_idx
         else
            pline("Warning: themeroom '"..themerooms[debug_rm_idx].name
                  .."' is ineligible")
         end
      end
      themerooms[actualrm].contents();
      return
   elseif debug_fill_idx ~= nil then
      -- when a fill is requested but not a room, still create the "default"
      -- room half of the time, and "default with themed fill" half of the time
      -- (themeroom_fill will take care of guaranteeing the fill in it)
      local actualrm = lookup_by_name(percent(50) and "Default room with themed fill"
                                                  or "default")
      themerooms[actualrm].contents();
      return
   end
   local pick = nil;
   local total_frequency = 0;
   for i = 1, #themerooms do
      if (type(themerooms[i]) ~= "table") then
         nh.impossible('themed room '..i..' is not a table')
      elseif is_eligible(themerooms[i], nil) then
         -- Reservoir sampling: select one room from the set of eligible rooms,
         -- which may change on different levels because of level difficulty.
         local this_frequency;
         if (themerooms[i].frequency ~= nil) then
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
   if pick == nil then
      nh.impossible('no eligible themed rooms?')
      return
   end
   themerooms[pick].contents();
end

-- called before any rooms are generated
function pre_themerooms_generate()
   local debug_themerm = nh.debug_themerm(false)
   local debug_fill = nh.debug_themerm(true)
   debug_rm_idx = lookup_by_name(debug_themerm, false)
   debug_fill_idx = lookup_by_name(debug_fill, true)
   if debug_themerm ~= nil and debug_rm_idx == nil then
      pline("Warning: themeroom '"..debug_themerm
            .."' not found in themerooms", true)
   end
   if debug_fill ~= nil and debug_fill_idx == nil then
      pline("Warning: themeroom fill '"..debug_fill
            .."' not found in themeroom_fills", true)
   end
end

-- called after all rooms have been generated
-- but before creating connecting corridors/doors, or filling rooms
function post_themerooms_generate()
end

function themeroom_fill(rm)
   if debug_fill_idx ~= nil then
      if is_eligible(themeroom_fills[debug_fill_idx], rm) then
         themeroom_fills[debug_fill_idx].contents(rm);
      else
         -- ideally this would be a debugpline, not a full pline, and offer
         -- some more context on whether it failed because of difficulty or
         -- because of eligible function returning false; the warning doesn't
         -- necessarily mean anything.
         pline("Warning: fill '"..themeroom_fills[debug_fill_idx].name
               .."' is not eligible in room that generated it")
      end
      return
   end
   local pick = nil;
   local total_frequency = 0;
   for i = 1, #themeroom_fills do
      if (type(themeroom_fills[i]) ~= "table") then
         nh.impossible('themeroom fill '..i..' must be a table')
      elseif is_eligible(themeroom_fills[i], rm) then
         -- Reservoir sampling: select one room from the set of eligible fills,
         -- which may change on different levels because of level difficulty.
         local this_frequency;
         if (themeroom_fills[i].frequency ~= nil) then
            this_frequency = themeroom_fills[i].frequency;
         else
            this_frequency = 1;
         end
         total_frequency = total_frequency + this_frequency;
         -- avoid rn2(0) if a fill has freq 0
         if this_frequency > 0 and nh.rn2(total_frequency) < this_frequency then
            pick = i;
         end
      end
   end
   if pick == nil then
      nh.impossible('no eligible themed room fills?')
      return
   end
   themeroom_fills[pick].contents(rm);
end

-- postprocess callback: create an engraving pointing at a location
function make_dig_engraving(data)
   local floors = selection.negate():filter_mapchar(".");
   local pos = floors:rndcoord(0);
   local tx = data.x - pos.x - 1;
   local ty = data.y - pos.y;
   local dig = "";
   if (tx == 0 and ty == 0) then
      dig = " here";
   else
      if (tx < 0 or tx > 0) then
         dig = string.format(" %i %s", math.abs(tx), (tx > 0) and "east" or "west");
      end
      if (ty < 0 or ty > 0) then
         dig = dig .. string.format(" %i %s", math.abs(ty), (ty > 0) and "south" or "north");
      end
   end
   des.engraving({ coord = pos, type = "burn", text = "Dig" .. dig });
end

-- postprocess callback: turn room walls into trees
function make_garden_walls(data)
   local sel = data.sel:grow();
   des.replace_terrain({ selection = sel, fromterrain="w", toterrain = "T" });
end

-- postprocess callback: make a trap
function make_a_trap(data)
   if (data.teledest == 1 and data.type == "teleport") then
      local locs = selection.negate():filter_mapchar(".");
      repeat
         data.teledest = locs:rndcoord(1);
      until (data.teledest.x ~= data.coord.x and data.teledest.y ~= data.coord.y);
   end
   des.trap(data);
end

-- called once after the whole level has been generated
function post_level_generate()
   for i, v in ipairs(postprocess) do
      v.handler(v.data);
   end
   postprocess = { };
end
