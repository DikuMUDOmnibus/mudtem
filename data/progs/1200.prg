#PROG
vnum 1200
descripcion oprog cama~
code if (isimmort(actor))

{

mob(ent,"say Bienvenido, " + nombre(actor) + "!");

mob(ent,"cast heal " + nombre(actor));

}

else

{

mob(ent,"say Eres arrogante, mortal.");

act("$n desaparece!", actor, NULL, NULL, TO_ROOM);

mob(ent,"transfer " + nombre(actor) + " random");

}

~
#END

