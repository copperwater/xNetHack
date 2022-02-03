
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
