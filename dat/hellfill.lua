-- Gehennom "fill" level template for when no specific level occupies the space
-- UNIMPLEMENTED
-- There are a few possible concepts for this, one is a simple cavern fill with
-- lava and lots of demons and traps, with demon dens burrowed into the walls;
-- another is to take a heck^2 approach and have a variety of specific
-- structures which can generate in the level, selected at random.

des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel");

-- no lit state defaults to "random" = lit above Minetown and unlit beneath
des.level_init({ style="mines", fg=".", bg=" ", smoothed=true ,joined=true, walled=true })

--
des.stair("up")
des.stair("down")
