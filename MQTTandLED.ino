#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

//-------------------VARIABLES GLOBALES--------------------------
int contconexion = 0;
char SERVER[50] = "34.251.42.52"; //"m21.cloudmqtt.com"
int SERVERPORT = 14259;
String USERNAME = "placa1";
char PASSWORD[50] = "12345678";
unsigned long previousMillis = 0;
char charPulsador[15];
String led1;
String strPulsador1;
String strPulsadorUltimo1;
char PLACA[50];
int LED1;
char valueStr[15];
char SALIDADIGITAL[50];

//-------------------------------------------------------------------------
WiFiClient espClient;
PubSubClient client(espClient);

//------------------------CALLBACK-----------------------------
void callback(char *topic, byte *payload, unsigned int length)
{

  char PAYLOAD[5] = "    ";

  Serial.print("Mensaje Recibido: [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    PAYLOAD[i] = (char)payload[i];
  }
  Serial.println(PAYLOAD);

  if (String(topic) == String(SALIDADIGITAL))
  {
    if (payload[0] == '1')
    {
      digitalWrite(2, HIGH);
    }
    if (payload[0] == '0')
    {
      digitalWrite(2, LOW);
    }
  }
}

//------------------------RECONNECT-----------------------------
void reconnect()
{
  uint8_t retries = 3;
  // Loop hasta que estamos conectados
  while (!client.connected())
  {
    Serial.print("Intentando conexion MQTT...");
    // Crea un ID de cliente al azar
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    USERNAME.toCharArray(PLACA, 50);
    if (client.connect("", PLACA, PASSWORD))
    {
      Serial.println("conectado");
      client.subscribe(SALIDADIGITAL);
    }
    else
    {
      Serial.print("fallo, rc=");
      Serial.print(client.state());
      Serial.println(" intenta nuevamente en 5 segundos");
      // espera 5 segundos antes de reintentar
      delay(5000);
    }
    retries--;
    if (retries == 0)
    {
      // esperar a que el WDT lo reinicie
      while (1)
        ;
    }
  }
}

//------------------------SETUP-----------------------------
void setup()
{
  // Inicia Serial
  Serial.begin(115200);
  Serial.println("");
  //prepara GPIO2 COMO ENTRADA
  pinMode(2, OUTPUT); // GPIO2 salida digital
  digitalWrite(2, LOW);
  // Entradas
  pinMode(0, INPUT); // GPIO0

  while (WiFi.status() != WL_CONNECTED and contconexion < 50)
  { //Cuenta hasta 50 si no se puede conectar lo cancela
    ++contconexion;
    delay(500);
    Serial.print(".");
  }
  if (contconexion < 50)
  {
    //para usar con ip fija
    IPAddress ip(192, 168, 1, 156);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.config(ip, gateway, subnet);

    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("");
    Serial.println("Error de conexion buscando WiFi");
    WiFiManager wifiManager;
    wifiManager.autoConnect("AutoConnectAP");
    Serial.println("connected...yeey :)");
  }

  client.setServer(SERVER, SERVERPORT);
  client.setCallback(callback);
  String salidaDigital = "/" + USERNAME + "/" + "salidaDigital";
  salidaDigital.toCharArray(SALIDADIGITAL, 50);
}

//--------------------------LOOP--------------------------------
void loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  LED1 = digitalRead(2);
  if (digitalRead(0) == 0)
  {
    strPulsador1 = "presionado";
    switch (LED1)
    {
    case 0:
      led1 = "1";
      break;
    case 1:
      led1 = "0";
      break;
    default:
      break;
    }
  }
  else
  {
    strPulsador1 = "NO presionado";
  }
  if (strPulsador1 != strPulsadorUltimo1)
  { //envia el estado del pulsador solamente cuando cambia.
    strPulsadorUltimo1 = strPulsador1;
    led1.toCharArray(valueStr, 15);
    client.publish(SALIDADIGITAL, valueStr);
  }
}
