void WiFiBegin() {
  //Manually change between WiFi and Accesspoint. AP will be used as a fallback, after 5 seconds
  if (UseAcessPointMode)
    CreateAccessPoint();
  else
    ConnectToAccessPoint();

  webServer.on("/", handle_root);
  webServer.on("/level", handle_level);
  webServer.on("/valuation", handle_valuation);
  webServer.on("/threshold", handle_threshold);
  webServer.on("/invertation", handle_invertation);
  webServer.on("/setup", handle_setup);
  webServer.on("/main.js", handle_script);
  webServer.on("/style.css", handle_style);
  webServer.on("/favicon.ico", handle_icon);
  webServer.on("/calibrate", handle_calibrate);
  webServer.on("/generate_204", handle_root);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  webServer.on("/fwlink", handle_root);   //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  webServer.onNotFound(handleNotFound);

  webServer.begin();
  Serial.println("HTTP webServer started");
  delay(100);
}

void ConnectToAccessPoint() {
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
  Serial.println("");
  Serial.println("WiFi connected successfully");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());  //Show ESP32 IP on serial
}

void CreateAccessPoint() {
  WiFi.disconnect();
  IPAddress local_ip(8, 8, 8, 8);
  IPAddress gateway(8, 8, 8, 8);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.mode(WIFI_AP);
  WiFi.softAP("Sport&Fun Leveler");
  delay(500);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(500);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  /* Setup the DNS webServer redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", local_ip);
}

void handle_root() {
  Serial.println(F("Handle Root"));
  String path = "/index.html";
  if (!SPIFFS.exists(path))
    return;

  File f = SPIFFS.open(path);
  webServer.streamFile(f, "text/html");
  f.close();
}

void handle_level() {
  // /level
  if (!accelInitialized) {
    webServer.send(400, "text/plain", "Gyro not initialized!");
    return;
  }
  getLevel();
  String txt = String(invertAxis ? levelY : levelX);
  txt.concat("|");
  txt.concat(String(invertAxis ? levelX : levelY));
  txt.concat("|");
  txt.concat(String(levelThreshold));
  webServer.send(200, "text/plain", txt);
}

void handle_valuation() {
  // /valuation?x=270&y=267
  Serial.println(F("Handle Valuation"));

  if (webServer.args() == 2) {
    Serial.println(String(F(" ")) + webServer.argName(0) + F(": ") + webServer.arg(0));
    Serial.println(String(F(" ")) + webServer.argName(1) + F(": ") + webServer.arg(1));
    valuationX = webServer.arg(0).toInt();
    valuationY = webServer.arg(1).toInt();
    StoreLevelValuation();
    String txt = "OK (";
    txt.concat(String(valuationX));
    txt.concat("/");
    txt.concat(String(valuationY));
    txt.concat(")");
    webServer.send(200, "text/plain", txt);
    return;
  }
  webServer.send(400, "text/plain", F("Parameter missing or out of Range!"));
}

void handle_threshold() {
  // /threshold?v=10
  Serial.println(F("Handle Threshold"));

  if (webServer.args() == 1) {
    Serial.print(String(F(" ")) + webServer.argName(0) + F(": ") + webServer.arg(0) + F("\n"));
    int i = webServer.arg(0).toInt();
    if (i > 0 && i <= 90) {
      levelThreshold = i;
      StoreLevelThreshold();
      String txt = "Threshold OK (";
      txt.concat(String(levelThreshold));
      txt.concat(")");
      webServer.send(200, "text/plain", txt);
      return;
    }
  }
  webServer.send(400, "text/plain", F("Parameter missing or out of Range!"));
}

void handle_invertation() {
  // /invertation?v=1
  Serial.println(F("Handle Invert Axis"));

  if (webServer.args() == 1) {
    Serial.print(String(F(" ")) + webServer.argName(0) + F(": ") + webServer.arg(0) + F("\n"));
    invertAxis = webServer.arg(0) == "1";
    StoreInvertation();
    String txt = "Invert Axis OK (";
    txt.concat(String(invertAxis));
    txt.concat(")");
    webServer.send(200, "text/plain", txt);
    return;
  }
  webServer.send(400, "text/plain", F("Parameter missing or out of Range!"));
}

void handle_setup() {
  // /setup
  Serial.println(F("Handle Setup"));

  String txt = String(accelInitialized);
  txt.concat("|");
  txt.concat(String(valuationX));
  txt.concat("|");
  txt.concat(String(valuationY));
  txt.concat("|");
  txt.concat(String(invertAxis));
  webServer.send(200, "text/plain", txt);
}

void handle_script() {
  Serial.println(F("Handle Script"));

  String path = "/main.js";
  if (!SPIFFS.exists(path))
    return;

  File f = SPIFFS.open(path);
  webServer.streamFile(f, "text/javascript");
  f.close();
}

void handle_style() {
  Serial.println(F("Handle Style"));

  String path = "/style.css";
  if (!SPIFFS.exists(path))
    return;

  File f = SPIFFS.open(path);
  webServer.streamFile(f, "text/css");
  f.close();
}

void handle_icon() {
  Serial.println(F("Handle Icon"));

  String path = "/favicon.ico";
  if (!SPIFFS.exists(path))
    return;

  File f = SPIFFS.open(path);
  webServer.streamFile(f, "image/x-icon");
  f.close();
}

void handle_calibrate() {
  Serial.println(F("Handle Calibration"));
  CalibrateLevel();
  String result = "Calibration OK (";
  result.concat(calibrationX);
  result.concat("/");
  result.concat(calibrationY);
  result.concat(")");
  webServer.send(200, "text/plaint", result);
}

void handleNotFound() {
  Serial.println(F("HandleNotFound"));

  for (uint8_t i = 0; i < webServer.args(); i++) {
    Serial.println(String(F(" ")) + webServer.argName(i) + F(": ") + webServer.arg(i) + F("\n"));
  }

  if (captivePortal())
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

boolean captivePortal() {
  Serial.print(F("Captive Check: "));
  Serial.println(webServer.hostHeader());
  if (!isIp(webServer.hostHeader())) {
    Serial.println("Request redirected to captive portal:");
    Serial.println(webServer.hostHeader());
    webServer.sendHeader("Location", String("http://") + toStringIp(webServer.client().localIP()), true);
    webServer.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    webServer.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}