/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "psu.h"
#include <scpi-parser.h>
#include "scpi_psu.h"
#include "scpi_syst.h"

#include "datetime.h"
#include "sound.h"
#include "profile.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_syst_CapabilityQ(scpi_t * context) {
    char text[sizeof(STR_SYST_CAP)];
    strcpy_P(text, PSTR(STR_SYST_CAP));
    SCPI_ResultText(context, text);
    
    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_ErrorNextQ(scpi_t * context) {
    return SCPI_SystemErrorNextQ(context);
}

scpi_result_t scpi_syst_ErrorCountQ(scpi_t * context) {
    return SCPI_SystemErrorCountQ(context);
}

scpi_result_t scpi_syst_VersionQ(scpi_t * context) {
    return SCPI_SystemVersionQ(context);
}

scpi_result_t scpi_syst_Power(scpi_t * context) {
    bool up;
    if (!SCPI_ParamBool(context, &up, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (temperature::isSensorTripped(temp_sensor::MAIN)) {
        SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_EXECUTE_BEFORE_CLEARING_PROTECTION);
        return SCPI_RES_ERR;
    }

    if (!psu::changePowerState(up)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_PowerQ(scpi_t * context) {
    SCPI_ResultBool(context, psu::isPowerUp());
    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_Date(scpi_t * context) {
    int32_t year;
    if (!SCPI_ParamInt(context, &year, TRUE)) {
        return SCPI_RES_ERR;
    }

    int32_t month;
    if (!SCPI_ParamInt(context, &month, TRUE)) {
        return SCPI_RES_ERR;
    }

    int32_t day;
    if (!SCPI_ParamInt(context, &day, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (year < 2000 || year > 2099) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    year = year - 2000;

    if (!datetime::isValidDate((uint8_t)year, (uint8_t)month, (uint8_t)day)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }

    if (!datetime::setDate((uint8_t)year, (uint8_t)month, (uint8_t)day)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_DateQ(scpi_t * context) {
    uint8_t year, month, day;
    if (!datetime::getDate(year, month, day)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    char buffer[16] = { 0 };
    sprintf_P(buffer, PSTR("%d, %d, %d"), (int)(year + 2000), (int)month, (int)day);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_Time(scpi_t * context) {
    int32_t hour;
    if (!SCPI_ParamInt(context, &hour, TRUE)) {
        return SCPI_RES_ERR;
    }

    int32_t minute;
    if (!SCPI_ParamInt(context, &minute, TRUE)) {
        return SCPI_RES_ERR;
    }

    int32_t second;
    if (!SCPI_ParamInt(context, &second, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (!datetime::isValidTime((uint8_t)hour, (uint8_t)minute, (uint8_t)second)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }

    if (!datetime::setTime((uint8_t)hour, (uint8_t)minute, (uint8_t)second)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TimeQ(scpi_t * context) {
    uint8_t hour, minute, second;
    if (!datetime::getTime(hour, minute, second)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    char buffer[16] = { 0 };
    sprintf_P(buffer, PSTR("%d, %d, %d"), (int)hour, (int)minute, (int)second);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_Beeper(scpi_t * context) {
    sound::playBeep(true);
    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_BeeperState(scpi_t * context) {
    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (enable != persist_conf::isBeepEnabled()) {
        persist_conf::enableBeep(enable);
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_BeeperStateQ(scpi_t * context) {
    SCPI_ResultBool(context, persist_conf::isBeepEnabled());
    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionClear(scpi_t * context) {
    int32_t sensor;
    if (!SCPI_ParamChoice(context, main_temp_sensor_choice, &sensor, false)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
    }

    temperature::clearProtection((temp_sensor::Type)sensor);

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionLevel(scpi_t * context) {
    float level;
    if (!get_temperature_param(context, level, OTP_MAIN_MIN_LEVEL, OTP_MAIN_MAX_LEVEL, OTP_MAIN_DEFAULT_LEVEL)) {
        return SCPI_RES_ERR;
    }

    int32_t sensor;
    if (!SCPI_ParamChoice(context, main_temp_sensor_choice, &sensor, false)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
    }

    temperature::prot_conf[sensor].level = level;
    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionLevelQ(scpi_t * context) {
    int32_t sensor;
    if (!SCPI_ParamChoice(context, main_temp_sensor_choice, &sensor, false)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
    }

    return result_float(context, temperature::prot_conf[sensor].level);
}

scpi_result_t scpi_syst_TempProtectionState(scpi_t * context) {
    bool state;
    if (!SCPI_ParamBool(context, &state, TRUE)) {
        return SCPI_RES_ERR;
    }

    int32_t sensor;
    if (!SCPI_ParamChoice(context, main_temp_sensor_choice, &sensor, false)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
    }

    temperature::prot_conf[sensor].state = state;
    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionStateQ(scpi_t * context) {
    int32_t sensor;
    if (!SCPI_ParamChoice(context, main_temp_sensor_choice, &sensor, false)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
    }

    SCPI_ResultBool(context, temperature::prot_conf[sensor].state);

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionDelay(scpi_t * context) {
    float delay;
    if (!get_duration_param(context, delay, OTP_MAIN_MIN_DELAY, OTP_MAIN_MAX_DELAY, OTP_MAIN_DEFAULT_DELAY)) {
        return SCPI_RES_ERR;
    }

    int32_t sensor;
    if (!SCPI_ParamChoice(context, main_temp_sensor_choice, &sensor, false)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
    }

    temperature::prot_conf[sensor].delay = delay;
    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionDelayQ(scpi_t * context) {
    int32_t sensor;
    if (!SCPI_ParamChoice(context, main_temp_sensor_choice, &sensor, false)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
    }

    SCPI_ResultFloat(context, temperature::prot_conf[sensor].delay);

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionTrippedQ(scpi_t * context) {
    int32_t sensor;
    if (!SCPI_ParamChoice(context, main_temp_sensor_choice, &sensor, false)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
    }

    SCPI_ResultBool(context, temperature::isSensorTripped((temp_sensor::Type)sensor));

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi