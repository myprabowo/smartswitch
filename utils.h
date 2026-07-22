/**
 * @file utils.h
 * @brief System Utilities & Cryptographic Helper Functions for Tuya Signatures
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#ifndef PUMP_REMOTE_UTILS_H
#define PUMP_REMOTE_UTILS_H

#include <Arduino.h>
#include <esp_system.h>
#include <mbedtls/md.h>

namespace Utils {

/**
 * @brief Computes SHA256 hash of string and returns uppercase hex string.
 * @param payload Input string
 * @return Uppercase hex SHA256 hash
 */
String sha256Hex(const String& payload);

/**
 * @brief Computes HMAC-SHA256 signature using secret key and returns uppercase hex.
 * @param payload Message string to sign
 * @param key Secret key string
 * @return Uppercase hex HMAC signature
 */
String hmacSha256(const String& payload, const String& key);

/**
 * @brief Converts reset reason enum to human-readable string.
 * @param reason esp_reset_reason_t
 * @return String description
 */
String getResetReasonString(esp_reset_reason_t reason);

/**
 * @brief Formats milliseconds into human readable HH:MM:SS string.
 * @param uptimeMs Milliseconds since boot
 * @return Formatted uptime string
 */
String formatUptime(unsigned long uptimeMs);

/**
 * @brief Sanitizes string for safe JSON output.
 * @param input Raw string
 * @return Escaped string
 */
String sanitizeJsonString(const String& input);

} // namespace Utils

#endif // PUMP_REMOTE_UTILS_H
