#include "main.h"

void Web_Server::WiFiBegin() {
  //Manually change between WiFi and Accesspoint. AP will be used as a fallback, after 5 seconds
  if (useAccessPointMode)
    CreateAccessPoint();
  else
    ConnectToAccessPoint();
  
  webServer.on("/level", handle_level);
  webServer.on("/setup", handle_setup); 
  webServer.on("/calibrate", handle_calibrate);
  webServer.on("/valuate", handle_valuation);
  webServer.on("/reset", handle_reset);
  webServer.on("/upload",  HTTP_POST,[](){ webServer.send(200);}, handle_fileupload);

  //Allways redirect to captive portal. Request comes with IP (8.8.8.8) or URL (connectivitycheck.XXX / captive.apple / etc.)  
  webServer.on("/generate_204", redirect);    //Android captive portal.
  webServer.on("/fwlink", redirect);   //Microsoft captive portal.
  
  webServer.on("/connecttest.txt", redirect); //www.msftconnecttest.com
  webServer.on("/hotspot-detect.html", redirect); //captive.apple.com
  
  webServer.on("/success.txt", handle_success); //detectportal.firefox.com/sucess.txt
  webServer.onNotFound(handleFileRead);

  const char* Headers[] = {"If-None-Match"};
  webServer.collectHeaders(Headers, sizeof(Headers) / sizeof(Headers[0]));

  webServer.begin();
  Serial.println(F("HTTP webServer started"));
  delay(100);
}

void Web_Server::ConnectToAccessPoint() {
  WiFi.begin(ssid, password);

  long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    if (millis() - start > 5000) {
      Serial.println("Wifi not found!");
      CreateAccessPoint();
      return;
    }
  }
  Serial.println();
  Serial.println(F("WiFi connected successfully"));
  Serial.print(F("Got IP: "));
  Serial.println(WiFi.localIP());  //Show ESP32 IP on serial
}

void Web_Server::CreateAccessPoint() {
  WiFi.disconnect();
  IPAddress local_ip(8, 8, 8, 8);
  IPAddress gateway(8, 8, 8, 8);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(deviceName);
  delay(500);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(500);
  Serial.print(F("AP IP address: "));
  Serial.println(WiFi.softAPIP());

  /* Setup the DNS webServer redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", local_ip);
}

void Web_Server::handle_root() {
  // /
  // /index.html  
  Serial.println(F("Handle Root"));

  PrintIncomingRequest();
  
  String path = "/index.html";
  if (!LittleFS.exists(path))
    return;

  File f = LittleFS.open(path);
  webServer.streamFile(f, "text/html");
  f.close();
}

void Web_Server::handle_level() {
  // /level
  if (!accelInitialized) {
    webServer.send(400, "text/plain", "Gyro not initialized!");
    return;    
  }

  String txt = String(invertAxis ? levelY : levelX);
  txt.concat("|");
  txt.concat(String(invertAxis ? levelX * -1 : levelY));
  txt.concat("|");
  txt.concat(String(levelThreshold));
  String customText = helper.GetCustomText();  
  if(customText.length() > 0){    
    txt.concat("|");
    txt.concat(customText);
  }
  webServer.send(200, "text/plain", txt);

  lastMillisClientAvailable = millis();
}

void Web_Server::handle_setup() {
  // /setup
  Serial.println(F("Handle Setup"));

  //With arguments:
  // /setup?x=123&y=321&inv=0&ap=1
  helper.ProcessSetupArguments();

  String txt = String(accelInitialized);
  txt.concat("|");
  txt.concat(String(valuationX));
  txt.concat("|");
  txt.concat(String(valuationY));
  txt.concat("|");
  txt.concat(String(invertAxis));
  txt.concat("|");
  txt.concat(String(useAccessPointMode));
  webServer.send(200, "text/plain", txt);
}

void Web_Server::handle_calibrate() {
  Serial.println(F("Handle Calibration"));
  level.CalibrateLevel();
  String result = "Calibration OK (";
  result.concat(calibrationX);
  result.concat("/");
  result.concat(calibrationY);
  result.concat(")");
  webServer.send(200, "text/plaint", result);
}

void Web_Server::handle_valuation() {
  Serial.println(F("Handle Valuation"));  
  String result = "Calibration ";
  if(valuationActive)
    result.concat("stopped");
  else {
    minValuationX = 0;
    minValuationY = 0;
    maxValuationX = 0;
    maxValuationY = 0;
    result.concat("started");
  }
  valuationActive = !valuationActive; 
  webServer.send(200, "text/plaint", result);
}

void Web_Server::handle_reset(){
  data.LoadData();
  webServer.send(200, "text/plaint", "OK");
}

void Web_Server::handleNotFound() {
  Serial.println(F("HandleNotFound"));

  PrintIncomingRequest();

  if (webserver.captivePortal())
    return;
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += webServer.uri();
  message += F("\nMethod: ");
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += webServer.args();
  message += F("\n");

  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += String(F(" ")) + webServer.argName(i) + F(": ") + webServer.arg(i) + F("\n");
  }
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.send(404, "text/plain", message);
}

void Web_Server::handle_success(){
  Serial.println(F("Handle success.txt"));
  webServer.send(200, "text/plain", "success");
}

boolean Web_Server::captivePortal() {
  Serial.print(F("Captive Check: "));
  Serial.println(webServer.hostHeader());
  if (!helper.isIp(webServer.hostHeader())) {
    Serial.println("-Request redirected to captive portal");
    redirect();
    return true;
  }
  return false;
}

void Web_Server::redirect(){
  webServer.sendHeader("Location", String("http://") + helper.toStringIp(webServer.client().localIP()), true);
  webServer.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
  webServer.client().stop(); // Stop is needed because we sent no content length
}

void Web_Server::PrintIncomingRequest(){  
  Serial.println(webServer.hostHeader());
  Serial.print("  ");
  Serial.println(webServer.uri());
  
  for (uint8_t i = 0; i < webServer.args(); i++)
    Serial.println(String(F(" ")) + webServer.argName(i) + F(": ") + webServer.arg(i) + F("\n"));
  
  for (int i = 0; i < webServer.headers(); i++)
    Serial.println(String(F("\t")) + webServer.headerName(i) + F(": ") + webServer.header(i));  
}

