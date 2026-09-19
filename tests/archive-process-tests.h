#ifndef MDPS_ARCHIVE_PROCESS_TESTS_H
#define MDPS_ARCHIVE_PROCESS_TESTS_H
#include "archive-history-contract-tests.h"
#include "../src/archive/archiveprocess.h"
#include <QThread>
#include <QCoreApplication>
#include <cerrno>
#ifdef Q_OS_UNIX
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#endif
#ifdef Q_OS_LINUX
#include <sys/prctl.h>
#endif
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace archive_process_tests {
using archive_history_tests::require;
using archive_history_tests::read;
using archive_history_tests::write;
// Fixture dispatch occurs before the ordinary tests. Its descendants write only
// inside a disposable directory and self-expire even if the old runner leaks one.
inline int fixture(const QStringList& args)
{
    if(args.size()<3)return -1;
    if(args[1]=="--archive-exit-fixture"){
        QTextStream(stdout)<<"stdout fixture\n";QTextStream(stderr)<<"stderr fixture\n";
        return args[2].toInt();
    }
    if(args[1]=="--archive-writer-fixture"){
        QFile heartbeat(QDir(args[2]).filePath("heartbeat"));
        if(!heartbeat.open(QIODevice::WriteOnly))return 91;
        for(int i=0;i<500;++i){if(heartbeat.write("x")!=1||!heartbeat.flush())return 92;QThread::msleep(10);}
        return 0;
    }
    if(args.size()<4||args[1]!="--archive-parent-fixture")return -1;
    QProcess child;child.start(QCoreApplication::applicationFilePath(),{"--archive-writer-fixture",args[2]});
    if(!child.waitForStarted(5000))return 93;
    QFile metadata(QDir(args[2]).filePath("descendant.pid"));
    if(!metadata.open(QIODevice::WriteOnly))return 94;
    metadata.write(QByteArray::number(child.processId()));metadata.close();
#ifdef Q_OS_UNIX
    QFile group(QDir(args[2]).filePath("group.pid"));if(!group.open(QIODevice::WriteOnly))return 95;
    group.write(QByteArray::number(::getpgrp()));group.close();
#endif
    QElapsedTimer ready;ready.start();
    while(!QFileInfo::exists(QDir(args[2]).filePath("heartbeat"))&&ready.elapsed()<4000)QThread::msleep(10);
    if(args[3]=="overflow"){
        QFile output;if(!output.open(stdout,QIODevice::WriteOnly))return 96;
        const QByteArray block(1024*1024,'x');
        for(int i=0;i<66;++i){if(output.write(block)!=block.size()||!output.flush())return 97;}
    }
    QThread::sleep(20);
    return 98;
}
struct DescendantCleanup {
    qint64 pid=0;
    ~DescendantCleanup(){
        if(pid<=1)return;
#ifdef Q_OS_UNIX
        ::kill(static_cast<pid_t>(pid),SIGKILL);
        int status;while(::waitpid(static_cast<pid_t>(pid),&status,0)<0&&errno==EINTR){}
#elif defined(Q_OS_WIN)
        QProcess::execute("taskkill",{"/PID",QString::number(pid),"/T","/F"});
#endif
    }
};
inline bool stopped(qint64 pid)
{
#ifdef Q_OS_UNIX
    int status;const auto reaped=::waitpid(static_cast<pid_t>(pid),&status,WNOHANG);
    return reaped==pid||(::kill(static_cast<pid_t>(pid),0)<0&&errno==ESRCH);
#elif defined(Q_OS_WIN)
    const auto process=OpenProcess(SYNCHRONIZE,FALSE,static_cast<DWORD>(pid));
    if(!process)return GetLastError()==ERROR_INVALID_PARAMETER;
    const bool done=WaitForSingleObject(process,0)==WAIT_OBJECT_0;CloseHandle(process);return done;
#else
    Q_UNUSED(pid);return true;
#endif
}
inline bool run(QString* error)
{
    try {
#ifdef Q_OS_LINUX
        // Test-only adoption allows proving descendant death, even under a
        // container init that does not promptly reap orphaned zombie processes.
        require(::prctl(PR_SET_CHILD_SUBREAPER,1)==0,"enable test-only child reaping");
#endif
        const auto executable=QCoreApplication::applicationFilePath();
        for(const auto exit:QStringList{"0","17"}){
            const auto result=archive::detail::runContainedProcess(executable,{"--archive-exit-fixture",exit},{},5000);
            require(result.ok==(exit=="0")&&result.exitCode==exit.toInt(),"ordinary process exit contract changed");
            require(result.standardOutput.contains("stdout fixture")&&result.standardError.contains("stderr fixture"),"process channels changed");
        }
        const auto absent=archive::detail::runContainedProcess("",{},{},100);
        require(!absent.ok&&!absent.error.isEmpty(),"missing executable accepted");
        for(const auto& mode:QStringList{"timeout","overflow"}){
            QTemporaryDir temp;require(temp.isValid(),"process fixture directory");
            archive::Paths paths(temp.path());archive::SyncLock lock(paths);require(lock.tryLock(),"acquire writer ownership");
            const auto result=archive::detail::runContainedProcess(executable,{"--archive-parent-fixture",temp.path(),mode},temp.path(),mode=="timeout"?1500:10000);
            const auto pidPath=QDir(temp.path()).filePath("descendant.pid");
            require(QFileInfo::exists(pidPath),"parent fixture never started its descendant");
            DescendantCleanup cleanup;cleanup.pid=read(pidPath).trimmed().toLongLong();
            require(cleanup.pid>1,"invalid fixture descendant identity");
            require(!result.ok&&result.error.contains(mode=="timeout"?"timed out":"safety limit"),"wrong process failure: "+result.error);
            const auto heartbeat=QDir(temp.path()).filePath("heartbeat");
            require(QFileInfo::exists(heartbeat)&&QFileInfo(heartbeat).size()>0,"descendant never held an active writer");
            require(!result.error.contains("could not be confirmed"),"termination confirmation failed: "+result.error);
            require(stopped(cleanup.pid),"descendant survived "+mode+" while Archive writer ownership could be released");
            cleanup.pid=0; // Already reaped/confirmed dead, never signal a reused PID.
#ifdef Q_OS_UNIX
            require(read(QDir(temp.path()).filePath("group.pid")).trimmed().toLongLong()!=::getpgrp(),"tool shared caller process group");
#endif
            const auto after=read(heartbeat);
            lock.unlock();archive::SyncLock next(paths);require(next.tryLock(),"subsequent writer remained blocked");
            QThread::msleep(100);require(read(heartbeat)==after,"descendant wrote after next writer started");
        }
        QTextStream(stdout)<<"process-containment: timeout/overflow descendants terminated before next writer; exit/channel cases passed\n";
        return true;
    }catch(const std::exception& ex){if(error)*error=QString::fromUtf8(ex.what());return false;}
}
}
#endif
