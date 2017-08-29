/**
 * @file profile.c
 * @brief Profile Generation for Position
 * Implements position profile based on Linear Function with
 * Parabolic Blends.
 * @author Synapticon GmbH <support@synapticon.com>
*/

#include "profile.h"

float rpm_to_ticks(int rpm, int ticks_per_turn) {
    return ((float)rpm * (float)ticks_per_turn)/(float)60;
}

float ticks_to_rpm(float ticks, int ticks_per_turn) {
    return (ticks*(float)60)/(float)ticks_per_turn;
}

void init_position_profile_limits(motion_profile_t *motion_profile, int max_torque, int max_torque_acceleration, int max_acceleration, int max_velocity, int max_position, int min_position, int ticks_per_turn)
{

    motion_profile->max_position =  (float)max_position;
    motion_profile->min_position = (float)min_position;
	motion_profile->max_acceleration =  rpm_to_ticks(max_acceleration, ticks_per_turn);
	motion_profile->max_velocity = rpm_to_ticks(max_velocity, ticks_per_turn);
    motion_profile->max_torque = max_torque;
    motion_profile->max_torque_acceleration = max_torque_acceleration;
    motion_profile->limit_factor = 10;
}

int init_position_profile(motion_profile_t *motion_profile, int target_position, int actual_position, int velocity, int acceleration, int deceleration, int ticks_per_turn) {

    motion_profile->qf = (float) target_position;
    motion_profile->qi = (float) actual_position;

    if (motion_profile->qf > motion_profile->max_position) {
        motion_profile->qf = motion_profile->max_position;
    } else if (motion_profile->qf < motion_profile->min_position) {
        motion_profile->qf = motion_profile->min_position;
    }

	motion_profile->vi = rpm_to_ticks(velocity, ticks_per_turn);
	motion_profile->acc =  rpm_to_ticks(acceleration, ticks_per_turn);
	motion_profile->dec =  rpm_to_ticks(deceleration, ticks_per_turn);

    if (motion_profile->vi > motion_profile->max_velocity) {
        motion_profile->vi = motion_profile->max_velocity;
    }

    // Internal parameters
    motion_profile->acc_too_low = 0;

    motion_profile->qid = 0.0f;

    // leads to shorter blend times in the beginning (if initialization condition != 0) non zero case - not yet considered
    motion_profile->qfd = 0.0f;

    // compute distance
    motion_profile->total_distance = motion_profile->qf - motion_profile->qi;

    // There is no point in calculating with these values
    if (motion_profile->total_distance == 0 || motion_profile->vi == 0) {
        return 0;
    }

    motion_profile->direction = 1;

    if (motion_profile->total_distance < 0) {
        motion_profile->total_distance = -motion_profile->total_distance;
        motion_profile->direction = -1;
    }

    if (motion_profile->acc > motion_profile->limit_factor * motion_profile->total_distance) {
        motion_profile->acc = motion_profile->limit_factor * motion_profile->total_distance;
    }

    if (motion_profile->acc > motion_profile->max_acceleration) {
        motion_profile->acc = motion_profile->max_acceleration;
    }

    if (motion_profile->dec > motion_profile->limit_factor * motion_profile->total_distance) {
        motion_profile->dec = motion_profile->limit_factor * motion_profile->total_distance;
    }

    if (motion_profile->dec > motion_profile->max_acceleration) {
        motion_profile->dec = motion_profile->max_acceleration;
    }

    motion_profile->tb_acc = motion_profile->vi / motion_profile->acc;

    motion_profile->tb_dec = motion_profile->vi / motion_profile->dec;

    motion_profile->distance_acc = (motion_profile->acc * motion_profile->tb_acc
                                       * motion_profile->tb_acc) / 2.0f;

    motion_profile->distance_dec = (motion_profile->dec * motion_profile->tb_dec
                                       * motion_profile->tb_dec) / 2.0f;

    motion_profile->distance_left = (motion_profile->total_distance -
                                        motion_profile->distance_acc - motion_profile->distance_dec);


    // check velocity and distance constraint

    if (motion_profile->distance_left < 0) {
        motion_profile->acc_too_low = 1;

        // acc too low to meet distance/vel constraint

        if (motion_profile->vi > motion_profile->total_distance) {
            motion_profile->vi = motion_profile->total_distance;

            motion_profile->acc_min = motion_profile->vi;

            if (motion_profile->acc < motion_profile->acc_min) {
                motion_profile->acc = motion_profile->acc_min;
            }

            if (motion_profile->dec < motion_profile->acc_min) {
                motion_profile->dec = motion_profile->acc_min;
            }
        } else if (motion_profile->vi < motion_profile->total_distance) {
            motion_profile->acc_min = motion_profile->vi;

            if (motion_profile->acc < motion_profile->acc_min) {
                motion_profile->acc = motion_profile->acc_min;
            }

            if (motion_profile->dec < motion_profile->acc_min) {
                motion_profile->dec = motion_profile->acc_min;
            }
        }

        motion_profile->tb_acc = motion_profile->vi / motion_profile->acc;

        motion_profile->tb_dec = motion_profile->vi / motion_profile->dec;

        motion_profile->distance_acc = (motion_profile->acc * motion_profile->tb_acc *
                                           motion_profile->tb_acc)/2.0f;

        motion_profile->distance_dec = (motion_profile->dec * motion_profile->tb_dec *
                                           motion_profile->tb_dec)/2.0f;

        motion_profile->distance_left = (motion_profile->total_distance -
                                            motion_profile->distance_acc - motion_profile->distance_dec);

    } else if (motion_profile->distance_left > 0) {
        motion_profile->acc_too_low = 0;
    }

    // check velocity and min acceleration constraint
    if (motion_profile->distance_left < 0) {
        motion_profile->acc_too_low = 1;

        // acc too low to meet distance/velocity constraint

        motion_profile->acc_min = motion_profile->vi;

        if (motion_profile->acc < motion_profile->acc_min) {
            motion_profile->acc = motion_profile->acc_min;
        }

        if (motion_profile->dec < motion_profile->acc_min) {
            motion_profile->dec = motion_profile->acc_min;
        }

        motion_profile->tb_acc = motion_profile->vi / motion_profile->acc;

        motion_profile->tb_dec = motion_profile->vi / motion_profile->dec;

        motion_profile->distance_acc = (motion_profile->acc * motion_profile->tb_acc
                                           * motion_profile->tb_acc)/2.0f;

        motion_profile->distance_dec = (motion_profile->dec * motion_profile->tb_dec
                                           * motion_profile->tb_dec)/2.0f;

        motion_profile->distance_left = (motion_profile->total_distance -
                                            motion_profile->distance_acc - motion_profile->distance_dec);
    } else if (motion_profile->distance_left > 0) {
        motion_profile->acc_too_low = 0;
    }

    motion_profile->distance_cruise = motion_profile->distance_left;

    motion_profile->t_cruise = (motion_profile->distance_cruise) / motion_profile->vi;

    motion_profile->tf = (motion_profile->tb_acc +
                             motion_profile->tb_dec + motion_profile->t_cruise);

    if (motion_profile->direction == -1) {
        motion_profile->vi = -motion_profile->vi;
    }

    // compute LFPB motion constants

    motion_profile->ai = motion_profile->qi;

    motion_profile->bi = motion_profile->qid;

    motion_profile->ci = (motion_profile->vi - motion_profile->qid) / (2.0f * motion_profile->tb_acc);

    motion_profile->di = (motion_profile->ai + motion_profile->tb_acc * motion_profile->bi +
                             motion_profile->ci * motion_profile->tb_acc * motion_profile->tb_acc -
                             motion_profile->vi * motion_profile->tb_acc);

    motion_profile->ei = motion_profile->qf;

    motion_profile->fi = motion_profile->qfd;

    motion_profile->gi = (motion_profile->di + (motion_profile->tf - motion_profile->tb_dec) *
                             motion_profile->vi + motion_profile->fi * motion_profile->tb_dec -
                             motion_profile->ei) / (motion_profile->tb_dec * motion_profile->tb_dec);

    motion_profile->T = motion_profile->tf / 0.01f;  // 1 ms

    motion_profile->s_time = 0.01f;                     // 1 ms

    motion_profile->steps = (int) round(motion_profile->T);

    return motion_profile->steps;
}

