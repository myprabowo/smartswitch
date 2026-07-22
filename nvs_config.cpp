/**
 * @file nvs_config.cpp
 * @brief Implementation of NVS Configuration Manager
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#include "nvs_config.h"
#include <ArduinoJson.h>

NVSConfigManager nvsConfigManager;

NVSConfigManager::NVSConfigManager() : _bootCount(0), _consecutiveCrashes(0) {
    loadDefaults();
}

void NVSConfigManager::loadDefaults() {
    _config.deviceName     = DEFAULT_DEVICE_NAME;
    _config.hostname       = DEFAULT_HOSTNAME;
    _config.otaPassword    = DEFAULT_OTA_PASSWORD;
    _config.tuyaAccessId   = TUYA_ACCESS_ID;
    _config.tuyaAccessSecret = TUYA_ACCESS_SECRET;
    _config.tuyaDeviceId   = TUYA_DEVICE_ID;
    _config.tuyaEndpoint   = TUYA_ENDPOINT;
    _config.tuyaSwitchCode = TUYA_SWITCH_CODE;
    _config.logLevel       = DEFAULT_LOG_LEVEL;
    _config.gmtOffsetSec   = DEFAULT_GMT_OFFSET_SEC;
}

void NVSConfigManager::begin() {
    _prefs.begin("pump_cfg", false);

    _config.deviceName       = _prefs.getString("dev_name", DEFAULT_DEVICE_NAME);
    _config.hostname         = _prefs.getString("hostname", DEFAULT_HOSTNAME);
    _config.otaPassword      = _prefs.getString("ota_pwd", DEFAULT_OTA_PASSWORD);
    _config.tuyaAccessId     = _prefs.getString("tuya_id", TUYA_ACCESS_ID);
    _config.tuyaAccessSecret = _prefs.getString("tuya_sec", TUYA_ACCESS_SECRET);
    _config.tuyaDeviceId     = _prefs.getString("tuya_dev", TUYA_DEVICE_ID);
    _config.tuyaEndpoint     = _prefs.getString("tuya_ep", TUYA_ENDPOINT);
    _config.tuyaSwitchCode   = _prefs.getString("tuya_code", TUYA_SWITCH_CODE);
    _config.logLevel         = _prefs.getInt("log_lvl", DEFAULT_LOG_LEVEL);
    _config.gmtOffsetSec     = _prefs.getLong("gmt_off", DEFAULT_GMT_OFFSET_SEC);

    _bootCount           = _prefs.getUInt("boot_cnt", 0);
    _consecutiveCrashes = _prefs.getUInt("crash_cnt", 0);

    _prefs.end();
}

bool NVSConfigManager::saveConfig(const SystemConfig& config) {
    _config = config;
    _prefs.begin("pump_cfg", false);
    _prefs.putString("dev_name", _config.deviceName);
    _prefs.putString("hostname", _config.hostname);
    _prefs.putString("ota_pwd", _config.otaPassword);
    _prefs.putString("tuya_id", _config.tuyaAccessId);
    _prefs.putString("tuya_sec", _config.tuyaAccessSecret);
    _prefs.putString("tuya_dev", _config.tuyaDeviceId);
    _prefs.putString("tuya_ep", _config.tuyaEndpoint);
    _prefs.putString("tuya_code", _config.tuyaSwitchCode);
    _prefs.putInt("log_lvl", _config.logLevel);
    _prefs.putLong("gmt_off", _config.gmtOffsetSec);
    _prefs.end();
    return true;
}

void NVSConfigManager::setTuyaCredentials(const String& accessId, const String& secret, const String& deviceId, const String& endpoint) {
    _config.tuyaAccessId = accessId;
    _config.tuyaAccessSecret = secret;
    _config.tuyaDeviceId = deviceId;
    _config.tuyaEndpoint = endpoint;
    saveConfig(_config);
}

uint32_t NVSConfigManager::incrementBootCount() {
    _prefs.begin("pump_cfg", false);
    _bootCount++;
    _consecutiveCrashes++;
    _prefs.putUInt("boot_cnt", _bootCount);
    _prefs.putUInt("crash_cnt", _consecutiveCrashes);
    _prefs.end();
    return _consecutiveCrashes;
}

void NVSConfigManager::clearConsecutiveCrashes() {
    _consecutiveCrashes = 0;
    _prefs.begin("pump_cfg", false);
    _prefs.putUInt("crash_cnt", 0);
    _prefs.end();
}

uint32_t NVSConfigManager::getConsecutiveCrashes() {
    return _consecutiveCrashes;
}

void NVSConfigManager::factoryReset() {
    _prefs.begin("pump_cfg", false);
    _prefs.clear();
    _prefs.end();
    loadDefaults();
}

String NVSConfigManager::exportJson(bool includeSecrets) const {
    StaticJsonDocument<512> doc;
    doc["deviceName"]     = _config.deviceName;
    doc["hostname"]       = _config.hostname;
    doc["tuyaAccessId"]   = _config.tuyaAccessId;
    doc["tuyaAccessSecret"] = includeSecrets ? _config.tuyaAccessSecret : "********";
    doc["tuyaDeviceId"]   = _config.tuyaDeviceId;
    doc["tuyaEndpoint"]   = _config.tuyaEndpoint;
    doc["tuyaSwitchCode"] = _config.tuyaSwitchCode;
    doc["logLevel"]       = _config.logLevel;
    doc["gmtOffsetSec"]   = _config.gmtOffsetSec;

    String output;
    serializeJson(doc, output);
    return output;
}

bool NVSConfigManager::importJson(const String& json) {
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err) return false;

    if (doc.containsKey("deviceName"))     _config.deviceName     = doc["deviceName"].as<String>();
    if (doc.containsKey("hostname"))       _config.hostname       = doc["hostname"].as<String>();
    if (doc.containsKey("tuyaAccessId"))   _config.tuyaAccessId   = doc["tuyaAccessId"].as<String>();
    if (doc.containsKey("tuyaAccessSecret") && doc["tuyaAccessSecret"] != "********") {
        _config.tuyaAccessSecret = doc["tuyaAccessSecret"].as<String>();
    }
    if (doc.containsKey("tuyaDeviceId"))   _config.tuyaDeviceId   = doc["tuyaDeviceId"].as<String>();
    if (doc.containsKey("tuyaEndpoint"))   _config.tuyaEndpoint   = doc["tuyaEndpoint"].as<String>();
    if (doc.containsKey("tuyaSwitchCode")) _config.tuyaSwitchCode = doc["tuyaSwitchCode"].as<String>();
    if (doc.containsKey("logLevel"))       _config.logLevel       = doc["logLevel"].as<int>();
    if (doc.containsKey("gmtOffsetSec"))   _config.gmtOffsetSec   = doc["gmtOffsetSec"].as<long>();

    return saveConfig(_config);
}
