#PROG
vnum 2101
descripcion MP GREET Troll~
code if (hastarget(ent)) return;
if (race(actor) != race_lookup("troll"))
{
  mob(ent,"say ALTO AHI!");
  mob(ent,"say DE QUE LADO ESTAS? OGROS O TROLLS?");
  mob(ent,"remember " + nombre(actor));
  mob(ent,"delay 10");
}
~
#END

