/*
 *
 *
 *  Copyright (c) 2022
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
#include "tests.h"
#include "../utils/single_instance.hpp"

#include "wget.h"
#include "yt-dlp.h"
#include "safaribooks.h"
#include "gallery-dl.h"
#include "svtplay-dl.h"
#include "lux.h"
#include "getsauce.h"

#include "../util.hpp"
#include "../utility.h"
#include "../directoryEntries.h"

#include <iostream>
#include <array>
#include <atomic>

#include <QString>
#include <QEventLoop>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>
#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#define TEST_ENGINE_PREFIX "--media-downloader-test-engine"

#if QT_VERSION >= QT_VERSION_CHECK( 5,10,0 )

#include <QRandomGenerator>

static int _getInterval( int x,int y )
{
	return QRandomGenerator::global()->bounded( x,y ) ;
}

#else

#include <random>

static int _getInterval( int x,int y )
{
	std::random_device rd ;
	std::mt19937 gen( rd() ) ;
	std::uniform_real_distribution<> dis( x,y ) ;
	return dis( gen ) ;
}

#endif


class Tests
{
public:
	struct engines
	{
		const char * arg ;
		const char *( *data )( void ) ;
	} ;
	auto begin() const
	{
		return m_engines.begin() ;
	}
	auto end() const
	{
		return m_engines.end() ;
	}
private:
	std::array< engines,10 > m_engines = { {
		{ TEST_ENGINE_PREFIX"-safaribooks",&safaribooks::testData },
		{ TEST_ENGINE_PREFIX"-yt-dlp",&yt_dlp::testYtDlp },
		{ TEST_ENGINE_PREFIX"-yt-dlp-playlist",&yt_dlp::testYtDlpPlayList },
		{ TEST_ENGINE_PREFIX"-yt-dlp-metadata",&yt_dlp::testYtDlpMetadata },
		{ TEST_ENGINE_PREFIX"-yt-dlp-ffmpeg",&yt_dlp::testFfmpeg },
		{ TEST_ENGINE_PREFIX"-wget",&wget::testData },
		{ TEST_ENGINE_PREFIX"-gallery-dl",&gallery_dl::testData },
		{ TEST_ENGINE_PREFIX"-getsauce",&getsauce::testData },
		{ TEST_ENGINE_PREFIX"-svtplay-dl",&svtplay_dl::testData },
		{ TEST_ENGINE_PREFIX"-lux",&lux::testData } } } ;
} ;

class testing
{
public:
	struct args
	{
		const QStringList& args ;
		QApplication& app ;
	} ;
	testing( const testing::args& args ) : m_args( args )
	{
	}
	void start( const QByteArray& )
	{
		for( const auto& arg : m_args.args ){
			if( arg == TEST_ENGINE_PREFIX"-proxy-security" ){
				return this->testProxySecurity() ;
			}
			if( arg == TEST_ENGINE_PREFIX"-library-filesystem-boundary" ){
				return this->testLibraryFilesystemBoundary() ;
			}
		}

		Tests tests ;

		QString s ;

		for( const auto& it : m_args.args ){

			for( const auto& xt : tests ){

				auto m = it.indexOf( "0xdeadbeef" ) ;

				if( m == -1 ){

					s = it ;
				}else{
					s = it.mid( 0,m ) ;
				}

				if( s == xt.arg ){

					return this->testEngine( xt.data() ) ;
				}
			}
		}

		this->done() ;
	}
	void done()
	{
		m_args.app.quit() ;
	}
	void testEngine( const char * output )
	{
		m_list = util::split( output,'\n' ) ;

		util::Timer( _getInterval( 100,600 ),[ this ]( int ){

			if( m_counter < m_list.size() ){

				std::cout << m_list.at( m_counter ).constData() << std::endl ;

				m_counter++ ;

				return false ;
			}else{
				this->done() ;

				return true ;
			}
		} ) ;
	}
	void testLibraryFilesystemBoundary()
	{
		QTemporaryDir temp ;
		bool ok = temp.isValid() ;
		const auto base = temp.path() ;
		const auto root = base + "/root" ;
		const auto nested = root + "/nested" ;
		const auto outside = base + "/outside" ;
		const auto outsideFile = outside + "/sentinel.txt" ;
		const auto outsideDirectoryFile = outside + "/directory-sentinel.txt" ;

		QDir().mkpath( nested ) ;
		QDir().mkpath( outside ) ;
		{
			QFile f( outsideFile ) ;
			ok = ok && f.open( QIODevice::WriteOnly ) && f.write( "sentinel" ) == 8 ;
		}
		{
			QFile f( outsideDirectoryFile ) ;
			ok = ok && f.open( QIODevice::WriteOnly ) && f.write( "directory-sentinel" ) == 18 ;
		}

		const auto directoryLink = nested + "/external-directory" ;
		const auto fileLink = nested + "/external-file" ;
		bool directoryLinked = false ;
		bool fileLinked = false ;
#ifdef Q_OS_WIN
		const DWORD allowUnprivilegedCreate = 0x2 ;
		auto nativeDirectoryLink = QDir::toNativeSeparators( directoryLink ).toStdWString() ;
		auto nativeOutside = QDir::toNativeSeparators( outside ).toStdWString() ;
		auto nativeFileLink = QDir::toNativeSeparators( fileLink ).toStdWString() ;
		auto nativeOutsideFile = QDir::toNativeSeparators( outsideFile ).toStdWString() ;
		directoryLinked = CreateSymbolicLinkW( nativeDirectoryLink.c_str(),nativeOutside.c_str(),
			SYMBOLIC_LINK_FLAG_DIRECTORY | allowUnprivilegedCreate ) != 0 ;
		fileLinked = CreateSymbolicLinkW( nativeFileLink.c_str(),nativeOutsideFile.c_str(),
			allowUnprivilegedCreate ) != 0 ;
#else
		directoryLinked = QFile::link( outside,directoryLink ) ;
		fileLinked = QFile::link( outsideFile,fileLink ) ;
#endif

		ok = ok && directoryLinked && fileLinked ;
		if( directoryLinked && fileLinked ){
			std::atomic_bool keepGoing{ true } ;
			directoryManager::removeDirectory( root,keepGoing ) ;
			ok = ok && QFileInfo::exists( outsideFile ) && QFileInfo::exists( outsideDirectoryFile ) ;
			ok = ok && !QFileInfo::exists( root ) ;
		}

		const auto renameRoot = base + "/rename" ;
		QDir().mkpath( renameRoot ) ;
		const auto source = renameRoot + "/source.txt" ;
		const auto collision = renameRoot + "/collision.txt" ;
		{
			QFile f( source ) ;
			ok = ok && f.open( QIODevice::WriteOnly ) && f.write( "source" ) == 6 ;
		}
		{
			QFile f( collision ) ;
			ok = ok && f.open( QIODevice::WriteOnly ) && f.write( "collision" ) == 9 ;
		}

		QString destination ;
		QString error ;
		ok = ok && !utility::libraryRenameDestination( renameRoot,"../outside.txt",destination,error ) ;
		ok = ok && !utility::libraryRenameDestination( renameRoot,"nested/name.txt",destination,error ) ;
		ok = ok && !utility::libraryRenameDestination( renameRoot,"nested\\name.txt",destination,error ) ;
		ok = ok && !utility::libraryRenameDestination( renameRoot,QDir( base ).absoluteFilePath( "absolute.txt" ),destination,error ) ;
		ok = ok && !utility::libraryRenameDestination( renameRoot,"collision.txt",destination,error ) ;
		ok = ok && utility::libraryRenameDestination( renameRoot,"renamed.txt",destination,error ) ;
		if( ok ){
			ok = utility::rename( source,destination ).isEmpty() ;
			ok = ok && QFileInfo::exists( destination ) && QFileInfo::exists( collision ) ;
		}

		if( ok ){
			std::cout << "library-filesystem-boundary=PASS" << std::endl ;
			m_args.app.exit( 0 ) ;
		}else{
			std::cerr << "library-filesystem-boundary=FAIL"
				<< " directoryLinked=" << directoryLinked
				<< " fileLinked=" << fileLinked << std::endl ;
			m_args.app.exit( 1 ) ;
		}
	}

	void testProxySecurity()
	{
		const QString secret = "DistinctiveProxySecret077" ;
		const QString proxy = "http://proxy-user:" + secret + "@proxy.example:8080" ;
		engines::engine::baseEngine::optionsEnvironment environment ;
		QStringList arguments ;
		wget::applyProxySetting( environment,arguments,proxy ) ;

		QString diagnostics ;
		const auto childEnvironment = environment.update( QProcessEnvironment(),diagnostics ) ;
		const auto httpProxy = childEnvironment.value( "http_proxy" ) ;
		const auto httpsProxy = childEnvironment.value( "https_proxy" ) ;
		const auto renderedPassword = engines::redactLogArgument( "--proxy-password=" + secret ) ;

		bool argvClean = true ;
		for( const auto& argument : arguments ){
			if( argument.contains( secret ) || argument.startsWith( "--proxy-password=" ) ){
				argvClean = false ;
			}
		}
		const bool childReceivesSecret = httpProxy.contains( secret ) && httpsProxy.contains( secret ) ;
		const bool diagnosticsClean = !diagnostics.contains( secret ) &&
			diagnostics.contains( "<REDACTED>" ) &&
			!renderedPassword.contains( secret ) &&
			renderedPassword.contains( "<REDACTED>" ) ;

		if( argvClean && childReceivesSecret && diagnosticsClean ){
			std::cout << "proxy-security=PASS" << std::endl ;
			m_args.app.exit( 0 ) ;
		}else{
			std::cerr << "proxy-security=FAIL argvClean=" << argvClean
				<< " childReceivesSecret=" << childReceivesSecret
				<< " diagnosticsClean=" << diagnosticsClean << std::endl ;
			m_args.app.exit( 1 ) ;
		}
	}
private:
	QList< QByteArray > m_list ;
	testing::args m_args ;
	int m_counter = 0 ;
} ;

static bool _run_test( const QStringList& args )
{
	for( const auto& it : args ){

		if( it.startsWith( TEST_ENGINE_PREFIX ) ){

			return true ;
		}
	}

	return false ;
}

bool tests::test_engine( const QStringList& args,QApplication& app )
{
	if( _run_test( args ) ){

		utils::app::appInfo< testing,testing::args > mm( { args,app },"",app,"" ) ;

		utils::app::runMultiInstances( std::move( mm ) ) ;

		return true ;
	}else{
		return false ;
	}
}
