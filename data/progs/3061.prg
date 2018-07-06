#PROG
vnum 3061
descripcion Cityguard greet prog~
code interpret(ent,"look " + nombre(actor));
if (isgood(actor))
{
  interpret(ent,"say Buenos dias.");
  interpret(ent,"smile " + nombre(actor));
}
else
{
  act("$n se pregunta si $N esta entre los criminales mas buscados.", ent, NULL, actor, TO_NOTVICT);
  act("$n se pregunta si estas entre los criminales mas buscados.", ent, NULL, actor, TO_VICT);
}
~
#END

