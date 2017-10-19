#ifndef PROFILER_H
#define PROFILER_H


class Profiler
{
public:
    Profiler();
    ~Profiler();

private:
    struct MotionProfile {
        float max_acceleration = 0.0;   // max acceleration
        float max_velocity = 0.0;
        int max_torque = 0; //mNm
        int max_torque_acceleration = 0; // mNm/s

        /*User Inputs*/

        float acc = 0.0;                // acceleration
        float dec = 0.0;                // deceleration
        float vi = 0.0;                 // velocity
        float qi = 0.0;                 // initial position
        float qf = 0.0;                 // final position

        /*Profile time*/

        float T = 0.0;                  // total no. of Samples
        float s_time = 0.0;             // sampling time

        int direction = 0;
        int acc_too_low = 0;          // flag for low acceleration constraint
        float acc_min = 0.0;            // constraint minimum acceleration
        float limit_factor = 0.0;       // max acceleration constraint

        /*LFPB motion profile constants*/

        float ai = 0.0;
        float bi = 0.0;
        float ci = 0.0;
        float di = 0.0;
        float ei = 0.0;
        float fi = 0.0;
        float gi = 0.0;

        /*internal velocity variables*/

        float qid = 0.0;                // initial velocity
        float qfd = 0.0;                // final velocity

        float distance_cruise = 0.0;
        float total_distance = 0.0;
        float distance_acc = 0.0;       // distance covered during acceleration
        float distance_dec = 0.0;       // distance covered during deceleration
        float distance_left = 0.0;      // distance left for cruise velocity

        float tb_acc = 0.0;             // blend time for acceleration profile
        float tb_dec = 0.0;             // blend time for deceleration profile
        float tf = 0.0;                 // total time for profile
        float t_cruise = 0.0;           // time for cruise velocity profile
        float ts = 0.0;                 // variable to hold current sample time

        float q = 0.0;                  // position profile

        float max_position = 0.0;
        float min_position = 0.0;

        int steps = 0;                 // number of steps required to execute this profile

    };

public:
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
    void init_position_profile_limits(MotionProfile *motion_profile, int max_torque, int max_torque_acceleration, int max_acceleration, int max_velocity, int max_position, int min_position, int ticks_per_turn);

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
    int init_position_profile(MotionProfile *motion_profile, int target_position, int actual_position, int velocity, int acceleration, int deceleration, int ticks_per_turn);

    /**
     * @brief Generate Position Profile
     *
     * @param motion_profile
     * @param step current step of the profile
     *
     * @return corresponding target position at the step input
     */
    int position_profile_generate(MotionProfile *motion_profile, int step);


    int init_velocity_profile(MotionProfile *motion_profile, int target_velocity, int actual_velocity, int acceleration, int deceleration, int ticks_per_turn);

    int linear_profile_generate_in_steps(MotionProfile *motion_profile, int step);

    int init_torque_profile(MotionProfile *motion_profile, int target_torque, int actual_torque, int acceleration, int deceleration);

    int init_linear_profile(MotionProfile *motion_profile);

    float ticks_to_rpm(float ticks, int ticks_per_turn);

    float rpm_to_ticks(int rpm, int ticks_per_turn);

    MotionProfile *motion_profile = nullptr;


};

#endif // PROFILER_H
