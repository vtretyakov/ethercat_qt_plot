/*
 * ecat_master.c
 */

#include "ecat_master.h"
#include <stdio.h>


/*
 * PDO access functions
 */

uint32_t pd_get_statusword(Ethercat_Master_t *master, int slaveid)
{
    Ethercat_Slave_t *slave = ecw_slave_get(master, slaveid);
    return (uint32_t)ecw_slave_get_in_value(slave, PDO_INDEX_STATUSWORD);
}

uint32_t pd_get_opmodedisplay(Ethercat_Master_t *master, int slaveid)
{
    Ethercat_Slave_t *slave = ecw_slave_get(master, slaveid);
    return (uint32_t)ecw_slave_get_in_value(slave, PDO_INDEX_OPMODEDISP);
}

uint32_t pd_get_position(Ethercat_Master_t *master, int slaveid)
{
    Ethercat_Slave_t *slave = ecw_slave_get(master, slaveid);
    return (uint32_t)ecw_slave_get_in_value(slave, PDO_INDEX_POSITION_VALUE);
}

uint32_t pd_get_velocity(Ethercat_Master_t *master, int slaveid)
{
    Ethercat_Slave_t *slave = ecw_slave_get(master, slaveid);
    return (uint32_t)ecw_slave_get_in_value(slave, PDO_INDEX_VELOCITY_VALUE);
}

uint32_t pd_get_torque(Ethercat_Master_t *master, int slaveid)
{
    Ethercat_Slave_t *slave = ecw_slave_get(master, slaveid);
    return (uint32_t)ecw_slave_get_in_value(slave, PDO_INDEX_TORQUE_VALUE);
}

int pd_set_controlword(Ethercat_Master_t *master, int slaveid, uint32_t controlword)
{
    Ethercat_Slave_t *slave = ecw_slave_get(master, slaveid);
    return ecw_slave_set_out_value(slave, PDO_INDEX_CONTROLWORD, controlword);
}

int pd_set_opmode(Ethercat_Master_t *master, int slaveid, uint32_t opmode)
{
    Ethercat_Slave_t *slave = ecw_slave_get(master, slaveid);
    return ecw_slave_set_out_value(slave, PDO_INDEX_OPMODE, opmode);
}

int pd_set_position(Ethercat_Master_t *master, int slaveid, uint32_t position)
{
    Ethercat_Slave_t *slave = ecw_slave_get(master, slaveid);
    return ecw_slave_set_out_value(slave, PDO_INDEX_POSITION_REQUEST, position);
}

int pd_set_torque(Ethercat_Master_t *master, int slaveid, uint32_t torque)
{
    Ethercat_Slave_t *slave = ecw_slave_get(master, slaveid);
    return ecw_slave_set_out_value(slave, PDO_INDEX_TORQUE_REQUEST, torque);
}

int pd_set_velocity(Ethercat_Master_t *master, int slaveid, uint32_t velocity)
{
    Ethercat_Slave_t *slave = ecw_slave_get(master, slaveid);
    return ecw_slave_set_out_value(slave, PDO_INDEX_VELOCITY_REQUEST, velocity);
}

void pd_get(Ethercat_Master_t *master, int slaveid, PDOInput *pdo_input)
{
    Ethercat_Slave_t *slave = ecw_slave_get(master, slaveid);

    pdo_input->statusword = (uint16_t)ecw_slave_get_in_value(slave, PDO_INDEX_STATUSWORD);
    pdo_input->op_mode_display = (int8_t)ecw_slave_get_in_value(slave, PDO_INDEX_OPMODEDISP);
    pdo_input->position_value = (int32_t)ecw_slave_get_in_value(slave, PDO_INDEX_POSITION_VALUE);
    pdo_input->velocity_value = (int32_t)ecw_slave_get_in_value(slave, PDO_INDEX_VELOCITY_VALUE);
    pdo_input->torque_value = (int16_t)ecw_slave_get_in_value(slave, PDO_INDEX_TORQUE_VALUE);
    pdo_input->secondary_position_value = (int32_t)ecw_slave_get_in_value(slave, PDO_INDEX_SECONDARY_POSITION_VALUE);
    pdo_input->secondary_velocity_value = (int32_t)ecw_slave_get_in_value(slave, PDO_INDEX_SECONDARY_VELOCITY_VALUE);
    pdo_input->analog_input1 = (uint16_t)ecw_slave_get_in_value(slave, PDO_INDEX_ANALOG_INPUT1);
    pdo_input->analog_input2 = (uint16_t)ecw_slave_get_in_value(slave, PDO_INDEX_ANALOG_INPUT2);
    pdo_input->analog_input3 = (uint16_t)ecw_slave_get_in_value(slave, PDO_INDEX_ANALOG_INPUT3);
    pdo_input->analog_input4 = (uint16_t)ecw_slave_get_in_value(slave, PDO_INDEX_ANALOG_INPUT4);
    pdo_input->tuning_status = (int32_t)ecw_slave_get_in_value(slave, PDO_INDEX_TUNING_STATUS);
    pdo_input->digital_input1 = (uint8_t)ecw_slave_get_in_value(slave, PDO_INDEX_DIGITAL_INPUT1);
    pdo_input->digital_input2 = (uint8_t)ecw_slave_get_in_value(slave, PDO_INDEX_DIGITAL_INPUT2);
    pdo_input->digital_input3 = (uint8_t)ecw_slave_get_in_value(slave, PDO_INDEX_DIGITAL_INPUT3);
    pdo_input->digital_input4 = (uint8_t)ecw_slave_get_in_value(slave, PDO_INDEX_DIGITAL_INPUT4);
    pdo_input->user_miso = (uint32_t)ecw_slave_get_in_value(slave, PDO_INDEX_USER_MISO);

    return;
}

