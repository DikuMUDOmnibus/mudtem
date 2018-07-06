#PROG
vnum 3301
descripcion RP: Advertencia Clan Dragonlance~
code if (clan(actor) != clan_lookup("dragonlance"))

send_to_ent(actor,"Algo te dice que seria #BMUY#b peligroso ir hacia el norte.\n\r");

~
#END

