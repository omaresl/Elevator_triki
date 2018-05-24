/* Control de Ascensor */
/* V1.0 */
/* Omar Sevilla */
/* omar.sevilla.lomeli@gmail.com */

/**********Descripción del Proyecto**********/
/*Conjunto de ascensores con dos cabinas y tres plantas 0,1 y 2.
Cada cabina tiene un led por planta, led indicador puerta abierta, led indicador subiendo, led indicador bajando, si tiene puerta cerrada y parado no enciende ningún led, no puede moverse con puerta abierta, y tres botones internos uno por planta.
En planta 0 hay un pulsador llama un ascensor para subir responde el más cercano.
En planta 1 hay dos pulsadores uno para subir otro para bajar, el más cercano es un ascensor en el mismo sentido llegando.
En planta 2 un pulsador para bajar.
Se debe usar la librería FreeRTOS
Explicar las funciones usadas de la librería FreeRTOS.*/

/**********GUIA DE CONEXION DE INDICADORES Y PULSADORES***********/
/*
INDICADORES (LEDS)
Todos los anodos de los leds se conectan a voltaje positivo (VDD) con una resistencia de entre 180 y 330 ohms por cada led
EJEMPLO________________________________________________________________________________________________________
VDD
___
 |                  220 ohms                 + LED -
  ----------------/\/\/\/\/\/\-----------------|>|-------------------------> pin de arduino correspondiente
_______________________________________________________________________________________________________________

Los catodos de cada led corresponden a los pines como a continuacion se describen
-------CABINA 1------
LED_PLANTA 0        pin 14
LED_PLANTA 1        pin 15
LED_PLANTA 2        pin 16
LED_SUBIENDO        pin 17
LED_BAJANDO         pin 18
LED_PUERTA ABIERTA  pin 19

-------CABINA 2------
LED_PLANTA 0        pin 14
LED_PLANTA 1        pin 15
LED_PLANTA 2        pin 16
LED_SUBIENDO        pin 17
LED_BAJANDO         pin 18
LED_PUERTA ABIERTA  pin 19

PULSADORES (BOTONES)
Todos los botones seran conectados a voltaje positivo (VDD) en una de sus terminales
VDD
___                pulsador
 |                ___|___
  ----------------o     o------------------------------------------> pin correspondiente
                                  |               |
                                  |               |
                                  |               |
                              resistencia      ________
                              de 1k a 47k      ________ 100pF a 1uF
                                  |               |
                                _____           _____
                                 GND             GND

La otra terminal sera conectada al pin correspondeiente como se describe a continuacion
-------CABINA 0------
BOTON_PLANTA 0        pin 3
BOTON_PLANTA 1        pin 4
BOTON_PLANTA 2        pin 5

-------CABINA 1------
BOTON_PLANTA 0        pin 3
BOTON_PLANTA 1        pin 4
BOTON_PLANTA 2        pin 5

-------PLANTA 0------
BOTON_SUBE        pin 8

-------PLANTA 1------
BOTON_SUBE        pin 9
BOTON_BAJA        pin 10

-------PLANTA 2------
BOTON_BAJA        pin 11

NOTA: Se recomienda usar una resistencia (pull down) de 1kohm hasta 47kohm para asegurar
un estado logico de "false" cuando no se este presionando. Tambien es recomendable pulsador
capacitores de entre 100pF hasta 1uF en paralelo a esta resistencia para eliminar picos inesperados
(pulsaciones falsas - Bouncing)
 */
/******************************************************************/

/* Librerias y headers */
#include <Arduino_FreeRTOS.h> //FreeRTOS Library
#include "ControlDeAscensor.h" //Header de configuracion

/* Constantes */

/* Variables */
T_PLANTA rub_PlantaActual[N_CABINAS]; //Arreglo de variables donde se almacena la planta actual de cada cabina
T_CABINA rub_MuestraCabina; //Variable que almacena que cabina se esta mostrando en lass salidas de los indicadores
T_DIRECCION re_DireccionActual[N_CABINAS]; //Arreglo de variables que almacena la direccion actual de las cabinas
T_PUERTA re_EstadoDePuerta[N_CABINAS]; //Arreglo que almacena el estado actual de la puerta de cada cabina
T_UBYTE rub_TestModeEnabled; //Variable que habilita o no los tests
T_UBYTE rub_TestStep; //Variable que define step de prueba

T_BOTONES rs_EstadoBotonesActual; //Estructura donde se almacena el estado actual de los botones
T_BOTONES rs_EstadoBotonesAnterior; //Estructura donde se almacena un estado anterior de los botones
T_UBYTE rub_PeticionPendiente; //Variable que indica si hay una accion pendiente de ejecutar
T_PLANTA re_Peticion; //Variable que almacena la peticion a realizar
T_CABINA re_PeticionCabina; //Cabina que ejecutara la peticion
T_DIRECCION re_UltimaDireccion[N_CABINAS]; //Arreglo que indica cual fue su ultima direccion de movimiento

/* Declaracion de funciones */
void app_InitPins(void);
void app_InitSys(void);
void app_OutputMngr( void *pvParameters );
void app_InputMngr( void *pvParameters );
void app_MovementMngr( void *pvParameters );
void app_MultiplexMngr( void *pvParameters );
void app_TestSequence( void *pvParameters );

/***********************************************************************************
 * Nombre: setup
 * Descripcion:  Inicializacion de sistema. El codigo
 *               contenido en esta funcion corre solo una vez al inicio.
 * Parametros:   N/A
 * Retrono:      N/A
 ************************************************************************************/
