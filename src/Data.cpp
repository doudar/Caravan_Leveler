#include "main.h"

void Data::LoadData(){
  this->LoadLevel();
  this->LoadValuation();
  this->LoadLevelThreshold();
  this->LoadInvertation();
  this->LoadAP();
}
void Data::LoadLevel() {
  calibrationX = helper.readIntFromEEPROM(EEPROM_LEVEL_X);
  calibrationY = helper.readIntFromEEPROM(EEPROM_LEVEL_Y);

  if (calibrationX == 0xffff || calibrationY == 0xffff) {
    calibrationX = -150;
    calibrationY = -125;
    StoreLevel();
  }
  Serial.print(F("Loaded X: "));
  Serial.print(calibrationX);
  Serial.print(F(" Y: "));
  Serial.println(calibrationY);
}

void Data::LoadValuation() {
  valuationX = helper.readIntFromEEPROM(EEPROM_VALUATION_X);
  valuationY = helper.readIntFromEEPROM(EEPROM_VALUATION_Y);

  if (valuationX == 0xffff || valuationY == 0xffff) {
    valuationX = 271;
    valuationY = 267;
    data.StoreLevelValuation();
  }
  Serial.print(F("Loaded Valuation: "));
  Serial.print(F(" X: "));
  Serial.print(valuationX);
  Serial.print(F(" Y: "));
  Serial.println(valuationY);
}
void Data::LoadLevelThreshold() {
  levelThreshold = EEPROM.read(EEPROM_LEVEL_THRESHOLD);
  if (levelThreshold == 0xff || levelThreshold == 0)
    levelThreshold = 5;
  Serial.print(F("Loaded Threshold: "));
  Serial.println(levelThreshold);
}
void Data::LoadInvertation() {
  invertAxis = EEPROM.read(EEPROM_LEVEL_INVERTATION);
  Serial.print(F("Loaded Inverted Axis: "));
  Serial.println(invertAxis);
}

void Data::LoadAP() {
  useAccessPointMode = EEPROM.read(EEPROM_LEVEL_ACCESSPOINT);
  Serial.print(F("Loaded AccessPoint: "));
  Serial.println(useAccessPointMode);
}

void Data::StoreLevel() {
  helper.writeIntIntoEEPROM(EEPROM_LEVEL_X, calibrationX);
  helper.writeIntIntoEEPROM(EEPROM_LEVEL_Y, calibrationY);
  EEPROM.commit();
}

void Data::StoreLevelValuation() {
  helper.writeIntIntoEEPROM(EEPROM_VALUATION_X, valuationX);
  helper.writeIntIntoEEPROM(EEPROM_VALUATION_Y, valuationY);
  EEPROM.commit();
}
void Data::StoreLevelThreshold() {
  EEPROM.write(EEPROM_LEVEL_THRESHOLD, levelThreshold);
  EEPROM.commit();
}
void Data::StoreInvertation() {
  EEPROM.write(EEPROM_LEVEL_INVERTATION, invertAxis);
  EEPROM.commit();
}

void Data::StoreAP() {
  EEPROM.write(EEPROM_LEVEL_ACCESSPOINT, useAccessPointMode);
  EEPROM.commit();
}

void Data::InitDeepPass()
{
  deepPassX.setInitial(0);
  deepPassY.setInitial(0);
}