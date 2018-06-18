# ESP8266-WEB-FTP-DNS-Basico
A Basic Firm To Host Your Customizable PS4 Exploit - Open Source ( No Include Exploits )

Incluye Los Fuentes De DNSServer 
- Los Pueden Volver A Descargar Desde El Siguiente LINK
https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer/src

Incluye Los Fuentes de ESP8266FtpServer  
- Los Pueden Volver A Descargar Desde El Siguiente LINK
https://github.com/nailbuster/esp8266FTPServer

Es Necesario Tener Instalado En El IDE Arduino El Repositorio
http://arduino.esp8266.com/stable/package_esp8266com_index.json 
Y La Tarjeta esp8266 by ESP8266 Community Version 2.4.1 (Minimamente)

La Configuracion WIFI Por Defecto Es:
WIFISSID = ESP8266
WIFIPass = 12345678

La Configuracion FTP Por Defecto Es:
FTPUser = Admin
FTPPass = Upload

La Configuracion IP Por Defecto Es:
IP = 13.13.13.13
Subnet = 255.255.255.0

La Configuracion DNS:
Resuelve Todas Las Peticiones A La IP Configurada Del ESP8266.
(Ej: www.google.com - 13.13.13.13 , www.playstation.com - 13.13.13.13)

Los Archivos De La Carpeta data, Se Pueden Subir Por FTP Con La Configuracion Anterior. 
Es Necesario Conectarse Por FTP Sin Cifrado (Only Use Plain FTP (Insecure) En El FileZilla Por Ej.)

O Flashear Por Medio Del IDE de Arduino
Desde Herramientas -> ESP8266 Sketch Data Upload (Esto Borrara Todos Los Archivos Que Se Encuentren En EL SPIFFS). 
Para Activar La Opcion Esta Seguir Este Tutorial: http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html#uploading-files-to-file-system

Una Vez Subidos Los Archivos upload.html Y correcto.html, Los Proximos Archivos Se Pueden Subir Directamente Desde: Http://13.13.13.13/upload



Este Sketch Se Puede Flashear, Sobre Un Bin Previo Por Ejemplo (#ESP8266XploitHost 2.7) Para Mantener Los Archivos Del Exploit.
Para Esto En El IDE De Arduino La Opcion Erase Flash, Debe Estar En Only Sketch. 

La Idea De Este Proyecto, Es Permitir A Los Usuarios A Tener Un WebHost Customizable Y Que Puedan Modificar El Funcionamiento Completo Del Mismo.
