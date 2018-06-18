#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "./DNSServer.h"
#include "./ESP8266FtpServer.h"
#include <ArduinoJson.h>

//Configuracion Puerto DNS
const byte PuertoDNS = 53;
//Configuracion Puerto HTTP
const byte PuertoHTTP = 80;

//Estructura Archivo De Configuracion
struct ArchivoConfiguracion {
  char *FTPUser = "Admin";
  char *FTPPass = "Upload";
  char *WIFISSID = "ESP8266";
  char *WIFIPass = "12345678";
  IPAddress IP = IPAddress(13,13,13,13);
  IPAddress Subnet = IPAddress(255,255,255,0);
};

//Defino El Archivo De Configuracion
ArchivoConfiguracion Configuracion;
//Defino El Archivo Para Subir
File ArchivoASubir;

//Creo Los Servidores
ESP8266WebServer WebServer(PuertoHTTP);
DNSServer DNS;
FtpServer FTP;

//Funcion Para Cargar La Configuracion Desde Un JSON
void LeerConfiguracion() {
  //Abro El Archivo Configuracion.json
  File ArchivoConfig = SPIFFS.open("/Configuracion.json", "r");
  //Creo Un Buffer Dinamico Para El JSON
  DynamicJsonBuffer jsonBuffer;

  //Parseo El Archivo A UN JSON
  JsonObject &JSON = jsonBuffer.parseObject(ArchivoConfig);
  //Si No Pude Leer El JSON
  if (!JSON.success()) {
    Serial.println(F("Fallo La Lectura De Archivo Config"));
  } else {
    //Copiar Valores 
    Configuracion.FTPUser = (char*)JSON["FTPUser"].as<char*>();
    Configuracion.FTPPass = (char*)JSON["FTPPass"].as<char*>();
    Configuracion.WIFISSID = (char*)JSON["WIFISSID"].as<char*>();
    Configuracion.WIFIPass = (char*)JSON["WIFIPass"].as<char*>();
    Configuracion.IP.fromString(JSON["IP"].as<String>());
    Configuracion.Subnet.fromString(JSON["Subnet"].as<String>());
    //Cierro El Archivo
    ArchivoConfig.close();
  }
}

//Funcion Para Grabar La Configuracion
void GrabarConfiguracion() {
  //Abro El Archivo Configuracion.json
  File ArchivoConfig = SPIFFS.open("/Configuracion.json", "w");
  //Creo Un Buffer Dinamico Para El JSON
  DynamicJsonBuffer jsonBuffer;
  //Creo El Objeto JSON
  JsonObject &JSON = jsonBuffer.createObject();
  //Creo El Raiz Del JSON
  JSON["FTPUser"] = Configuracion.FTPUser;
  JSON["FTPPass"] = Configuracion.FTPPass;
  JSON["WIFISSID"] = Configuracion.WIFISSID;
  JSON["WIFIPass"] = Configuracion.WIFIPass;
  JSON["IP"] = IpAddress2String(Configuracion.IP);
  JSON["Subnet"] = IpAddress2String(Configuracion.Subnet);
  //Escribo El Contenido De La Configuracion Y Devuelvo Un Error Si Algo Fallo
  if (JSON.printTo(ArchivoConfig) == 0) { Serial.println(F("Error Al Grabar La Configuracion")); }
  //Cierro El Archivo
  ArchivoConfig.close();
}


//Funcion Para Convertir Un IPAdress En Un String
String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}


//Funcion Para Convertir Un String En Un char*
char* String2Char(String Texto) {
  //Si El Texto Tiene Mas De 0 Caracteres Los Devuelvo Como String
  if (Texto.length()!=0) { return const_cast<char*>(Texto.c_str()); }
  //Sino Devuelvo Un String Vacio
  else return "";
}

//Funcion Para Configurar El WIFI y El FTP
void ConfigurarWIFIyFTP() {
  //Seteo El Modo AP
  WiFi.mode(WIFI_AP);
  //Configuro La IP, Puerta De Enlace Y Mascara De Red
  WiFi.softAPConfig(Configuracion.IP, Configuracion.IP, Configuracion.Subnet);
  //Si El Password No Esta Vacio - El 10 Es El Canal, Y El False Es Para Que No Sea Invisible
  if (sizeof(Configuracion.WIFIPass)-1 != 0) { WiFi.softAP(Configuracion.WIFISSID, Configuracion.WIFIPass, 10, false); } 
  //Si El Password Esta Vacio
  else { WiFi.softAP(Configuracion.WIFISSID); }
  //Inicio El Servidor FTP
  FTP.begin(Configuracion.FTPUser, Configuracion.FTPPass);
}


