#include "role.h"
#include "shared.h"

char const * PROJECT_TAG = (
    #ifndef ROLE
        I am using syntax error to denote undefined macro here!
    #endif
    #if ROLE == 'F'
        "Music_X_Flute"
    #elif ROLE == 'G'
        #ifndef ROLE_GLOVE_WHICH
            I am using syntax error to denote undefined macro here!
        #endif
        #if ROLE_GLOVE_WHICH == 'L'
            "Music_X_GlovL"
        #elif ROLE_GLOVE_WHICH == 'R'
            "Music_X_GlovR"
        #endif
    #endif
);

inline void delayTaskMs(int ms) {
    // not precise. Depends on when's the next tick. 
    assert(ms >= portTICK_PERIOD_MS);
    vTaskDelay(pdMS_TO_TICKS(ms));
}
