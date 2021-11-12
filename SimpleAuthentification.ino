#include "pagina.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <FS.h>
#include "SPIFFS.h"

#define file_ssid "/ssid.txt"
#define file_pass "/pass.txt"

bool rede_conhecida = false;
String nome_rede, senha_rede;

#define LED 2

//const char* ssid = "";      //"VIVOFIBRA-9501"
//const char* password = "";  //"rgw7ucm3GT"

WebServer server(80);

//Check if header is present and correct
bool is_authentified() {
  Serial.println("Enter is_authentified");
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentification Successful");
      return true;
    }
  }
  Serial.println("Authentification Failed");
  return false;
}

//login page, also called for disconnect
void handleLogin() {
  String msg;
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
    server.send(301);
    return;
  }
    
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")) {
    if (server.arg("USERNAME") == "igor" &&  server.arg("PASSWORD") == "1234") {
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
      server.send(301);
      Serial.println("Log in Successful");
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
  }
  String content = "<html><body><div align='center'><form action='/login' method='POST'>To log in, please use : igor/1234<br>";
  content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  content += "You also can go <a href='/inline'>here</a></div></body></html>";
  server.send(200, "text/html", content);
}
//=============================================================================//
//root page can be accessed only if authentification is ok
void handleRoot() {
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authentified()) {
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }

  if(server.arg("LED") == "on"){
    digitalWrite(LED, HIGH);
  }
  else if(server.arg("LED") == "off"){
    digitalWrite(LED, LOW);
  }
/*  
  String content = "<html><body><H2>hello, you successfully connected to esp8266!</H2><br>";
  if (server.hasHeader("User-Agent")) {
    content += "the user agent used is : " + server.header("User-Agent") + "<br><br>";
  }
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  
  content = "<html><body><div align='center'><form action='/teste' method='POST'>Digite um valor<br>";
  content += "Valor:<input type='text' name='valor' placeholder='VALOR'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form><br>";
  content += "<div align='center'><button><a href='/?LED=on'>Liga</a></button><button><a href='/?LED=off'>Desliga</a></button></div>";
  content += "</div></body></html>";
*/ 
  server.send(200, "text/html", webpageCode);
}//end handleRoot()
//=============================================================================//
//no need authentification
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}//end handleNotFound()
//=============================================================================//
void setup(void) {
  Serial.begin(115200);
  while(!rede_conhecida){
    Serial.println("Recuperando Dados");
    openFS();
    delay(1000);
  }//enquanto nao houver dados salvos
  Serial.println("Inicio Setup");

  char nome[50];
  nome_rede.toCharArray(nome, 50);
  Serial.println(nome);
  char senha[50];
  senha_rede.toCharArray(senha, 50);
  Serial.println(senha);
  
  pinMode(LED, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(nome, senha);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(nome);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  server.on("/", handleRoot);
  server.on("/login", handleLogin); 
  server.on("/teste", teste); 
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works without need of authentification");
  });

  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize);
  server.begin();
  Serial.println("HTTP server started");
}//en setup()
//=============================================================================//
void loop(void) {
  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks
}//end loop()
//=============================================================================//
void teste(){
  Serial.println("Enter teste");
  String header;
  if (!is_authentified()) {
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }  
  
  if(server.arg("valor")){
    String  content = "<html><body><H1>TESTE ESP32!</H1><hr>";
    content += "<p>" + server.arg("valor") + "</p>";
    content += "<button><a href='/'>Voltar</a></button></body></html>";  
    server.send(200, "text/html", content);
  }
}//end teste()
//=============================================================================//
//=============================================================================//
void openFS(void){
  if(!SPIFFS.begin()){
    Serial.println("\n SPIFFS Erro ao abrir n");
  } else {
    nome_rede = readFile(file_ssid);
    Serial.print("Rede: ");
    Serial.println(nome_rede);
    
    senha_rede = readFile(file_pass);
    Serial.print("Pass: ");
    Serial.println(senha_rede);
  }
  if(nome_rede == "" or senha_rede == ""){
    Serial.println("Sem dados salvos");//CHAMAR TELA DE ENTRADA DE DADOS DA REDE
    rede_conhecida = false;
  }else{
    Serial.println("Ok");
    rede_conhecida = true;
  }//end teste de dados
}//end open FS
String readFile(String path) {
  File rFile = SPIFFS.open(path,"r");
  if (!rFile) {
    Serial.println("Erro ao ler o arquivo");
  }
  String content = rFile.readStringUntil('\r'); //desconsidera '\r\n'
  rFile.close();
  return content;
}//end read file
//=============================================================================//
void writeFile(String state, String path) {
  File rFile = SPIFFS.open(path,"w+"); 
  if(!rFile){
    Serial.println("Erro ao abrir arquivo!");
  } else {
    rFile.println(state);
    Serial.println("Arquivo criado");
  }
  rFile.close();
}//end write file
