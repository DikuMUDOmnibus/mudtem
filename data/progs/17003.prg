#PROG
vnum 17003
descripcion Conde dracula~
code if (level(actor) > 45)

{

if(race(actor) == race_lookup("vampire"))

mob(ent,"say " + nombre(actor) + " eres digno de pelear a mi lado... unete a #BMI#b");

else

{

mob(ent,"say " + nombre(actor) + " eres digno de luchar conmigo... Empecemos");

interpret(ent,"emote #Bse lanza contra TI#b");

mob(ent,"kill " + nombre(actor));

}

}

else

{

if ( race(actor) == race_lookup("vampire"))

{

mob(ent,"say " + nombre(actor) + " no eres digno de pelear con tu maestro");

mob(ent,"transfer " + nombre(actor) + " 2231");

}

else

{

mob(ent,"say " + nombre(actor) + " no eres digno de pelear conmigo");

interpret(actor,"remove espada");

interpret(actor,"drop espada");

mob(ent,"transfer " + nombre(actor) + " 2231");

}

}

~
#END