int position_profile_generate(motion_profile_t *motion_profile, int step) {

    // If the actual and the target positions are already equal,
    // just return one of them as a target position
    if (motion_profile->qi == motion_profile->qf) {
        return motion_profile->qf;
    }

    motion_profile->ts = motion_profile->s_time * step ;

    if (motion_profile->ts < motion_profile->tb_acc) {

        motion_profile->q = (motion_profile->ai + motion_profile->ts * motion_profile->bi +
                                motion_profile->ci * motion_profile->ts * motion_profile->ts);

    } else if ( (motion_profile->tb_acc <= motion_profile->ts) &&
                (motion_profile->ts < (motion_profile->tf - motion_profile->tb_dec)) ) {

        motion_profile->q = motion_profile->di + motion_profile->vi * motion_profile->ts;

    } else if ( ((motion_profile->tf - motion_profile->tb_dec) <= motion_profile->ts) &&
                (motion_profile->ts <= motion_profile->tf) ) {

        motion_profile->q = (motion_profile->ei + (motion_profile->ts - motion_profile->tf) *
                                motion_profile->fi + (motion_profile->ts - motion_profile->tf) *
                                (motion_profile->ts - motion_profile->tf) * motion_profile->gi);
    }

    return (int) round(motion_profile->q);
}





