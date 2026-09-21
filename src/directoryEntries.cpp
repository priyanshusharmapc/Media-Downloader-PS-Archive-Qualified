/*
 *
 *  Copyright (c) 2023
 *  name : Francis Banyikwa
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "directoryEntries.h"

#include "utils/miscellaneous.hpp"

#include <QDir>
#include <QFile>

#include <cstring>
#include <cwchar>

#ifdef Q_OS_WIN

#include <windows.h>

class dManager
{
public:
	dManager( const QString& path,std::atomic_bool& c ) :
		m_path( this->setPath( path ) ),m_continue( c )
	{
	}
	void removeDirectoryContents()
	{
		this->removeDirectory( m_path,false ) ;
	}
	void removeDirectory()
	{
		this->removeDirectory( m_path,true ) ;
	}
	directoryEntries readAll()
	{
		handle h( m_path ) ;

		if( h.valid() ){

			directoryEntries entries ;

			this->add( entries,h.data() ) ;

			while( m_continue && this->read( entries,h ) ){}

			return entries ;
		}else{
			return {} ;
		}
	}
private:
	std::wstring setPath( const QString& path )
	{
		// Win32 extended paths have different namespaces for local drives and
		// UNC shares. Blindly prepending "\\\\?\\" to a normal UNC path
		// produces an invalid path such as "\\\\?\\\\server\\share".
		// Keep already-qualified paths unchanged, translate UNC paths through
		// the UNC namespace, and reject relative paths instead of inventing an
		// absolute identity for them.
		auto clean = QDir::cleanPath( QDir::fromNativeSeparators( path ) ) ;

		// Device namespaces are not Library filesystem paths. Accept only the
		// two extended filesystem forms this enumerator understands: drive paths
		// and UNC shares. GLOBALROOT/pipe/device namespaces fail closed.
		if( clean.startsWith( "//./" ) )return {} ;

		if( clean.startsWith( "//?/" ) ){
			const auto tail = clean.mid( 4 ) ;
			const bool extendedUnc = tail.startsWith( "UNC/" ) && tail.mid( 4 ).split( '/',Qt::SkipEmptyParts ).size() >= 2 ;
			const bool extendedDrive = tail.size() >= 3 && tail[ 0 ].isLetter() && tail[ 1 ] == ':' && tail[ 2 ] == '/' ;
			if( !extendedUnc && !extendedDrive )return {} ;
			return QDir::toNativeSeparators( clean ).toStdWString() ;
		}

		QString qualified ;

		if( clean.startsWith( "//" ) ){
			if( clean.mid( 2 ).split( '/',Qt::SkipEmptyParts ).size() < 2 )return {} ;
			qualified = "//?/UNC/" + clean.mid( 2 ) ;
		}else if( QDir::isAbsolutePath( clean ) ){
			qualified = "//?/" + clean ;
		}else{
			return {} ;
		}

		return QDir::toNativeSeparators( qualified ).toStdWString() ;
	}
	class handle
	{
	public:
		handle( std::wstring s )
		{
			// setPath() deliberately returns an empty string for malformed,
			// relative and unsupported native namespaces. Preserve that fail-closed
			// result instead of dereferencing rbegin() on an empty string.
			if( s.empty() ){
				return ;
			}

			if( *s.rbegin() == L'\\' ){

				s += L"*" ;
			}else{
				s += L"\\*" ;
			}

			m_handle = FindFirstFileW( s.data(),&m_data ) ;
		}
		bool valid()
		{
			return m_handle != INVALID_HANDLE_VALUE ;
		}
		bool findNext()
		{
			return FindNextFileW( m_handle,&m_data ) != 0 ;
		}
		HANDLE get()
		{
			return m_handle ;
		}
		const WIN32_FIND_DATAW& data()
		{
			return m_data ;
		}
		~handle()
		{
			if( m_handle != INVALID_HANDLE_VALUE ){
				FindClose( m_handle ) ;
			}
		}
	private:
		WIN32_FIND_DATAW m_data{} ;
		HANDLE m_handle = INVALID_HANDLE_VALUE ;
	};
	void removePath( const std::wstring& w,const wchar_t * name,const WIN32_FIND_DATAW& data )
	{
		if( std::wcscmp( name,L"." ) != 0 && std::wcscmp( name,L".." ) != 0 ){

			auto m = w + L'\\' + name ;

			// Directory junctions and symbolic links are name-surrogate reparse
			// points. Recursing through them would cross the user's selected
			// Library tree and can delete data owned by another filesystem path.
			// Treat every reparse point as a leaf and remove only the link itself.
			if( data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ){

				if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
					RemoveDirectoryW( m.data() ) ;
				}else{
					DeleteFileW( m.data() ) ;
				}
			}else if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){

				this->removeDirectory( m,true ) ;
			}else{
				DeleteFileW( m.data() ) ;
			}
		}
	}
	void removeDirectory( const std::wstring& w,bool removeDirectory )
	{
		// A top-level directory passed by Library may itself be a junction or
		// directory symlink. Never enumerate through a reparse point.
		const auto attributes = GetFileAttributesW( w.data() ) ;
		if( attributes != INVALID_FILE_ATTRIBUTES && ( attributes & FILE_ATTRIBUTE_REPARSE_POINT ) ){

			if( removeDirectory ){
				if( attributes & FILE_ATTRIBUTE_DIRECTORY ){
					RemoveDirectoryW( w.data() ) ;
				}else{
					DeleteFileW( w.data() ) ;
				}
			}
			return ;
		}

		handle h( w ) ;

		if( h.valid() ){

			const auto& mm = h.data() ;

			this->removePath( w,mm.cFileName,mm ) ;

			while( m_continue ){

				if( h.findNext() ){

					const auto& m = h.data() ;

					this->removePath( w,m.cFileName,m ) ;
				}else{
					break ;
				}
			}

			if( removeDirectory ){

				RemoveDirectoryW( w.data() ) ;
			}
		}
	}
	void add( directoryEntries& entries,const WIN32_FIND_DATAW& data )
	{
		auto m = data.cFileName ;

		if( entries.valid( m ) ){

			// Reparse points may resolve outside the configured Library root.
			// They are managed as link leaves by deletion code and must never be
			// presented as ordinary navigable folders.
			if( data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ){
				return ;
			}

			LARGE_INTEGER filesize ;

			filesize.LowPart = data.ftLastWriteTime.dwLowDateTime ;
			filesize.HighPart = data.ftLastWriteTime.dwHighDateTime ;

			if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){

				entries.addFolder( filesize.QuadPart,QString::fromWCharArray( m ) ) ;
			}else{
				entries.addFile( filesize.QuadPart,QString::fromWCharArray( m ) ) ;
			}
		}
	}
	bool read( directoryEntries& entries,handle& h )
	{
		if( h.findNext() ){

			this->add( entries,h.data() ) ;

			return true ;
		}else{
			return false ;
		}
	}
	std::wstring m_path ;
	std::atomic_bool& m_continue ;
} ;

#else

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <limits.h>

class dManager
{
public:
	dManager( const QString& path,std::atomic_bool& c ) :
		m_path( QFile::encodeName( path ).constData() ),
		m_continue( c )
	{
	}
	dManager( const QByteArray& nativePath,std::atomic_bool& c ) :
		m_path( nativePath.constData(),static_cast< size_t >( nativePath.size() ) ),
		m_continue( c )
	{
	}
	directoryEntries readAll()
	{
		auto handle = utils::misc::unique_rsc( opendir,closedir,m_path.data() ) ;

		if( handle ){

			directoryEntries entries ;

			while( m_continue && this->read( entries,m_path,handle.get() ) ){}

			return entries ;
		}else{
			return {} ;
		}
	}
	void removeDirectoryContents()
	{
		this->removeDirectory( m_path,false ) ;
	}
	void removeDirectory()
	{
		this->removeDirectory( m_path,true ) ;
	}
private:
	void removeDirectory( const std::string& pm,bool removeFolder )
	{
		auto handle = utils::misc::unique_rsc( opendir,closedir,pm.data() ) ;

		if( handle ){

			while( m_continue ){

				auto e = readdir( handle.get() ) ;

				if( e ){

					this->removePath( pm,e->d_name ) ;
				}else{
					break ;
				}
			}

			if( removeFolder ){

				rmdir( pm.data() ) ;
			}
		}
	}
	void removePath( const std::string& pm,const char * name )
	{
		if( std::strcmp( name,"." ) != 0 && std::strcmp( name,".." ) != 0 ){

			struct stat m ;

			auto pp = pm + "/" + name ;

			if( lstat( pp.data(),&m ) == 0 ){

				if( S_ISDIR( m.st_mode ) ){

					this->removeDirectory( pp,true ) ;
				}else{
					unlink( pp.data() ) ;
				}
			}
		}
	}
	bool read( directoryEntries& entries,const std::string& mm,DIR * dir )
	{
		auto e = readdir( dir ) ;

		if( e ){

			const auto name = e->d_name ;

			if( entries.valid( name ) ){

				struct stat m ;

				auto s = mm + '/' + name ;

				// lstat() preserves link identity. stat() followed directory
				// symlinks and allowed Library navigation to cross its filesystem
				// ownership boundary.
				if( lstat( s.data(),&m ) == 0 ){
                    const QByteArray nativeName( name,static_cast< int >( std::strlen( name ) ) ) ;
                    auto displayName = QString::fromUtf8( nativeName.constData(),nativeName.size() ) ;

                    // A POSIX filename may be arbitrary bytes. If UTF-8 cannot
                    // round-trip it exactly, render an unambiguous escaped form
                    // while retaining nativeName as the authoritative identity.
                    if( displayName.toUtf8() != nativeName ){
                        displayName.clear() ;
                        for( const auto byte : nativeName ){
                            const auto value = static_cast< unsigned char >( byte ) ;
                            if( value >= 0x20 && value < 0x7f && value != '\\' ){
                                displayName += QChar( value ) ;
                            }else{
                                displayName += QString( "\\x%1" ).arg( value,2,16,QLatin1Char( '0' ) ) ;
                            }
                        }
                    }

					if( S_ISREG( m.st_mode ) ){

						entries.addFile( m.st_mtime,displayName,nativeName ) ;

					}else if( S_ISDIR( m.st_mode ) ){

						entries.addFolder( m.st_mtime,displayName,nativeName ) ;
					}
				}
			}

			return true ;
		}else{
			return false ;
		}
	}
	std::string m_path ;
	std::atomic_bool& m_continue ;
} ;

#endif

bool directoryEntries::valid( const char * e )
{
	if( std::strcmp( e,".." ) == 0 || std::strcmp( e,"." ) == 0 ){

		return false ;
	}


	return true ;
}

bool directoryEntries::valid( const wchar_t * s )
{
	if( std::wcscmp( s,L".." ) == 0 || std::wcscmp( s,L"." ) == 0 ){

		return false ;
	}


	return true ;
}

directoryEntries directoryManager::readAll( const QString& e )
{
	std::atomic_bool s{ true } ;

	return directoryManager::readAll( e,s ) ;
}

directoryEntries directoryManager::readAll( const QString& e,std::atomic_bool& s )
{
	return dManager( e,s ).readAll() ;
}

void directoryManager::removeDirectoryContents( const QString& e,std::atomic_bool& s )
{
	return dManager( e,s ).removeDirectoryContents() ;
}

void directoryManager::removeDirectory( const QString& e,std::atomic_bool& s )
{
	return dManager( e,s ).removeDirectory() ;
}

#ifdef Q_OS_UNIX
namespace
{
class nativeFd
{
public:
	explicit nativeFd( int fd = -1 ) : m_fd( fd ) {}
	nativeFd( const nativeFd& ) = delete ;
	nativeFd& operator=( const nativeFd& ) = delete ;
	nativeFd( nativeFd&& other ) noexcept : m_fd( other.m_fd ){ other.m_fd = -1 ; }
	nativeFd& operator=( nativeFd&& other ) noexcept
	{
		if( this != &other ){
			if( m_fd >= 0 )::close( m_fd ) ;
			m_fd = other.m_fd ;
			other.m_fd = -1 ;
		}
		return *this ;
	}
	~nativeFd(){ if( m_fd >= 0 )::close( m_fd ) ; }
	bool valid() const { return m_fd >= 0 ; }
	int get() const { return m_fd ; }
private:
	int m_fd ;
} ;

int nativeDirectoryFlags()
{
	int flags = O_RDONLY | O_DIRECTORY | O_NOFOLLOW ;
#ifdef O_CLOEXEC
	flags |= O_CLOEXEC ;
#endif
	return flags ;
}

bool validNativeComponent( const QByteArray& value )
{
	return !value.isEmpty() && value != "." && value != ".." &&
		!value.contains( '/' ) && !value.contains( '\0' ) ;
}

bool splitAbsoluteNativePath( const QByteArray& path,std::vector< QByteArray >& out )
{
	if( path.isEmpty() || path.contains( '\0' ) || !path.startsWith( '/' ) )return false ;
	for( const auto& component : path.split( '/' ) ){
		if( component.isEmpty() )continue ;
		if( !validNativeComponent( component ) )return false ;
		out.emplace_back( component ) ;
	}
	return true ;
}

bool relativeNativeComponents( const QByteArray& root,const QByteArray& path,
			       std::vector< QByteArray >& rootComponents,
			       std::vector< QByteArray >& relativeComponents )
{
	if( !splitAbsoluteNativePath( root,rootComponents ) )return false ;
	if( path == root )return true ;
	QByteArray prefix = root ;
	if( !prefix.endsWith( '/' ) )prefix.append( '/' ) ;
	if( !path.startsWith( prefix ) )return false ;
	const auto relative = path.mid( prefix.size() ) ;
	for( const auto& component : relative.split( '/' ) ){
		if( component.isEmpty() )continue ;
		if( !validNativeComponent( component ) )return false ;
		relativeComponents.emplace_back( component ) ;
	}
	return true ;
}

nativeFd openNativeDirectoryNoFollow( const QByteArray& root,const QByteArray& path )
{
	std::vector< QByteArray > rootComponents,relativeComponents ;
	if( !relativeNativeComponents( root,path,rootComponents,relativeComponents ) )return {} ;
	nativeFd current( ::open( "/",nativeDirectoryFlags() ) ) ;
	if( !current.valid() )return {} ;
	auto descend = [ & ]( const QByteArray& component ){
		nativeFd next( ::openat( current.get(),component.constData(),nativeDirectoryFlags() ) ) ;
		if( !next.valid() )return false ;
		current = std::move( next ) ;
		return true ;
	} ;
	for( const auto& component : rootComponents )if( !descend( component ) )return {} ;
	for( const auto& component : relativeComponents )if( !descend( component ) )return {} ;
	return current ;
}

bool removeDirectoryContentsFd( int directory,std::atomic_bool& keepGoing )
{
	if( !keepGoing.load() )return false ;
	const auto duplicate = ::dup( directory ) ;
	if( duplicate < 0 )return false ;
	DIR * raw = ::fdopendir( duplicate ) ;
	if( raw == nullptr ){ ::close( duplicate ) ; return false ; }
	std::unique_ptr< DIR,decltype( &::closedir ) > stream( raw,&::closedir ) ;
	errno = 0 ;
	while( keepGoing.load() ){
		auto * entry = ::readdir( stream.get() ) ;
		if( entry == nullptr )return errno == 0 ;
		if( std::strcmp( entry->d_name,"." ) == 0 || std::strcmp( entry->d_name,".." ) == 0 )continue ;
		if( !keepGoing.load() )return false ;
		struct stat state{} ;
		if( ::fstatat( directory,entry->d_name,&state,AT_SYMLINK_NOFOLLOW ) != 0 ){
			if( errno == ENOENT )continue ;
			return false ;
		}
		if( !keepGoing.load() )return false ;
		if( S_ISDIR( state.st_mode ) && !S_ISLNK( state.st_mode ) ){
			nativeFd child( ::openat( directory,entry->d_name,nativeDirectoryFlags() ) ) ;
			if( !child.valid() || !removeDirectoryContentsFd( child.get(),keepGoing ) )return false ;
			if( !keepGoing.load() )return false ;
			if( ::unlinkat( directory,entry->d_name,AT_REMOVEDIR ) != 0 && errno != ENOENT )return false ;
		}else{
			if( ::unlinkat( directory,entry->d_name,0 ) != 0 && errno != ENOENT )return false ;
		}
	}
	return false ;
}
}

directoryEntries directoryManager::readAllNative( const QByteArray& e,std::atomic_bool& s )
{
	return dManager( e,s ).readAll() ;
}

bool directoryManager::nativeDirectoryIsSafe( const QByteArray& root,const QByteArray& path )
{
	return openNativeDirectoryNoFollow( root,path ).valid() ;
}

bool directoryManager::removeEntryNative( const QByteArray& root,const QByteArray& parent,
					  const QByteArray& name,std::atomic_bool& keepGoing )
{
	if( !keepGoing.load() || !validNativeComponent( name ) )return false ;
	auto parentFd = openNativeDirectoryNoFollow( root,parent ) ;
	if( !parentFd.valid() || !keepGoing.load() )return false ;
	struct stat state{} ;
	if( ::fstatat( parentFd.get(),name.constData(),&state,AT_SYMLINK_NOFOLLOW ) != 0 )return errno == ENOENT ;
	if( !keepGoing.load() )return false ;
	if( S_ISDIR( state.st_mode ) && !S_ISLNK( state.st_mode ) ){
		nativeFd child( ::openat( parentFd.get(),name.constData(),nativeDirectoryFlags() ) ) ;
		if( !child.valid() || !removeDirectoryContentsFd( child.get(),keepGoing ) )return false ;
		if( !keepGoing.load() )return false ;
		return ::unlinkat( parentFd.get(),name.constData(),AT_REMOVEDIR ) == 0 || errno == ENOENT ;
	}
	if( !keepGoing.load() )return false ;
	return ::unlinkat( parentFd.get(),name.constData(),0 ) == 0 || errno == ENOENT ;
}

bool directoryManager::removeDirectoryContentsNative( const QByteArray& root,const QByteArray& path,
						       std::atomic_bool& keepGoing )
{
	if( !keepGoing.load() )return false ;
	auto directory = openNativeDirectoryNoFollow( root,path ) ;
	if( !directory.valid() || !keepGoing.load() )return false ;
	return removeDirectoryContentsFd( directory.get(),keepGoing ) ;
}

QString directoryManager::renameEntryNative( const QByteArray& root,const QByteArray& parent,
					     const QByteArray& oldName,const QString& newName,
					     QByteArray& newNativeName )
{
	if( !validNativeComponent( oldName ) || newName.isEmpty() || newName == "." || newName == ".." ||
	    newName.contains( '/' ) || newName.contains( '\\' ) )
		return QObject::tr( "Rename requires a file or folder name, not a path." ) ;
	newNativeName = QFile::encodeName( newName ) ;
	if( !validNativeComponent( newNativeName ) )
		return QObject::tr( "Rename destination cannot be represented safely on this filesystem." ) ;
	auto parentFd = openNativeDirectoryNoFollow( root,parent ) ;
	if( !parentFd.valid() )return QObject::tr( "Current Library directory is no longer safe." ) ;
	struct stat oldState{} ;
	if( ::fstatat( parentFd.get(),oldName.constData(),&oldState,AT_SYMLINK_NOFOLLOW ) != 0 )
		return QObject::tr( "Rename source is no longer available." ) ;
	struct stat existing{} ;
	if( ::fstatat( parentFd.get(),newNativeName.constData(),&existing,AT_SYMLINK_NOFOLLOW ) == 0 )
		return QObject::tr( "Rename destination already exists." ) ;
	if( errno != ENOENT )
		return QObject::tr( "Unable to validate rename destination: %1" ).arg( QString::fromLocal8Bit( std::strerror( errno ) ) ) ;
	if( ::renameat( parentFd.get(),oldName.constData(),parentFd.get(),newNativeName.constData() ) != 0 )
		return QObject::tr( "Renaming failed: %1" ).arg( QString::fromLocal8Bit( std::strerror( errno ) ) ) ;
	return {} ;
}
#endif
