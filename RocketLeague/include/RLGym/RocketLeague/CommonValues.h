#pragma once

#include <RLGym/RocketLeague/Framework.h>

#include <array>
#include <Eigen/Core>

START_RL_NS(CommonValues)
    //==========================================================================
    // Field dimensions
    //==========================================================================

    constexpr float SIDE_WALL_X = 4096.f;
    constexpr float BACK_WALL_Y = 5120.f;
    constexpr float CEILING_Z = 2044.f;
    constexpr float BACK_NET_Y = 6000.f;
    constexpr float FIELD_BOUNDING_BOX_SIZE = 8064.f;
    constexpr float CORNER_CATHETUS_LENGTH = 1152.f;
    constexpr float RAMP_HEIGHT = 256.f;

    //==========================================================================
    // Goal dimensions
    //==========================================================================

    constexpr float GOAL_HEIGHT = 642.775f;
    constexpr float GOAL_CENTER_TO_POST = 892.755f;

    //==========================================================================
    // Goal locations
    //==========================================================================

    inline const Eigen::Vector3f ORANGE_GOAL_CENTER{ 0.f, BACK_WALL_Y, GOAL_HEIGHT * 0.5f };
    inline const Eigen::Vector3f BLUE_GOAL_CENTER{ 0.f, -BACK_WALL_Y, GOAL_HEIGHT * 0.5f };

    inline const Eigen::Vector3f ORANGE_GOAL_BACK{ 0.f, BACK_NET_Y, GOAL_HEIGHT * 0.5f };
    inline const Eigen::Vector3f BLUE_GOAL_BACK{ 0.f, -BACK_NET_Y, GOAL_HEIGHT * 0.5f };

    inline const Eigen::Vector3f ORANGE_GOAL_TOP_LEFT{ GOAL_CENTER_TO_POST,  BACK_WALL_Y, GOAL_HEIGHT };
    inline const Eigen::Vector3f BLUE_GOAL_TOP_LEFT{ -GOAL_CENTER_TO_POST, -BACK_WALL_Y, GOAL_HEIGHT };

    inline const Eigen::Vector3f ORANGE_GOAL_TOP_RIGHT{ -GOAL_CENTER_TO_POST,  BACK_WALL_Y, GOAL_HEIGHT };
    inline const Eigen::Vector3f BLUE_GOAL_TOP_RIGHT{ GOAL_CENTER_TO_POST, -BACK_WALL_Y, GOAL_HEIGHT };

    inline const Eigen::Vector3f ORANGE_GOAL_BOTTOM_LEFT{ GOAL_CENTER_TO_POST,  BACK_WALL_Y, 0.f };
    inline const Eigen::Vector3f BLUE_GOAL_BOTTOM_LEFT{ -GOAL_CENTER_TO_POST, -BACK_WALL_Y, 0.f };

    inline const Eigen::Vector3f ORANGE_GOAL_BOTTOM_RIGHT{ -GOAL_CENTER_TO_POST,  BACK_WALL_Y, 0.f };
    inline const Eigen::Vector3f BLUE_GOAL_BOTTOM_RIGHT{ GOAL_CENTER_TO_POST, -BACK_WALL_Y, 0.f };

    inline const Eigen::Vector3f ORANGE_FIELD_TOP_LEFT{ SIDE_WALL_X,  BACK_WALL_Y, CEILING_Z };
    inline const Eigen::Vector3f BLUE_FIELD_TOP_LEFT{ -SIDE_WALL_X, -BACK_WALL_Y, CEILING_Z };

    inline const Eigen::Vector3f ORANGE_FIELD_TOP_RIGHT{ -SIDE_WALL_X,  BACK_WALL_Y, CEILING_Z };
    inline const Eigen::Vector3f BLUE_FIELD_TOP_RIGHT{ SIDE_WALL_X, -BACK_WALL_Y, CEILING_Z };

    inline const Eigen::Vector3f ORANGE_FIELD_BOTTOM_LEFT{ SIDE_WALL_X,  BACK_WALL_Y, 0.f };
    inline const Eigen::Vector3f BLUE_FIELD_BOTTOM_LEFT{ -SIDE_WALL_X, -BACK_WALL_Y, 0.f };

    inline const Eigen::Vector3f ORANGE_FIELD_BOTTOM_RIGHT{ -SIDE_WALL_X,  BACK_WALL_Y, 0.f };
    inline const Eigen::Vector3f BLUE_FIELD_BOTTOM_RIGHT{ SIDE_WALL_X, -BACK_WALL_Y, 0.f };

    constexpr float GOAL_THRESHOLD = 5215.5f;

    //==========================================================================
    // Time
    //==========================================================================

    constexpr int TICKS_PER_SECOND = 120;
    constexpr float SMALL_PAD_RECHARGE_SECONDS = 4.f;
    constexpr float BIG_PAD_RECHARGE_SECONDS = 10.f;
    constexpr float DEMO_RESPAWN_SECONDS = 3.f;
    constexpr float BOOST_CONSUMPTION_RATE = 33.3f;

    //==========================================================================
    // Physics
    //==========================================================================

    constexpr float BOOST_ACCELERATION = 991.666f;
    constexpr float GRAVITY = 650.f;

    //==========================================================================
    // Sizes
    //==========================================================================

    constexpr float BALL_RADIUS = 91.25f;
    constexpr float BALL_RESTING_HEIGHT = 93.15f;
    constexpr float UNREAL_UNITS_PER_METER = 100.f;

    //==========================================================================
    // Masses
    //==========================================================================

    constexpr float CAR_MASS = 180.f;
    constexpr float BALL_MASS = 30.f;

    //==========================================================================
    // Speed limits
    //==========================================================================

    constexpr float BALL_MAX_SPEED = 6000.f;
    constexpr float CAR_MAX_SPEED = 2300.f;
    constexpr float SUPERSONIC_THRESHOLD = 2200.f;
    constexpr float CAR_MAX_ANG_VEL = 5.5f;

    //==========================================================================
    // Teams
    //==========================================================================

    enum Team
    {
        BLUE_TEAM = 0,
        ORANGE_TEAM = 1
    };

    //==========================================================================
    // Hitboxes
    //==========================================================================

    enum Hitbox
    {
        OCTANE = 0,
        DOMINUS,
        PLANK,
        BREAKOUT,
        HYBRID,
        MERC
    };

    //==========================================================================
    // Actions
    //==========================================================================

    constexpr int NUM_ACTIONS = 8;

    enum Action
    {
        THROTTLE = 0,
        STEER,
        PITCH,
        YAW,
        ROLL,
        JUMP,
        BOOST,
        HANDBRAKE
    };

    constexpr float DOUBLEJUMP_MAX_DELAY = 1.25f;
    constexpr float FLIP_TORQUE_TIME = 0.65f;
    constexpr float JUMP_MAX_TIME = 0.2f;

    inline const std::array<Eigen::Vector3f, 34> BOOST_LOCATIONS{ {
        {    0.f, -4240.f, 70.f},
        {-1792.f, -4184.f, 70.f},
        { 1792.f, -4184.f, 70.f},
        {-3072.f, -4096.f, 73.f},
        { 3072.f, -4096.f, 73.f},
        { -940.f, -3308.f, 70.f},
        {  940.f, -3308.f, 70.f},
        {    0.f, -2816.f, 70.f},
        {-3584.f, -2484.f, 70.f},
        { 3584.f, -2484.f, 70.f},
        {-1788.f, -2300.f, 70.f},
        { 1788.f, -2300.f, 70.f},
        {-2048.f, -1036.f, 70.f},
        {    0.f, -1024.f, 70.f},
        { 2048.f, -1036.f, 70.f},
        {-3584.f,     0.f, 73.f},
        {-1024.f,     0.f, 70.f},
        { 1024.f,     0.f, 70.f},
        { 3584.f,     0.f, 73.f},
        {-2048.f,  1036.f, 70.f},
        {    0.f,  1024.f, 70.f},
        { 2048.f,  1036.f, 70.f},
        {-1788.f,  2300.f, 70.f},
        { 1788.f,  2300.f, 70.f},
        {-3584.f,  2484.f, 70.f},
        { 3584.f,  2484.f, 70.f},
        {    0.f,  2816.f, 70.f},
        { -940.f,  3310.f, 70.f},
        {  940.f,  3308.f, 70.f},
        {-3072.f,  4096.f, 73.f},
        { 3072.f,  4096.f, 73.f},
        {-1792.f,  4184.f, 70.f},
        { 1792.f,  4184.f, 70.f},
        {    0.f,  4240.f, 70.f}
    } };

END_RL_NS