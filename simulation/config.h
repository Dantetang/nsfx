/**
 * @file
 *
 * @brief Simulation support for Network Simulation Frameworks.
 *
 * @version 1.0
 * @author  Wei Tang <gauchyler@uestc.edu.cn>
 * @date    2017-12-17
 *
 * @copyright Copyright (c) 2017.
 *   National Key Laboratory of Science and Technology on Communications,
 *   University of Electronic Science and Technology of China.
 *   All rights reserved.
 */

#ifndef CONFIG_H__CC642C06_21DD_48A9_92CD_4C753FCDE2C5
#define CONFIG_H__CC642C06_21DD_48A9_92CD_4C753FCDE2C5


#include <nsfx/config.h>
#include <nsfx/chrono/duration.h>
#include <nsfx/chrono/virtual-time-point.h>


/**
 * @defgroup Simulator
 * @brief Support for discreet event simulation.
 *
 * Depends upon Exception, Component, Event and Chrono modules.
 *
 */


NSFX_OPEN_NAMESPACE


////////////////////////////////////////////////////////////////////////////////
// Chrono.
typedef chrono::VirtualTimePoint  TimePoint;
typedef TimePoint::Duration       Duration;
using chrono::AttoSeconds;
using chrono::FemtoSeconds;
using chrono::PicoSeconds;
using chrono::NanoSeconds;
using chrono::MilliSeconds;
using chrono::MicroSeconds;
using chrono::Seconds;
using chrono::Minutes;
using chrono::Hours;
using chrono::Days;


////////////////////////////////////////////////////////////////////////////////
/**
 * @ingroup Simulator
 * @brief The id of an event.
 */
typedef uint64_t  event_id_t;


NSFX_CLOSE_NAMESPACE


#endif // CONFIG_H__CC642C06_21DD_48A9_92CD_4C753FCDE2C5

