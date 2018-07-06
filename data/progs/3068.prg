#PROG
vnum 3068
descripcion MP Cityguard anti-armas~
code if (ispc(actor) and uses(actor,"wield"))

mob(ent,"tell " + nombre(actor) + " Hey! no puedes entrar a Midgaard esgrimiendo armas. Es la ley.");

~
#END

