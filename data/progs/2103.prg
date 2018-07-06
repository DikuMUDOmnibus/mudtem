#PROG
vnum 2103
descripcion MP SPEECH Troll troll~
code if (not hastarget(ent) or target(ent) != actor)
{
mob(ent,"say QUE QUIERES!");
interpret(ent,"growl " + nombre(actor));
return;
}
mob(ent,"say MAS TE VALE!!!");
interpret(ent,"grin " + nombre(actor));
mob(ent,"forget");
mob(ent,"cancel");
~
#END

