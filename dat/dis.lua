-- City of Dis: about halfway down Gehennom, ruled from its citadel by Dispater.
-- Replaces Orcus's ghost town as the built-up area in the main branch of
-- Gehennom.

des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel", "hardfloor", "nommap-boss");

des.message('You approach a city shrouded in gloom.')
des.message('A great inverted pyramid towers above it.')
des.map([[
.....................}|.....|..|.......|......|.....--......................
.....................}|.....+..|.......|......|.....--............-----.----
.....................}|.....|..+..---..|--+----...........---------..|..|...
.....................}|.....|..|..|.----..........---------.........--..|...
....................}}|------..----.......---------..---...........--...|---
..................}}}--..............------.........--.---.........|....|...
.................}}---.......--+--....---...........|....-+-......--..-+-...
.................}--....-----|...--.....---........--......---....|...|.....
.................}|.....|....|....----....---......+.........---.--...|-----
.................}|.....|....|...--..----...---...--.......-------...--.....
.................}|.....--...|...|......|.....---.|..----+--....|....|......
.................}--.....--+------+------.......------.........--....|......
.................}}---.........|....|.............---.........--.....--+----
..................}}}--........|.|..|.---+---.......---.......|.............
....................}}|.------...|....|.....|.--+--...---....--.---+----....
.....................}|.|....|----..--|.....|.|...|.....---..|..|......|.---
.....................}|.|....|........|.....|.|...---.....----..|......|....
.....................}|.+....|........-------.|.....|..--...-...|......|....
.....................}|.|....|................-------..--.......|......|....
]]);

des.non_diggable()

-- Phasing is allowed but not past the outer wall or into the pyramid
des.non_passwall(selection.area(18,00,22,18))
des.non_passwall(selection.area(54,01,70,05))
des.non_passwall(selection.area(37,05,41,07))
des.non_passwall(selection.area(42,03,68,12))
des.non_passwall(selection.area(52,13,58,15))
des.non_passwall(selection.area(58,13,62,17))

-- Constrain arrival location by levelport
des.teleport_region({ region={00,00,12,18}, dir="down" })

-- Special areas: a seminary (adopted from the old random special SEMINARY room)
des.region({ region={39,14,43,16}, type="temple", lit=1, filled=1, contents = function()
   for i = 1, d(3) do
      des.monster({ id='aligned cleric', align='noalign', peaceful=0 })
   end
end })
-- this must come after the temple declaration for some reason, otherwise no
-- priest
des.altar({ x=41, y=15, align="noalign", type="shrine" })
--- and a graveyard
des.region({ region={30,16,37,18}, lit=0, type="morgue", filled=1 })

-- City doors
des.door("random", 28,01)
des.door("random", 31,02)
des.door("random", 42,02)
des.door("random", 31,06)
des.door("random", 71,06)
des.door("random", 27,11)
des.door("random", 34,11)
des.door("random", 71,12)
des.door("random", 41,13)
des.door("random", 48,14)
des.door("random", 67,14)
des.door("random", 24,17)
-- Pyramid doors
des.door("random", 51,08)
des.door("random", 58,06)
des.door("random", 57,10)
local outerdoorspots = { {68,01}, {61,02}, {69,02}, {52,03}, {56,03}, {47,04},
                         {41,05}, {67,05}, {41,07}, {47,10}, {64,10}, {53,13},
                         {57,15}, {61,15} }
shuffle(outerdoorspots)
des.door({ state="secret", locked=true, iron=true,
           x=outerdoorspots[1][1], y=outerdoorspots[1][2] })
-- Stairs to the upper level of the Citadel
local innerstairs = selection.floodfill(54,08):rndcoord()
des.levregion({ region={innerstairs.x,innerstairs.y,innerstairs.x,innerstairs.y},
                type='branch' })
-- Wall "ruins"
des.object("boulder", 34,15)
des.object("boulder", 35,15)
des.object("boulder", 72,15)

if percent(50) then
   des.stair("down", 75,00)
else
   des.stair("down", 75,14)
end
local upstairy = nh.rn2(19)
local outside = selection.floodfill(03,03)
local swamp_possibility = outside:clone()
-- force the space outside the drawbridge to be dry
swamp_possibility:set(16,09, 0)
-- also force a small area around the upstairs to be dry
swamp_possibility = swamp_possibility - selection.circle(00, upstairy, 1, 1)
des.replace_terrain({ selection=swamp_possibility,
                      fromterrain='.', toterrain='}', chance=60 })
local outside_dry = outside:filter_mapchar('.')
des.stair("up", 00, upstairy)
-- add some grass to the swamp with the same chance as in styxmarsh to indicate
-- that it's the same swamp
des.replace_terrain({ selection=swamp_possibility, fromterrain='.', toterrain='g', chance=40 })

-- The gate
des.drawbridge({ dir="east", state="closed", x=17, y=09 })

-- Define regions in the city
local inpyramid = selection.floodfill(47,07) + selection.floodfill(54,08) +
                  selection.floodfill(62,05) + selection.floodfill(57,12)
local incity = selection.area(19,00,75,18):filter_mapchar('.')
-- remove areas outside the walls that the initial .area() included
incity = incity - selection.area(19,00,21,04) - selection.area(19,14,21,18)
local incity_nopyramid = incity - inpyramid

-- Various traps, not necessarily inside the walls
des.trap("spiked pit")
des.trap("spiked pit")
des.trap("sleep gas")
des.trap("sleep gas")
des.trap("anti magic")
des.trap("anti magic")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("magic")
des.trap("magic")
des.trap("magic")
des.trap()
des.trap()
des.trap()

-- Random objects (note that the shops will be stocked with their own objects)
for i = 1,10 do
   des.object()
end

-- Monsters outside
for i = 1, 2 + d(2) do
   des.monster({ class = ';', coord = outside:rndcoord() })
end
for i = 1, 3 do
   des.monster({ id = 'erinys', coord = selection.area(14,06,16,12):rndcoord() })
end

-- Monsters inside
function docitymon(str, number)
   local template = { }
   if string.len(str) == 1 then
      template['class'] = str
   else
      template['id'] = str
   end
   for i = 1, number do
      template['coord'] = incity:rndcoord()
      des.monster(template)
   end
end

docitymon('shade', 3)
docitymon('horned devil', 3)
docitymon('barbed devil', 3)
docitymon('V', 2)
docitymon('imp', 3 + d(3))
docitymon('lemure', 5 + d(2))
docitymon('&', 2 + d(3))

-- Monsters inside the pyramid
for i = 1, 2 do
   des.monster({ id = 'shade', coord = inpyramid:rndcoord() })
   des.monster({ id = 'horned devil', coord = inpyramid:rndcoord() })
   des.monster({ id = 'barbed devil', coord = inpyramid:rndcoord() })
end

-- Monsters wherever
for i = 1, 4 do
   des.monster('ghost')
   des.monster()
end
if percent(80) then
   des.monster('Nazgul')
end
