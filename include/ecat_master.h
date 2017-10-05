/*
 * ecat_master.h
 */

#ifndef _ECAT_MASTER_H
#define _ECAT_MASTER_H

#include <stdint.h>

#include <ethercat_wrapper.h>
#include <ethercat_wrapper_slave.h>
#include <readsdoconfig.h>

#ifdef __cplusplus
extern "C" {
#endif


struct _slave_config {
    int id;
    enum eSlaveType type;
    void *input;
    void *output;
};

typedef struct {
    uint16_t statusword;
    int8_t op_mode_display;
    int32_t position_value;
    int32_t velocity_value;
    int16_t torque_value;
    int32_t secondary_position_value;
    int32_t secondary_velocity_value;
    uint16_t analog_input1;
    uint16_t analog_input2;
    uint16_t analog_input3;
    uint16_t analog_input4;
    uint32_t tuning_status;
    uint8_t digital_input1;
    uint8_t digital_input2;
    uint8_t digital_input3;
    uint8_t digital_input4;
    uint32_t user_miso;
} PDOInput;

typedef struct {
    uint16_t controlword;
    int8_t op_mode;
    int16_t target_torque;
    int32_t target_position;
    int32_t target_velocity;
    int32_t offset_torque;
    uint32_t tuning_command;
    uint8_t digital_output1;
    uint8_t digital_output2;
    uint8_t digital_output3;
    uint8_t digital_output4;
    uint32_t user_mosi;
} PDOOutput;


#define PDO_INDEX_STATUSWORD                  0
#define PDO_INDEX_OPMODEDISP                  1
#define PDO_INDEX_POSITION_VALUE              2
#define PDO_INDEX_VELOCITY_VALUE              3
#define PDO_INDEX_TORQUE_VALUE                4
#define PDO_INDEX_SECONDARY_POSITION_VALUE    5
#define PDO_INDEX_SECONDARY_VELOCITY_VALUE    6
#define PDO_INDEX_ANALOG_INPUT1               7
#define PDO_INDEX_ANALOG_INPUT2               8
#define PDO_INDEX_ANALOG_INPUT3               9
#define PDO_INDEX_ANALOG_INPUT4              10
#define PDO_INDEX_TUNING_STATUS              11
#define PDO_INDEX_DIGITAL_INPUT1             12
#define PDO_INDEX_DIGITAL_INPUT2             14
#define PDO_INDEX_DIGITAL_INPUT3             16
#define PDO_INDEX_DIGITAL_INPUT4             18
#define PDO_INDEX_USER_MISO                  20

/* Index of sending (out) PDOs */
#define PDO_INDEX_CONTROLWORD                 0
#define PDO_INDEX_OPMODE                      1
#define PDO_INDEX_TORQUE_REQUEST              2
#define PDO_INDEX_POSITION_REQUEST            3
#define PDO_INDEX_VELOCITY_REQUEST            4
#define PDO_INDEX_OFFSET_TORQUE               5
#define PDO_INDEX_TUNING_COMMAND              6
#define PDO_INDEX_DIGITAL_OUTPUT1             7
#define PDO_INDEX_DIGITAL_OUTPUT2             9
#define PDO_INDEX_DIGITAL_OUTPUT3            11
#define PDO_INDEX_DIGITAL_OUTPUT4            13
#define PDO_INDEX_USER_MOSI                  15


/*
 * Indexes of SDO elements
 */
#define DICT_FEEDBACK_SENSOR_PORTS                    0x2100
#define SUB_ENCODER_FUNCTION                               2
#define SUB_ENCODER_RESOLUTION                             3


/*
 * Access functions for SLAVE_TYPE_CIA402_DRIVE
 * return error if slave is of the wrong type!
 */
uint32_t pd_get_statusword(Ethercat_Master_t *master, int slaveid);
uint32_t pd_get_opmodedisplay(Ethercat_Master_t *master, int slaveid);
uint32_t pd_get_position(Ethercat_Master_t *master, int slaveid);
uint32_t pd_get_velocity(Ethercat_Master_t *master, int slaveid);
uint32_t pd_get_torque(Ethercat_Master_t *master, int slaveid);
void pd_get(Ethercat_Master_t *master, int slaveid, PDOInput *pdo_input);

int pd_set_controlword(Ethercat_Master_t *master, int slaveid, uint32_t controlword);
int pd_set_opmode(Ethercat_Master_t *master, int slaveid, uint32_t opmode);
int pd_set_position(Ethercat_Master_t *master, int slaveid, uint32_t position);
int pd_set_velocity(Ethercat_Master_t *master, int slaveid, uint32_t velocity);
int pd_set_torque(Ethercat_Master_t *master, int slaveid, uint32_t torque);
void pd_set(Ethercat_Master_t *master, int slaveid, PDOOutput pdo_output);



int pdo_handler(Ethercat_Master_t *master, PDOInput *pdo_input, PDOOutput *pdo_output, int slaveid);


/**
 * @brief configuration object for SDO download
 */
struct _ecat_sdo_config {
    uint16_t index;
    uint8_t  subindex;
    uint32_t value;
    int bytesize;
};

/**
 * @brief Write specific SDO to slave device
 *
 * @param slave  pointer to the slave device
 * @param *config pointer to SDO configuration object
 * @return 0 on success, != 0 otherwise
 */
int write_sdo(Ethercat_Slave_t *slave, SdoParam_t *conf);

/**
 * @brief Write list of configuration SDO objects to slave device
 *
 * @param master  pointer to the master device
 * @param slave   slave number
 * @param *config pointer to list of configuration objects
 * @param max_objects number of objects to transfer
 * @return 0 on success, != 0 otherwise
 */
int write_sdo_config(Ethercat_Master_t *master, int slave_number, SdoParam_t *config, size_t max_objects);


/**
 * @brief Read a sdo object from a slave
 *
 * @param master  pointer to the master device
 * @param slave_number   slave number
 * @param index of the object
 * @param subindex of the object
 *
 * @return value of the object, -1 or if not found
 */
int read_sdo(Ethercat_Master_t *master, int slave_number, int index, int subindex);

#ifdef __cplusplus
}
#endif

#endif /* _ECAT_MASTER_H */
