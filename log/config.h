/**
 * @file
 *
 * @brief Logging support for Network Simulation Frameworks.
 *
 * @version 1.0
 * @author  Fuzzier Tang <gauchyler@gmail.com>
 * @date    2015-02-26
 *
 * @copyright Copyright (c) 2015.
 *            National Key Laboratory of Science and Technology on Communications,
 *            University of Electronic Science and Technology of China.
 *            All rights reserved.
 */

#ifndef CONFIG_H__A5BE74BF_215D_493B_8BB1_8F90D9316F15
#define CONFIG_H__A5BE74BF_215D_493B_8BB1_8F90D9316F15


#include <nsfx/config.h>


////////////////////////////////////////////////////////////////////////////////
/**
 * @defgroup Log
 *
 * @brief Log support.
 *
 * To use the default logger, users simply use the following macros:
 * NSFX_LOG(severity)  << "a message";
 * NSFX_LOG_UNCOND()   << "a message";
 * NSFX_LOG_FATAL()    << "a message";
 * NSFX_LOG_ERROR()    << "a message";
 * NSFX_LOG_WARNING()  << "a message";
 * NSFX_LOG_INFO()     << "a message";
 * NSFX_LOG_DEBUG()    << "a message";
 * NSFX_LOG_FUNCTION() << "a message";
 * NSFX_LOG_TRACE()    << "a message";
 *
 * Users can also use a custom logger, and use the following macros:
 * NSFX_LOG_L(logger, severity) << "a message";
 * NSFX_LOG_UNCOND_L(logger)    << "a message";
 * NSFX_LOG_FATAL_L(logger)     << "a message";
 * NSFX_LOG_ERROR_L(logger)     << "a message";
 * NSFX_LOG_WARNING_L(logger)   << "a message";
 * NSFX_LOG_INFO_L(logger)      << "a message";
 * NSFX_LOG_DEBUG_L(logger)     << "a message";
 * NSFX_LOG_FUNCTION_L(logger)  << "a message";
 * NSFX_LOG_TRACE_L(logger)     << "a message";
 *
 *
 * # Concept
 * The key function of a logger is to pass runtime information to log sinks.
 *
 * Typical information includes human-readable message, severity level,
 * timestamp, object name, function name, file name and line number, etc.
 *
 * Information can be organized into (field, value) pairs.
 * Each pair is call a log record.
 * The log sources generate log records, and pass them to log sinks via loggers.
 * And the log sinks are responsible to format, display or store the records.
 *
 * The records are transparent to loggers, since loggers do not care about
 * the actually meanings of the fields and values.
 * However, the log sinks usually have to understand the meanings of the records
 * generated by the log sources in order to format and display them.
 *
 * Therefore, log sources and sinks are usually coupled that have contracts
 * upon the sets of fields and ranges of values of the records, while loggers
 * are common and reusable among different couples of log sources and sinks.
 *
 * # Default log source
 * Fields are contracted at design-time between log sources and sinks,
 * while values are usually generated by log sources at runtime.
 * However, it would be painful for users to provide common information at every
 * logging site, such as timestamps, function names and line numbers.
 *
 * To ease the coding burden of users, certain values should be generated
 * automatically.
 * Since values are generated by log sources, this means each logger should be
 * associated with a default log source that generates common information.
 *
 * Users can configure the default log source.
 * To let the default log source generate a record automatically, a user
 * provides the name of the field, along with a value generator object.
 * The value generator object can be viewed as a callback that can be invoked
 * by the default log source to query the value at runtime from the users.
 * Usually, the value generator consists of a callback method and a set of
 * parameters.
 * For this purpose, the library provides \c Attribute class that takes the
 * responsibility of value generators.
 *
 * # Log record
 * The field of a log record is a string.
 * However, to make a reusable log record that is type-neutral (that supports
 * any type of value), the value of a log record is must harder to implement.
 *
 * The library provides \c AttributeValue class.
 * It is able to hold any type of value, and the underlying value can be queried
 * by log sinks at runtime.
 *
 * The library assumes that the log sink knows the type of the underlying value
 * of each record.
 * Thus, the \c AttributeValue directly exposes the underlying value to the log
 * sink.
 *
 * \c AttributeValue itself is type neutral, and it holds an object of virtual
 * base class \c IAttributeValueBase.
 *
 * \c IAttributeValueBase provides two methods:
 * \c IAttributeValueBase::GetTypeId() and \c IAttributeValueBase::ToString().
 * The first method provides the type information of the value, and the second
 * method provides a basic means of retrieving the value without using its type.
 *
 * <code>IAttributeValue<></code> interface that derives from
 * \c IAttributeValueBase is defined to provide type-specific operations upon
 * the underlying value.
 * User-defined attribute value classes must implement <code>IAttributeValue<></code>.
 *
 * \c AttributeValue class holds a poiner to <code>IAttributeValue<></code>.
 * To be type-neutral, it actaully holds the a pointer to \c IAttributeValueBase.
 * Upon the request of log sinks, the pointer to \c IAttributeValueBase is
 * converted to type-specific <code>IAttributeValue<></code>, so the log sinks
 * is able to access the type-specific value.
 *
 * Users that implement attribute value classes must make sure that the
 * underlying value is kept available when log sinks access it.
 * However, the log sources may not always know when the underlying value will
 * be accessed by the log sinks.
 * e.g., it is not a problem in a single-threaded environment, the logger will
 * deliver log records to log sinks immediately and wait for them to complete.
 * However, in a multithreaded environment, the log sinks may reside in a
 * different thread and consumes the log records in a later time.
 * Therefore, user-defined attribute value classes shall store or refer to
 * persistent values whenever possible.
 */


////////////////////////////////////////////////////////////////////////////////
#define NSFX_LOG_OPEN_NAMESPACE     \
    NSFX_OPEN_NAMESPACE             \
    namespace log {

#define NSFX_LOG_CLOSE_NAMESPACE    \
    } /* namespace log */           \
    NSFX_CLOSE_NAMESPACE


#endif // CONFIG_H__A5BE74BF_215D_493B_8BB1_8F90D9316F15
