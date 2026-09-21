
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

#ifndef DIRECTORY_ENTRIES_H
#define DIRECTORY_ENTRIES_H

#include <QString>
#include <QByteArray>
#include <QObject>
#include <QtGlobal>

#include <atomic>
#include <vector>

class directoryEntries
{
private:
	class entry
	{
	public:
		entry( qint64 d,QString p,QByteArray nativeName,bool f ) :
			m_dateCreated( d ),
			m_path( std::move( p ) ),
			m_nativeName( std::move( nativeName ) ),
			m_folder( f )
		{
		}
		bool isFolder() const
		{
			return m_folder ;
		}
		const QString& path() const
		{
			return m_path ;
		}
		qint64 dateCreated() const
		{
			return m_dateCreated ;
		}
		const QByteArray& nativeName() const
		{
			return m_nativeName ;
		}
	private:
		qint64 m_dateCreated ;
		QString m_path ;
		QByteArray m_nativeName ;
		bool m_folder ;
	} ;
	class wrapper
	{
	public:
		wrapper( const directoryEntries::entry& it ) : m_entry( &it )
		{
		}
		const directoryEntries::entry * operator->() const
		{
			return m_entry ;
		}
	private:
		const directoryEntries::entry * m_entry ;
	} ;
	std::vector< directoryEntries::entry > m_folders ;
	std::vector< directoryEntries::entry > m_files ;
	std::vector< directoryEntries::wrapper > m_joined ;
	std::vector< directoryEntries::wrapper > m_globalJoined ;
public:
	template< typename Function >
	void forEachFile( Function function )
	{
		for( const auto& it : m_files ){

			function( it.path() ) ;
		}
	}
	enum class ICON{ FILE,FOLDER } ;
	directoryEntries move()
	{
		return std::move( *this ) ;
	}
	bool valid( const char * ) ;
	bool valid( const wchar_t * ) ;

	void clear()
	{
		m_folders.clear() ;
		m_files.clear() ;
		m_joined.clear() ;
		m_globalJoined.clear() ;
	}
	void sortByDateAscending()
	{
		struct meaw
		{
			bool operator()( const entry& lhs,const entry& rhs )
			{
				return lhs.dateCreated() < rhs.dateCreated() ;
			}
		} ;

		this->sort( meaw() ) ;
	}
	void sortByDateDescending()
	{
		struct meaw
		{
			bool operator()( const entry& lhs,const entry& rhs )
			{
				return lhs.dateCreated() > rhs.dateCreated() ;
			}
		} ;

		this->sort( meaw() ) ;
	}
	void sortByNameAscending()
	{
		struct meaw
		{
			bool operator()( const entry& lhs,const entry& rhs )
			{
				return lhs.path().toLower() < rhs.path().toLower() ;
			}
		} ;

		this->sort( meaw() ) ;
	}
	void sortByNameDescending()
	{
		struct meaw
		{
			bool operator()( const entry& lhs,const entry& rhs )
			{
				return lhs.path().toLower() > rhs.path().toLower() ;
			}
		} ;

		this->sort( meaw() ) ;
	}
	template< typename sorter>
	void sort( sorter s )
	{
		std::sort( m_folders.begin(),m_folders.end(),s ) ;
		std::sort( m_files.begin(),m_files.end(),s ) ;

		// Keep a globally sorted mixed sequence as well. This is the model used
		// when folder-first grouping is disabled.
		m_joined.clear() ;
		for( const auto& it : m_folders ){
			m_joined.emplace_back( it ) ;
		}
		for( const auto& it : m_files ){
			m_joined.emplace_back( it ) ;
		}
		std::sort( m_joined.begin(),m_joined.end(),[ & ]( const wrapper& lhs,const wrapper& rhs ){
			return s( *lhs.operator->(),*rhs.operator->() ) ;
		} ) ;

		m_globalJoined = m_joined ;
	}
	void addFile( qint64 dateCreated,QString path,QByteArray nativeName = {} )
	{
		m_files.emplace_back( dateCreated,std::move( path ),std::move( nativeName ),false ) ;
	}
	void addFolder( qint64 dateCreated,QString path,QByteArray nativeName = {} )
	{
		m_folders.emplace_back( dateCreated,std::move( path ),std::move( nativeName ),true ) ;
	}

	class iter
	{
	private:
		struct snapshot
		{
			QString displayName ;
			QByteArray nativeName ;
			ICON icon ;
		} ;
	public:
		iter() = default ;
		iter( const std::vector< directoryEntries::wrapper >& e,quint64 generation ) :
			m_generation( generation )
		{
			m_entries.reserve( e.size() ) ;
			for( const auto& wrapper : e ){
				const auto * item = wrapper.operator->() ;
				m_entries.push_back( {
					item->path(),
					item->nativeName(),
					item->isFolder() ? ICON::FOLDER : ICON::FILE
				} ) ;
			}
		}
		bool hasNext() const
		{
			return m_position < m_entries.size() ;
		}
		const QString& value() const
		{
			return m_entries[ m_position ].displayName ;
		}
		const QByteArray& nativeName() const
		{
			return m_entries[ m_position ].nativeName ;
		}
		directoryEntries::ICON icon() const
		{
			return m_entries[ m_position ].icon ;
		}
		quint64 generation() const
		{
			return m_generation ;
		}
		iter next() const
		{
			auto m = *this ;
			m.m_position++ ;
			return m ;
		}
	private:
		size_t m_position = 0 ;
		quint64 m_generation = 0 ;
		std::vector< snapshot > m_entries ;
	} ;

	void join( bool folderFirst )
	{
		if( folderFirst ){

			m_joined.clear() ;

			for( const auto& it : m_folders ){

				m_joined.emplace_back( it ) ;
			}
			for( const auto& it : m_files ){

				m_joined.emplace_back( it ) ;
			}
		}else{
			// Restore the last globally sorted mixed sequence. This makes toggling
			// folder grouping reversible without requiring another sort operation.
			m_joined = m_globalJoined ;
		}
	}

	directoryEntries::iter Iter( quint64 generation = 0 )
	{
		// Queue-safe iterators own an immutable snapshot. They never retain
		// pointers into m_joined, which is replaced/reordered by later scans.
		return { m_joined,generation } ;
	}
} ;

Q_DECLARE_METATYPE( directoryEntries::iter )

namespace directoryManager
{
	directoryEntries readAll( const QString&,std::atomic_bool& ) ;

	directoryEntries readAll( const QString& ) ;

	void removeDirectoryContents( const QString&,std::atomic_bool& ) ;

	void removeDirectory( const QString&,std::atomic_bool& ) ;

#ifdef Q_OS_UNIX
	// POSIX filenames are byte strings, not guaranteed UTF-8. These entry
	// points keep an already-captured native path byte-for-byte intact.
	directoryEntries readAllNative( const QByteArray&,std::atomic_bool& ) ;
	void removeDirectoryContentsNative( const QByteArray&,std::atomic_bool& ) ;
	void removeDirectoryNative( const QByteArray&,std::atomic_bool& ) ;
#endif
}

#endif
