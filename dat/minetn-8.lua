-- Minetown variant 8 (formerly 1)
-- "Frontier Town" - not yet pillaged by orcs

des.room({ type = "ordinary",lit=1,x=25,y=3,
           xalign="left",yalign="top",w=31,h=15,
           contents = function()
              des.feature("fountain", 13,07)
              des.feature("fountain", 20,07)

              if math.random(0,99) < 90 then
                 des.room({ type = "shop", x=2,y=2, w=3,h=4,
                            contents = function()
                               des.door({ state="closed", wall="south" })
                            end
                 })
              end

              if math.random(0,99) < 90 then
                 des.room({ type = "tool shop", x=2,y=9, w=3,h=4,
                            contents = function()
                               des.door({ state="closed", wall="north" })
                            end
                 })
              end

              des.room({ type = "ordinary", x=6,y=2, w=3,h=4,
                         contents = function()
                            des.door({ state="closed", wall="south" })
                         end
              })

              des.room({ type = "ordinary", x=6,y=9, w=3,h=4,
                         contents = function()
                            des.door({ state="closed", wall="north" })
                         end
              })

              if math.random(0,99) < 90 then
                 des.room({ type = "food shop", x=10,y=2, w=2,h=3,
                            contents = function()
                               des.door({ state="closed", wall="south" })
                            end
                 })
              end

              des.room({ type = "candle shop", x=22,y=2, w=3,h=3,
                         contents = function()
                            des.door({ state="closed", wall="south" })
                         end
              })

              des.room({ type = "ordinary", x=10,y=10, w=2,h=3,
                         contents = function()
                            des.door({ state="locked", wall="east" })
                            des.monster("gnome")
                         end
              })

              des.room({ type = "ordinary", x=19,y=2, w=2,h=3,
                         contents = function()
                            des.door({ state="locked", wall="west" })
                            des.monster("gnome")
                         end
              })

              des.room({ type = "temple", x=15,y=9, w=4,h=4,
                         contents = function()
                            des.door({ state="closed", wall="north" })
                            des.altar({ x=02, y=02, align=align[1],type="shrine" })
                            des.monster("gnomish wizard")
                            des.monster("gnomish wizard")
                         end
              })

              des.room({ type = "ordinary", x=22,y=10, w=2,h=3,
                         contents = function()
                            des.door({ state="locked", wall="west" })
                         end
              })

              des.room({ type = "ordinary", x=26,y=2, w=3,h=3,
                         contents = function()
                            des.door({ state="closed", wall="south" })
                            des.monster("gnome lord")
                         end
              })

              des.room({ type = "ordinary", x=25,y=10, w=4,h=3,
                         contents = function()
                            des.door({ state="closed", wall="north" })
                            des.monster("gnome")
                         end
              })

  des.monster({ id = "watchman", peaceful = 1 })
  des.monster({ id = "watchman", peaceful = 1 })
  des.monster({ id = "watchman", peaceful = 1 })
  des.monster({ id = "watchman", peaceful = 1 })
  des.monster({ id = "watch captain", peaceful = 1 })

            end
})

des.room({ contents = function()
              des.stair("up")
                      end
})

des.room({ contents = function()
              des.stair("down")
              des.trap()
              des.monster("gnome")
              des.monster("gnome")
                      end
})

des.room({ contents = function()
              des.monster("dwarf")
                      end
})

des.room({ contents = function()
              des.trap()
              des.monster("gnome")
                      end
})

des.random_corridors()
