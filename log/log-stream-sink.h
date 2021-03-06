﻿/**
 * @file
 *
 * @brief Logging support for network simulation frameworks.
 *
 * @version 1.0
 * @author  Wei Tang <gauchyler@uestc.edu.cn>
 * @date    2018-05-17
 *
 * @copyright Copyright (c) 2018.
 *            National Key Laboratory of Science and Technology on Communications,
 *            University of Electronic Science and Technology of China.
 *            All rights reserved.
 */

#ifndef LOG_STREAM_SINK_H__CE0B27F7_A742_4F2B_8120_9BB3835E148A
#define LOG_STREAM_SINK_H__CE0B27F7_A742_4F2B_8120_9BB3835E148A


#include <nsfx/log/config.h>
#include <nsfx/log/i-log.h>
#include <nsfx/log/i-log-stream-sink.h>
#include <nsfx/log/i-log-formatter.h>
#include <nsfx/log/detail/log-pending-value-pool.h>
#include <nsfx/component/class-registry.h>
#include <fstream>


NSFX_OPEN_NAMESPACE


////////////////////////////////////////////////////////////////////////////////
/**
 * @ingroup Log
 * @brief The default log sink.
 *
 * # Interfaces
 * * Uses
 *   + `ILogFormatter`
 * * Provides
 *   + `ILogStreamSink`
 * * Events
 *   + `ILogEventSink`
 */
class LogStreamSink :
    public ILogFormatterUser,
    public ILogStreamSink
{
    typedef LogStreamSink  ThisClass;

public:
    LogStreamSink(void);
    virtual ~LogStreamSink(void) {}

public:
    // ILogFormatterUser
    virtual void Use(Ptr<ILogFormatter> formatter) NSFX_OVERRIDE;

    // ILogStreamSink
    virtual void Fire(LogRecord record) NSFX_OVERRIDE;

    virtual bool AddValue(const std::string& name, LogValue value) NSFX_OVERRIDE;
    virtual void UpdateValue(const std::string& name, LogValue value) NSFX_OVERRIDE;
    virtual void RemoveValue(const std::string& name) NSFX_OVERRIDE;

    virtual void SetFilter(Ptr<ILogFilter> filter) NSFX_OVERRIDE;

    virtual void AddStream(std::ostream& os) NSFX_OVERRIDE;
    virtual void AddFile(const std::string& filename,
                         std::ios_base::openmode mode) NSFX_OVERRIDE;

private:
    /**
     * @brief Normalize a log value.
     *
     * A first-order log value will be made a second-order log value.
     * A second-order or higher-order log value is unchanged.
     */
    LogValue NormalizeLogValue(LogValue value);

private:
    NSFX_INTERFACE_MAP_BEGIN(ThisClass)
        NSFX_INTERFACE_ENTRY(ILogFormatterUser)
        NSFX_INTERFACE_ENTRY(ILogEventSink)
        NSFX_INTERFACE_ENTRY(ILogStreamSink)
    NSFX_INTERFACE_MAP_END()

private:
    // The log formatter.
    Ptr<ILogFormatter>  formatter_;

    // The pending log values.
    LogPendingValuePool  pendingValuePool_;

    // The log filter.
    Ptr<ILogFilter> filter_;

    // The output streams.
    vector<std::ostream*>  ostreams_;
    vector<std::ofstream>  files_;
};

NSFX_REGISTER_CLASS(LogStreamSink, "edu.uestc.nsfx.LogStreamSink");


////////////////////////////////////////////////////////////////////////////////
inline LogStreamSink::LogStreamSink(void)
{
}

inline void LogStreamSink::Use(Ptr<ILogFormatter> formatter)
{
    if (!formatter)
    {
        BOOST_THROW_EXCEPTION(InvalidPointer());
    }
    formatter_ = formatter;
}

inline void LogStreamSink::Fire(LogRecord record)
{
    if (!formatter_)
    {
        BOOST_THROW_EXCEPTION(Uninitialized());
    }
    do
    {
        pendingValuePool_.Apply(record);
        if (!!filter_)
        {
            if (filter_->Decide(record) != LOG_ACCEPT)
            {
                break;
            }
        }
        for (auto it = ostreams_.cbegin(); it != ostreams_.cend(); ++it)
        {
            std::ostream* os = *it;
            formatter_->Format(*os, record);
        }
    }
    while (false);
}

inline bool LogStreamSink::AddValue(const std::string& name, LogValue value)
{
    return pendingValuePool_.Add(name, value);
}

inline void LogStreamSink::UpdateValue(const std::string& name, LogValue value)
{
    pendingValuePool_.Update(name, value);
}

inline void LogStreamSink::RemoveValue(const std::string& name)
{
    pendingValuePool_.Remove(name);
}

inline void LogStreamSink::SetFilter(Ptr<ILogFilter> filter)
{
    filter_ = std::move(filter);
}

inline void LogStreamSink::AddStream(std::ostream& os)
{
    if (!os)
    {
        BOOST_THROW_EXCEPTION(
            InvalidPointer() <<
            ErrorMessage("Invalid output stream."));
    }
    ostreams_.push_back(&os);
}

inline void LogStreamSink::AddFile(const std::string& filename,
                                   std::ios_base::openmode mode)
{
    std::ofstream ofs(filename, mode);
    if (!ofs)
    {
        BOOST_THROW_EXCEPTION(
            Unexpected() <<
            ErrorMessage("Cannot create log file."));
    }
    files_.push_back(std::move(ofs));
    ostreams_.push_back(&files_.back());
}


NSFX_CLOSE_NAMESPACE


#endif // LOG_STREAM_SINK_H__CE0B27F7_A742_4F2B_8120_9BB3835E148A

