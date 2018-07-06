#PROG
vnum 17502
descripcion MP: pinguino busca hongo~
code look $n

if carries $n hongo

, llora al mirarte.

mob say Maldito!! Tu tomaste mis hongos!!

mob kill $n

mob flee

else

mob say mm.. grr.. donde esta mi alimento.

endif

~
#END

