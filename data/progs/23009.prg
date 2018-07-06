#PROG
vnum 23009
descripcion OP:Arma de Troll~
code if (race(actor) != race_lookup("troll"))

{

mob(ent,"cast 'heat metal' " + nombre(actor));

mob(ent,"cast 'heat metal' " + nombre(actor));

mob(ent,"cast 'heat metal' " + nombre(actor));

mob(ent,"cast 'fireball' " + nombre(actor));

mob(ent,"cast 'fireball' " + nombre(actor));

mob(ent,"cast 'fireball' " + nombre(actor));

mob(ent,"oremove " + nombre(actor) + " wield");

mob(ent,"oremove " + nombre(actor) + " secondary");

mob(ent,"odrop " + nombre(actor) + " troll");

}

~
#END

