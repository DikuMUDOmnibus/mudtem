#PROG
vnum 17001
descripcion mpigor saludo~
code if (race(actor) == race_lookup("vampire"))

mob(ent,"say Saludos " + nombre(actor) + ", acaso vienes por mi maestro?");

else

mob(ent,"say Te aconsejo que no entres, " + nombre(actor) + "...tienes un bonito cuello");

~
#END

