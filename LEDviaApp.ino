/*****************************************************************************
* Model: LEDviaApp.qm
* File:  ./LEDviaApp.ino
*
* This code has been generated by QM tool (see state-machine.com/qm).
* DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*****************************************************************************/
/*${.::LEDviaApp.ino} ......................................................*/
#include "qpn.h"                       // QP-nano framework
#include "Arduino.h"                   // Arduino API

#include <NeoPixels_SPI.h>             // from Nick Gammon
#include <SPI.h>                       // Pin 11 = MOSI

//============================================================================
// declare all AO classes...

#if ((QP_VERSION < 591) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8)))
#error qpn version 5.9.1 or higher required
#endif

/*${AOs::LEDviaApp} ........................................................*/
typedef struct LEDviaApp {
/* protected: */
    QActive super;

/* private: */
    uint8_t value;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t program;
    uint8_t led_index;
    uint8_t led_x;
    uint8_t run_fwd;
    uint8_t brightness;
    uint8_t dim_up;
    uint8_t rain_x;
} LEDviaApp;

/* protected: */
static QState LEDviaApp_initial(LEDviaApp * const me);
static QState LEDviaApp_branch(LEDviaApp * const me);
static QState LEDviaApp_display(LEDviaApp * const me);
static QState LEDviaApp_running_fwd(LEDviaApp * const me);
static QState LEDviaApp_running_bwd(LEDviaApp * const me);
static QState LEDviaApp_dimming_up(LEDviaApp * const me);
static QState LEDviaApp_dimming_down(LEDviaApp * const me);
static QState LEDviaApp_rainbow(LEDviaApp * const me);
static QState LEDviaApp_communication(LEDviaApp * const me);
static QState LEDviaApp_process_data(LEDviaApp * const me);

//...

// AO instances and event queue buffers for them...
LEDviaApp AO_LEDviaApp;
static QEvt l_ledviaappQSto[10]; // Event queue storage for LEDviaApp
//...

//============================================================================
// QF_active[] array defines all active object control blocks ----------------
QActiveCB const Q_ROM QF_active[] = {
    { (QActive *)0,             (QEvt *)0,       0U                     },
    { (QActive *)&AO_LEDviaApp, l_ledviaappQSto, Q_DIM(l_ledviaappQSto) }
};

//============================================================================
// various constants for the application...
enum {
// number of system clock ticks in one second
    BSP_TICKS_PER_SEC      = 100,

    COMMUNICATION_TICK     = BSP_TICKS_PER_SEC / 5U, // handshake timing 20 ms

//    PIXELS = 120,                      // number of LEDs in the stripe
    PIXELS = 8,                        // number of LED in the stick

    BLUETOOTH_POWER = 4,               // Pin of the transitor base

    STOP_SIG,                          // end of data

    COMMUNICATION_SIG,                 // communication request
    DISPLAY_SIG,                       // display colour
    RUNNING_SIG,                       // running light animation
    DIMMING_SIG,                       // dimming animation
    RAINBOW_SIG,                       // rainbow animation
    DEBUG_LED = 7                      // optional debuging LED
};

// store the rainbox in memory
// WARNING! 3 bytes per pixel - take care you don't exceed available RAM
colour pixelArray [PIXELS];

//............................................................................
void setup() {
    // initialize the QF-nano framework
    QF_init(Q_DIM(QF_active));

    // initialize all AOs...
    QActive_ctor(&AO_LEDviaApp.super, Q_STATE_CAST(&LEDviaApp_initial));

    // initialize the hardware used in this sketch...
    pinMode(DEBUG_LED, OUTPUT);        // set the DEBUG_LED pin to output

    pinMode(BLUETOOTH_POWER, OUTPUT);  // Pin mode of the transitor control
    delay(3000);                       // switch on delay for program upload
    digitalWrite(BLUETOOTH_POWER, HIGH); // switch on the Bluetooth module

    // set the highest standard baud rate of 115200 bps
    Serial.begin(115200);

    ledsetup();                        // setup SPI
    showColor(PIXELS, 0, 0, 0);        // all LED off
}

//............................................................................
void loop() {
    QF_run(); // run the QF-nano framework
}

