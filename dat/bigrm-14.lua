-- "Tea Party" by jonadab

des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel", "noflip");
des.map([[
------------------------------------ ------------------------------------
|..................................| |..................................|
|--..............................-------..............................--|
|.........--..--....................|....................--..--.........|
|-....---..|S-|..---..............-----..............---..|-S|..---....-|
|.......|--|..|--|........--..--.........--..--........|--|..|--|.......|
----..---..|--|..---...--..|--|..--...--..|--|..--...---..|--|..---..----
 |........--..--........|--|  |--|.....|--|  |--|........--..--........| 
 --....................--..|--|..--...--..|--|..--....................-- 
  |.......................--..--.........--..--.......................|  
  |...................................................................|  
  |.......................--..--.........--..--.......................|  
 --....................--..|--|..--...--..|--|..--....................-- 
 |........--..--........|--|  |--|.....|--|  |--|........--..--........| 
----..---..|--|..---...--..|--|..--...--..|--|..--...---..|--|..---..----
|.......|--|..|--|........--..--.........--..--........|--|..|--|.......|
|-....---..|S-|..---..............-----..............---..|-S|..---....-|
|.........--..--....................|....................--..--.........|
|--..............................-------..............................--|
|..................................| |..................................|
------------------------------------ ------------------------------------
]]);
-- Dungeon Description
des.region(selection.area(01,01,72,20),"lit");
-- Stairs
des.stair("up");
des.stair("down");
-- Non diggable walls
des.non_diggable();
----------------------------------------------------------------------------------
des.door({ state="locked", x=12, y=04 })
des.door({ state="locked", x=12, y=16 })
des.door({ state="locked", x=60, y=04 })
des.door({ state="locked", x=60, y=16 })

-- Objects
for i = 1,15 do
   des.object();
end
-- Random traps
for i = 1,6 do
   des.trap();
end
-- Random monsters
for i = 1,28 do
   des.monster();
end

-- Guests.
-- Most of these names came directly from an 8 year old girl. And in one case,
-- her father, who obsesses over woodchucks named Carl.
rndslimy = function()
   classes = { "b", "j", "P" };
   return classes[d(#classes)];
end
guests = {
   { name = "Blinky Pinky", montype = "pony" },
   { name = "Mister Frosty", montype = nh.rn2(2) and "frost giant" or "ice troll" },
   { name = "Miss Betsy" },
   { name = "Fred" },
   { name = "Bob" },
   { name = "Joe" },
   { name = "Gooey", montype=rndslimy() },
   { name = "Goopy", montype=rndslimy() },
   { name = "Goopifer", montype=rndslimy() },
   { name = "Susie" },
   { name = "Tootles" },
   { name = "Cindy Lou" },
   { name = "Mr Spotty Pants" },
   { name = "Banana Joe", montype="Y" },
   { name = "Carl", montype="woodchuck" }
}
places = { {03,10},{69,10},{25,01},{47,01},{25,19},{47,19} }
shuffle(guests)
shuffle(places)
for i = 1, d(3) + 1 do
   statue = { id="statue", name=guests[i].name, x=places[i][1], y=places[i][2] };
   if guests[i].montype ~= nil then
      statue.montype = guests[i].montype;
   end
   des.object(statue)
end
