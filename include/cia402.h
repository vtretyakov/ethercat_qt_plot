/*
 * cia402.h
 *
 *
 * 2016-06-16, synapticon
 */

#ifndef _CIA402_H
#define _CIA402_H

#include <stdint.h>

#define OPMODE_NONE                  0
#define OPMODE_CSP                   8
#define OPMODE_CSV                   9
#define OPMODE_CST                   10

#define STATUS_WORD_MASQ_A           0x6f
#define STATUS_WORD_MASQ_B           0x4f

#define STATUS_NOT_READY             0x00   /* masq B */
#define STATUS_SWITCH_ON_DISABLED    0x40   /* masq B */
#define STATUS_READY_SWITCH_ON       0x21
#define STATUS_SWITCHED_ON           0x23
#define STATUS_OP_ENABLED            0x27
#define STATUS_QUICK_STOP            0x07
#define STATUS_FAULT_REACTION_ACTIVE 0x0f   /* masq B */
#define STATUS_FAULT                 0x08   /* masq B */

#define CONTROL_BIT_ENABLE_OP        0x08
#define CONTROL_BIT_QUICK_STOP       0x04
#define CONTROL_BIT_ENABLE_VOLTAGE   0x02
#define CONTROL_BIT_SWITCH_ON        0x01
#define CONTROL_BIT_FAULT_RESET      0x80

typedef enum {
     CIASTATE_NOT_READY = 0
    ,CIASTATE_SWITCH_ON_DISABLED
    ,CIASTATE_READY_SWITCH_ON
    ,CIASTATE_SWITCHED_ON
    ,CIASTATE_OP_ENABLED
    ,CIASTATE_QUICK_STOP
    ,CIASTATE_FAULT_REACTION_ACTIVE
    ,CIASTATE_FAULT
} CIA402State;

typedef enum {
    CIA402_CMD_NONE = 0,
    CIA402_CMD_SHUTDOWN,
    CIA402_CMD_SWITCH_ON,
    CIA402_CMD_DISABLE_VOLTAGE,
    CIA402_CMD_QUICK_STOP,
    CIA402_CMD_DISABLE_OPERATION,
    CIA402_CMD_ENABLE_OPERATION,
    CIA402_CMD_FAULT_RESET,
} CIA402Command;

#define CIA402_ERROR_CODE_DC_LINK_OVER_VOLTAGE             0x3210
#define CIA402_ERROR_CODE_DC_LINK_UNDER_VOLTAGE            0x3220

#define CIA402_ERROR_CODE_PHASE_FAILURE_L1                 0x3131
#define CIA402_ERROR_CODE_PHASE_FAILURE_L2                 0x3132
#define CIA402_ERROR_CODE_PHASE_FAILURE_L3                 0x3133

#define CIA402_ERROR_CODE_EXCESS_TEMPERATURE_DEVICE        0x4210

#define CIA402_ERROR_CODE_SENSOR                           0x7300
#define CIA402_ERROR_CODE_MOTOR_COMMUTATION                0x7122

#define CIA402_ERROR_CODE_MOTOR_BLOCKED                    0x7121

/* for all error in this control which could not further specified */
#define CIA402_ERROR_CODE_CONTROL                          0x8A00

#define CIA402_ERROR_CODE_COMMUNICATION                    0x7500


CIA402State cia402_read_state(uint16_t statusword);

uint16_t cia402_command(CIA402Command command, uint16_t controlword);

uint16_t cia402_go_to_state(CIA402State target_state, CIA402State current_state , uint16_t controlword, int skip_state);


#endif /* _CIA402_H */
