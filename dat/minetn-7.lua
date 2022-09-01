-- NetHack mines minetn-7.lua	$NHDT-Date: 1652196032 2022/05/10 15:20:32 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.3 $
--	Copyright (c) 1989-95 by Jean-Christophe Collet
--	Copyright (c) 1991-95 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
-- "Bazaar Town" by Kelly Bailey

-- There can be a multitude of interesting, non-exclusive shops in this town.
-- Only Izchak's and the temple are guaranteed, but it wouldn't be great to use
-- independent chances for all the other ones, because then you can have a wide
-- gulf between games that generate most of or all the shops and games that
-- generate none of them. So this attempts to guarantee a moderate amount of
-- random shops that doesn't tilt the scale too heavily in or against the
-- player's favor.
-- 1. Every potential shop (besides Izchak) is assigned a number. This number
--    roughly represents how "good" this shop is, and how much the allotment for
--    other shop should shrink by adding this. For instance, a 3x3 tool shop
--    ought to get a much higher weight than a 2x2 food shop in terms of its
--    utility to the player.
-- 2. Randomly shuffle these shops and weights.
-- 3. Take an allotment of 100 points, and iterate through the shop/weight
--    pairs, marking each one as to-be-created and deducting the weight from the
--    allotment if there is sufficient allotment left. Skip a shop if there is
--    not sufficient allotment left for it.

