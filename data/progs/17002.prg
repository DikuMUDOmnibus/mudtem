#PROG
vnum 17002
descripcion Condesa Saludo~
code if (race(actor) == race_lookup("vampire"))

{

mob(ent,"say Porque pelear, " + nombre(actor) + ", si podemos amarnos...");

interpret(ent,"kiss " + nombre(actor));

interpret(ent,"love " + nombre(actor));

interpret(ent,"emote se comienza a sacar la ropa...prenda por prenda");

}

else

{

interpret(ent,"emote #B#Fse lanza hacia ti enterrando sus dientes en tu cuello#b#f");

mob(ent,"kill " + nombre(actor));

}

~
#END

