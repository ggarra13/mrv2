.. _port_forwarding:

##################################
Creando un server para la internet
##################################

Permitir a mrv2 comunicarse por la internet significa tener un servidor con acceso a ella y el uso de "remote ssh port forwarding" (redireccionamiento remoto de puertos de ssh) para encodificar la transmisión de los datos.

En este tutorial usaremos:

https://ngrok.com/

para establecer un servicio básico de conexión de uno a uno en la internet.

Si neceista multiples usuarios conectando a mrv2 actuando como servidor, necesitará solicitar un plan pago de ngrok o usar otro provedor de servidores de internet.

Nótese que mrv2 no transmite ninguna película ni imagen cuando está conectada  a la red.  Solo commandos son transmitidos.  Cada usuario en cada destino debe bajar y cargar los archivos manualmente antes de conectarse (o ponerlos en un directorio prestablecido y usar Mapeo de Carpetas).

Como setear ngrok
-----------------

Primero, debe bajar ngrok desde:

https://ngrok.com/download

y usar unzip en Windows o extraer el archivo tar en sistemas Unix.

Luego debe colocar el ngrok ejecutable en tu PATH, como::

    C:/Windows/System32 en Windows

    /usr/local/bin      en macOS or Linux


Subscribiéndose al servicio de grok
-----------------------------------

LLene el formulario en:

https://dashboard.ngrok.com/signup

Una vez logueado, debes setear el Authtoken.

Si vas a:

https://dashboard.ngrok.com/get-started/setup

Listará tu token de autorización.  Deberás agregarlo a ngrok como::

   ngrok config add-authtoken <auth_token_id>

Y con eso terminarás la configuracion de ngrok.


Arrancando un mrv2 como servidor
--------------------------------

Arranque mrv2 y vaya a Panel->Red.  Setéolo como Servidor desde la selección de Tipo Cliente/Servidor.  Para este ejemplo, lo dejaremos en su port por defecto de 55150.  Cliquée en Crear.

mrv2 comenzará a esperar conexiones en el puerto 55150.

Desde la terminal (en Linux o macOS) o desde cmd.exe en Windows, ejecute::

    ngrok tcp 55150

Eso arrancará el redireccionamiento del puerto.  ngrok respondera con un mensaje, en particular con una línea como::

    Forwarding                 tcp://0.tcp.sa.ngrok.io:12489 -> localhost:55150

Deberás proveer la dirección tcp://* a tu cliente.


Arrancando mrv2 como cliente
----------------------------

Arranque mrv2 en la máquina cliente (remota) y use Panel->Red. Deje el tipo como Cliente.  Entre la dirección de tcp (tcp://0.tcp.sa.ngrok.io:12489 en nuestro ejemplo) como el Huesped.  Puede usar CTRL+C para copiar la dirección de un email, por ejemplo, y pegarla en el formulario de Huesped con CTRL+V.  Cliquée en Conectar.

Con ello, mrv2 como cliente sincronizará con el servidor de mrv2.  El servidor mandará la lista de archivos que tiene cargada y el client intentará cargarlos o empatar el nombre base del archivo en caso de que los archivos ya esten cargados en el mrv2 que corre como cliente.

Y a partir de entonces, tanto el servidor como el cliente estarán sincronizados.
