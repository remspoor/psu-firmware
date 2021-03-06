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
#include "scpi_psu.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

static scpi_choice_def_t channel_choice[] = {
    { "CH1", 1 },
    { "CH2", 2 },
    SCPI_CHOICE_LIST_END /* termination of option list */
};

#define MAIN_TEMP_SENSOR_CHOICE { "MAIN", temp_sensor::MAIN }

scpi_choice_def_t main_temp_sensor_choice[] = {
    MAIN_TEMP_SENSOR_CHOICE,
    SCPI_CHOICE_LIST_END /* termination of option list */
};

#define CHANNEL_TEMP_SENSOR_CHOICE \
    { "S1",   temp_sensor::S1   }, \
    { "S2",   temp_sensor::S2   }, \
    { "BAT1", temp_sensor::BAT1 }, \
    { "BAT2", temp_sensor::BAT2 }  \

scpi_choice_def_t channel_temp_sensor_choice[] = {
    CHANNEL_TEMP_SENSOR_CHOICE,
    SCPI_CHOICE_LIST_END /* termination of option list */
};

scpi_choice_def_t all_temp_sensor_choice[] = {
    MAIN_TEMP_SENSOR_CHOICE,
    CHANNEL_TEMP_SENSOR_CHOICE,
    SCPI_CHOICE_LIST_END /* termination of option list */
};

////////////////////////////////////////////////////////////////////////////////

bool check_channel(scpi_t *context, int32_t ch) {
    if (ch < 1 || ch > CH_NUM) {
        SCPI_ErrorPush(context, SCPI_ERROR_CHANNEL_NOT_FOUND);
        return false;
    }

    if (!psu::isPowerUp()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return false;
    }

    if (Channel::get(ch - 1).isTestFailed()) {
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_ERROR);
        return false;
    }

    if (!Channel::get(ch - 1).isPowerOk()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CHANNEL_FAULT_DETECTED);
        return false;
    }

    if (!Channel::get(ch - 1).isOk()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return false;
    }

    return true;
}

Channel *param_channel(scpi_t *context, scpi_bool_t mandatory, scpi_bool_t skip_channel_check) {
    int32_t ch;

    if (!SCPI_ParamChoice(context, channel_choice, &ch, mandatory)) {
        if (mandatory || SCPI_ParamErrorOccurred(context)) {
            return 0;
        }
        scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
        ch = psu_context->selected_channel_index;
    }

    if (!skip_channel_check && !check_channel(context, ch)) return 0;

    return &Channel::get(ch - 1);
}

Channel *set_channel_from_command_number(scpi_t *context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    int32_t ch;
    SCPI_CommandNumbers(context, &ch, 1, psu_context->selected_channel_index);
    if (ch < 1 || ch > CH_NUM) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return 0;
    }

    if (!check_channel(context, ch)) return 0;

    return &Channel::get(ch - 1);
}

bool get_voltage_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_voltage_from_param(context, param, value, channel, cv);
}

bool get_current_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_current_from_param(context, param, value, channel, cv);
}

bool get_power_param(scpi_t *context, float &value, float min, float max, float def) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_power_from_param(context, param, value, min, max, def);
}

bool get_temperature_param(scpi_t *context, float &value, float min, float max, float def) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_temperature_from_param(context, param, value, min, max, def);
}

bool get_duration_param(scpi_t *context, float &value, float min, float max, float def) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_duration_from_param(context, param, value, min, max, def);
}


bool get_voltage_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = channel->U_MAX;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = channel->U_MIN;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = channel->U_DEF;
        }
        else if (param.tag == SCPI_NUM_UP && cv) {
            value = cv->set + cv->step;
            if (value > channel->U_MAX) value = channel->U_MAX;
        }
        else if (param.tag == SCPI_NUM_DOWN && cv) {
            value = cv->set - cv->step;
            if (value < channel->U_MIN) value = channel->U_MIN;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_VOLT) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < channel->U_MIN || value > channel->U_MAX) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

bool get_current_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = channel->I_MAX;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = channel->I_MIN;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = channel->I_DEF;
        }
        else if (param.tag == SCPI_NUM_UP && cv) {
            value = cv->set + cv->step;
            if (value > channel->I_MAX) value = channel->I_MAX;
        }
        else if (param.tag == SCPI_NUM_DOWN && cv) {
            value = cv->set - cv->step;
            if (value < channel->I_MIN) value = channel->I_MIN;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_AMPER) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < channel->I_MIN || value > channel->I_MAX) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

bool get_power_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = max;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = min;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < min || value > max) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }
    return true;
}

bool get_temperature_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = max;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = min;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_CELSIUS) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < min || value > max) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

bool get_duration_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = max;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = min;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_SECONDS) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < min || value > max) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

scpi_result_t result_float(scpi_t * context, float value) {
    char buffer[32] = { 0 };
    util::strcatFloat(buffer, value);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));
    return SCPI_RES_OK;
}

bool get_profile_location_param(scpi_t * context, int &location, bool all_locations) {
    int32_t param;
    if (!SCPI_ParamInt(context, &param, true)) {
        return false;
    }

    if (param < (all_locations ? 0 : 1) || param > NUM_PROFILE_LOCATIONS - 1) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return false;
    }

    location = (int)param;

    return true;
}

}
}
} // namespace eez::psu::scpi