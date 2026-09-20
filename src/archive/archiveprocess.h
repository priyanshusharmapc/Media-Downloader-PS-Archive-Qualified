#ifndef MDPS_ARCHIVEPROCESS_H
#define MDPS_ARCHIVEPROCESS_H
#include "archivecore.h"
#include <QElapsedTimer>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QThread>
#include <cerrno>
#include <cstring>
#include <limits>
#ifdef Q_OS_UNIX
#include <signal.h>
#include <unistd.h>
#endif
namespace archive { namespace detail {
// Each Archive tool has its own process group, not the GUI/terminal group.
// Setup happens in the child before exec. Only async-signal-safe POSIX calls
// are permitted here because the parent application can be multithreaded.
class ContainedProcess : public QProcess
{
public:
    ContainedProcess()
    {
#if defined(Q_OS_UNIX) && QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        setChildProcessModifier([this]{
            if(::setpgid(0,0)!=0){
#if QT_VERSION >= QT_VERSION_CHECK(6,7,0)
                failChildProcessModifier("Archive setpgid",errno);
#else
                ::_exit(127); // Never exec without the ownership boundary.
#endif
            }
        });
#endif
    }
#if defined(Q_OS_UNIX) && QT_VERSION < QT_VERSION_CHECK(6,0,0)
protected:
    void setupChildProcess() override
    {
        if(::setpgid(0,0)!=0)::_exit(127);
    }
#endif
};

#ifdef Q_OS_UNIX
inline bool processGroupStopped(pid_t group)
{
    if(::kill(-group,0)<0&&errno==ESRCH)return true;
#ifdef Q_OS_LINUX
    // A zombie has already released every file descriptor. Container PID 1
    // may retain zombies indefinitely, so waiting for ESRCH alone can hang.
    // Only members of the owned group are considered, never unrelated PIDs.
    const QDir proc("/proc");if(!proc.exists())return false;
    bool sawMember=false;
    for(const auto& name:proc.entryList(QDir::Dirs|QDir::NoDotAndDotDot)){
        bool numeric=false;name.toLongLong(&numeric);if(!numeric)continue;
        QFile stat(proc.filePath(name+"/stat"));if(!stat.open(QIODevice::ReadOnly))continue;
        const auto bytes=stat.readAll();const auto end=bytes.lastIndexOf(')');
        if(end<0)continue;const auto fields=bytes.mid(end+2).simplified().split(' ');
        if(fields.size()<3||fields[2].toLongLong()!=group)continue;
        sawMember=true;if(fields[0]!="Z"&&fields[0]!="X")return false;
    }
    return sawMember||(::kill(-group,0)<0&&errno==ESRCH);
#else
    return false; // Other POSIX systems wait until the group no longer exists.
#endif
}

inline bool terminateProcessGroup(ContainedProcess& process,qint64 processId,QString* error)
{
    // Keep the identity captured after startup: QProcess::processId() becomes
    // zero when Qt observes an exit, even if grandchildren still own files.
    if(processId<=1||processId>std::numeric_limits<pid_t>::max()||processId==::getpgrp()){
        process.kill();process.waitForFinished(5000);
        if(error)*error="Invalid Archive process-group identity";return false;
    }
    const auto group=static_cast<pid_t>(processId);
    if(::kill(-group,SIGKILL)<0&&errno!=ESRCH){
        const auto message=QString::fromLocal8Bit(std::strerror(errno));
        process.kill();process.waitForFinished(5000);
        if(error)*error="Unable to terminate Archive process group: "+message;return false;
    }
    process.waitForFinished(5000);
    // Do not hand the writer boundary back merely because the leader exited.
    // Group members must be gone (or terminal zombies with no open files).
    QElapsedTimer deadline;deadline.start();
    while(!processGroupStopped(group)){
        if(deadline.elapsed()>=5000){if(error)*error="Archive process-group termination could not be confirmed";return false;}
        if(::kill(-group,SIGKILL)<0&&errno!=ESRCH){if(error)*error="Archive process-group termination failed";return false;}
        QThread::msleep(10);
    }
    return true;
}
#endif

inline ProcessResult runContainedProcess(const QString& program,const QStringList& args,const QString& cwd,int timeoutMs,
                                         const std::atomic_bool* cancelRequested=nullptr,
                                         const QProcessEnvironment* environment=nullptr)
{
    ProcessResult r;
    if(program.isEmpty()){r.error="Required executable was not found";return r;}
    ContainedProcess process;if(!cwd.isEmpty())process.setWorkingDirectory(cwd);
    if(environment)process.setProcessEnvironment(*environment);
    process.setProcessChannelMode(QProcess::SeparateChannels);process.start(program,args);
    if(!process.waitForStarted(10000)){r.error=process.errorString();return r;}
    const auto childId=process.processId();
    process.closeWriteChannel();
    QByteArray output,errors;QElapsedTimer timer;timer.start();bool overflow=false,timedOut=false,cancelled=false;QString terminationError;
    const auto drain=[&]{
        const auto chunk=process.readAllStandardOutput();
        if(output.size()+chunk.size()>64*1024*1024)overflow=true;else output+=chunk;
        errors+=process.readAllStandardError();
        if(errors.size()>256*1024)errors=QByteArray("WARNING: earlier diagnostic output truncated\n")+errors.right(256*1024-64);
    };
    while(process.state()!=QProcess::NotRunning){
        process.waitForReadyRead(100);drain();
        timedOut=timer.elapsed()>timeoutMs;
        cancelled=cancelRequested&&cancelRequested->load();
        if(overflow||timedOut||cancelled){
#ifdef Q_OS_WIN
            const auto pid=process.processId();
            if(pid>0){
                QProcess killer;killer.start("taskkill",{"/PID",QString::number(pid),"/T","/F"});
                killer.waitForFinished(5000);
            }else process.kill();
#elif defined(Q_OS_UNIX)
            terminateProcessGroup(process,childId,&terminationError);
#else
            process.kill();
#endif
            process.waitForFinished(5000);break;
        }
    }
    drain();r.standardOutput=QString::fromUtf8(output);r.standardError=QString::fromUtf8(errors);
    if(overflow||timedOut||cancelled){
        r.error=cancelled?"Process cancelled":overflow?"Process output exceeded the safety limit":"Process timed out";
        if(!terminationError.isEmpty())r.error+="; "+terminationError;return r;}
    r.exitCode=process.exitCode();r.ok=process.exitStatus()==QProcess::NormalExit&&r.exitCode==0;
    if(!r.ok)r.error=QString("Process exited with code %1").arg(r.exitCode);return r;
}

}}
#endif
