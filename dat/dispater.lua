-- Dispater's Citadel

des.level_init({ style = "solidfill", fg = " " })
des.level_flags("mazelevel", "noflip", "noteleport", "nommap-boss")

--234567890123456789012345678901234567890
des.map([[
                                 --------
                    --------------.....| 
      ---------------...........--....-- 
-------...--.....................-+-..|  
 --......--........................----  
  ---....+..........................--   
    ---.--..........--+--..........--    
      ---..........--...--.........|     
       ---.........+.....+.......---     
         ---.......--...--.......--      
           --.......--+--.......--       
            ---................--        
              ---..............|         
                ---.......------         
                  --.---+--..--          
                   ---......--           
                     ---....|            
                       ---.--            
                         ---             
]])

des.non_diggable()
des.non_passwall()

-- Corners
des.door({ state="locked", iron=1, x=09, y=05 })
des.door({ state="locked", iron=1, x=34, y=03 })
des.door({ state="locked", iron=1, x=24, y=14 })

local corner1 = selection.floodfill(04,04)
local corner2 = selection.floodfill(35,01)
local corner3 = selection.floodfill(24,16)
local allcorners = corner1 + corner2 + corner3

allcorners:iterate(function(x,y)
   if percent(40) then
      if percent(75) then
         des.monster('shade', x, y)
      else
         des.monster('W', x, y)
      end
   end
   if percent(30) then
      des.object({ coord={x,y} })
   end
end)

if nh.is_wish_dlord('Dispater') then
   local lootspots = { {38,01}, {03,04}, {26,17} }
   shuffle(lootspots)
   des.object({ id='chest', locked=1, coord=lootspots[1], material='iron', contents=function()
      des.object({ class='/', id='wishing', spe=1 })
   end })
end

-- Center (stairway room)
des.door({ state="closed", iron=1, x=22, y=06 })
des.door({ state="closed", iron=1, x=19, y=08 })
des.door({ state="closed", iron=1, x=25, y=08 })
des.door({ state="closed", iron=1, x=22, y=10 })
des.levregion({ region={22,08,22,08}, type='branch' })

-- Everything in between
local bigroom = selection.floodfill(21,04)

des.monster({ id = 'Dispater', waiting = 1, coord = bigroom:rndcoord(), inventory = function()
   -- this ought to be a quarterstaff but it isn't because he can't wield that
   -- while wearing a shield
   des.object({ id = 'club', material = 'iron', spe = 3 + d(3) })
   des.object({ id = 'large shield', material = 'iron', spe = 3 + d(3) })
   des.object({ id = 'plate mail', material = 'iron', spe = 3 + d(3) })
end })

for i = 1, 4 + d(6) do
   des.monster({ class = '&', align = 'law', coord = bigroom:rndcoord() })
end
for i = 1, d(2) do
   des.monster({ id = 'pit fiend', coord = bigroom:rndcoord() })
end
for i = 1, d(3) do
   des.monster({ id = 'iron golem', coord = bigroom:rndcoord() })
end
for i = 1,20 do
   des.object({ id = 'iron chain', coord = bigroom:rndcoord() })
end
