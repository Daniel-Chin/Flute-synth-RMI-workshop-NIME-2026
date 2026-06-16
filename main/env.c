#include "env.h"
#include "role.h"

#ifndef ROLE
    I am using syntax error to denote undefined macro here!
#endif
#if ROLE == 'G'
    #if ROLE_GLOVE_WHICH == 'L'
        int const SERVO_PINS[N_SERVOS] = {25, 26, 27};
    #endif
    #if ROLE_GLOVE_WHICH == 'R'
        int const SERVO_PINS[N_SERVOS] = {27, 26, 25};
    #endif
#endif