void setup() {

	/* Funcion de Configuracion de Pines */
	app_InitPins();

	/* Funcion de inicializacion del sistema */
	app_InitSys();

	/* Creacion de Tarea de muestra de salidas */
	xTaskCreate(
			app_OutputMngr //Funcion que sera añadida al scheduler
			,  (const portCHAR *)"Output Manager"   // Identificador de tarea (No relevante para funcionalidad)
			,  128  // Tamaño del stack
			,  NULL
			,  0  // Prioridad (3 es la mas alta)
			,  NULL );

	/* Creacion de Tarea de Multiplexor */
	xTaskCreate(
			app_MultiplexMngr //Funcion que sera añadida al scheduler
			,  (const portCHAR *)"Multiplexor Manager"   // Identificador de tarea (No relevante para funcionalidad)
			,  128  // Tamaño del stack
			,  NULL
			,  0  // Prioridad (3 es la mas alta)
			,  NULL );

	/* Modo Prueba */
	rub_TestModeEnabled = false;//Prueba Desactivada
	//Modo prueba esta activado?
	if(true == rub_TestModeEnabled)
	{
		/* Creacion de Tarea de prueba */
		xTaskCreate(
				app_TestSequence //Funcion que sera añadida al scheduler
				,  (const portCHAR *)"Test"   // Identificador de tarea (No relevante para funcionalidad)
				,  128  // Tamaño del stack
				,  NULL
				,  0  // Prioridad (3 es la mas alta)
				,  NULL );
		//Inicializar maquina de estados de prueba
		rub_TestStep = 0;
	}
	else
	{

		/* Creacion de Tarea de muestreo de entradas */
		xTaskCreate(
				app_InputMngr //Funcion que sera añadida al scheduler
				,  (const portCHAR *)"Input Manager"   // Identificador de tarea (No relevante para funcionalidad)
				,  128  // Tamaño del stack
				,  NULL
				,  1  // Prioridad (3 es la mas alta)
				,  NULL );

		/* Creacion de Tarea de movimiento de ascensor */
		xTaskCreate(
				app_MovementMngr //Funcion que sera añadida al scheduler
				,  (const portCHAR *)"Movement Manager"   // Identificador de tarea (No relevante para funcionalidad)
				,  128  // Tamaño del stack
				,  NULL
				,  0  // Prioridad (3 es la mas alta)
				,  NULL );
	}
}

/***********************************************************************************
 * Nombre: loop
 * Descripcion:  Tarea equivalente al main. El codigo dentro de esta funcion
 *               se repetira de manera indefinida.
 * Parametros:   N/A
 * Retrono:      N/A
 ************************************************************************************/
void loop() {
	// Las funcionalidades de este sistema se ejecutaran en el FreeRTOS. Esta funcion quedara vacia

}

/***********************************************************************************
 * Nombre: app_InitPins
 * Descripcion:  En esta funcion se inicializan todos los pines que seran usados durante
 *               la ejecucucion del sistema
 * Parametros:   N/A
 * Retrono:      N/A
 ************************************************************************************/
void app_InitPins(void)
{
	pinMode(Cabina0_Planta0_Led    ,OUTPUT); digitalWrite(Cabina0_Planta0_Led   ,HIGH);  //Configuracion de pin de Salida de LED Indicador Planta 0
	pinMode(Cabina0_Planta1_Led     ,OUTPUT); digitalWrite(Cabina0_Planta1_Led   ,HIGH);  //Configuracion de pin de Salida de LED Indicador Planta 1
	pinMode(Cabina0_Planta2_Led     ,OUTPUT); digitalWrite(Cabina0_Planta2_Led   ,HIGH);  //Configuracion de pin de Salida de LED Indicador Planta 2
	pinMode(Cabina0_Subiendo_Led  ,OUTPUT); digitalWrite(Cabina0_Subiendo_Led  ,HIGH);  //Configuracion de pin de Salida de LED Indicador de Subida
	pinMode(Cabina0_Bajando_Led     ,OUTPUT); digitalWrite(Cabina0_Bajando_Led   ,HIGH);  //Configuracion de pin de Salida de LED Indicador de Bajada
	pinMode(Cabina0_Puerta_Abierta  ,OUTPUT); digitalWrite(Cabina0_Puerta_Abierta,HIGH);  //Configuracion de pin de Salida de LED Indicador de Puerta Abierta
	pinMode(Cabina0_Planta0_Boton   ,INPUT);  //Configuracion de pin de Entrada de boton para selección de planta 0
	pinMode(Cabina0_Planta1_Boton   ,INPUT);  //Configuracion de pin de Entrada de boton para selección de planta 1
	pinMode(Cabina0_Planta2_Boton   ,INPUT);  //Configuracion de pin de Entrada de boton para selección de planta 2
	pinMode(Cabina0_Power_Source  ,OUTPUT); digitalWrite(Cabina0_Power_Source  ,LOW);  //Configuracion de pin de Salida de transistor que activara fuente para cabina 0
	pinMode(Cabina1_Planta0_Led     ,OUTPUT); digitalWrite(Cabina1_Planta0_Led   ,HIGH);  //Configuracion de pin de Salida de LED Indicador Planta 0
	pinMode(Cabina1_Planta1_Led     ,OUTPUT); digitalWrite(Cabina1_Planta1_Led   ,HIGH);  //Configuracion de pin de Salida de LED Indicador Planta 1
	pinMode(Cabina1_Planta2_Led     ,OUTPUT); digitalWrite(Cabina1_Planta2_Led   ,HIGH);  //Configuracion de pin de Salida de LED Indicador Planta 2
	pinMode(Cabina1_Subiendo_Led  ,OUTPUT); digitalWrite(Cabina1_Subiendo_Led  ,HIGH);  //Configuracion de pin de Salida de LED Indicador de Subida
	pinMode(Cabina1_Bajando_Led     ,OUTPUT); digitalWrite(Cabina1_Bajando_Led   ,HIGH);  //Configuracion de pin de Salida de LED Indicador de Bajada
	pinMode(Cabina1_Puerta_Abierta  ,OUTPUT); digitalWrite(Cabina1_Puerta_Abierta,HIGH);  //Configuracion de pin de Salida de LED Indicador de Puerta Abierta
	pinMode(Cabina1_Planta0_Boton   ,INPUT);  //Configuracion de pin de Entrada de boton para selección de planta 0
	pinMode(Cabina1_Planta1_Boton   ,INPUT);  //Configuracion de pin de Entrada de boton para selección de planta 1
	pinMode(Cabina1_Planta2_Boton   ,INPUT);  //Configuracion de pin de Entrada de boton para selección de planta 2
	pinMode(Cabina1_Power_Source  ,OUTPUT); digitalWrite(Cabina1_Power_Source  ,LOW); //Configuracion de pin de Salida de transistor que activara fuente para cabina 1
	pinMode(Planta0_Sube_Boton      ,INPUT);  //Configuracion de pin de Entrada de boton para llamada de ascensor a subir
	pinMode(Planta1_Sube_Boton      ,INPUT);  //Configuracion de pin de Entrada de boton para llamada de ascensor a subir
	pinMode(Planta1_Baja_Boton      ,INPUT);  //Configuracion de pin de Entrada de boton para llamada de ascensor a bajar
	pinMode(Planta2_Baja_Boton      ,INPUT);  //Configuracion de pin de Entrada de boton para llamada de ascensor a bajar
}

