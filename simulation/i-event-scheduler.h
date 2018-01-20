/**
 * @file
 *
 * @brief Simulation support for Network Simulation Frameworks.
 *
 * @version 1.0
 * @author  Wei Tang <gauchyler@uestc.edu.cn>
 * @date    2017-12-25
 *
 * @copyright Copyright (c) 2017.
 *   National Key Laboratory of Science and Technology on Communications,
 *   University of Electronic Science and Technology of China.
 *   All rights reserved.
 */

#ifndef I_EVENT_SCHEDULER_H__B3B744CD_6782_43F2_A063_8B25FD33374E
#define I_EVENT_SCHEDULER_H__B3B744CD_6782_43F2_A063_8B25FD33374E


#include <nsfx/simulation/config.h>
#include <nsfx/simulation/i-event-handle.h>
#include <nsfx/event/i-event-sink.h>
#include <nsfx/component/ptr.h>


NSFX_OPEN_NAMESPACE


////////////////////////////////////////////////////////////////////////////////
// Types.
class IEventScheduler;
class IEventSchedulerUser;

#define NSFX_IID_IEventScheduler      NSFX_UUID_OF(::nsfx::IEventScheduler)
#define NSFX_IID_IEventSchedulerUser  NSFX_UUID_OF(::nsfx::IEventSchedulerUser)


////////////////////////////////////////////////////////////////////////////////
// IEventScheduler.
/**
 * @ingroup Simulator
 * @brief The event scheduler interface.
 */
class IEventScheduler :/*{{{*/
    virtual public IObject
{
public:
    virtual ~IEventScheduler(void) BOOST_NOEXCEPT {}

    /**
     * @brief Schedule an event.
     *
     * @param[in] sink The event sink.
     *
     * @throw OutOfMemory
     * @throw Uninitialized  The scheduler is not initialized.
     * @throw InvalidPointer The sink is \c nullptr.
     */
    virtual Ptr<IEventHandle> ScheduleNow(Ptr<IEventSink<> > sink) = 0;

    /**
     * @brief Schedule an event.
     *
     * @param[in] sink The event sink.
     *
     * @throw OutOfMemory
     * @throw Uninitialized   The scheduler is not initialized.
     * @throw InvalidPointer  The sink is \c nullptr.
     * @throw InvalidArgument The duration is invalid.<br/>
     *                         e.g., it is negative.
     */
    virtual Ptr<IEventHandle> ScheduleIn(const Duration& dt,
                                         Ptr<IEventSink<> > sink) = 0;

    /**
     * @brief Schedule an event.
     *
     * @param[in] sink The event sink.
     *
     * @throw OutOfMemory
     * @throw Uninitialized   The scheduler is not initialized.
     * @throw InvalidPointer  The sink is \c nullptr.
     * @throw InvalidArgument The time point is invalid.
     *                         e.g., it is earlier than the current time.
     */
    virtual Ptr<IEventHandle> ScheduleAt(const TimePoint& t,
                                         Ptr<IEventSink<> > sink) = 0;

    /**
     * @brief Get the number of events in the scheduler.
     *
     * The events may be pending or cancelled.<br/>
     * The currently running event is not counted.
     */
    virtual size_t GetNumEvents(void) = 0;

    /**
     * @brief Get the next event in the scheduler.
     *
     * @return Return \c nullptr if there is no events in the scheduler.
     *
     * The events may be pending or cancelled.<br/>
     *
     * This function <b>cannot</b> return a free pointer to \c IEventHandle,
     * since the \c IEventHandle does not share the same lifetime as the
     * \c IEventScheduler.
     */
    virtual Ptr<IEventHandle> GetNextEvent(void) = 0;

    /**
     * @brief Remove the next event in the scheduler.
     *
     * @return Return \c nullptr if there is no events in the scheduler.
     *
     * The events may be pending or cancelled.<br/>
     */
    virtual Ptr<IEventHandle> RemoveNextEvent(void) = 0;

}; // class IEventScheduler /*}}}*/


NSFX_DEFINE_CLASS_UUID(IEventScheduler, 0x4A9E5C84, 0x1D35, 0x4CAA, 0x9FA3F155AA5E5B30LL);


////////////////////////////////////////////////////////////////////////////////
// IEventSchedulerUser.
/**
 * @ingroup Simulator
 * @brief The event scheduler user interface.
 */
class IEventSchedulerUser :/*{{{*/
    virtual public IObject
{
public:
    virtual ~IEventSchedulerUser(void) BOOST_NOEXCEPT {}

    virtual void UseEventScheduler(Ptr<IEventScheduler> scheduler) = 0;

}; // class IEventScheduler /*}}}*/


NSFX_DEFINE_CLASS_UUID(IEventSchedulerUser, 0xA72965EB, 0x80C9, 0x42ED, 0x934FC8CD3E1B3670LL);


NSFX_CLOSE_NAMESPACE


#endif // I_EVENT_SCHEDULER_H__B3B744CD_6782_43F2_A063_8B25FD33374E
