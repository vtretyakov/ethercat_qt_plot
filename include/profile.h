/**
 * @file profile.h
 * @brief Profile Generation for Position, Velocity and Torque
 *      Implements position profile based on Linear Function with
 *      Parabolic Blends, velocity profile and torque profiles are
 *      based on linear functions.
 * @author Synapticon GmbH <support@synapticon.com>
*/

#include <stdio.h>
#include <math.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct motion_profile_t {
    float max_acceleration;   // max acceleration
    float max_velocity;
    int max_torque; //mNm
    int max_torque_acceleration; // mNm/s

    /*User Inputs*/

    float acc;                // acceleration
    float dec;                // deceleration
    float vi;                 // velocity
    float qi;                 // initial position
    float qf;                 // final position

    /*Profile time*/

    float T;                  // total no. of Samples
    float s_time;             // sampling time

    int direction;
    int acc_too_low;          // flag for low acceleration constraint
    float acc_min;            // constraint minimum acceleration
    float limit_factor;       // max acceleration constraint

    /*LFPB motion profile constants*/

    float ai;
    float bi;
    float ci;
    float di;
    float ei;
    float fi;
    float gi;

    /*internal velocity variables*/

    float qid;                // initial velocity
    float qfd;                // final velocity

    float distance_cruise;
    float total_distance;
    float distance_acc;       // distance covered during acceleration
    float distance_dec;       // distance covered during deceleration
    float distance_left;      // distance left for cruise velocity

    float tb_acc;             // blend time for acceleration profile
    float tb_dec;             // blend time for deceleration profile
    float tf;                 // total time for profile
    float t_cruise;           // time for cruise velocity profile
    float ts;                 // variable to hold current sample time

    float q;                  // position profile

    float max_position;
    float min_position;

    int steps;                 // number of steps required to execute this profile

} motion_profile_t;

/*Profile Position Mode*/

/**
 * @brief Initialize Position Profile Limits
 *
 * @param motion_profile
 * @param max_acceleration for the position profile
 * @param max_velocity for the position profile
 * @param max_position
 * @param min_position
 * @param ticks_per_turn number of ticks in one turn
 *
 */
void init_position_profile_limits(motion_profile_t *motion_profile, int max_torque, int max_torque_acceleration, int max_acceleration, int max_velocity, int max_position, int min_position, int ticks_per_turn);

/**
 * @brief Initialize Position Profile
 *
 * @param motion_profile
 * @param target_position
 * @param actual_position
 * @param velocity for the position profile
 * @param acceleration for the position profile
 * @param deceleration for the position profile
 * @param ticks_per_turn number of ticks in one turn
 *
 * @return no. of steps for position profile : range [1 - steps]
 */
int init_position_profile(motion_profile_t *motion_profile, int target_position, int actual_position, int velocity, int acceleration, int deceleration, int ticks_per_turn);

/**
 * @brief Generate Position Profile
 *
 * @param motion_profile
 * @param step current step of the profile
 *
 * @return corresponding target position at the step input
 */
int position_profile_generate(motion_profile_t *motion_profile, int step);


int init_velocity_profile(motion_profile_t *motion_profile, int target_velocity, int actual_velocity, int acceleration, int deceleration, int ticks_per_turn);

int linear_profile_generate_in_steps(motion_profile_t *motion_profile, int step);

int init_torque_profile(motion_profile_t *motion_profile, int target_torque, int actual_torque, int acceleration, int deceleration);

int init_linear_profile(motion_profile_t *motion_profile);

float ticks_to_rpm(float ticks, int ticks_per_turn);

#ifdef __cplusplus
}
#endif