/***********************************************************************************
 * Nombre: app_InitSys
 * Descripcion:  En esta funcion se inicializan todas las variables relevantes para
 *               el correcto funcionamiento del sistema
 * Parametros:   N/A
 * Retrono:      N/A
 ************************************************************************************/
void app_InitSys(void)
{
	/* Inicializar plantas de cabinas */
	rub_PlantaActual[CABINA0] = PLANTA0; //Inicia Cabina0 en Planta0
	rub_PlantaActual[CABINA1] = PLANTA0; //Inicia Cabina1 en Planta0

	/* Inicializa Muestra de Indicadores de Cabina */
	rub_MuestraCabina = CABINA0; //Inicia Mostrando Cabina 0

	//Inicializa direcciones
	re_DireccionActual[CABINA0] = DETENIDO; //Inicia cabina 0
	re_DireccionActual[CABINA1] = DETENIDO; //Inicia cabina 1
	re_UltimaDireccion[CABINA0] = SUBIENDO; //Inicializa Ultima Direccion de cabina 0
	re_UltimaDireccion[CABINA1] = SUBIENDO; //Inicializa Ultima Direccion de cabina 1

	//Inicializa la puerta cerrada
	re_EstadoDePuerta[CABINA0] = CERRADA; //Inicia puerta cerrada en cabina 0
	re_EstadoDePuerta[CABINA1] = CERRADA; //Inicia puerta cerrada en cabina 1

	//Inicializa estado actual
	rs_EstadoBotonesActual.Cabina0_Planta0 = false; //Inicializa estado de la entrada asignada al boton de Cabina 0 Planta 0
	rs_EstadoBotonesActual.Cabina0_Planta1 = false; //Inicializa estado de la entrada asignada al boton de Cabina 0 Planta 1
	rs_EstadoBotonesActual.Cabina0_Planta2 = false; //Inicializa estado de la entrada asignada al boton de Cabina 0 Planta 2
	rs_EstadoBotonesActual.Cabina1_Planta0 = false; //Inicializa estado de la entrada asignada al boton de Cabina 1 Planta 0
	rs_EstadoBotonesActual.Cabina1_Planta1 = false; //Inicializa estado de la entrada asignada al boton de Cabina 1 Planta 1
	rs_EstadoBotonesActual.Cabina1_Planta2 = false; //Inicializa estado de la entrada asignada al boton de Cabina 1 Planta 2
	rs_EstadoBotonesActual.Planta0_Sube = false; //Inicializa estado de la entrada asignada al boton de Planta 0 Sube
	rs_EstadoBotonesActual.Planta1_Sube = false; //Inicializa estado de la entrada asignada al boton de Planta 1 Sube
	rs_EstadoBotonesActual.Planta1_Baja = false; //Inicializa estado de la entrada asignada al boton de Planta 1 Baja
	rs_EstadoBotonesActual.Planta2_Baja = false; //Inicializa estado de la entrada asignada al boton de Planta 2 Baja

	//Inicializa estado anterior
	rs_EstadoBotonesAnterior.Cabina0_Planta0 = false; //Inicializa estado de la entrada asignada al boton de Cabina 0 Planta 0
	rs_EstadoBotonesAnterior.Cabina0_Planta1 = false; //Inicializa estado de la entrada asignada al boton de Cabina 0 Planta 1
	rs_EstadoBotonesAnterior.Cabina0_Planta2 = false; //Inicializa estado de la entrada asignada al boton de Cabina 0 Planta 2
	rs_EstadoBotonesAnterior.Cabina1_Planta0 = false; //Inicializa estado de la entrada asignada al boton de Cabina 1 Planta 0
	rs_EstadoBotonesAnterior.Cabina1_Planta1 = false; //Inicializa estado de la entrada asignada al boton de Cabina 1 Planta 1
	rs_EstadoBotonesAnterior.Cabina1_Planta2 = false; //Inicializa estado de la entrada asignada al boton de Cabina 1 Planta 2
	rs_EstadoBotonesAnterior.Planta0_Sube = false; //Inicializa estado de la entrada asignada al boton de Planta 0 Sube
	rs_EstadoBotonesAnterior.Planta1_Sube = false; //Inicializa estado de la entrada asignada al boton de Planta 1 Sube
	rs_EstadoBotonesAnterior.Planta1_Baja = false; //Inicializa estado de la entrada asignada al boton de Planta 1 Baja
	rs_EstadoBotonesAnterior.Planta2_Baja = false; //Inicializa estado de la entrada asignada al boton de Planta 2 Baja

	//Inicializa peticion pendiente
	rub_PeticionPendiente = false;
}

