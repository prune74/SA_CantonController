/*
   Settings.cpp
*/

#include "Settings.h"

bool Settings::WIFI_ON = true;
bool Settings::DISCOVERY_ON = true;

String Settings::ssid_str;
String Settings::password_str;

// 🔥 Buffers agrandis (sécurité + compatibilité WPA2)
char Settings::ssid[64] = {};
char Settings::password[64] = {};

bool Settings::isMainReady = false;
Node *Settings::node = nullptr;

void Settings::sMainReady(bool val) { Settings::isMainReady = val; }
bool Settings::discoveryOn() { return DISCOVERY_ON; }
void Settings::discoveryOn(bool val) { Settings::DISCOVERY_ON = val; }
bool Settings::wifiOn() { return WIFI_ON; }
void Settings::wifiOn(bool val) { Settings::WIFI_ON = val; }

/*-------------------------------------------------------------
                           setup
--------------------------------------------------------------*/

void Settings::setup(Node *nd)
{
  node = nd;

  if (!SPIFFS.begin(true))
  {
    Serial.printf("[Settings %d] : An Error has occurred while mounting SPIFFS\n\n", __LINE__);
    return;
  }
  else
    Serial.printf("[Settings %d] : SPIFFS ok mounting\n", __LINE__);

  readFile();

  Serial.printf("[Settings %d] : Setup terminé\n", __LINE__);

  isMainReady = true; // Indique à la carte Discovery_Master_Board que le satellite est prêt et que les réglages sont chargés
}

/*-------------------------------------------------------------
                           begin
--------------------------------------------------------------*/

bool Settings::begin()
{
  uint8_t countReset = 0;
  Serial.printf("[Settings %d] : Attente de reponse en provenance de la carte Discovery_Master_Board.\n", __LINE__);

  do
  {
    CanMsg::sendMsg(0, 0xB2, 0, node->ID());
    vTaskDelay(pdMS_TO_TICKS(1000));
    Serial.print(".");

    if (countReset == 10)
    {
      Serial.printf(" \n\n [Settings %d] : *** Redemarrage dans 5 secondes ***\n\n", __LINE__);
      delay(5000);
      esp_restart();
    }
    countReset++;

  } while (!isMainReady);

  if (node->ID() == UNUSED_ID)
    Serial.printf("\n[Settings %d] : Le satellite ne possede pas d'identifiant.\n", __LINE__);

  while (node->ID() == UNUSED_ID)
  {
    CanMsg::sendMsg(0, 0xB4, 0, node->ID());
    vTaskDelay(pdMS_TO_TICKS(1000));

    if (node->ID() != UNUSED_ID)
      writeFile();
    else
      Serial.print(".");
  }

  Serial.printf("\n[Settings %d] : End settings\n", __LINE__);
  Serial.printf("-----------------------------------\n\n");

  return 0;
}

/*-------------------------------------------------------------
                           readFile
--------------------------------------------------------------*/

