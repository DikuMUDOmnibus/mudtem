#ifndef puntos_h
#define puntos_h

#define PUNTOS_FILENAME DATA_DIR "puntos"
#define LARGO_TABLA_PUNTOS 10
extern	struct	rec_muerte puntajes[MAX_LEVEL/10 + 1][LARGO_TABLA_PUNTOS];

struct rec_muerte {
	int	muertes;
        int	activo;
	int 	nivel;
	char	*name;
	char	*who_name;
};

#endif