void setup() {
  Serial.begin(115200);
  //Inicio El FileSystem Donde Subo Los Html
  SPIFFS.begin();

  //Leer Configuracion
  LeerConfiguracion();
    
  //Configuracion WIFI Y FTP
  ConfigurarWIFIyFTP();

  //Configuracion DNS
  DNS.setTTL(300);
  DNS.setErrorReplyCode(DNSReplyCode::ServerFailure);
  DNS.start(PuertoDNS, "*", Configuracion.IP);

  //Web Para Subir Los Archivos - Actualizar
  WebServer.on("/upload", HTTP_GET, []() {
    //Si Se Encontro El Archivo Se Envia Y Devuelve Un True
    if (!ManejarArchivo("/upload.html"))
      //Si No Se Encuentra El Archivo, Se Envia Un 404 Y Devuelve Un False
      WebServer.send(404, "text/plain", "Que Haces Loro?...");
  });

  //Web Para Recibir Los Archivos - Actualizar (POST)
  WebServer.on("/upload", HTTP_POST,
            //Enviar Status 200 (OK), Para Decirle Al Cliente Que Estamos Listos Para Recivir
            []() { WebServer.send(200);},
            //Recibo El Archivo
            subirArchivo
  );
           
  //Siempre Busco El Archivo
  WebServer.onNotFound([]() {
    //Si Se Encontro El Archivo Se Envia Y Devuelve Un True
    if (!ManejarArchivo(WebServer.uri()))
      //Si No Se Encuentra El Archivo, Se Envia Un 404 Y Devuelve Un False
      WebServer.send(404, "text/plain", "Que Haces Loro?...");
  });

  //Inicio El Server WEB
  WebServer.begin();
}

//Obtengo El MIME Type Segun La Extension
String obtenerTipo(String filename) {
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  //Creo La Extension Comprimida Para Menor Trafico De Red
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

//Envio El Archivo Solicitado Si Existe
bool ManejarArchivo(String path) {
  //Si Me Piden Un Carpeta Los Llevo Al Archivo index De Esa Carpeta
  if (path.endsWith("/")) path += "index.html";
  //Obtener el MIME Type
  String mimeType = obtenerTipo(path);
  //Archivo Comprimido GZ
  String pathComprimido = path + ".gz";
  //Si El Archivo Existe
  if (SPIFFS.exists(pathComprimido) || SPIFFS.exists(path)) {
    //Si Existe Comprimido Le Envio El Comprimido
    if(SPIFFS.exists(pathComprimido)) path += ".gz";
    //Abro El Archivo En Modo Lectura ;)
    File file = SPIFFS.open(path, "r");
    //Se Lo Envio Al Cliente
    size_t sent = WebServer.streamFile(file, mimeType);
    //Cierro El Archivo
    file.close();
    return true;
  }
  //Si El Archivo No Existe Devuelvo Falso
  return false;
}

//Subir Un Nuevo Archivo a SPIFFS
void subirArchivo() {
  HTTPUpload& upload = WebServer.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    //Abrir El Archivo Para Escribir En SPIFFS (Lo Creo Si No Existe)
    ArchivoASubir = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (ArchivoASubir)
      //Escribo Los Bytes Recibido Al Archvio
      ArchivoASubir.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    //Si El Archivo Se Subio Correctamente
    if (ArchivoASubir) {
      //Cierro El Archivo
      ArchivoASubir.close();
      //Redirecciono El Cliente A La Pagina De Exito
      WebServer.sendHeader("Location", "/correcto.html");
      WebServer.send(303);
    } else {
      //Sino Muestro Un Error
      WebServer.send(500, "text/plain", "No Pude Subir El Archivo Loro.");
    }
  }
}

void loop() {
  //Espero Los Request
  WebServer.handleClient();
  //Proceso Los Request DNS
  DNS.processNextRequest();
  //Manejo Las Peticiones FTP
  FTP.handleFTP();
}

