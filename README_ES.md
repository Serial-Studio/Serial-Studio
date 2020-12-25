<a href="#">
    <img width="192px" height="192px" src="doc/icon.png" align="right" />
</a>

# Serial Studio

[![Build Status](https://travis-ci.org/Serial-Studio/Serial-Studio.svg?branch=master)](https://travis-ci.org/Serial-Studio/Serial-Studio)

Serial Studio es un programa de visualización de datos de puertos serial multiplataforma y multipropósito. El objetivo de este proyecto es permitir a los desarrolladores y creadores de sistemas embebidos visualizar, presentar y analizar fácilmente los datos generados por sus proyectos y dispositivos, sin la necesidad de escribir software especializado para cada proyecto.

La necesidad de este proyecto surgió durante el desarrollo del software de estación terrestre para varios concursos de CanSat. Simplemente no es sostenible desarrollar y mantener diferentes programas para cada competencia. La solución inteligente es tener un software de estación terrestre común y permitir que cada CanSat defina cómo se presentan los datos al usuario final mediante un protocolo de comunicación extensible.

Además, este enfoque se puede extender a casi cualquier tipo de proyecto que implique algún tipo de adquisición y medición de datos.

*Lea esto en otros idiomas:* [English](README.md)

![Screenshot](doc/screenshot.png)

## Protocolo de comunicación (LECTURA OBLIGATORIA)

El protocolo de comunicación se implementa a través de un documento JSON con la siguiente estructura:

- **`t`**: título del proyecto (*string*, obligatorio)
- **`g`**: grupos de datos (*arreglo*)
  - **`t`**: título del grupo (*string*, obligatorio)
  - **`w`**: tipo de widget (*string*; opcional, puede tener los siguientes valores:)
    - `map`: crear un widget que muestre una ubicación en un mapa
    - `bar`: barra de progreso vertical (con valores `max` y` min`)
    - `gauge`: manómentro (con valores `max` y` min`)
    - `gyro`: indicador de giro
    - `accelerometer`: indicador de aceleración
    - `tank`: indicador de tanque vertical (con valores `max` y` min`)
  - **`d`**: conjuntos de datos de grupo (*arreglo*)
    - **`t`**:  título del conjunto de datos (*string*, optional)
    - **`v`**:  valor del conjunto de datos (*variante*, obligatorio)
    - **`u`**:  unidad del conjunto de datos (*string*, opcional)
    - **`g`**:  gráficar conjunto de datos (*boolean*, opcional)
    - **`w`**:  tipo de widget (*string*, depende del tipo de widget de grupo, los valores posibles son:)
        - Para los widgets `gyro` y` acelerometer`: 
            - `x`: valor para eje X
            - `y`: valor para eje Y
            - `z`: valor para eje Z
        - Para el widget `map`: 
            - `lat`: latitud
            - `lon`: longitud
        - Para los widgets `bar`,` tank` y `gauge`:
            - `max`: valor máximo
            - `min`: valor mínimo
            - `value`: valor actual
    
Esta información es procesada por Serial Studio, que construye la interfaz de usuario de acuerdo con la información contenida en cada marco. Esta información también se utiliza para generar un archivo CSV con todas las lecturas recibidas del dispositivo en serie, el archivo CSV se puede utilizar para análisis y procesamiento de datos dentro de MATLAB.

** NOTA: ** los tipos de widgets se pueden repetir en diferentes grupos sin ningún problema.

### Modos de comunicación

Serial Studio puede procesar la información serial entrante de dos formas:

1. El dispositivo serie envía periódicamente una trama de datos JSON completa (**modo automático**).
2. El usuario especifica la estructura JSON en un archivo y el dispositivo serie solo envía datos separados por comas (**modo manual**).

El modo manual es útil si no desea utilizar una biblioteca JSON en su programa de microcontrolador, o si necesita enviar grandes cantidades de información. Un ejemplo de un archivo JSON es:

```json
{
   "t":"%1",
   "g":[
      {
         "t":"Estado de Misión",
         "d":[
            {
               "t":"Tiempo",
               "v":"%2",
               "u":"s"
            },
            {
               "t":"Número de trama",
               "v":"%3"
            },
            {
               "t":"Voltaje de bateria",
               "v":"%4",
               "g":true,
               "u":"V"
            }
         ]
      },
      {
         "t":"Lecturas",
         "d":[
            {
               "t":"Temperatura",
               "v":"%5",
               "g":true,
               "u":"C"
            },
            {
               "t":"Altitud",
               "v":"%6",
               "u":"m"
            },
            {
               "t":"Presión",
               "v":"%7",
               "u":"KPa",
               "g":true
            },
            {
               "t":"Temperatura Externa",
               "v":"%8",
               "g":true,
               "u":"C"
            },
            {
               "t":"Humedad Externa",
               "v":"%9",
               "g":true,
               "u":"C"
            }
         ]
      },
      {
         "t":"GPS",
         "w":"map",
         "d":[
            {
               "t":"Tiempo GPS",
               "v":"%10"
            },
            {
               "t":"Longitud",
               "v":"%11",
               "u":"°N",
               "w":"lon"
            },
            {
               "t":"Latitud",
               "v":"%12",
               "u":"°E",
               "w":"lat"
            },
            {
               "t":"Altitud",
               "v":"%13",
               "u":"m"
            },
            {
               "t":"Num. Sats",
               "v":"%14"
            }
         ]
      },
      {
         "t":"Acelerómetro",
         "w":"accelerometer",
         "d":[
            {
               "t":"X",
               "v":"%15",
               "u":"m/s^2",
               "g":true,
               "w":"x"
            },
            {
               "t":"Y",
               "v":"%16",
               "u":"m/s^2",
               "g":true,
               "w":"y"
            },
            {
               "t":"Z",
               "v":"%17",
               "u":"m/s^2",
               "g":true,
               "w":"z"
            }
         ]
      },
      {
         "t":"Giróscopo",
         "w":"gyro",
         "d":[
            {
               "t":"X",
               "v":"%18",
               "u":"rad/s",
               "g":true,
               "w":"x"
            },
            {
               "t":"Y",
               "v":"%19",
               "u":"rad/s",
               "g":true,
               "w":"y"
            },
            {
               "t":"Z",
               "v":"%20",
               "u":"rad/s",
               "g":true,
               "w":"z"
            }
         ]
      }
   ]
}
```

Cómo puede suponer, *Serial Studio* reemplazará los valores `%1`,`%2`, `%3`,`...`,`%20` por los valores del índice correspondientes en una trama con valores separados por comas. El formato de datos correspondiente enviado por el microcontrolador para el mapa JSON dado es:

`/*KAANSATQRO,%s,%s,%s,%s,%s,%s,%,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s*/`

### Secuencias de inicio/fin de trama

Para procesar todas las tramas, Serial Studio necesita tener una forma confiable de saber cuándo comienza y cuándo termina una trama. La solución con la que vine es tener una secuencia de inicio/finalización específica, que corresponde a:

- `/*` Inicio de trama
- `*/` Fin de trama

Las secuencias de inicio/fin **se aplican** en ambos modos de comunicación (**automático** y **manual**).

### Ejemplo

Suponga que recibe los siguientes datos de un microcontrolador:

`/*KAANSATQRO,2051,2,5,26,10,101.26,27,32,1001,21.1619,86.8515,10,4,1.23,9.81,0.23,0,0,0*/`

Serial Studio está configurado para interpretar los datos entrantes utilizando el archivo de mapeo JSON presentado anteriormente. Los datos se separarán como:

| Índice          |  0           |  1     |  2   |  3   |  4    |  5    |  6        |  7    |  8    |  9      |  10        |  11        |  12   |  13   |  14     |  15     |  16     |  17   |  18   |  19   |
|----------------|--------------|--------|------|------|-------|-------|-----------|-------|-------|---------|------------|------------|-------|-------|---------|---------|---------|-------|-------|-------|
| Match en archivo JSON | `%1`         | `%2`   | `%3` | `%4` | `%5`  | `%6`  | `%7`      | `%8`  | `%9`  | `%10`   | `%11`      | `%12`      | `%13` | `%14` | `%15`   | `%16`   | `%17`   | `%18` | `%19` | `%20` |
| Valor remplazado con  | `KAANSATQRO` | `2051` | `2`  | `5`  | `26`  | `10`  | `101.26`  | `27`  | `32`  | `1001`  | `21.1619`  | `86.8515`  | `10`  | `4`   | `1.23`  | `9.81`  | `0.23`  | `0`   | `0`   | `0`   | 

## Instrucciones de complicación

##### Requisitos

El único requisito para compilar la aplicación es tener [Qt](http://www.qt.io/download-open-source/) instalado en su sistema. La aplicación se compilará con Qt 5.15 o superior.

### Clonado de este repositorio

Este repositorio hace uso de [`git submodule`](https://git-scm.com/docs/git-submodule). Para clonarlo, tiene dos opciones:

Comando breve:

    git clone --recursive https://github.com/Serial-Studio/Serial-Studio

Procedimiento normal:

    git clone https://github.com/Serial-Studio/Serial-Studio
    cd Serial-Studio
    git submodule init
    git submodule update
    
###### Compilación

Una vez que haya instalado Qt, abra * Serial-Studio.pro * en Qt Creator y haga clic en el botón de "Ejecutar".

Alternativamente, también puede utilizar los siguientes comandos:

	qmake
	make -j4

## Licencia

Este proyecto se publica bajo la licencia MIT, para obtener más información, consulte el archivo [LICENSE](LICENSE.md).

