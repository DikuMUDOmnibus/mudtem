#PROG
vnum 3064
descripcion Janitor rand~
code if (rand(5))

act("$n recoge un poco de basura.", ent, NULL, NULL, TO_ROOM);

else

if (rand(5))

act("$n saca una colilla de cigarrillo de su bolsillo y la enciende.", ent, NULL, NULL, TO_ROOM);

~
#END

