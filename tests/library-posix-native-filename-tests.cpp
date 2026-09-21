#include "directoryEntries.h"

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

    directoryManager::removeDirectoryNative( rawDirectory,keepGoing ) ;
    if( existsNative( rawDirectory ) )return 9 ;

    // The colliding sibling files were not addressed by that native-directory
    // deletion, proving operations remain bound to their captured byte path.
    if( !existsNative( child( root,invalidName ) ) )return 10 ;
    if( !existsNative( child( root,literalEscapedName ) ) )return 11 ;

    return 0 ;
#endif
}