/*
 * Velocity profile
 *
 */

int init_velocity_profile(motion_profile_t *motion_profile, int target_velocity, int actual_velocity, int acceleration, int deceleration, int ticks_per_turn)
{
    motion_profile->qid = (float) actual_velocity;
    motion_profile->qfd = (float) target_velocity;

    //limit velocity
    if(motion_profile->qfd > ticks_to_rpm(motion_profile->max_velocity, ticks_per_turn)) {
        motion_profile->qfd = ticks_to_rpm(motion_profile->max_velocity, ticks_per_turn);
    } else if(motion_profile->qfd < -ticks_to_rpm(motion_profile->max_velocity, ticks_per_turn)) {
        motion_profile->qfd = -ticks_to_rpm(motion_profile->max_velocity, ticks_per_turn);
    }

    //set acceleration
    if (motion_profile->qfd >= motion_profile->qid) {
        motion_profile->acc = (float) acceleration;
    } else {
        motion_profile->acc = -(float) deceleration;
    }

    //limit acceleration
    if (motion_profile->acc > ticks_to_rpm(motion_profile->max_acceleration, ticks_per_turn)) {
        motion_profile->acc = ticks_to_rpm(motion_profile->max_acceleration, ticks_per_turn);
    } else if (motion_profile->acc < -ticks_to_rpm(motion_profile->max_acceleration, ticks_per_turn)) {
        motion_profile->acc = -ticks_to_rpm(motion_profile->max_acceleration, ticks_per_turn);
    }

    return init_linear_profile(motion_profile);
}


int init_torque_profile(motion_profile_t *motion_profile, int target_torque, int actual_torque, int acceleration, int deceleration)
{
    motion_profile->qid = (float) actual_torque;

    //limit torque
    if(target_torque > motion_profile->max_torque) {
        target_torque = motion_profile->max_torque;
    } else if(target_torque < -motion_profile->max_torque) {
        target_torque = -motion_profile->max_torque;
    }
    motion_profile->qfd = (float) target_torque;


    //set acceleration
    if (motion_profile->qfd >= motion_profile->qid) {
        motion_profile->acc = (float) acceleration;
    } else {
        motion_profile->acc = -(float) deceleration;
    }

    //limit acceleration
    if (motion_profile->acc > motion_profile->max_torque_acceleration) {
        motion_profile->acc = motion_profile->max_torque_acceleration;
    } else if (motion_profile->acc < -motion_profile->max_torque_acceleration) {
        motion_profile->acc = -motion_profile->max_torque_acceleration;
    }

    return init_linear_profile(motion_profile);
}

int init_linear_profile(motion_profile_t *motion_profile)
{
    motion_profile->s_time = .001f;

    if (motion_profile->qfd == motion_profile->qid) {
        return 0;
    }

    // compute time needed (in seconds)
    float total_time = (motion_profile->qfd - motion_profile->qid)/motion_profile->acc;

    //number of steps needed (1 step every 1ms)
    float steps = total_time/motion_profile->s_time;
    motion_profile->s_time = total_time/steps;

    return (int)round(steps);
}

int linear_profile_generate_in_steps(motion_profile_t *motion_profile, int step)
{
    return (int) round( motion_profile->qid + motion_profile->acc * motion_profile->s_time * step);
}
