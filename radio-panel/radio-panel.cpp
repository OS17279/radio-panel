/*
 * Flight Simulator Radio Panel
 * Copyright (c) 2021 Scott Vincent
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include "gpioctrl.h"
#include "globals.h"
#include "settings.h"
#include "simvars.h"
#include "radio.h"

const char* radioVersion = "v1.2.2";
const bool Debug = false;

struct globalVars globals;

radio* rad;

/// <summary>
/// Initialise
/// </summary>
void init(const char *settingsFile = NULL)
{
    // Init wiringPi ourselves as it is needed by both
    // gpioCtrl and sevenSegment.
    wiringPiSetupGpio();

    globals.allSettings = new settings(settingsFile);
    globals.simVars = new simvars();
    globals.gpioCtrl = new gpioctrl(false);
}

/// <summary>
/// Fetch latest values of common variables
/// </summary>
void updateCommon()
{
    SimVars* simVars = &globals.simVars->simVars;

    // Electrics check
    if (globals.aircraft == FBW_A320NEO) {
        // Radio only comes on if both batteries on or
        // have external power, APU or main engines running.
        globals.electrics = globals.connected && (simVars->dcVolts > 25.4 ||
            (simVars->elecBat1 > 0 && simVars->elecBat2 > 0));
    }
    else {
        globals.electrics = globals.connected && simVars->dcVolts > 0;
    }

    // Avionics check
    globals.avionics = globals.connected && (simVars->com1Status == 0 || simVars->com2Status == 0);
}

/// <summary>
/// Update everything before the next frame
/// </summary>
void doUpdate()
{
    updateCommon();

    rad->update();
}

///
/// main
///
int main(int argc, char **argv)
{
    printf("radio-panel %s\n", radioVersion);
    fflush(stdout);

    if (argc > 1) {
        init(argv[1]);
    }
    else {
        init();
    }

    rad = new radio();

    while (!globals.quit) {
        doUpdate();
        rad->render();

        // Update 10 times per second
        usleep(100000);
    }

    return 0;
}