/*****************************************************************************************************/
/****************************************FreeRTOS Tasks***********************************************/
/*****************************************************************************************************/

/***********************************************************************************
 * Nombre: app_OutputMngr
 * Descripcion:  Essta funcion administra el estado de las salidas dependiendo
 *               el estado de variables de los indicadores como los leds
 *               de planta,led de puerta abierta, leds de movimiento
 *
 * Parametros:   void *pvParameters - parametro FreeRTOS
 * Retrono:      N/A
 ************************************************************************************/
void app_OutputMngr( void *pvParameters )
{
	(void) pvParameters;

	for(;;)
	{
		//Periodo de la tarea. 30 ms
		vTaskDelay( 1 / portTICK_PERIOD_MS );

		//Apaga las fuentes antes de actualizar los indicadores
		digitalWrite(Cabina0_Power_Source,LOW);//Apaga la fuente de cabina0
		digitalWrite(Cabina1_Power_Source,LOW);//Apaga la fuente de cabina1

		//Es el turno de cabina 0 de mostrarse?
		if(CABINA0 == rub_MuestraCabina)
		{//SI
			//Apaga todos los indicadores de cabina 0
			digitalWrite(Cabina0_Planta0_Led,HIGH);//Apaga el indicador de planta 0
			digitalWrite(Cabina0_Planta1_Led,HIGH);//Apaga el indicador de planta 1
			digitalWrite(Cabina0_Planta2_Led,HIGH);//Apaga el indicador de planta 2
			digitalWrite(Cabina0_Subiendo_Led,HIGH);//Apaga el indicador de Subiendo
			digitalWrite(Cabina0_Bajando_Led,HIGH);//Apaga el indicador de Bajando
			digitalWrite(Cabina0_Puerta_Abierta,HIGH);//Apaga el indicador de Puerta Abierta

			/*Actualiza Indicador de Planta Actual de Cabina*/
			//Cabina 0 esta en planta 2?
			if(PLANTA2 == rub_PlantaActual[CABINA0])
			{//SI
				digitalWrite(Cabina0_Planta2_Led,LOW);//Enciende el indicador de planta 2
			}
			//Cabina 0 esta en planta 1?
			else if(PLANTA1 == rub_PlantaActual[CABINA0])
			{//SI
				digitalWrite(Cabina0_Planta1_Led,LOW);//Enciende el indicador de planta 1
			}
			//Cabina 0 esta en planta 0?
			else if(PLANTA0 == rub_PlantaActual[CABINA0])
			{//SI
				digitalWrite(Cabina0_Planta0_Led,LOW);//Enciende el indicador de planta 0
			}
			//Cabina 0 esta en planta desconocida?
			else //Error - corrupcion de RAM
			{//SI
				//Medidas de correccion - Reinicia planta actual a planta 0
				rub_PlantaActual[CABINA0] = PLANTA0;
			}

			/* Actualiza los indicadores de direccion */
			//Cabina 0 esta subiendo?
			if(SUBIENDO == re_DireccionActual[CABINA0])
			{//SI
				digitalWrite(Cabina0_Subiendo_Led,LOW);//Enciende indicador de subida
			}//NO
			//Cabina 0 esta bajando?
			else if(BAJANDO)
			{//SI
				digitalWrite(Cabina0_Bajando_Led,LOW);//Enciende indicador de bajada
			}//NO
			//Cabina 0 esta detenida
			else
			{
				//No hacer nada - Se mantienen apagados los indicadores
			}

			/* Actualizar Indicador de Puerta */
			//La puerta de cabina 0 esta abierta?
			if(ABIERTA == re_EstadoDePuerta[CABINA0])
			{//SI
				digitalWrite(Cabina0_Puerta_Abierta,LOW);//Enciende indicador de puerta abierta
			}
			else
			{//NO
				//No hacer nada -  Mantener apagado
			}

			/* Encender Fuente de Cabina 0 para mostrar indicadores actualizados */
			digitalWrite(Cabina0_Power_Source,HIGH);//Enciende cabina 0
		}
		//Es el turno de cabina 1 de mostrarse?
		else if(CABINA1 == rub_MuestraCabina)
		{//SI - NOTA: El siguiente codigo es identico al del 'if' anterior. Fue diseñado a proposito por si en algun momento las macros usadas aqui cambiaran en la configuracion
			//Apaga todos los indicadores de cabina 1
			digitalWrite(Cabina1_Planta0_Led,HIGH);//Apaga el indicador de planta 0
			digitalWrite(Cabina1_Planta1_Led,HIGH);//Apaga el indicador de planta 1
			digitalWrite(Cabina1_Planta2_Led,HIGH);//Apaga el indicador de planta 2
			digitalWrite(Cabina1_Subiendo_Led,HIGH);//Apaga el indicador de Subiendo
			digitalWrite(Cabina1_Bajando_Led,HIGH);//Apaga el indicador de Bajando
			digitalWrite(Cabina1_Puerta_Abierta,HIGH);//Apaga el indicador de Puerta Abierta

			//Actualiza Indicador de Planta Actual de Cabina
			if(PLANTA2 == rub_PlantaActual[CABINA1])
			{
				digitalWrite(Cabina1_Planta2_Led,LOW);//Enciende el indicador de planta 2
			}
			else if(PLANTA1 == rub_PlantaActual[CABINA1])
			{
				digitalWrite(Cabina1_Planta1_Led,LOW);//Enciende el indicador de planta 1
			}
			else if(PLANTA0 == rub_PlantaActual[CABINA1])
			{
				digitalWrite(Cabina1_Planta0_Led,LOW);//Enciende el indicador de planta 0
			}
			else //Error - corrupcion de RAM
			{
				//Medidas de correccion - Reinicia planta actual a planta 0
				rub_PlantaActual[CABINA1] = PLANTA0;
			}

			/* Actualiza los indicadores de direccion */
			//Cabina 1 esta subiendo?
			if(SUBIENDO == re_DireccionActual[CABINA1])
			{//SI
				digitalWrite(Cabina1_Subiendo_Led,LOW);//Enciende indicador de subida
			}//NO
			//Cabina 1 esta bajando?
			else if(BAJANDO)
			{//SI
				digitalWrite(Cabina1_Bajando_Led,LOW);//Enciende indicador de Bajada
			}//NO
			//Cabina 1 esta detenida
			else
			{
				//No hacer nada - Se mantienen apagados los indicadores
			}

			/* Actualizar Indicador de Puerta */
			//La puerta de cabina 1 esta abierta?
			if(ABIERTA == re_EstadoDePuerta[CABINA1])
			{//SI
				digitalWrite(Cabina1_Puerta_Abierta,LOW);//Enciende indicador de puerta abierta
			}
			else
			{//NO
				//No hacer nada -  Mantener apagado
			}

			/* Encender Fuente de Cabina 1 para mostrar indicadores actualizados */
			digitalWrite(Cabina1_Power_Source,HIGH);//Enciende cabina 1
		}
		else //Error - corrupcion de RAM
		{
			//Medidas de correccion - Reinicia cabina a mostrar
			rub_MuestraCabina = CABINA0;
		}

	}//end for(;;)
}

