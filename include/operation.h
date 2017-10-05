/*
 * operation.h
 *
 *  Created on: Nov 6, 2016
 *      Author: synapticon
 */

#ifndef OPERATION_H_
#define OPERATION_H_

#include <stdint.h>

#include "cia402.h"

typedef enum {
    NO_MODE,
    QUIT_MODE,
    CYCLIC_SYNCHRONOUS_MODE
} AppMode;

typedef struct {
    int mode_1;
    int mode_2;
    int mode_3;
    int value;
    int sign;
    int last_command;
    int last_value;
    int init;
    int init_display;
    int select;
    int fault_ack;
    int debug;
    int manual;
    CIA402State *target_state;
    AppMode app_mode;
} OutputValues;


#include "profile.h"

typedef struct {
    motion_profile_t motion_profile;
    int max_torque_acceleration;
    int max_torque;
    int max_acceleration;
    int max_speed;
    int profile_speed;
    int profile_acceleration;
    int profile_torque_acceleration;
    int max_position;
    int min_position;
    int target_position;
    int ticks_per_turn;
    int step;
    int steps;
} PositionProfileConfig;

typedef struct {
    int32_t target_position;
    int32_t position;
    int32_t velocity;
    int16_t torque;
} RecordData;

#include "ecat_master.h"
#include "display.h"

void target_generate(PositionProfileConfig *config, PDOOutput *pdo_output, PDOInput *pdo_input, int number_slaves);

void cs_command(WINDOW *wnd, Cursor *cursor, PDOOutput *pdo_output, PDOInput *pdo_input, size_t number_slaves, OutputValues *output, PositionProfileConfig *profiler_config);

void state_machine_control(PDOOutput *pdo_output, PDOInput *pdo_input, size_t number_slaves, OutputValues *output);

void cyclic_synchronous_mode(WINDOW *wnd, Cursor *cursor, PDOOutput *pdo_output, PDOInput *pdo_input, size_t number_slaves, OutputValues *output, PositionProfileConfig *profile_config);

int quit_mode(PDOOutput *pdo_output, PDOInput *pdo_input, size_t number_slaves);


#endif /* OPERATION_H_ */
