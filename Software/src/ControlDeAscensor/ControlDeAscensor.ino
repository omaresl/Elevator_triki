/* Control de Ascensor */
/* V1.0 */
/* Omar Sevilla */
/* omar.sevilla.lomeli@gmail.com */

/*Descripción del Proyecto
Conjunto de ascensores con dos cabinas y tres plantas 0,1 y 2.

Cada cabina tiene un led por planta, led indicador puerta abierta, led indicador subiendo, led indicador bajando, si tiene puerta cerrada y parado no enciende ningún led, no puede moverse con puerta abierta, y tres botones internos uno por planta.

En planta 0 hay un pulsador llama un ascensor para subir responde el más cercano.

En planta 1 hay dos pulsadores uno para subir otro para bajar, el más cercano es un ascensor en el mismo sentido llegando.

En planta 2 un pulsador para bajar.

Se debe usar la librería FreeRTOS

Explicar las funciones usadas de la librería FreeRTOS.*/

/* Librerias y headers */
#include <Arduino_FreeRTOS.h> //FreeRTOS Library
#include "ControlDeAscensor.h" //Header de configuracion

/* Constantes */

/* Variables */
T_PLANTA rub_PlantaActual[N_CABINAS]; //Arreglo de variables donde se almacena la planta actual de cada cabina
T_CABINA rub_MuestraCabina; //Variable que almacena que cabina se esta mostrando en lass salidas de los indicadores
T_DIRECCION re_DireccionActual[N_CABINAS]; //Arreglo de variables que almacena la direccion actual de las cabinas
T_PUERTA re_EstadoDePuerta[N_CABINAS]; //Arreglo que almacena el estado actual de la puerta de cada cabina

/* Declaracion de funciones */
void app_InitPins(void);
void app_InitSys(void);
void app_OutputMngr( void *pvParameters );
void app_InputMngr( void *pvParameters );
void app_MovementMngr( void *pvParameters );
void app_MultiplexMngr( void *pvParameters );

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
    ,  1  // Prioridad (3 es la mas alta)
    ,  NULL );
    
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

/* Creacion de Tarea de movimiento de ascensor */
xTaskCreate(      
    app_MultiplexMngr //Funcion que sera añadida al scheduler
    ,  (const portCHAR *)"Movement Manager"   // Identificador de tarea (No relevante para funcionalidad)
    ,  128  // Tamaño del stack
    ,  NULL
    ,  2  // Prioridad (3 es la mas alta)
    ,  NULL );

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
  rub_Planta Actual[CABINA0] = PLANTA0 //Inicia Cabina0 en Planta0
  rub_Planta Actual[CABINA1] = PLANTA0 //Inicia Cabina1 en Planta0

  /* Inicializa Muestra de Indicadores de Cabina */
  rub_MuestraCabina = CABINA0; //Inicia Mostrando Cabina 0

  //Inicializa direcciones
  re_DireccionActual[CABINA0] = SUBIENDO; //Inicia cabina 0 subiendo
  re_DireccionActual[CABINA1] = SUBIENDO; //Inicia cabina 1 subiendo

  //Inicializa la puerta cerrada
  re_EstadoDePuerta = CERRADA; //Inicia puerta cerrada
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
  //Periodo de la tarea. 30 ms
  vTaskDelay( 30 / portTICK_PERIOD_MS );

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
    if(ABIERTA = re_EstadoDePuerta[CABINA0])
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
  }
  else //Error - corrupcion de RAM
  {
    //Medidas de correccion - Reinicia cabina a mostrar
    rub_MuestraCabina = CABINA0;
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
    if(ABIERTA = re_EstadoDePuerta[CABINA1])
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
  
}
