#PROG
vnum 17000
descripcion greetprog hunter~
code if (race(actor) == race_lookup("vampire"))

mob(ent,"kill " + nombre(actor));

else

mob(ent,"say Bienvenido si no eres vampiro!");

~
#END

