#PROG
vnum 2102
descripcion MP DELAY Troll~
code if (not hastarget(ent))
break;
mob(ent,"say COMO TE ATREVES A IGNORARME MALDITO!!!");
mob(ent,"kill " + nombre(target(ent)));
mob(ent,"forget");
~
#END

