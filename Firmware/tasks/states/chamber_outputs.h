#pragma once

#include "FreeRTOS.h"
#include "thermo_control_task.h"

/* All thermo load GPIO (heat, compressor, condenser, internal fan) is driven
 * from chamber_outputs.c — heater and compressor contactor cannot both be on. */

void chamber_outputs_apply_idle(TickType_t now);
void chamber_outputs_apply_heating(const thermo_control_config_t *cfg,
                                   float air_sp, TickType_t now);
void chamber_outputs_apply_cool_slow(TickType_t now);
void chamber_outputs_apply_cool_fast(TickType_t now);
void chamber_outputs_apply_dehumidify(TickType_t now);
void chamber_outputs_apply_all_off(TickType_t now);

bool chamber_outputs_compressor_is_on(void);
TickType_t chamber_outputs_compressor_on_time(void);
TickType_t chamber_outputs_compressor_off_time(void);
