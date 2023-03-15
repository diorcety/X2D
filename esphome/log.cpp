
#include "esphome/core/log.h"

extern "C" int arduino_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    esphome::esp_log_vprintf_(ESPHOME_LOG_LEVEL_INFO, "X2D", 0, format, args);
    va_end(args);
    return 0;
}