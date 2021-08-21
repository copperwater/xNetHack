-- Mine end level variant 4
-- "The Gnomish Sewer" by jonadab

des.level_init({ style = "solidfill", fg = " " })
des.level_flags("mazelevel")
des.map([[
                                         }}}                  }}}        }  
   }       }..}       ....           }}}} }.}    }}} }}      }.....}}  .}   
  }}       .}}.....}}..  ..   ..}}}}.......}}}  }   .}     }}.  .  ..}}}.  }
  }        .}  }}} ...    }} }}}....}}     ...}}   .} }}}}...  ..   }.  .}} 
  ..      .}      }  ......}}...            }}..}}}.......}       }}   }}.} 
   .}   ..}}}      } }   . ..}       }}    .}   .... .{.}}....  }}}  .}  }.}
   }. ..}}}}        }    .   .}    }...}...}         ...     ....}}..}.. }. 
   }..}}  . }    } }    {.   .... }. }}. }}...}      .     ...   ..}}  .}.  
  }.      .}    } }}.       }.....}    }.   }}...}....}}} ..        }}  }.  
  }.....  ..  ..    }..    }} ... .}    }}     }}.}}      }}  ......}}}}.}  
 }.  .... }......    }}.  }.     }..}}}...}}.    }. }}....}}... }}}}.... }} 
 .   .......  }...  ...}}}.      .}....}  ..}}  }}....   .... }}   .}  }.  }
 }.       }.    }} }}} .} .     ...}}}  }}}  }}....         }}.     .}  }.} 
  }.      }.     }} ...}.}.    .. }.  }}   }}..}}       }}}}..}}    }.  }.} 
   }}    .}..}}     }.}  }. ....}} }.  }} ...}...}}}. }}   .  ...   }..  }. 
    }  ..}.  ..}}}  .}... }..}}}    }. }...   }}....}}     .    }.}} }..}}. 
    .  .}}     ...}}.  }}..}}        .}..    .}.}}}}. }}}..      }..} }.} . 
   .}. .   }...}  ... ...}}.    }    }.}..   .}}}}  .}}}.}}.     .}}.. }.}}.
  .} }.}. ..}}      }}}}}  }..}}  } }  }}}.}.}}      ...}} }.   .}   .  }}.}
 .}    }}.}        }} }     }}...}  }}... }.}           .}} }}}}..}}}. }}   
  .}}    }       }}}}         }}}.....}}}  .}}}}       }}  }}}..}}.  }.}    
]])

des.stair("up", 31, 8)
des.feature("fountain", 54,5)
des.feature("fountain", 24,7)

-- luckstone and touchstone used to be random but that's a bit too cruel
local stone = { {2,4},{2,18},{21,9},{45,16},{64,14},{62,20},{74,18},{71,1},{59,8} }
shuffle(stone)
des.object({ id="luckstone", coord=stone[1], buc="not-cursed", achievement=1 })
des.object("touchstone", stone[2])

-- The predator of the depths
local eelplace = { {19,7},{56,5},{59,18} }
shuffle(eelplace)
des.monster("giant eel", eelplace[1])

-- Random monsters
des.monster("sewer rat")
des.monster("sewer rat")
des.monster("sewer rat")
des.monster("sewer rat")
des.monster("sewer rat")
des.monster("sewer rat")
des.monster("r")
des.monster("r")
des.monster("r")
des.monster("shrieker")
des.monster("shrieker")
des.monster("F")
des.monster("F")
des.monster("F")
des.monster("F")
des.monster("water moccasin")
des.monster("S")
des.monster("S")
des.monster("S")
-- Janitors?
des.monster("G")
des.monster("G")
-- "alligators"
des.monster("crocodile")
des.monster("crocodile")
des.monster("jellyfish")
des.monster("jellyfish")
des.monster("jellyfish")
des.monster("piranha")
des.monster("piranha")
des.monster("brown pudding")
des.monster("gray ooze")
for i=1,3 do
   if math.random(0,99) < 50 then
      if math.random(0,99) < 50 then
         des.monster("piranha")
      else
         des.monster(";")
      end
   end
end

if math.random(0,99) < 50 then
   des.monster("chameleon")
end
if math.random(0,99) < 75 then
   if math.random(0,99) < 20 then
      des.monster("purple worm")
   else
      des.monster("baby purple worm")
   end
end

if math.random(0,99) < 5 then
   des.monster("gremlin")
else
   des.monster("brown pudding")
   des.monster("gray ooze")
end

-- Objects
-- Why so many rings? Well, a lot of rings fall down sinks, and they end up in
-- the sewer...
des.object("=")
des.object("=")
des.object("=")
des.object("=")
des.object("=")
des.object("=")
des.object("=")
des.object("blank paper")
des.object("blank paper")
for i=1,2 do
   des.object({ id="figurine", montype="j" })
end
-- the method of specifying erosion is odd, with the two least significant bits
-- being oeroded and the next two being oeroded2. We want these to be rotted, so
-- if oeroded2 = 1, then 1 << 2 = 4
des.object({ id = "high boots", material = "leather", eroded = 4 })
-- this may be helpful in crossing water
des.object({ class="/", id = "cold" })
-- one of each soft gemstone
des.object("chrysoberyl")
des.object("amethyst")
des.object("dilithium crystal")
des.object("turquoise")
des.object("citrine")
des.object("amber")
des.object("jet")
des.object("opal")
des.object("garnet")
des.object("jasper")
des.object("fluorite")
des.object("jade")
des.object("obsidian")
des.object("agate")
des.object("worthless piece of red glass")
des.object("worthless piece of orange glass")
des.object("worthless piece of yellow glass")
des.object("worthless piece of yellowish brown glass")
des.object("worthless piece of green glass")
des.object("worthless piece of blue glass")
des.object("worthless piece of violet glass")
des.object("worthless piece of white glass")
des.object("worthless piece of black glass")
des.object("*")
des.object("*")
-- Traps
des.trap("rust")
des.trap("rust")
des.trap("rust")
des.trap("rust")
des.trap("rust")
des.trap("rust")
des.trap()
des.trap()
des.trap()