//============================================================================
// interrupts...
ISR(TIMER2_COMPA_vect) {
    QF_tickXISR(0U); // process time events for tick rate 0
}

//============================================================================
// QF callbacks...
void QF_onStartup(void) {
    // set Timer2 in CTC mode, 1/1024 prescaler, start the timer ticking...
    TCCR2A = (1U << WGM21) | (0U << WGM20);
    TCCR2B = (1U << CS22 ) | (1U << CS21) | (1U << CS20); // 1/2^10
    ASSR  &= ~(1U << AS2);
    TIMSK2 = (1U << OCIE2A); // enable TIMER2 compare Interrupt
    TCNT2  = 0U;

    // set the output-compare register based on the desired tick frequency
    OCR2A  = (F_CPU / BSP_TICKS_PER_SEC / 1024U) - 1U;
}

//............................................................................
void QV_onIdle(void) { // called with interrupts DISABLED
    // Put the CPU and peripherals to the low-power mode. You might
    // need to customize the clock management for your application,
    // see the datasheet for your particular AVR MCU.
    SMCR = (0 << SM0) | (1 << SE); // idle mode, adjust to your project
    QV_CPU_SLEEP(); // atomically go to sleep and enable interrupts
}

//............................................................................
void Q_onAssert(char const Q_ROM * const file, int line) {
    // implement the error-handling policy for your application!!!
    QF_INT_DISABLE(); // disable all interrupts
    QF_RESET(); // reset the CPU
}

