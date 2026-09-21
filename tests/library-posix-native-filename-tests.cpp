#include "../src/directoryEntries.h"

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QTemporaryDir>

#include <atomic>
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
bool createFile( const QByteArray& path )
{
    const auto fd = ::open( path.constData(),O_CREAT | O_EXCL | O_WRONLY,0600 ) ;
    if( fd < 0 )return false ;
    const char byte = 'x' ;
    const auto written = ::write( fd,&byte,1 ) ;
    ::close( fd ) ;
    return written == 1 ;
}

bool existsNative( const QByteArray& path )
{
    struct stat st{} ;
    return ::lstat( path.constData(),&st ) == 0 ;
}

QByteArray child( QByteArray parent,const QByteArray& name )
{
    if( !parent.endsWith( '/' ) )parent.append( '/' ) ;
    parent.append( name ) ;
    return parent ;
}
}

int main()
{
#ifndef Q_OS_UNIX
    return 0 ;
#else
    QTemporaryDir temporary ;
    if( !temporary.isValid() )return 1 ;

    const auto root = QFile::encodeName( temporary.path() ) ;

    QByteArray invalidName( "collision-" ) ;
    invalidName.append( static_cast< char >( 0xff ) ) ;
    const QByteArray literalEscapedName( "collision-\\xff" ) ;

    if( !createFile( child( root,invalidName ) ) )return 2 ;
    if( !createFile( child( root,literalEscapedName ) ) )return 3 ;

    std::atomic_bool keepGoing{ true } ;
    auto entries = directoryManager::readAllNative( root,keepGoing ) ;
    entries.sortByNameAscending() ;
    entries.join( false ) ;

    auto iter = entries.Iter( 73 ) ;

    // The queued iterator must own its row snapshot. Replacing the source
    // directory model must not invalidate or alter this already queued chain.
    entries.clear() ;
    entries.addFile( 1,QStringLiteral( "replacement" ),QByteArray( "replacement" ) ) ;
    entries.sortByNameAscending() ;
    entries.join( false ) ;
    if( iter.generation() != 73 )return 12 ;

    bool foundInvalid = false ;
    bool foundLiteral = false ;
    QString invalidDisplay ;
    QString literalDisplay ;

    while( iter.hasNext() ){
        if( iter.nativeName() == invalidName ){
            foundInvalid = true ;
            invalidDisplay = iter.value() ;
        }
        if( iter.nativeName() == literalEscapedName ){
            foundLiteral = true ;
            literalDisplay = iter.value() ;
        }
        iter = iter.next() ;
    }

    // The two rows intentionally collide in display text. The regression is
    // that their native identities remain distinct and selectable regardless.
    if( !foundInvalid || !foundLiteral )return 4 ;
    if( invalidName == literalEscapedName )return 5 ;
    if( invalidDisplay != literalDisplay )return 6 ;

    QByteArray rawDirectoryName( "dir-" ) ;
    rawDirectoryName.append( static_cast< char >( 0xfe ) ) ;
    const auto rawDirectory = child( root,rawDirectoryName ) ;
    if( ::mkdir( rawDirectory.constData(),0700 ) != 0 )return 7 ;

    const auto nested = child( rawDirectory,QByteArray( "nested" ) ) ;
    if( !createFile( nested ) )return 8 ;

    if( !directoryManager::removeEntryNative(
            root,root,rawDirectoryName,keepGoing ) )return 9 ;
    if( existsNative( rawDirectory ) )return 13 ;

    // The colliding sibling files were not addressed by that native-directory
    // deletion, proving operations remain bound to their captured byte path.
    if( !existsNative( child( root,invalidName ) ) )return 10 ;
    if( !existsNative( child( root,literalEscapedName ) ) )return 11 ;

    const QByteArray cancelledName( "cancel-before-worker" ) ;
    if( !createFile( child( root,cancelledName ) ) )return 14 ;
    keepGoing.store( false ) ;
    if( directoryManager::removeEntryNative(
            root,root,cancelledName,keepGoing ) )return 15 ;
    if( !existsNative( child( root,cancelledName ) ) )return 16 ;
    keepGoing.store( true ) ;

    const QByteArray confirmedName( "confirmed-delete-all" ) ;
    const QByteArray siblingName( "sibling-untouched" ) ;
    const auto confirmed = child( root,confirmedName ) ;
    const auto sibling = child( root,siblingName ) ;
    if( ::mkdir( confirmed.constData(),0700 ) != 0 )return 17 ;
    if( ::mkdir( sibling.constData(),0700 ) != 0 )return 18 ;
    if( !createFile( child( confirmed,QByteArray( "victim" ) ) ) )return 19 ;
    if( !createFile( child( sibling,QByteArray( "survivor" ) ) )return 20 ;
    if( !directoryManager::removeDirectoryContentsNative(
            root,confirmed,keepGoing ) )return 21 ;
    if( existsNative( child( confirmed,QByteArray( "victim" ) ) ) )return 22 ;
    if( !existsNative( child( sibling,QByteArray( "survivor" ) ) ) )return 23 ;

    QTemporaryDir outsideTemporary ;
    if( !outsideTemporary.isValid() )return 24 ;
    const auto outsideRoot = QFile::encodeName( outsideTemporary.path() ) ;
    if( !createFile( child( outsideRoot,QByteArray( "victim" ) ) ) )return 25 ;

    const QByteArray insideName( "inside" ) ;
    const auto inside = child( root,insideName ) ;
    const auto insideReal = child( root,QByteArray( "inside-real" ) ) ;
    if( ::mkdir( inside.constData(),0700 ) != 0 )return 26 ;
    if( !createFile( child( inside,QByteArray( "victim" ) ) ) )return 27 ;
    if( ::rename( inside.constData(),insideReal.constData() ) != 0 )return 28 ;
    if( ::symlink( outsideRoot.constData(),inside.constData() ) != 0 )return 29 ;

    if( directoryManager::removeEntryNative(
            root,inside,QByteArray( "victim" ),keepGoing ) )return 30 ;
    if( !existsNative( child( outsideRoot,QByteArray( "victim" ) ) ) )return 31 ;
    if( !existsNative( child( insideReal,QByteArray( "victim" ) ) ) )return 32 ;

    return 0 ;
#endif
}
