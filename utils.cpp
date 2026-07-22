/**
 * @file utils.cpp
 * @brief Implementation of Cryptographic and System Utilities
 * @author Embedded Systems Team
 * @version 1.0.0
 */

#include "utils.h"

namespace Utils {

String sha256Hex(const String& payload) {
    byte shaResult[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char*) payload.c_str(), payload.length());
    mbedtls_md_finish(&ctx, shaResult);
    mbedtls_md_free(&ctx);

    char hexBuffer[65];
    for (int i = 0; i < 32; i++) {
        sprintf(hexBuffer + (i * 2), "%02X", shaResult[i]);
    }
    hexBuffer[64] = '\0';

    return String(hexBuffer);
}

String hmacSha256(const String& payload, const String& key) {
    byte hmacResult[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1); // 1 = HMAC enabled
    mbedtls_md_hmac_starts(&ctx, (const unsigned char*) key.c_str(), key.length());
    mbedtls_md_hmac_update(&ctx, (const unsigned char*) payload.c_str(), payload.length());
    mbedtls_md_hmac_finish(&ctx, hmacResult);
    mbedtls_md_free(&ctx);

    char hexBuffer[65];
    for (int i = 0; i < 32; i++) {
        sprintf(hexBuffer + (i * 2), "%02X", hmacResult[i]);
    }
    hexBuffer[64] = '\0';

    return String(hexBuffer);
}

String getResetReasonString(esp_reset_reason_t reason) {
    switch (reason) {
        case ESP_RST_POWERON:    return "Power-On Reset";
        case ESP_RST_EXT:        return "External Pin Reset";
        case ESP_RST_SW:         return "Software Reset";
        case ESP_RST_PANIC:      return "Exception/Panic Reset";
        case ESP_RST_INT_WDT:    return "Interrupt Watchdog Reset";
        case ESP_RST_TASK_WDT:   return "Task Watchdog Reset";
        case ESP_RST_WDT:        return "Other Watchdog Reset";
        case ESP_RST_DEEPSLEEP:  return "Deep Sleep Exit";
        case ESP_RST_BROWNOUT:   return "Brownout Reset";
        case ESP_RST_SDIO:       return "SDIO Reset";
        default:                 return "Unknown Reset";
    }
}

String formatUptime(unsigned long uptimeMs) {
    unsigned long totalSeconds = uptimeMs / 1000;
    unsigned long days = totalSeconds / 86400;
    unsigned long hours = (totalSeconds % 86400) / 3600;
    unsigned long minutes = (totalSeconds % 3600) / 60;
    unsigned long seconds = totalSeconds % 60;

    char buf[64];
    if (days > 0) {
        snprintf(buf, sizeof(buf), "%lud %02luh %02lum %02lus", days, hours, minutes, seconds);
    } else {
        snprintf(buf, sizeof(buf), "%02luh %02lum %02lus", hours, minutes, seconds);
    }
    return String(buf);
}

String sanitizeJsonString(const String& input) {
    String output = "";
    output.reserve(input.length() + 8);
    for (unsigned int i = 0; i < input.length(); i++) {
        char c = input.charAt(i);
        if (c == '"') output += "\\\"";
        else if (c == '\\') output += "\\\\";
        else if (c == '\b') output += "\\b";
        else if (c == '\f') output += "\\f";
        else if (c == '\n') output += "\\n";
        else if (c == '\r') output += "\\r";
        else if (c == '\t') output += "\\t";
        else output += c;
    }
    return output;
}

} // namespace Utils
