-- NetHack nhlib.lua	$NHDT-Date: 1652196140 2022/05/10 15:22:20 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.4 $
--	Copyright (c) 2021 by Pasi Kallinen
-- NetHack may be freely redistributed.  See license for details.
-- compatibility shim
math.random = function(...)
   local arg = {...};
   if (#arg == 1) then
      return 1 + nh.rn2(arg[1]);
   elseif (#arg == 2) then
      return nh.random(arg[1], arg[2] + 1 - arg[1]);
   else
      -- we don't support reals
      error("NetHack math.random requires at least one parameter");
   end
end

function shuffle(list)
   for i = #list, 2, -1 do
      local j = math.random(i)
      list[i], list[j] = list[j], list[i]
   end
end

align = { "law", "neutral", "chaos" };
shuffle(align);

-- d(2,6) = 2d6
-- d(20) = 1d20 (single argument = implicit 1 die)
function d(dice, faces)
   if (faces == nil) then
      -- 1-arg form: argument "dice" is actually the number of faces
      return math.random(1, dice)
   else
      local sum = 0
      for i=1,dice do
         sum = sum + math.random(1, faces)
      end
      return sum
   end
end

-- percent(20) returns true 20% of the time
function percent(threshold)
   return math.random(0, 99) < threshold
end

function monkfoodshop()
   if (u.role == "Monk") then
      return "health food shop";
   end
   return "food shop";
end

-- find all dead-ends on the level enclosed by walls on 7 sides;
-- return selection containing all such points
function get_deadends()
   local deadendN = [[
www
w.w
wxw]]
   local deadendE = [[
www
x.w
www]]
   local deadendS = [[
wxw
w.w
www]]
   local deadendW = [[
www
w.x
www]]
   return selection.match(deadendN) | selection.match(deadendE) | selection.match(deadendS) | selection.match(deadendW)
end

-- dumb function for stuff that requires a number
function bool2int(b)
   if b then
      return 1
   else
      return 0
   end
end

-- for Cocytus monster generation to avoid repeating this list
function get_icymon_list()
   return { 'ice vortex', 'ice troll', 'freezing sphere', 'winter wolf',
            'blue jelly', 'white dragon', 'brown mold' }
end

-- pline with variable number of arguments
function pline(fmt, ...)
   nh.pline(string.format(fmt, table.unpack({...})));
end

-- wrapper to make calling from nethack core easier
function nh_set_variables_string(key, tbl)
   return "nh_lua_variables[\"" .. key .. "\"]=" .. table_stringify(tbl) .. ";";
end

-- wrapper to make calling from nethack core easier
function nh_get_variables_string(tbl)
   return "return " .. table_stringify(tbl) .. ";";
end

-- return the (simple) table tbl converted into a string
function table_stringify(tbl)
   local str = "";
   for key, value in pairs(tbl) do
      local typ = type(value);
      if (typ == "table") then
         str = str .. "[\"" .. key .. "\"]=" .. table_stringify(value);
      elseif (typ == "string") then
         str = str .. "[\"" .. key .. "\"]=[[" .. value .. "]]";
      elseif (typ == "boolean") then
         str = str .. "[\"" .. key .. "\"]=" .. tostring(value);
      elseif (typ == "number") then
         str = str .. "[\"" .. key .. "\"]=" .. value;
      elseif (typ == "nil") then
         str = str .. "[\"" .. key .. "\"]=nil";
      end
      str = str .. ",";
   end
   -- pline("table_stringify:(%s)", str);
   return "{" .. str .. "}";
end

--
-- TUTORIAL
--

-- extended commands NOT available in tutorial
local tutorial_blacklist_commands = {
   ["save"] = true,
};

function tutorial_cmd_before(cmd)
   -- nh.pline("TUT:cmd_before:" .. cmd);

   if (tutorial_blacklist_commands[cmd]) then
      return false;
   end
   return true;
end

function tutorial_enter()
   -- nh.pline("TUT:enter");
   nh.gamestate();
end

function tutorial_leave()
   -- nh.pline("TUT:leave");

   -- remove the tutorial level callbacks
   nh.callback("cmd_before", "tutorial_cmd_before", true);
   nh.callback("level_enter", "tutorial_enter", true);
   nh.callback("level_leave", "tutorial_leave", true);
   nh.callback("end_turn", "tutorial_turn", true);
   nh.gamestate(true);
end

local tutorial_events = {
   {
      func = function()
         if (u.uhunger < 148) then
            local o = obj.new("blessed food ration");
            o:placeobj(u.ux, u.uy);
            nh.pline("Looks like you're getting hungry.  You'll starve to death, unless you eat something.", true);
            nh.pline("Comestibles are eaten with '" .. nh.eckey("eat") .. "'", true);
            return true;
         end
      end
   },
};

function tutorial_turn()
   for k, v in pairs(tutorial_events) do
      if ((v.ucoord and u.ux == v.ucoord[1] + 3 and u.uy == v.ucoord[2] + 3)
         or (v.ucoord == nil)) then
         if (v.func() or v.remove) then
            tutorial_events[k] = nil;
         end
      end
   end
   -- nh.pline("TUT:turn");
end
