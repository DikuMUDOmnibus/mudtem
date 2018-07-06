#PROG
vnum 3015
descripcion MP Healer fly~
code if (level(actor) < 10)

{

mob(ent,"cast fly " + nombre(actor));

interpret(ent, "smile " + nombre(actor));

}

else

if (money(actor) < 1000)

mob(ent,"say El servicio cuesta 1000 monedas de plata, " + nombre(actor) + ".");

else

{

mob(ent,"withdraw " + nombre(actor) + " 1000");

mob(ent,"cast fly " + nombre(actor));

}

~
#END