//============================================================================
// define all AO classes (state machine)...
/*${AOs::LEDviaApp} ........................................................*/
/*${AOs::LEDviaApp::SM} ....................................................*/
static QState LEDviaApp_initial(LEDviaApp * const me) {
    /* ${AOs::LEDviaApp::SM::initial} */
    return Q_TRAN(&LEDviaApp_branch);
}
/*${AOs::LEDviaApp::SM::branch} ............................................*/
static QState LEDviaApp_branch(LEDviaApp * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::LEDviaApp::SM::branch} */
        case Q_ENTRY_SIG: {
            if (Serial.read() == 'R') {
                Serial.print(F("T"));
                QACTIVE_POST((QActive *)me, COMMUNICATION_SIG, 0U);
            }
            else {
                if (me->program == 1)
                    QACTIVE_POST((QActive *)me, DISPLAY_SIG, 0U);
                else if (me->program == 2)
                    QACTIVE_POST((QActive *)me, RUNNING_SIG, 0U);
                else if (me->program == 3)
                    QACTIVE_POST((QActive *)me, DIMMING_SIG, 0U);
                else if (me->program == 4)
                    QACTIVE_POST((QActive *)me, RAINBOW_SIG, 0U);
            }

            QActive_armX((QActive *)me, 0U, COMMUNICATION_TICK, 0U);
            status_ = Q_HANDLED();
            break;
        }
        /* ${AOs::LEDviaApp::SM::branch} */
        case Q_EXIT_SIG: {
            QActive_disarmX((QActive *)me, 0U);
            status_ = Q_HANDLED();
            break;
        }
        /* ${AOs::LEDviaApp::SM::branch::COMMUNICATION} */
        case COMMUNICATION_SIG: {
            status_ = Q_TRAN(&LEDviaApp_communication);
            break;
        }
        /* ${AOs::LEDviaApp::SM::branch::DISPLAY} */
        case DISPLAY_SIG: {
            status_ = Q_TRAN(&LEDviaApp_display);
            break;
        }
        /* ${AOs::LEDviaApp::SM::branch::RUNNING} */
        case RUNNING_SIG: {
            /* ${AOs::LEDviaApp::SM::branch::RUNNING::[fwd]} */
            if (!me->run_fwd) {
                status_ = Q_TRAN(&LEDviaApp_running_fwd);
            }
            /* ${AOs::LEDviaApp::SM::branch::RUNNING::[bwd]} */
            else {
                status_ = Q_TRAN(&LEDviaApp_running_bwd);
            }
            break;
        }
        /* ${AOs::LEDviaApp::SM::branch::DIMMING} */
        case DIMMING_SIG: {
            /* ${AOs::LEDviaApp::SM::branch::DIMMING::[dim_up]} */
            if (!me->dim_up) {
                status_ = Q_TRAN(&LEDviaApp_dimming_up);
            }
            /* ${AOs::LEDviaApp::SM::branch::DIMMING::[dim_down]} */
            else {
                status_ = Q_TRAN(&LEDviaApp_dimming_down);
            }
            break;
        }
        /* ${AOs::LEDviaApp::SM::branch::RAINBOW} */
        case RAINBOW_SIG: {
            status_ = Q_TRAN(&LEDviaApp_rainbow);
            break;
        }
        /* ${AOs::LEDviaApp::SM::branch::Q_TIMEOUT} */
        case Q_TIMEOUT_SIG: {
            status_ = Q_TRAN(&LEDviaApp_branch);
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}
/*${AOs::LEDviaApp::SM::branch::display} ...................................*/
static QState LEDviaApp_display(LEDviaApp * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::LEDviaApp::SM::branch::display} */
        case Q_ENTRY_SIG: {
            showColor(PIXELS, me->red, me->green, me->blue);
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&LEDviaApp_branch);
            break;
        }
    }
    return status_;
}
/*${AOs::LEDviaApp::SM::branch::running_fwd} ...............................*/
static QState LEDviaApp_running_fwd(LEDviaApp * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::LEDviaApp::SM::branch::running_fwd} */
        case Q_ENTRY_SIG: {
            QF_INT_DISABLE();
            for (me->led_index = 0U; me->led_index < PIXELS; me->led_index++) {
                if (me->led_index == me->led_x) {
                    sendPixel(me->red, me->green, me->blue);
                }
                else {
                    sendPixel(0U, 0U, 0U);
                }
            }
            QF_INT_ENABLE();

            show();
            status_ = Q_HANDLED();
            break;
        }
        /* ${AOs::LEDviaApp::SM::branch::running_fwd} */
        case Q_EXIT_SIG: {
            me->led_x++;

            if (me->led_x == PIXELS - 1U) {
                me->run_fwd = 1U;
            }
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&LEDviaApp_branch);
            break;
        }
    }
    return status_;
}
/*${AOs::LEDviaApp::SM::branch::running_bwd} ...............................*/
static QState LEDviaApp_running_bwd(LEDviaApp * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::LEDviaApp::SM::branch::running_bwd} */
        case Q_ENTRY_SIG: {
            QF_INT_DISABLE();
            for (me->led_index = 0U; me->led_index < PIXELS; me->led_index++) {
                if (me->led_index == me->led_x) {
                    sendPixel(me->red, me->green, me->blue);
                }
                else {
                    sendPixel(0U, 0U, 0U);
                }
            }
            QF_INT_ENABLE();

            show();
            status_ = Q_HANDLED();
            break;
        }
        /* ${AOs::LEDviaApp::SM::branch::running_bwd} */
        case Q_EXIT_SIG: {
            me->led_x--;

            if (me->led_x == 0U) {
                me->run_fwd = 0U;
            }
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&LEDviaApp_branch);
            break;
        }
    }
    return status_;
}
/*${AOs::LEDviaApp::SM::branch::dimming_up} ................................*/
static QState LEDviaApp_dimming_up(LEDviaApp * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::LEDviaApp::SM::branch::dimming_up} */
        case Q_ENTRY_SIG: {
            showColor(PIXELS,
                me->red / 255.0 * me->brightness,
                me->green / 255.0 * me->brightness,
                me->blue / 255.0 * me->brightness);
            status_ = Q_HANDLED();
            break;
        }
        /* ${AOs::LEDviaApp::SM::branch::dimming_up} */
        case Q_EXIT_SIG: {
            me->brightness = me->brightness + 8U;

            if (me->brightness > 246U) {
                me->dim_up = 1U;
            }
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&LEDviaApp_branch);
            break;
        }
    }
    return status_;
}
/*${AOs::LEDviaApp::SM::branch::dimming_down} ..............................*/
static QState LEDviaApp_dimming_down(LEDviaApp * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::LEDviaApp::SM::branch::dimming_down} */
        case Q_ENTRY_SIG: {
            showColor(PIXELS,
                me->red / 255.0 * me->brightness,
                me->green / 255.0 * me->brightness,
                me->blue / 255.0 * me->brightness);
            status_ = Q_HANDLED();
            break;
        }
        /* ${AOs::LEDviaApp::SM::branch::dimming_down} */
        case Q_EXIT_SIG: {
            me->brightness = me->brightness - 8U;

            if (me->brightness < 10U) {
                me->dim_up = 0U;
            }
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&LEDviaApp_branch);
            break;
        }
    }
    return status_;
}
/*${AOs::LEDviaApp::SM::branch::rainbow} ...................................*/
static QState LEDviaApp_rainbow(LEDviaApp * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::LEDviaApp::SM::branch::rainbow} */
        case Q_ENTRY_SIG: {
            // cycle the starting point
            if (me->rain_x>=256) {
                me->rain_x=0;
            }
            else {
                me->rain_x++;
            }

            // build into in-memory array, as these calculations take too long to do on the fly
            for (me->led_index = 0; me->led_index < PIXELS; me->led_index++) {
                byte r, g, b;
                Wheel ((me->led_index + me->rain_x) & 255, r, g, b);
                pixelArray [me->led_index].r = r;
                pixelArray [me->led_index].g = g;
                pixelArray [me->led_index].b = b;
            }

            // now show results
            QF_INT_DISABLE();
            for (me->led_index = 0; me->led_index < PIXELS; me->led_index++) {
                sendPixel (pixelArray [me->led_index].r, pixelArray [me->led_index].g, pixelArray [me->led_index].b);
            }
            QF_INT_ENABLE();

            show();
            // end of for each cycle
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&LEDviaApp_branch);
            break;
        }
    }
    return status_;
}
/*${AOs::LEDviaApp::SM::communication} .....................................*/
static QState LEDviaApp_communication(LEDviaApp * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::LEDviaApp::SM::communication} */
        case Q_ENTRY_SIG: {
            QActive_armX((QActive *)me, 0U, COMMUNICATION_TICK, 0U);
            status_ = Q_HANDLED();
            break;
        }
        /* ${AOs::LEDviaApp::SM::communication} */
        case Q_EXIT_SIG: {
            QActive_disarmX((QActive *)me, 0U);
            status_ = Q_HANDLED();
            break;
        }
        /* ${AOs::LEDviaApp::SM::communication::Q_TIMEOUT} */
        case Q_TIMEOUT_SIG: {
            /* ${AOs::LEDviaApp::SM::communication::Q_TIMEOUT::[detect_start_sign]} */
            if (Serial.read() == '<') {
                me->value = 0U;
                status_ = Q_TRAN(&LEDviaApp_process_data);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}
/*${AOs::LEDviaApp::SM::communication::process_data} .......................*/
static QState LEDviaApp_process_data(LEDviaApp * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /* ${AOs::LEDviaApp::SM::communication::process_data} */
        case Q_ENTRY_SIG: {
            while (Serial.available()) {
                uint8_t data = Serial.read();
                switch (data) {
                    case '0' ... '9':
                        me->value *= 10;
                        me->value += data - '0';
                        break;
                    case 'r':
                        me->red = me->value;
                        break;
                    case 'g':
                        me->green = me->value;
                        break;
                    case 'b':
                        me->blue = me->value;
                        break;
                    case 'p':
                        me->program = me->value;
                        break;
                    case '>':
                        QACTIVE_POST((QActive *)me, STOP_SIG, 0U);
                        break;
                }
            }
            status_ = Q_HANDLED();
            break;
        }
        /* ${AOs::LEDviaApp::SM::communication::process_data} */
        case Q_EXIT_SIG: {
            Serial.print(F("A"));
            status_ = Q_HANDLED();
            break;
        }
        /* ${AOs::LEDviaApp::SM::communication::process_data::STOP} */
        case STOP_SIG: {
            status_ = Q_TRAN(&LEDviaApp_branch);
            break;
        }
        default: {
            status_ = Q_SUPER(&LEDviaApp_communication);
            break;
        }
    }
    return status_;
}

//...