-- The "id" strings in this table must be unique.
local shopweights = { {id='general', shtype='shop',         weight=30}, -- 4 items for sale
                      {id='food1',   shtype=monkfoodshop(), weight=15}, -- 4 % items
                      {id='food2',   shtype='food shop',    weight=15}, -- 4 % items; NOT monk food
                      {id='tool1',   shtype='tool shop',    weight=40}, -- 6 ( items
                      {id='tool2',   shtype='tool shop',    weight=25}, -- 4 ( items
                      -- above this is in vanilla; below this is added in xNetHack
                      {id='armor',   shtype='armor shop',   weight=15}, -- 4 [ items
                      {id='weapon',  shtype='weapon shop',  weight=10}, -- 4 ) items
                      {id='scroll',  shtype='scroll shop',  weight=20}, -- 4 ? items
                      {id='ring',    shtype='ring shop',    weight=30}, -- 4 = items
                      {id='wand',    shtype='wand shop',    weight=20}} -- 4 / items
shuffle(shopweights)
-- max number of 4-item shops with allotment=70 is 4 (10 + 15 + 15 + 15, leaving
-- 15 which cannot fit any more shops). In practice there are 7 eligible rooms
-- for 4-item shops, so there's wiggle room for adjusting these numbers.

-- Tuning allotment up and down can control the total amount of shops.
local allotment = 70
local selected = {}
for i=1,#shopweights do
   if allotment >= shopweights[i]['weight'] then
      selected[shopweights[i]['id']] = true
      allotment = allotment - shopweights[i]['weight']
   else
      selected[shopweights[i]['id']] = false
   end
end

for str, domake in pairs(selected) do
   if domake then
      nh.pline(str)
   end
end

des.room({ type="ordinary", lit=1, x=25,y=3,
           xalign="left",yalign="top", w=30,h=15,
           contents = function()
   des.feature("fountain", 12, 07)
   des.feature("fountain", 11, 13)

   -- first the larger tool shop (the room is guaranteed even if there is no shop)

   local rtype = 'ordinary'
   if selected['tool1'] then
      rtype = 'tool shop'
   end
   des.room({ type=rtype, lit=1, x=25,y=2, w=3,h=3, contents = function()
      des.door({ state = "closed", wall="west" })
   end })

   -- potential 4-item shops
   -- There are 7 possible rooms; randomize the shop types along with the
   -- ordinary rooms.

   local room_types = { 'ordinary', 'ordinary', 'ordinary', 'ordinary', 'ordinary',
                        'ordinary', 'ordinary' }
   local idx = 1
   for i = 1, #shopweights do
      if selected[shopweights[i]['id']] then
         room_types[idx] = shopweights[i]['shtype']
         idx = idx + 1
      end
   end

   shuffle(room_types)

   if room_types[1] ~= 'ordinary' or percent(75) then
      des.room({ type=room_types[1], x=2,y=2, w=4,h=2, contents = function()
         des.door({ state = "closed", wall="south" })
      end })
   end

   if room_types[2] ~= 'ordinary' or percent(75) then
      des.room({ type=room_types[2], x=14,y=2, w=4,h=2, contents = function()
         des.door({ state = "closed", wall="south", pos=0 })
      end })
   end

   des.room({ type=room_types[3], lit=1, x=19,y=5, w=2,h=3, contents = function()
      des.door({ state = "closed", wall="south" })
   end })

   des.room({ type=room_types[4], lit=1, x=2,y=10, w=2,h=3, contents = function()
      des.door({ state = "closed", wall="south" })
   end })

   des.room({ type=room_types[5], lit=1, x=14,y=10, w=2,h=3, contents = function()
      des.door({ state = "closed", wall="north" })
   end })

   if room_types[6] ~= 'ordinary' or percent(75) then
      des.room({ type=room_types[6], x=17,y=11, w=4,h=2, contents = function()
         des.door({ state = "closed", wall="north" })
      end })
   end

   des.room({ type=room_types[7], lit=1, x=25,y=11, w=3,h=2, contents = function()
      des.door({ state = "closed", wall="east" })
   end })

   -- the one guaranteed shop

   des.room({ type="candle shop", lit=1, x=5,y=10, w=3,h=3, contents = function()
      des.door({ state = "closed", wall="north" })
   end })

   -- non-shop rooms

   if percent(75) then
      des.room({ type="ordinary", x=7,y=2, w=2,h=2, contents = function()
         des.door({ state = "closed", wall="north" })
      end })
   end

   if percent(75) then
      des.room({ type="ordinary", x=7,y=5, w=2,h=2, contents = function()
         des.door({ state = "closed", wall="south" })
         des.monster("n")
      end })
   end

   if percent(75) then
      des.room({ type="ordinary", lit=1, x=10,y=2, w=3,h=4, contents = function()
         des.monster("gnome")
         des.monster("monkey")
         des.monster("monkey")
         des.monster("monkey")
         des.monster('n')
         des.door({ state = "closed", wall="south" })
         des.door({ state = "closed", wall="west" })
      end })
   end

   if percent(75) then
      des.room({ type="ordinary", x=16,y=5, w=2,h=2, contents = function()
         des.door({ state = "closed", wall="south" })
      end })
   end

   if percent(75) then
      des.room({ type="ordinary", lit=0, x=19,y=2, w=2,h=2, contents = function()
         des.door({ state = "locked", wall="east" })
         des.monster("gnome king")
      end })
   end

   if percent(75) then
      des.room({ type="ordinary", x=2,y=7, w=2,h=2, contents = function()
         des.door({ state = "closed", wall="east" })
      end })
   end

   if percent(75) then
      des.room({ type="ordinary", x=11,y=10, w=2,h=2, contents = function()
         des.door({ state = "locked", wall="west" })
         des.monster("G")
      end })
   end

   if percent(75) then
      des.room({ type="ordinary", x=22,y=11, w=2,h=2, contents = function()
         des.door({ state = "closed", wall="south" })
         des.door({ state = "secret", wall="north" })
         des.feature("sink", 00,00)
      end })
   end

   des.room({ type="temple", lit=1, x=24,y=6, w=4,h=4, contents = function()
      des.door({ state = "closed", wall = "west" })
      des.altar({ x=02, y=01, align=align[1], type="shrine" })
      des.monster("gnomish wizard")
      des.monster("gnomish wizard")
   end })

   des.monster({ id = "watchman", peaceful = 1 })
   des.monster({ id = "watchman", peaceful = 1 })
   des.monster({ id = "watchman", peaceful = 1 })
   des.monster({ id = "watchman", peaceful = 1 })
   des.monster({ id = "watch captain", peaceful = 1 })
   des.monster("gnome")
   des.monster("gnome")
   des.monster("gnome")
   des.monster("gnome lord")
   des.monster("monkey")
   des.monster("monkey")

end })

des.room({ type="ordinary", contents = function()
   des.stair("up")
end })

des.room({ type="ordinary", contents = function()
   des.stair("down")
   des.trap()
   des.monster("gnome")
   des.monster("gnome")
end })

des.room({ type="ordinary", contents = function()
   des.monster("dwarf")
end })

des.room({ type="ordinary", contents = function()
   des.trap()
   des.monster("gnome")
end })

des.random_corridors()