void pd_set(Ethercat_Master_t *master, int slaveid, PDOOutput pdo_output)
{
    Ethercat_Slave_t *slave = ecw_slave_get(master, slaveid);

    ecw_slave_set_out_value(slave, PDO_INDEX_CONTROLWORD, pdo_output.controlword);
    ecw_slave_set_out_value(slave, PDO_INDEX_OPMODE, pdo_output.op_mode);
    ecw_slave_set_out_value(slave, PDO_INDEX_TORQUE_REQUEST, pdo_output.target_torque);
    ecw_slave_set_out_value(slave, PDO_INDEX_POSITION_REQUEST, pdo_output.target_position);
    ecw_slave_set_out_value(slave, PDO_INDEX_VELOCITY_REQUEST, pdo_output.target_velocity);
    ecw_slave_set_out_value(slave, PDO_INDEX_OFFSET_TORQUE, pdo_output.offset_torque);
    ecw_slave_set_out_value(slave, PDO_INDEX_TUNING_COMMAND, pdo_output.tuning_command);
    ecw_slave_set_out_value(slave, PDO_INDEX_DIGITAL_OUTPUT1, pdo_output.digital_output1);
    ecw_slave_set_out_value(slave, PDO_INDEX_DIGITAL_OUTPUT2, pdo_output.digital_output2);
    ecw_slave_set_out_value(slave, PDO_INDEX_DIGITAL_OUTPUT3, pdo_output.digital_output3);
    ecw_slave_set_out_value(slave, PDO_INDEX_DIGITAL_OUTPUT4, pdo_output.digital_output4);
    ecw_slave_set_out_value(slave, PDO_INDEX_USER_MOSI, pdo_output.user_mosi);

    return;
}


int pdo_handler(Ethercat_Master_t *master, PDOInput *pdo_input, PDOOutput *pdo_output, int slaveid)
{
    size_t number_of_slaves = ecw_master_slave_count(master);

    if (slaveid >= 0 && slaveid < number_of_slaves) {
        pd_get(master, slaveid, &pdo_input[slaveid]);
        pd_set(master, slaveid, pdo_output[slaveid]);
    } else {
        for (int i = 0; i < number_of_slaves; i++) {
            pd_get(master, i, &pdo_input[i]);
            pd_set(master, i, pdo_output[i]);
        }
    }

    return 0;
}

int write_sdo(Ethercat_Slave_t *slave, SdoParam_t *conf)
{
    int ret = ecw_slave_set_sdo_value(slave, conf->index, conf->subindex, conf->value);
    if (ret < 0) {
        fprintf(stderr, "Error Slave %d, could not download object 0x%04x:%d, value: %d\n",
                ecw_slave_get_slaveid(slave), conf->index, conf->subindex, conf->value);
        return -1;
    }

    return 0;
}


int write_sdo_config(Ethercat_Master_t *master, int slave_number, SdoParam_t *config, size_t max_objects)
{
    int ret = -1;
    Ethercat_Slave_t *slave = ecw_slave_get(master, slave_number);
    if (slave == NULL) {
        fprintf(stderr, "Error could not get slave with id %d\n", slave_number);
        return -1;
    }

    /* FIXME add bytesize from SDO entry info into parameter list */
    for (size_t i = 0; i < max_objects; i++) {
        ret = write_sdo(slave, config+i);

        if (ret != 0)
            break;
    }

    return ret;
}

int read_sdo(Ethercat_Master_t *master, int slave_number, int index, int subindex) {
    int sdo_value = 0;

    int ret = ecw_slave_get_sdo_value(ecw_slave_get(master, slave_number), index, subindex, &sdo_value);

    if (ret == 0) {
        return sdo_value;
    } else {
        fprintf(stderr, "Error Slave %d, could not read sdo object 0x%04x:%d, error code %d\n",
                slave_number, index, subindex, ret);
    }

    return -1;
}