void Settings::readFile()
{
  File file = SPIFFS.open("/settings.json", "r");
  if (!file)
    return;

#ifdef DEBUG
  debug.printf("\nInformations du fichier \"settings.json\" : \n\n");
  while (file.available())
    Serial.write(file.read());
  Serial.printf("\n\n");
  file.seek(0);
#endif

  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, file);
  vTaskDelay(pdMS_TO_TICKS(100));

  if (!error)
  {
    node->ID(doc["idNode"] | UNUSED_ID);
    Discovery::comptAig(doc["comptAig"]);
    node->masqueAig(doc["masqueAig"]);

    WIFI_ON = doc["wifi_on"];
    DISCOVERY_ON = doc["discovery_on"];

    // 🔥 Lecture SSID / Password (version String)
    ssid_str = doc["ssid"].as<String>();
    password_str = doc["password"].as<String>();

    ssid_str.trim();
    password_str.trim();

    // 🔥 Copie sécurisée dans les buffers char[]
    strncpy(ssid, ssid_str.c_str(), sizeof(ssid) - 1);
    strncpy(password, password_str.c_str(), sizeof(password) - 1);

    node->maxSpeed(doc["maxSpeed"]);
    node->sensMarche(doc["sensMarche"]);

    // --- Nœuds
    const char *index[] = {"p00", "p01", "p10", "p11", "m00", "m01", "m10", "m11"};
    for (byte i = 0; i < nodePsize; i++)
    {
      if (doc[index[i]] != "null")
      {
        if (node->nodeP[i] == nullptr)
          node->nodeP[i] = new NodePeriph;
        node->nodeP[i]->ID(doc[index[i]]);
      }
    }

    // --- Aiguilles
    for (byte i = 0; i < aigSize; i++)
    {
      if (doc["aig" + String(i)] != "null")
      {
        if (node->aig[i] == nullptr)
          node->aig[i] = new Aig;

        node->aig[i]->ID(doc["aig" + String(i) + "id"]);
        node->aig[i]->posDroit(doc["aig" + String(i) + "posDroit"]);
        node->aig[i]->posDevie(doc["aig" + String(i) + "posDevie"]);
        node->aig[i]->speed(doc["aig" + String(i) + "speed"]);
        node->aig[i]->pin(doc["aig" + String(i) + "pin"]);
        node->aig[i]->setup();
      }
    }

    // --- Signaux
    for (byte i = 0; i < signalSize; i++)
    {
      if (doc["sign" + String(i)] != "null")
      {
        if (node->signal[i] == nullptr)
          node->signal[i] = new Signal;

        node->signal[i]->type(doc["sign" + String(i) + "type"]);
        node->signal[i]->position(doc["sign" + String(i) + "position"]);
      }
    }
  }

  file.close();
}

/*-------------------------------------------------------------
                           writeFile
--------------------------------------------------------------*/

void Settings::writeFile()
{
  File file = SPIFFS.open("/settings.json", "w");
  if (!file)
    return;

  DynamicJsonDocument doc(1024);

  doc["idNode"] = node->ID();
  doc["comptAig"] = Discovery::comptAig();
  doc["masqueAig"] = node->masqueAig();
  doc["wifi_on"] = WIFI_ON;
  doc["discovery_on"] = DISCOVERY_ON;

  // 🔥 Sauvegarde des Strings complètes (pas les buffers tronqués)
  doc["ssid"] = ssid_str;
  doc["password"] = password_str;

  doc["maxSpeed"] = node->maxSpeed();
  doc["sensMarche"] = node->sensMarche();

  const String index[] = {"p00", "p01", "p10", "p11", "m00", "m01", "m10", "m11"};
  for (byte i = 0; i < nodePsize; i++)
    doc[index[i]] = node->nodeP[i] ? String(node->nodeP[i]->ID()) : "null";

  for (byte i = 0; i < aigSize; i++)
  {
    if (node->aig[i] == nullptr)
      doc["aig" + String(i)] = "null";
    else
    {
      doc["aig" + String(i) + "id"] = node->aig[i]->ID();
      doc["aig" + String(i) + "posDroit"] = node->aig[i]->posDroit();
      doc["aig" + String(i) + "posDevie"] = node->aig[i]->posDevie();
      doc["aig" + String(i) + "speed"] = node->aig[i]->speed();
      doc["aig" + String(i) + "pin"] = node->aig[i]->pin();
    }
  }

  for (byte i = 0; i < signalSize; i++)
  {
    if (node->signal[i] == nullptr)
      doc["sign" + String(i)] = "null";
    else
    {
      doc["sign" + String(i) + "type"] = node->signal[i]->type();
      doc["sign" + String(i) + "position"] = node->signal[i]->position();
    }
  }

  String output;
  serializeJson(doc, output);
  file.print(output);
  file.close();
}
