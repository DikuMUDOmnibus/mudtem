#PROG
vnum 21033
descripcion Pared Aniquiladora~
code act("Las paredes se cierran!", ent, NULL, NULL, TO_ALL);

act("Las paredes te #B>>> #FANIQUILAN#f <<<#b !!!", ent, NULL, NULL, TO_ALL);

mob(ent,"damage all 150 200 lethal");

~
#END