/***********************************************************************************
 * Nombre: app_InputMngr
 * Descripcion:  Essta funcion administra la lectura de todas las entradas del sistema.
 *               Los valores de las entradas son salvadas en RAM para su posterior
 *               procesamiento.
 * Parametros:   void *pvParameters - parametro FreeRTOS
 * Retrono:      N/A
 ************************************************************************************/
void app_InputMngr( void *pvParameters )
{
	(void) pvParameters;
	for(;;)
	{
		//Periodo de tarea
		vTaskDelay( 100 / portTICK_PERIOD_MS );

		//Actualiza estado anterior
		rs_EstadoBotonesAnterior = rs_EstadoBotonesActual; //Copia toda la estructura actual a la anterior

		//Actualiza estado actual
		rs_EstadoBotonesActual.Cabina0_Planta0 = digitalRead(Cabina0_Planta0_Boton); //Leer estado de la entrada asignada al boton de Cabina 0 Planta 0
		rs_EstadoBotonesActual.Cabina0_Planta1 = digitalRead(Cabina0_Planta1_Boton); //Leer estado de la entrada asignada al boton de Cabina 0 Planta 1
		rs_EstadoBotonesActual.Cabina0_Planta2 = digitalRead(Cabina0_Planta2_Boton); //Leer estado de la entrada asignada al boton de Cabina 0 Planta 2
		rs_EstadoBotonesActual.Cabina1_Planta0 = digitalRead(Cabina1_Planta0_Boton); //Leer estado de la entrada asignada al boton de Cabina 1 Planta 0
		rs_EstadoBotonesActual.Cabina1_Planta1 = digitalRead(Cabina1_Planta1_Boton); //Leer estado de la entrada asignada al boton de Cabina 1 Planta 1
		rs_EstadoBotonesActual.Cabina1_Planta2 = digitalRead(Cabina1_Planta2_Boton); //Leer estado de la entrada asignada al boton de Cabina 1 Planta 2
		rs_EstadoBotonesActual.Planta0_Sube = digitalRead(Planta0_Sube_Boton); //Leer estado de la entrada asignada al boton de Planta 0 Sube
		rs_EstadoBotonesActual.Planta1_Sube = digitalRead(Planta1_Sube_Boton); //Leer estado de la entrada asignada al boton de Planta 1 Sube
		rs_EstadoBotonesActual.Planta1_Baja = digitalRead(Planta1_Baja_Boton); //Leer estado de la entrada asignada al boton de Planta 1 Baja
		rs_EstadoBotonesActual.Planta2_Baja = digitalRead(Planta2_Baja_Boton); //Leer estado de la entrada asignada al boton de Planta 2 Baja

		//Prepara peticion
		rub_PeticionPendiente = true;

		//Hubo presion de Planta 0 en cabina 0?
		if( ( false == rs_EstadoBotonesActual.Cabina0_Planta0 ) && 
				( true  == rs_EstadoBotonesAnterior.Cabina0_Planta0 ))
		{//SI
			//Peticion de ir a planta 0
			re_Peticion = PLANTA0;
			//Cabina 0 ejecutara peticion
			re_PeticionCabina = CABINA0;
		}
		//Hubo presion de Planta 1 en cabina 0?
		else if( ( false == rs_EstadoBotonesActual.Cabina0_Planta1 ) && 
				( true  == rs_EstadoBotonesAnterior.Cabina0_Planta1 ))
		{//SI
			//Peticion de ir a planta 1
			re_Peticion = PLANTA1;
			//Cabina 0 ejecutara peticion
			re_PeticionCabina = CABINA0;
		}
		//Hubo presion de Planta 2 en cabina 0?
		else if( ( false == rs_EstadoBotonesActual.Cabina0_Planta2 ) && 
				( true  == rs_EstadoBotonesAnterior.Cabina0_Planta2 ))
		{//SI
			//Peticion de ir a planta 2
			re_Peticion = PLANTA2;
			//Cabina 0 ejecutara peticion
			re_PeticionCabina = CABINA0;
		}
		//Hubo presion de Planta 0 en cabina 1?
		else if( ( false == rs_EstadoBotonesActual.Cabina1_Planta0 ) && 
				( true  == rs_EstadoBotonesAnterior.Cabina1_Planta0 ))
		{//SI
			//Peticion de ir a planta 0
			re_Peticion = PLANTA0;
			//Cabina 1 ejecutara peticion
			re_PeticionCabina = CABINA1;
		}
		//Hubo presion de Planta 1 en cabina 1?
		else if( ( false == rs_EstadoBotonesActual.Cabina1_Planta1 ) && 
				( true  == rs_EstadoBotonesAnterior.Cabina1_Planta1 ))
		{//SI
			//Peticion de ir a planta 1
			re_Peticion = PLANTA1;
			//Cabina 1 ejecutara peticion
			re_PeticionCabina = CABINA1;
		}
		//Hubo presion de Planta 2 en cabina 1?
		else if( ( false == rs_EstadoBotonesActual.Cabina1_Planta2 ) && 
				( true  == rs_EstadoBotonesAnterior.Cabina1_Planta2 ))
		{//SI
			//Peticion de ir a planta 2
			re_Peticion = PLANTA2;
			//Cabina 1 ejecutara peticion
			re_PeticionCabina = CABINA1;
		}
		//Hubo presion de Subir en planta 0?
		else if( ( false == rs_EstadoBotonesActual.Planta0_Sube ) && 
				( true  == rs_EstadoBotonesAnterior.Planta0_Sube ))
		{//SI
			//Peticion de ir a planta 0
			re_Peticion = PLANTA0;

			//Selecciona cabina que sera llamada
			if(rub_PlantaActual[CABINA0] <= rub_PlantaActual[CABINA1])
			{
				//Cabina 0 ejecutara peticion
				re_PeticionCabina = CABINA0;
			}
			else
			{
				//Cabina 1 ejecutara peticion
				re_PeticionCabina = CABINA1;
			}
		}
		//Hubo presion de Subir en planta 1?
		else if( ( false == rs_EstadoBotonesActual.Planta1_Sube ) && 
				( true  == rs_EstadoBotonesAnterior.Planta1_Sube ))
		{//SI
			//Peticion de ir a planta 1
			re_Peticion = PLANTA1;

			//Selecciona cabina que sera llamada
			if(SUBIENDO == re_UltimaDireccion[CABINA0]) //Cabina 0 estaba subiendo?
			{
				//Cabina 0 ejecutara peticion
				re_PeticionCabina = CABINA0;
			}
			else if(SUBIENDO == re_UltimaDireccion[CABINA1]) //Cabina 1 estaba subiendo?
			{
				//Cabina 1 ejecutara peticion
				re_PeticionCabina = CABINA1;
			}
			else
			{
				//Cabina 0 ejecutara peticion
				re_PeticionCabina = CABINA0;
			}
		}
		//Hubo presion de Bajar en planta 1?
		else if( ( false == rs_EstadoBotonesActual.Planta1_Baja ) && 
				( true  == rs_EstadoBotonesAnterior.Planta1_Baja ))
		{//SI
			//Peticion de ir a planta 1
			re_Peticion = PLANTA1;

			//Selecciona cabina que sera llamada
			if(BAJANDO == re_UltimaDireccion[CABINA0]) //Cabina 0 estaba subiendo?
			{
				//Cabina 0 ejecutara peticion
				re_PeticionCabina = CABINA0;
			}
			else if(BAJANDO == re_UltimaDireccion[CABINA1]) //Cabina 1 estaba subiendo?
			{
				//Cabina 1 ejecutara peticion
				re_PeticionCabina = CABINA1;
			}
			else
			{
				//Cabina 0 ejecutara peticion
				re_PeticionCabina = CABINA0;
			}
		}
		//Hubo presion de Bajar en planta 2?
		else if( ( false == rs_EstadoBotonesActual.Planta2_Baja ) && 
				( true  == rs_EstadoBotonesAnterior.Planta2_Baja ))
		{//SI
			//Peticion de ir a planta 2
			re_Peticion = PLANTA2;

			//Selecciona cabina que sera llamada
			if(rub_PlantaActual[CABINA0] >= rub_PlantaActual[CABINA1])
			{
				//Cabina 0 ejecutara peticion
				re_PeticionCabina = CABINA0;
			}
			else
			{
				//Cabina 1 ejecutara peticion
				re_PeticionCabina = CABINA1;
			}
		}
		//Ningun evento se presento
		else
		{
			//Limpia variable que indica peticion pendiente
			rub_PeticionPendiente = false;
		}
	}

  //Define direccion desde seleccion de boton
  //Planta actual de la cabina seleccionada es igual a la solicitada?
  if(rub_PlantaActual[re_PeticionCabina] == re_Peticion)
  {//SI
    /* Actualiza Variable de direccion a detenido */
    re_DireccionActual[re_PeticionCabina] = DETENIDO;
  }
  /* Planta Actual es menor a solicitada? */
  else if(rub_PlantaActual[re_PeticionCabina] < re_Peticion)
  {//SI
    /* Actualiza Variable de direccion a subiendo */
    re_DireccionActual[re_PeticionCabina] = SUBIENDO;
  }
  /* Planta Actual es mayor a solicitada */
  else
  {
    /* Actualiza Variable de direccion a bajando */
    re_DireccionActual[re_PeticionCabina] = BAJANDO;
  }
}

