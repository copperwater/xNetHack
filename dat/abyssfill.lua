-- The Abyss filler levels
-- Most of the level is open air, with a walkable fringe around the top or
-- bottom (only need to implement one) constituting part of a narrow rim hinting
-- at a vast circular gulf.
-- Possibly, falling down from any upper level drops you all the way to the
-- bottom, rather than standard behavior of dropping you to the next one down.

des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel");

des.map([[
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
...........................................................................
]]);

local foo = selection.ellipse(nh.rn2(75),-5, 90, 20, 1);
des.replace_terrain({ selection=foo, fromterrain='.', toterrain='A' })

-- assumption is that the ellipse will leave spaces of floor on the left and
-- right edges
des.levregion({ region={00,00,00,17}, type="stair-up" })
des.levregion({ region={74,00,74,17}, type="stair-down" })
des.levregion({ region={00,17,74,17}, type="branch" })

-- Define areas
local everything = selection.area(00,00,74,19)

local demotivations = {
   'Cast yourself down.',
   'Descent is the only option.',
   'Nothing waits for you above.',
   'Existence is meaningless.',
   'Know the sweetness of oblivion.',
   'The void is calling.',
   'Jump in and embrace true emptiness.',
   'To strive is a mistake.',
   'Sit on the edge and let it crumble beneath you.',
   'Bravery is a fiction. Only by jumping in will you know it.',
   'The true victory is to give up.',
   'Nothing matters.',
   'It only gets worse after this.',
   'You can never succeed.',
   'Every triumph you find will soon feel hollow.',
   'Surrender to your despair.'
}
local engrspots = everything:filter_mapchar('.')
for i = 1, d(3) do
   des.engraving({ coord=engrspots:rndcoord(),
                   type=percent(90) and 'blood' or 'burn',
                   text=demotivations[d(#demotivations)] })
end

-- Monsters
for i = 1, 8 + d(5) do
   des.monster({ class='&', align=percent(90) and 'chaos' or 'random' })
   if percent(30) then
      des.monster('i')
   end
end
for i = 1, d(4) do
   des.monster()
end
des.monster('L')

-- Traps
for i = 1, 15 do
   des.trap()
end

-- Objects
for i = 1, 3 + d(2) do
   des.object()
end
