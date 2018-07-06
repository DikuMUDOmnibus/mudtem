#PROG
vnum 17500
descripcion RP: Caida (Poseidon)~
code if (ispc(actor))

{

send_to_room(actor, "<<< LA TRAMPA TE CAPTURA >>>");

send_to_room(actor, "<<<  CAES   -   FALL     >>>");

mob(ent,"damage all 50 100 lethal");

send_to_room(actor, "<<<  CAES   -   FALL     >>>");

send_to_room(actor, "<<<  CAES   -   FALL     >>>");

mob(ent,"damage all 50 100 lethal");

mob(ent,"transfer " + nombre(actor) + " 17515");

send_to_ent(actor,"<<<  PAFF   -   PAFF     >>>\n\r");

}

~
#END