/***********************************************************************************
 * Nombre: app_MovementMngr
 * Descripcion:  Esta funcion administra el movimiento de plantas de ambas cabinas.
 *               Aqui es donde se revisa si es posible moverse y tambien se
 *               evalua la direccion.
 * Parametros:   void *pvParameters - parametro FreeRTOS
 * Retrono:      N/A
 ************************************************************************************/
void app_MovementMngr( void *pvParameters )
{
	(void) pvParameters;

	for(;;)
	{
		//Period of the task
		vTaskDelay( 1000 / portTICK_PERIOD_MS );

    //Cierra puertas
    /* Esta abierta la puerta de la cabina 0? */
    if(ABIERTA == re_EstadoDePuerta[CABINA0])
    {
      /* Cierra la puerta */
      re_EstadoDePuerta[CABINA0] = CERRADA;
    }
    else
    {//Esta cerrada no hagas nada
    }
    /* Esta abierta la puerta de la cabina 1? */
    if(ABIERTA == re_EstadoDePuerta[CABINA1])
    {
      /* Cierra la puerta */
      re_EstadoDePuerta[CABINA1] = CERRADA;
    }
    else
    {//Esta cerrada no hagas nada}
    }
      
    //Revisa si hay tareas pendientes
    if(true == rub_PeticionPendiente)
    {
      //Planta actual de la cabina seleccionada es igual a la solicitada?
      if(rub_PlantaActual[re_PeticionCabina] == re_Peticion)
      {//SI
        /*Abre la puerta*/
        re_EstadoDePuerta[re_PeticionCabina] = ABIERTA;
        /* Actualiza Variable de direccion a detenido */
        re_DireccionActual[re_PeticionCabina] = DETENIDO;
        
        /* Revisa si llegamos a plantas limites */
        if(PLANTA0 == re_Peticion)
        {//Planta 0
          /* Actualiza Variable de ultima direccion a SUBIENDO */
          re_UltimaDireccion[re_PeticionCabina] = SUBIENDO;
        }
        else if(PLANTA2 == re_Peticion)
        {//Planta 2
          /* Actualiza Variable de ultima direccion a BAJANDO */
          re_UltimaDireccion[re_PeticionCabina] = BAJANDO;
        }
        else
        {
          /* No actualizes variable de ultima direccion */
        }

        /* Limpia variable de peticion pendiente */
        rub_PeticionPendiente = false;
      }
      /* Planta Actual es menor a solicitada? */
      else if(rub_PlantaActual[re_PeticionCabina] < re_Peticion)
      {//SI
        /* Ve a la siguiente planta */
        rub_PlantaActual[re_PeticionCabina] = (T_PLANTA)((int)rub_PlantaActual[re_PeticionCabina] + 1);

        /* Actualiza Variable de direccion a subiendo */
        re_DireccionActual[re_PeticionCabina] = SUBIENDO;
        re_UltimaDireccion[re_PeticionCabina] = SUBIENDO;
      }
      /* Planta actual es mayor a solicitada */
      else
      {
        /* Ve a planta anterior*/
        rub_PlantaActual[re_PeticionCabina] = (T_PLANTA)((int)rub_PlantaActual[re_PeticionCabina] - 1);

        /* Actualiza Variable de direccion a subiendo */
        re_DireccionActual[re_PeticionCabina] = BAJANDO;
        re_UltimaDireccion[re_PeticionCabina] = BAJANDO;
      }
    }//fin de peticion pendiente
    else
    {
      //No hay peticiones - Hacer Nada
    }
	}
}

