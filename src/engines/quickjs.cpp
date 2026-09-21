/*
 *
 *  Copyright (c) 2025
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

#include "quickjs.h"

utility::addJsonCmd::entry::args quickjs::entryCmd( const QString& e )
{
	utility::addJsonCmd::entry::args data ;

	if( e == "Windows" ){

		data.emplace_back( "win7x86","qjs.exe" ) ;
		data.emplace_back( "win7amd64","qjs.exe" ) ;
		data.emplace_back( "x86","qjs.exe" ) ;
		data.emplace_back( "amd64","qjs.exe" ) ;

	}else{
		data.emplace_back( "x86","qjs" ) ;
		data.emplace_back( "aarch64","qjs" ) ;
		data.emplace_back( "amd64","qjs" ) ;
	}

	return data ;
}

QJsonObject quickjs::init( Logger& logger,const engines::enginePaths& enginePath )
{
	auto m = enginePath.enginePath( "quickjs.json" ) ;

	if( QFile::exists( m ) ){

		return QJsonObject() ;
	}

	QJsonObject mainObj ;

	utility::addJsonCmd json( mainObj ) ;

	json.add( "Generic",quickjs::entryCmd ) ;

	json.add( "Windows",quickjs::entryCmd ) ;

	json.done() ;

	mainObj.insert( "Version","1" ) ;

	// Upstream LATEST metadata does not publish a trusted SHA-256 for the
	// selected archive. Do not offer an update that mandatory verification
	// would have to reject.
	mainObj.insert( "DownloadUrl","" ) ;

	mainObj.insert( "DownloadUrlWin7","" ) ;

	mainObj.insert( "AutoUpdate",false ) ;

	mainObj.insert( "Name","quickjs" ) ;

	mainObj.insert( "VersionArgument","--version" ) ;

	mainObj.insert( "BackendPath",utility::stringConstants::defaultPath() ) ;

	mainObj.insert( "VersionStringLine",1 ) ;

	mainObj.insert( "VersionStringPosition",2 ) ;

	mainObj.insert( "LikeYoutubeDl",false ) ;

	engines::file( m,logger ).write( mainObj ) ;

	return mainObj ;
}

void quickjs::remove( Logger&,const engines::enginePaths& enginePath )
{
	auto m = enginePath.enginePath( "quickjs.json" ) ;

	if( QFile::exists( m ) ){

		QFile::remove( m ) ;
	}

	m = enginePath.binPath( utility::platformIsWindows() ? "qjs.exe" : "qjs" ) ;

	if( QFile::exists( m ) ){

		QFile::remove( m ) ;
	}
}

quickjs::~quickjs()
{
}

QString quickjs::namePrefix()
{
	const utility::CPU cpu ;

	// Bellard's native OS-specific QuickJS binary archives currently expose
	// i686/x86_64 naming. ARM64 is packaged separately (Cosmopolitan), so do
	// not silently install an x86_64 archive as though it were native ARM64.
	if( cpu.aarch64() ){
		return {} ;
	}

	QString platform = utility::platformIsWindows() ? "win" : "linux" ;
	QString arch     = cpu.x86_32() ? "-i686" : "-x86_64" ;

	return "quickjs-" + platform + arch ;
}

QString quickjs::urlFileName( const QString& version )
{
	const auto prefix = this->namePrefix() ;

	return prefix.isEmpty() ? QString() : prefix + "-" + version + ".zip" ;
}

engines::metadata quickjs::parseJsonDataFromGitHub( const QJsonDocument& e )
{
	auto version = e.object().value( "version" ).toString() ;
	const auto prefix = this->namePrefix() ;

	if( !version.isEmpty() &&
	    !prefix.isEmpty() &&
	    ( utility::platformIsLinux() || utility::platformIsWindows() ) ){

		auto fileName = QString( "%1-%2.zip" ).arg( prefix,version ) ;
		auto url      = "https://bellard.org/quickjs/binary_releases/" + fileName ;

		QJsonObject obj ;

		obj.insert( "browser_download_url",url ) ;
		obj.insert( "tag_name",version ) ;
		obj.insert( "name",fileName ) ;
		obj.insert( "digest","" ) ;
		obj.insert( "size",0 ) ;

		return obj ;
	}else{
		return {} ;
	}
}

engines::engine::baseEngine::removeFilesStatus quickjs::removeFiles( const QStringList& e,const QString& a )
{
	auto m = e ;

	if( utility::platformIsLinux() ){

		m.append( a + "/run-test262" ) ;

		return engines::engine::baseEngine::removeFiles( m,a ) ;
	}else{
		m.append( a + "/libwinpthread-1.dll" ) ;

		return engines::engine::baseEngine::removeFiles( m,a ) ;
	}
}

bool quickjs::foundNetworkUrl( const QString& s )
{
	const auto prefix = this->namePrefix() ;

	return !prefix.isEmpty() && s.startsWith( prefix ) && s.endsWith( ".zip" ) ;
}

QString quickjs::parseVersionInfo( const utils::qprocess::outPut& r )
{
	auto s = util::split( r.stdOut,'\n' ) ;

	if( s.size() ){

		auto e = s[ 0 ] + "\n" + s[ 0 ] ;

		e.replace( "-","." ) ;

		return e ;
	}else{
		return {} ;
	}
}

quickjs::quickjs( const engines& e,const engines::engine& s,QJsonObject& obj ) :
	engines::engine::baseEngine( e.Settings(),s,e.processEnvironment() )
{
	// Persisted definitions from older builds may still carry the unsigned
	// feed. Force the runtime view fail-closed until a trusted digest exists.
	obj.insert( "DownloadUrl","" ) ;
	obj.insert( "AutoUpdate",false ) ;
	if( utility::platformisFlatPak() ){

		auto path = e.Settings().flatpakIntance().appDataLocation() + "/bin/qjs" ;

		if( QFile::exists( path ) ){

			QFile::remove( path ) ;
		}
	}
}
