
/* Definicion de tipos de datos */
typedef unsigned char T_UBYTE;

/* Enumeraciones */
//Esta enumeracion corresponde al numero de cabinas a controlar
typedef enum CABINAS
{
  CABINA0,
  CABINA1,
  N_CABINAS
}T_CABINA;

typedef enum PLANTAS
{
  PLANTA0,
  PLANTA1,
  PLANTA2,
  NPLANTAS
}T_PLANTA;

typedef enum DIRECCION
{
  SUBIENDO,
  BAJANDO,
  DETENIDO,
  N_DIRECCIONES
}T_DIRECCION;

typedef enum PUERTA
{
  ABIERTA,
  CERRADA,
  N_ESTADOS_PUERTA
}T_PUERTA;

/* Definicion de pines */
#define Cabina0_Planta0_Led		14
#define Cabina0_Planta1_Led	15
#define Cabina0_Planta2_Led	16
#define Cabina0_Subiendo_Led	17
#define Cabina0_Bajando_Led	18
#define Cabina0_Puerta_Abierta	19
#define Cabina0_Planta0_Boton	3
#define Cabina0_Planta1_Boton	4
#define Cabina0_Planta2_Boton	5
#define Cabina0_Power_Source	0
#define Cabina1_Planta0_Led	14
#define Cabina1_Planta1_Led	15
#define Cabina1_Planta2_Led	16
#define Cabina1_Subiendo_Led	17
#define Cabina1_Bajando_Led	18
#define Cabina1_Puerta_Abierta	19
#define Cabina1_Planta0_Boton	3
#define Cabina1_Planta1_Boton	4
#define Cabina1_Planta2_Boton	5
#define Cabina1_Power_Source	1
#define Planta0_Sube_Boton	8
#define Planta1_Sube_Boton	9
#define Planta1_Baja_Boton	10
#define Planta2_Baja_Boton	11