/***********************************************************************************
 * Nombre: app_MultiplexMngr
 * Descripcion:  Esta funcion administra que conjunto de entradas y salidas operaran
 *               en los siguientes ciclos. Aqui se define la cabina a operar.
 * Parametros:   void *pvParameters - parametro FreeRTOS
 * Retrono:      N/A
 ************************************************************************************/
void app_MultiplexMngr( void *pvParameters )
{
	(void) pvParameters;

	for(;;)
	{
		//Periodo de la tarea. 100 ms
		vTaskDelay( 10 / portTICK_PERIOD_MS );

		//Cabina 0 se esta mostrando?
		if(CABINA0 == rub_MuestraCabina)
		{
			rub_MuestraCabina = CABINA1; //Cambia a cabina 1
		}
		//Cabina 0 no se esta mostrando
		else
		{
			rub_MuestraCabina = CABINA0; //Cambia a cabina 0
		}
	}
}

/***********************************************************************************
 * Nombre: app_TestSequence
 * Descripcion:  Esta funcion esta dedicada para la secuencia de prueba. solo
 *               se ejecutara si la variable de prueba es igual a TRUE
 * Parametros:   void *pvParameters - parametro FreeRTOS
 * Retrono:      N/A
 ************************************************************************************/
void app_TestSequence( void *pvParameters )
{
	(void) pvParameters;

	for(;;)
	{
		//Periodo de la tarea. 1s
		vTaskDelay( 1000 / portTICK_PERIOD_MS );


		switch (rub_TestStep) {
		case 0:{
			/* Esta abierta la puerta de la cabina 0? */
			if(ABIERTA == re_EstadoDePuerta[CABINA0])
			{
				/* Cierra la puerta */
				re_EstadoDePuerta[CABINA0] = CERRADA;
			}
			/* Planta actual de cabina 0 no ha llegado hasta el ultimo piso? */
			else if(rub_PlantaActual[CABINA0] < NPLANTAS - 1)
			{
				/* Incrementa planta*/
				rub_PlantaActual[CABINA0] = (T_PLANTA)((int)rub_PlantaActual[CABINA0] + 1);

				/* Actualiza Variable de direccion a subiendo */
				re_DireccionActual[CABINA0] = SUBIENDO;
			}
			else
			{
				/* Llegado a planta mas alta abre puerta */
				re_EstadoDePuerta[CABINA0] = ABIERTA;

				/* Actualiza Variable de direccion a detenido */
				re_DireccionActual[CABINA0] = DETENIDO;

				/* Cambia a siguiente estado */
				rub_TestStep = 1;
			}
		}break;
		case 1:{
			/* Esta abierta la puerta de la cabina 1? */
			if(ABIERTA == re_EstadoDePuerta[CABINA1])
			{
				/* Cierra la puerta de cabina 1 */
				re_EstadoDePuerta[CABINA1] = CERRADA;
			}
			/* Planta actual de cabina 1 no ha llegado hasta el ultimo piso? */
			else if(rub_PlantaActual[CABINA1] < NPLANTAS - 1)
			{
				/* Incrementa planta*/
				rub_PlantaActual[CABINA1] = (T_PLANTA)((int)rub_PlantaActual[CABINA1] + 1);

				/* Actualiza Variable de direccion a subiendo */
				re_DireccionActual[CABINA1] = SUBIENDO;
			}
			else
			{
				/* Llegado a planta mas alta abre puerta */
				re_EstadoDePuerta[CABINA1] = ABIERTA;

				/* Actualiza Variable de direccion a detenido */
				re_DireccionActual[CABINA1] = DETENIDO;

				/* Cambia a siguiente estado */
				rub_TestStep = 2;
			}
		}break;
		case 2:{
			/* Esta abierta la puerta de la cabina 0? */
			if(ABIERTA == re_EstadoDePuerta[CABINA0])
			{
				/* Cierra la puerta */
				re_EstadoDePuerta[CABINA0] = CERRADA;
			}
			/* Planta actual de cabina 1 no ha llegado hasta el ultimo piso? */
			else if(rub_PlantaActual[CABINA0] > 0)
			{
				/* Decrementa planta*/
				rub_PlantaActual[CABINA0] = (T_PLANTA)((int)rub_PlantaActual[CABINA0] - 1);

				/* Actualiza Variable de direccion a bajando */
				re_DireccionActual[CABINA0] = BAJANDO;
			}
			else
			{
				/* Llegado a planta mas alta abre puerta */
				re_EstadoDePuerta[CABINA0] = ABIERTA;

				/* Actualiza Variable de direccion a bajando */
				re_DireccionActual[CABINA0] = DETENIDO;

				/* Cambia a siguiente estado */
				rub_TestStep = 3;
			}
		}break;
		case 3:{
			/* Esta abierta la puerta de la cabina 1? */
			if(ABIERTA == re_EstadoDePuerta[CABINA1])
			{
				/* Cierra la puerta */
				re_EstadoDePuerta[CABINA1] = CERRADA;
			}
			/* Planta actual de cabina 1 no ha llegado hasta el ultimo piso? */
			else if(rub_PlantaActual[CABINA1] > 0)
			{
				/* Decrementa planta*/
				rub_PlantaActual[CABINA1] = (T_PLANTA)((int)rub_PlantaActual[CABINA1] - 1);

				/* Actualiza Variable de direccion a bajando */
				re_DireccionActual[CABINA1] = BAJANDO;
			}
			else
			{
				/* Llegado a planta mas alta abre puerta */
				re_EstadoDePuerta[CABINA1] = ABIERTA;

				/* Actualiza Variable de direccion a bajando */
				re_DireccionActual[CABINA1] = DETENIDO;

				/* Vuelve a estado inicial */
				rub_TestStep = 0;
			}
		}break;
		default:
		{
			//Reinicia estado
			rub_TestStep = 0;
		}break;
		}
	}
}
