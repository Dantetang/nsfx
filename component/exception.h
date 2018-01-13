/**
 * @file
 *
 * @brief Component support for Network Simulation Frameworks.
 *
 * @version 1.0
 * @author  Wei Tang <gauchyler@uestc.edu.cn>
 * @date    2017-12-14
 *
 * @copyright Copyright (c) 2017.
 *   National Key Laboratory of Science and Technology on Communications,
 *   University of Electronic Science and Technology of China.
 *   All rights reserved.
 */

#ifndef EXCEPTION_H__F58D565F_FEC4_4D7A_BE86_F688E62F28BD
#define EXCEPTION_H__F58D565F_FEC4_4D7A_BE86_F688E62F28BD


#include <nsfx/component/config.h>


NSFX_OPEN_NAMESPACE


////////////////////////////////////////////////////////////////////////////////
// Base.
/**
 * @ingroup Component
 * @brief The base class for all exceptions in component module.
 */
struct ComponentException : Exception {};


////////////////////////////////////////////////////////////////////////////////
// Object.
/**
 * @ingroup Component
 * @brief The queried interface is not supported.
 */
struct NoInterface : ComponentException {};

/**
 * @ingroup Component
 * @brief Practice aggregation in a wrong way.
 */
struct BadAggregation : ComponentException {};


////////////////////////////////////////////////////////////////////////////////
// Object initialization.
/**
 * @ingroup Component
 * @brief An object is not initialized.
 */
struct Uninitialized : ComponentException {};

/**
 * @ingroup Component
 * @brief An initialized object cannot be initialized again.
 */
struct CannotReinitialize : ComponentException {};


////////////////////////////////////////////////////////////////////////////////
// Class registration.
/**
 * @ingroup Component
 * @brief Cannot create an object since its class factory is not registered.
 */
struct ClassNotRegistered : ComponentException {};

/**
 * @ingroup Component
 * @brief Cannot register a class factory that is already registered.
 */
struct ClassIsRegistered : ComponentException {};


////////////////////////////////////////////////////////////////////////////////
// Sink connection.
/**
 * @ingroup Component
 * @brief The event provider cannot accept additional connections any more.
 */
struct ConnectionLimit : ComponentException {};

/**
 * @ingroup Component
 * @brief The sink does not support the required interface.
 */
struct CannotConnect : ComponentException {};

/**
 * @ingroup Component
 * @brief No sink has been connected.
 */
struct Connected : ComponentException {};


NSFX_CLOSE_NAMESPACE


#endif // EXCEPTION_H__F58D565F_FEC4_4D7A_BE86_F688E62F28BD

