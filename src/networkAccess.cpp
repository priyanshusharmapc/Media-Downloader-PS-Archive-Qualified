/*
 *
 *  Copyright (c) 2021
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

#include "networkAccess.h"

#include "networkAccess.h"
#include "tabmanager.h"
#include "basicdownloader.h"
#include "settings.h"
#include "utils/threads.hpp"
#include "context.hpp"

#include <QFile>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDateTime>
#include <QUrl>
#include <QUuid>

#include <chrono>

#if QT_VERSION >= QT_VERSION_CHECK( 5,4,0 )
static QString _sslLibraryVersionString()
{
	return QSslSocket::sslLibraryBuildVersionString() ;
}
#else
static QString _sslLibraryVersionString()
{
	return {} ;
}
#endif

namespace
{
bool updatePathExists( const QString& path )
{
    const QFileInfo info( path ) ;
    return info.exists() || info.isSymLink() ;
}

QString removeUpdatePath( const QString& path )
{
    if( !updatePathExists( path ) )return {} ;
    const QFileInfo info( path ) ;
    return info.isDir() && !info.isSymLink() ? utility::removeFolder( path ) : utility::removeFile( path ) ;
}

QString uniqueUpdateSibling( const QString& path,const QString& tag )
{
    return path + "." + tag + "-" + QUuid::createUuid().toString( QUuid::WithoutBraces ) ;
}

QString restoreUpdateBackup( const QString& backup,const QString& destination )
{
    if( backup.isEmpty() || !updatePathExists( backup ) )return {} ;
    const auto cleanup = removeUpdatePath( destination ) ;
    if( !cleanup.isEmpty() )return QObject::tr( "Failed to remove incomplete update at %1: %2" ).arg( destination,cleanup ) ;
    const auto restore = utility::rename( backup,destination ) ;
    if( !restore.isEmpty() )return QObject::tr( "Failed to restore previous engine at %1: %2" ).arg( destination,restore ) ;
    return {} ;
}

// Promote one already-validated staged path. The live destination is moved to
// a sibling backup only at the final commit boundary. If promotion fails, the
// previous payload is restored before the attempt is reported as failed.
QString promoteUpdatePath( const QString& staged,const QString& destination,QString* cleanupWarning = nullptr )
{
    if( cleanupWarning )cleanupWarning->clear() ;
    if( !updatePathExists( staged ) )return QObject::tr( "Staged engine payload is missing: %1" ).arg( staged ) ;

    QString backup ;
    if( updatePathExists( destination ) ){
        backup = uniqueUpdateSibling( destination,"mdps-update-backup" ) ;
        const auto save = utility::rename( destination,backup ) ;
        if( !save.isEmpty() )return QObject::tr( "Failed to preserve previous engine before update: %1" ).arg( save ) ;
    }

    const auto promote = utility::rename( staged,destination ) ;
    if( !promote.isEmpty() ){
        const auto rollback = restoreUpdateBackup( backup,destination ) ;
        return rollback.isEmpty()
            ? QObject::tr( "Failed to promote staged engine: %1" ).arg( promote )
            : QObject::tr( "Failed to promote staged engine: %1; rollback also failed: %2" ).arg( promote,rollback ) ;
    }

    if( !backup.isEmpty() ){
        const auto cleanup = removeUpdatePath( backup ) ;
        if( cleanupWarning && !cleanup.isEmpty() )*cleanupWarning = QObject::tr( "Updated engine is active but old backup cleanup failed: %1" ).arg( cleanup ) ;
    }
    return {} ;
}

struct UpdateMove
{
    QString destination ;
    QString backup ;
    bool promoted = false ;
};

// Archive extraction completes in a private directory first. Top-level entries
// are then committed as one rollback-capable batch. Existing payloads remain in
// backup until every staged entry has been promoted successfully.
QString promoteUpdateDirectoryContents( const QString& stageRoot,const QString& liveRoot,QString* cleanupWarning = nullptr )
{
    if( cleanupWarning )cleanupWarning->clear() ;
    QDir stage( stageRoot ) ;
    if( !stage.exists() )return QObject::tr( "Update staging directory is missing: %1" ).arg( stageRoot ) ;

    const auto entries = stage.entryInfoList( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System,QDir::Name ) ;
    if( entries.isEmpty() )return QObject::tr( "Extracted update contains no payload" ) ;

    const auto backupRoot = QDir( liveRoot ).filePath( ".mdps-update-backup-" + QUuid::createUuid().toString( QUuid::WithoutBraces ) ) ;
    QVector< UpdateMove > moves ;
    QString failure ;

    for( const auto& entry : entries ){
        const auto source = entry.filePath() ;
        const auto destination = QDir( liveRoot ).filePath( entry.fileName() ) ;
        UpdateMove move ; move.destination = destination ;

        if( updatePathExists( destination ) ){
            if( !QDir().mkpath( backupRoot ) ){
                failure = QObject::tr( "Unable to create update rollback directory: %1" ).arg( backupRoot ) ;
                moves.append( move ) ;
                break ;
            }
            move.backup = QDir( backupRoot ).filePath( entry.fileName() ) ;
            const auto save = utility::rename( destination,move.backup ) ;
            if( !save.isEmpty() ){
                failure = QObject::tr( "Failed to preserve existing engine entry %1: %2" ).arg( destination,save ) ;
                moves.append( move ) ;
                break ;
            }
        }

        const auto promote = utility::rename( source,destination ) ;
        if( !promote.isEmpty() ){
            failure = QObject::tr( "Failed to promote extracted engine entry %1: %2" ).arg( destination,promote ) ;
            moves.append( move ) ;
            break ;
        }
        move.promoted = true ;
        moves.append( move ) ;
    }

    if( !failure.isEmpty() ){
        QStringList rollbackErrors ;
        for( auto it = moves.crbegin(); it != moves.crend(); ++it ){
            if( it->promoted ){
                const auto remove = removeUpdatePath( it->destination ) ;
                if( !remove.isEmpty() )rollbackErrors << remove ;
            }
            if( !it->backup.isEmpty() && updatePathExists( it->backup ) ){
                const auto restore = utility::rename( it->backup,it->destination ) ;
                if( !restore.isEmpty() )rollbackErrors << restore ;
            }
        }
        if( rollbackErrors.isEmpty() ){
            removeUpdatePath( backupRoot ) ;
        }else{
            // Never delete forensic recovery material when restoration failed.
            // The backup directory is intentionally retained for manual repair.
            failure += QObject::tr( "; rollback errors: %1; previous payload retained at %2" )
                .arg( rollbackErrors.join( "; " ),backupRoot ) ;
        }
        return failure ;
    }

    const auto backupCleanup = removeUpdatePath( backupRoot ) ;
    const auto stageCleanup = removeUpdatePath( stageRoot ) ;
    if( cleanupWarning ){
        QStringList warnings ;
        if( !backupCleanup.isEmpty() )warnings << backupCleanup ;
        if( !stageCleanup.isEmpty() )warnings << stageCleanup ;
        *cleanupWarning = warnings.join( "; " ) ;
    }
    return {} ;
}
}

networkAccess::networkAccess( const Context& ctx ) :
	m_ctx( ctx ),
	m_network( m_ctx.Settings().networkTimeOut() ),
	m_basicdownloader( m_ctx.TabManager().basicDownloader() ),
	m_tabManager( m_ctx.TabManager() ),
	m_appName( m_ctx.appName() )
{
	auto& settings = m_ctx.Settings() ;

	auto m = settings.showLocalAndLatestVersionInformation() ;
	auto e = settings.showVersionInfoAndAutoDownloadUpdates() ;
	auto s = settings.showLocalVersionInformationOnly() ;

	if( utility::platformIsWindows() && ( m || e || s ) ){

		auto& e = m_ctx.logger() ;
		auto s = QSslSocket::sslLibraryVersionString() ;

		auto id = utility::loggerID() ;

		auto mm = QObject::tr( "Checking installed version of %1" ) ;

		QStringList list ;

		if( utility::Qt6Version() ){

			list.append( mm.arg( QObject::tr( "Windows' Secure Channel" ) ) ) ;

			if( !s.isEmpty() ){

				s = util::split( s," " ).last() ;
			}
		}else{
			list.append( mm.arg( "OpenSSL" ) ) ;
		}

		if( s.isEmpty() ){

			for( const auto& it : list ){

				e.add( it,id ) ;
			}

			auto q = _sslLibraryVersionString() ;
			auto m = QObject::tr( "Failed to find version information, make sure \"%1\" is installed and works properly" ).arg( q ) ;
			e.add( m,id ) ;
		}else{
			list.append( QObject::tr( "Found version" ) + ": " + s ) ;

			if( utility::cliArguments::debug() ){

				for( const auto& it : list ){

					e.add( it,id ) ;
				}
			}
		}
	}
}

void networkAccess::updateMediaDownloader( networkAccess::Status status,const QJsonDocument& json ) const
{
	class meaw
	{
	public:
		meaw( bool Qt6 ) : m_name( Qt6 ? "MediaDownloaderQt6" : "MediaDownloaderQt5" )
		{
		}
		bool operator()( const QJsonObject& obj )
		{
			const auto url = obj.value( "browser_download_url" ).toString() ;
			const QUrl parsed( url ) ;
			const auto expectedPrefix = QString(
				"/priyanshusharmapc/Media-Downloader-PS-Archive-Qualified/releases/download/" ) ;

			// Asset identity is part of the update trust boundary. A release JSON
			// object may name arbitrary URLs, so reject anything outside this
			// repository before a byte is staged or extracted.
			if( parsed.scheme().compare( "https",Qt::CaseInsensitive ) != 0 ||
			    parsed.host().compare( "github.com",Qt::CaseInsensitive ) != 0 ||
			    !parsed.path().startsWith( expectedPrefix ) ){
				return false ;
			}

			return url.contains( m_name ) && url.endsWith( ".zip",Qt::CaseInsensitive ) ;
		}
	private:
		QString m_name ;
	} ;

	auto obj = utility::parseJsonDataFromGitHub( json,meaw( utility::Qt6Version() ) ) ;

	if( obj.isEmpty() ){

		status.done() ;

		auto m = QObject::tr( "Failed to parse json file from github" ) ;

		this->post( m_appName,m,status.id() ) ;

		m_tabManager.enableAll() ;
	}else{
		this->updateMediaDownloader( networkAccess::updateMDOptions( obj,status.move() ) ) ;
	}
}

void networkAccess::updateMediaDownloader( networkAccess::Status status ) const
{
	this->postStartDownloading( m_appName,status.id() ) ;

	m_tabManager.disableAll() ;

	m_basicdownloader.setAsActive().enableQuit() ;

	auto u = this->networkRequest( m_ctx.Settings().gitHubDownloadUrl() ) ;

	this->get( u,status.move(),this,&networkAccess::uMediaDownloaderN ) ;
}

void networkAccess::uMediaDownloaderN( networkAccess::Status& status,
				       const utils::network::progress& p ) const
{
	if( p.finished() ){

		if( p.success() ){

			auto e = utility::jsonDoc( p.data() ) ;

			if( e.valid() ){

				this->updateMediaDownloader( status.move(),e.get() ) ;
			}else{
				status.done() ;

				auto mm = QObject::tr( "Download Failed" ) ;

				mm += ": " + e.errorString() ;

				this->post( m_appName,mm,status.id() ) ;

				m_tabManager.enableAll() ;
			}
		}else{
			status.done() ;

			this->post( m_appName,this->reportError( p ),status.id() ) ;

			m_tabManager.enableAll() ;
		}
	}else{
		this->post( m_appName,"...",status.id() ) ;
	}
}

void networkAccess::uMediaDownloaderM( networkAccess::updateMDOptions& md,
				       const utils::network::progress& p ) const
{
	if( p.finished() ){

		md.file.close() ;

		if( md.file.writeFailed() ){
			md.status.done() ;
			this->post( m_appName,QObject::tr( "Download Failed: could not persist complete payload: %1" ).arg( md.file.writeError() ),md.id ) ;
			utility::removeFile( md.tmpFile ) ;
			m_tabManager.enableAll() ;
			return ;
		}

		if( p.success() ){			

			if( md.hash.isEmpty() ){

				auto m = QObject::tr( "Skipping Checking Download Hash" ) ;

				this->post( m_appName,m,md.id ) ;

				this->extractMediaDownloader( md.move() ) ;
			}else{
				auto m = md.hashCalculator->result().toHex().toLower() ;

				if( utility::cliArguments::useFakeMdHash() ){

					m = "bogusHashValue" ;
				}

				if( md.hash == m ){

					this->extractMediaDownloader( md.move() ) ;
				}else{
					this->hashDoNotMatch( md.hash,m,md.id ) ;

					md.status.done() ;

					m_tabManager.enableAll() ;
				}
			}
		}else{
			md.status.done() ;

			this->post( m_appName,this->reportError( p ),md.id ) ;

			m_tabManager.enableAll() ;
		}
	}else{
		auto data = p.data() ;

		if( md.file.write( data ) ){
			// The digest must describe bytes accepted by the file device, not
			// merely bytes delivered by the network stack.
			md.hashCalculator->addData( data ) ;
		}

		auto speed = md.speed.calculate( p ) ;

		this->postDownloadingProgress( m_appName,speed,md.id ) ;
	}
}

void networkAccess::updateMediaDownloader( networkAccess::updateMDOptions md ) const
{
	auto e = m_ctx.Engines().engineDirPaths().tmp( md.name ) ;

	md.tmpFile = QDir::fromNativeSeparators( e ) ;

	this->postDownloading( m_appName,md.url,md.id ) ;

	this->postDestination( m_appName,md.tmpFile,md.id ) ;

	auto url = this->networkRequest( md.url ) ;

	if( md.file.open( md.tmpFile ) ){

		md.speed.setInitialTimeStamp() ;

		this->get( url,md.move(),this,&networkAccess::uMediaDownloaderM ) ;
	}else{
		auto bar = utility::barLine() ;

		auto m = QObject::tr( "Failed To Open Path For Writing: %1" ).arg( md.tmpFile ) ;

		this->post( m_appName,bar,md.id ) ;
		this->post( m_appName,m,md.id ) ;
		this->post( m_appName,"Err: " + utility::errorMessage(),md.id ) ;
		this->post( m_appName,bar,md.id ) ;

		md.status.done() ;

		m_ctx.TabManager().enableAll() ;
	}
}

void networkAccess::emDownloader( networkAccess::updateMDOptions md,const utils::qprocess::outPut& s ) const
{
	auto mm = utility::removeFile( md.tmpFile ) ;

	if( !mm.isEmpty() ){

		this->failedToRemove( m_appName,md.tmpFile,mm,md.id ) ;
	}

	if( s.success() ){

		auto mm = md.name ;

		auto extractedPath = md.tmpPath + "/" + mm.mid( 0,mm.size() - 4 ) ;

		auto e = utility::rename( extractedPath,md.finalPath ) ;

		if( e.isEmpty() ){

			QFile f( md.finalPath + "/media-downloader.exe" ) ;

			f.setPermissions( f.permissions() | QFileDevice::ExeOwner ) ;

			QDir().rmdir( md.finalPath + "/local" ) ;

			md.status.done() ;

			auto m = QObject::tr( "Update Complete, Restart To Use New Version" ) ;

			this->post( m_appName,m,md.id ) ;
		}else{
			md.status.done() ;

			this->failedToRename( md.name,extractedPath,md.finalPath,e,md.id ) ;
		}
	}else{
		md.status.done() ;

		this->failedToExtract( md.exeArgs,s,md.id ) ;
	}
}

void networkAccess::failedToExtract( const networkAccess::cmdArgs& e,
				     const utils::qprocess::outPut& s,
				     int id ) const
{
	this->post( m_appName,utility::barLine(),id ) ;

	this->post( m_appName,QObject::tr( "Failed To Extract" ),id ) ;
	this->post( m_appName,"Exe Path: " + e.exe(),id ) ;
	this->post( m_appName,"Exe Args: " + e.args(),id ) ;
	this->post( m_appName,"StdOut: "   + s.stdOut,id ) ;
	this->post( m_appName,"StdError: " + s.stdError,id ) ;

	this->post( m_appName,utility::barLine(),id ) ;
}

void networkAccess::failedToRemove( const QString& name,
				    const engines::engine::baseEngine::removeFilesStatus& err,
				    int id ) const
{
	this->post( m_appName,utility::barLine(),id ) ;
	this->post( name,QObject::tr( "Failed To Remove" ),id ) ;

	for( const auto& it : err ){

		this->post( name,"Src: " + it.src(),id ) ;
		this->post( name,"Err: " + it.err(),id ) ;
	}

	this->post( m_appName,utility::barLine(),id ) ;
}

void networkAccess::failedToRemove( const QString& name,
				    const QString& src,
				    const QString& err,
				    int id ) const
{
	this->failedToRemove( name,{ src,err },id ) ;
}

void networkAccess::failedToRename( const QString& name,
				    const QString& src,
				    const QString& dst,
				    const QString& err,
				    int id ) const
{
	this->post( m_appName,utility::barLine(),id ) ;
	this->post( name,QObject::tr( "Failed To Rename" ),id ) ;
	this->post( name,"Src: " + src,id ) ;
	this->post( name,"Dst: " + dst,id ) ;
	this->post( name,"Err: " + err,id ) ;
	this->post( m_appName,utility::barLine(),id ) ;
}

void networkAccess::extractMediaDownloader( networkAccess::updateMDOptions md ) const
{
	this->post( m_appName,QObject::tr( "Extracting archive: " ) + md.tmpFile,md.id ) ;

	const auto& paths = m_ctx.Engines().engineDirPaths() ;

	md.tmpPath = paths.basePath() ;

	md.finalPath = paths.updateNewPath() ;

	class meaw
	{
	public:
		meaw( const networkAccess& na,networkAccess::updateMDOptions md ) :
			m_parent( na ),m_md( md.move() )
		{
		}
		void bg()
		{
			m_err = utility::removeFolder( m_md.finalPath ) ;
		}
		void fg()
		{
			if( !m_err.isEmpty() ){

				m_parent.failedToRemove( m_parent.m_appName,m_md.finalPath,m_err,m_md.id ) ;
			}

			auto exe = m_parent.m_ctx.Engines().findExecutable( "bsdtar.exe" ) ;

			auto args = QStringList{ "-x","-f",m_md.tmpFile,"-C",m_md.tmpPath } ;

			auto m = QProcess::MergedChannels ;

			m_md.exeArgs = { exe,args } ;

			utils::qprocess::run( exe,args,m,m_md.move(),&m_parent,&networkAccess::emDownloader ) ;
		}
	private:
		QString m_err ;
		const networkAccess& m_parent ;
		networkAccess::updateMDOptions m_md ;
	} ;

	utils::qthread::run( meaw( *this,md.move() ) ) ;
}

QNetworkRequest networkAccess::networkRequest( const QString& url,const QByteArray& userAgent,const QByteArray& referer ) const
{
	QNetworkRequest networkRequest( url ) ;
#if QT_VERSION >= QT_VERSION_CHECK( 5,9,0 )
	auto a = QNetworkRequest::RedirectPolicyAttribute ;
	auto b = QNetworkRequest::NoLessSafeRedirectPolicy ;

	networkRequest.setAttribute( a,b ) ;
#else
	#if QT_VERSION >= QT_VERSION_CHECK( 5,6,0 )
		auto c = QNetworkRequest::FollowRedirectsAttribute ;
		networkRequest.setAttribute( c,true ) ;
	#endif
#endif
	if( !userAgent.isEmpty() ){

		networkRequest.setRawHeader( "User-Agent",userAgent ) ;
	}

	if( !referer.isEmpty() ){

		networkRequest.setRawHeader( "referer",referer ) ;
	}

	return networkRequest ;
}

QString networkAccess::timeOutErrorString() const
{
	auto m = QString::number( m_ctx.Settings().networkTimeOut() / 1000 ) ;
	return QObject::tr( "Network Failed To Respond Within %1 seconds" ).arg( m ) ;
}

QByteArray networkAccess::defaultUserAgent() const
{
	return "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36" ;
}

void networkAccess::printVersion( networkAccess::Opts opts,bool s ) const
{
	m_ctx.getVersionInfo().check( opts.moveIter(),s ) ;
}

void networkAccess::download( const QByteArray& data,networkAccess::Opts opts ) const
{
	const auto& engine = opts.engine() ;

	util::Json json( data ) ;

	if( json ){

		opts.add( engine.parseJsonDataFromGitHub( json.doc() ) ) ;

		this->download( opts.move() ) ;
	}else{
		auto m = QObject::tr( "Failed to parse json file from github" ) ;

		this->post( engine.name(),m + ": " + json.errorString(),opts.id ) ;

		m_tabManager.enableAll() ;

		opts.reportFailed() ;

		engine.setBroken() ;

		this->printVersion( opts.move(),false ) ;
	}
}

void networkAccess::download( engines::Iterator e,networkAccess::reportDone rd ) const
{
	networkAccess::iterator iter( e.move(),rd.move() ) ;

	const auto& engine = iter.engine() ;

	auto ee = m_ctx.Engines().engineDirPaths().binPath() ;

	auto exeFolderPath = QDir::fromNativeSeparators( ee ) ;

	auto m = QDir::fromNativeSeparators( engine.exePath().realExe() ) ;

	auto a = m.lastIndexOf( '/' ) ;

	auto exePath = a == -1 ? exeFolderPath + "/" + m : exeFolderPath + "/" + m.mid( a + 1 ) ;

	QDir dir ;

	auto path = engine.exeFolderPath() ;

	int id = utility::loggerID() ;

	if( !dir.exists( path ) ){

		if( !dir.mkpath( path ) ){

			iter.repordFailed() ;

			auto m = QObject::tr( "Failed to download, Following path can not be created: " ) ;

			this->post( engine.name(),m + path,id ) ;

			return ;
		}
	}

	this->postStartDownloading( engine.name(),id ) ;

	m_tabManager.disableAll() ;

	m_basicdownloader.setAsActive().enableQuit() ;

	networkAccess::Opts opts{ iter.move(),exePath,exeFolderPath,id } ;

	auto url = this->networkRequest( engine.downloadUrl() ) ;

	this->get( url,opts.move(),this,&networkAccess::downloadP2 ) ;
}

void networkAccess::downloadP2( networkAccess::Opts& opts,const utils::network::progress& p ) const
{
	const auto& engine = opts.engine() ;

	if( p.finished() ){

		if( p.success() ){

			this->download( p.data(),opts.move() ) ;
		}else{
			this->post( engine.name(),this->reportError( p ),opts.id ) ;

			m_tabManager.enableAll() ;

			opts.reportFailed() ;

			engine.setBroken() ;

			this->printVersion( opts.move(),false ) ;
		}
	}else{
		this->post( engine.name(),"...",opts.id ) ;
	}
}

void networkAccess::download( networkAccess::Opts opts ) const
{
	const auto& engine = opts.engine() ;

	if( opts.metadata.url().isEmpty() || opts.metadata.fileName().isEmpty() ){

		auto a = "Download Failed: Invalid Url or FileName Not Found" ;
		auto b = "Url: " + opts.metadata.url() ;
		auto c = "Metadata File Name: " + opts.metadata.fileName() ;
		auto d = "Online File Name: " + engine.urlFileName( opts.metadata.version() ) ;
		auto s = utility::barLine() ;

		opts.networkError.add( s,a,b,c,d,s ) ;

		return this->finished( opts.move() ) ;
	}

	engine.updateEnginePaths( m_ctx,opts.filePath,opts.exeBinPath,opts.tempPath ) ;

	if( opts.file.open( opts.filePath ) ){

		this->postDownloading( engine.name(),opts.metadata.url(),opts.id ) ;

		this->postDestination( engine.name(),opts.filePath,opts.id ) ;

		auto url = this->networkRequest( opts.metadata.url() ) ;

		opts.speed.setInitialTimeStamp() ;

		this->get( url,opts.move(),this,&networkAccess::downloadP ) ;
	}else{
		auto m = QObject::tr( "Failed To Open Path For Writing: %1" ).arg( opts.filePath ) ;

		// A file-open failure belongs to this engine attempt, not to the
		// startup scan as a whole. Feed it through the same failed-download
		// path as transport errors so versionInfo advances to the next engine.
		opts.reportFailed() ;
		opts.networkError.add( m,"Err: " + utility::errorMessage() ) ;

		this->finished( opts.move() ) ;
	}
}

void networkAccess::downloadP( networkAccess::Opts& opts,const utils::network::progress& p ) const
{
	const auto& engine = opts.engine() ;

	if( p.finished() ){

		opts.file.close() ;

		if( opts.file.writeFailed() ){
			opts.reportFailed() ;
			opts.networkError.add( QObject::tr( "Download Failed: could not persist complete payload: %1" ).arg( opts.file.writeError() ) ) ;
		}else if( p.success() ){

			if( opts.metadata.hash().isEmpty() ){

				auto m = QObject::tr( "Skipping Checking Download Hash" ) ;

				this->post( m_appName,m,opts.id ) ;
			}else{
				auto m = opts.hashCalculator->result().toHex().toLower() ;

				if( opts.metadata.hash() != m ){

					this->hashDoNotMatch( opts.metadata.hash(),m,opts.id ) ;

					opts.reportFailed() ;

					opts.networkError.setbadDownload() ;
				}
			}
		}else{
			opts.reportFailed() ;

			opts.networkError = this->reportError( p ) ;
		}

		this->finished( opts.move() ) ;
	}else{
		auto data = p.data() ;

		if( opts.file.write( data ) ){
			opts.hashCalculator->addData( data ) ;
		}

		auto speed = opts.speed.calculate( p ) ;

		this->postDownloadingProgress( engine.name(),speed,opts.id ) ;
	}
}

void networkAccess::finished( networkAccess::Opts opts ) const
{
	const auto& engine = opts.engine() ;

	if( opts.networkError.isNotEmpty() ){

		for( const auto& it : opts.networkError ){

			this->post( engine.name(),it,opts.id ) ;
		}

		m_tabManager.enableAll() ;

		engine.setBroken() ;

		this->printVersion( opts.move(),opts.networkError.badDownload() ) ;
	}else{
		this->post( engine.name(),QObject::tr( "Download complete" ),opts.id ) ;

		if( opts.isArchive ){

			this->extractArchive( opts.move() ) ;
		}else{
			auto mm = QObject::tr( "Renaming file to: %1" ).arg( opts.exeBinPath ) ;

			this->post( engine.name(),mm,opts.id ) ;

            QString cleanupWarning ;
            const auto m = promoteUpdatePath( opts.file.src(),opts.exeBinPath,&cleanupWarning ) ;

            if( m.isEmpty() ){

                // Promotion is the commit point. A failure restores the prior
                // destination before this update attempt is reported as bad.
                utility::setPermissions( opts.exeBinPath ) ;

                engine.updateCmdPath( m_ctx.logger(),opts.exeBinPath ) ;

                if( !cleanupWarning.isEmpty() )this->post( engine.name(),cleanupWarning,opts.id ) ;

                this->printVersion( opts.move(),true ) ;
            }else{
                this->failedToRename( engine.name(),opts.file.src(),opts.exeBinPath,m,opts.id ) ;

                engine.setBroken() ;
                this->printVersion( opts.move(),true ) ;
            }
		}
	}
}

void networkAccess::extractArchiveOuput( networkAccess::Opts opts,
                                     const utils::qprocess::outPut& result ) const
{
    const auto& engine = opts.engine() ;

    if( !result.success() ){
        removeUpdatePath( opts.updateStagePath ) ;
        this->failedToExtract( opts.exeArgs,result,opts.id ) ;
        engine.setBroken() ;
        this->printVersion( opts.move(),true ) ;
        return ;
    }

    if( engine.archiveContainsFolder() ){
        // Normalize a versioned top-level folder entirely inside staging. The
        // previously working engine is still untouched at this point.
        auto rename = engine.renameArchiveFolder( opts.filePath,opts.updateStagePath ) ;
        if( !rename.success() ){
            removeUpdatePath( opts.updateStagePath ) ;
            this->failedToRename( engine.name(),rename.src(),rename.dst(),rename.err(),opts.id ) ;
            engine.setBroken() ;
            this->printVersion( opts.move(),true ) ;
            return ;
        }
    }

    // Validate the expected executable in staging before touching the live
    // engine tree. A successfully extracted but structurally wrong archive is
    // an update failure, not a payload that should be committed and diagnosed
    // only after the previous working version has been discarded.
    const auto expectedRelative = QDir( opts.tempPath ).relativeFilePath( opts.exeBinPath ) ;
    const auto stagedExecutable = QDir( opts.updateStagePath ).filePath( expectedRelative ) ;
    if( QDir::isAbsolutePath( expectedRelative ) || expectedRelative == ".." || expectedRelative.startsWith( "../" ) ||
        !QFileInfo( stagedExecutable ).isFile() ){
        removeUpdatePath( opts.updateStagePath ) ;
        this->post( engine.name(),QObject::tr( "Extracted update is missing the expected executable: %1" ).arg( expectedRelative ),opts.id ) ;
        engine.setBroken() ;
        this->printVersion( opts.move(),true ) ;
        return ;
    }

    QString cleanupWarning ;
    const auto promotion = promoteUpdateDirectoryContents( opts.updateStagePath,opts.tempPath,&cleanupWarning ) ;
    if( !promotion.isEmpty() ){
        removeUpdatePath( opts.updateStagePath ) ;
        this->failedToRename( engine.name(),opts.updateStagePath,opts.tempPath,promotion,opts.id ) ;
        engine.setBroken() ;
        this->printVersion( opts.move(),true ) ;
        return ;
    }

    if( !cleanupWarning.isEmpty() )this->post( engine.name(),cleanupWarning,opts.id ) ;

    if( engine.archiveContainsFolder() ){
        auto exe = engine.updateCmdPath( m_ctx.logger(),opts.tempPath ) ;
        QFile file( exe ) ;
        file.setPermissions( file.permissions() | QFileDevice::ExeOwner ) ;
    }else{
        QFile file( opts.exeBinPath ) ;
        file.setPermissions( file.permissions() | QFileDevice::ExeOwner ) ;
    }

    // The downloaded archive is no longer required after a fully successful
    // promotion. Cleanup failure is diagnostic and does not undo a good engine.
    const auto cleanup = utility::removeFile( opts.filePath ) ;
    if( !cleanup.isEmpty() )this->failedToRemove( engine.name(),opts.filePath,cleanup,opts.id ) ;

    this->printVersion( opts.move(),true ) ;
}

void networkAccess::postStartDownloading( const QString& engineName,int id ) const
{
	auto m = QObject::tr( "Start Downloading" ) + " " + engineName + " ..." ;
	this->post( engineName,m,id ) ;
}

void networkAccess::postDownloading( const QString& engineName,
				     const QString& component,
				     int id ) const
{
	this->post( engineName,QObject::tr( "Downloading" ) + ": " + component,id ) ;
}

void networkAccess::postDestination( const QString& engineName,
				     const QString& component,
				     int id ) const
{
	this->post( engineName,QObject::tr( "Destination" ) + ": " + component,id ) ;
}

void networkAccess::postDownloadingProgress( const QString& name,
					     const QString& cmp,
					     int id ) const
{
	this->post( name,QObject::tr( "Downloading" ) + " " + name + ": " + cmp,id ) ;
}

void networkAccess::hashDoNotMatch( const QString& hash1,const QString& hash2,int id ) const
{
	this->post( m_appName,utility::barLine(),id ) ;
	this->post( m_appName,QObject::tr( "Ignoring Download Because Hashes Do Not Match" ),id ) ;
	this->post( m_appName,QObject::tr( "Expected \"%1\" but obtained \"%2\"" ).arg( hash1,hash2 ),id ) ;
	this->post( m_appName,utility::barLine(),id ) ;
}

void networkAccess::extractArchive( networkAccess::Opts opts ) const
{
	const engines::engine& engine = opts.engine() ;

	auto mm = QObject::tr( "Extracting archive: " ) + opts.filePath ;

	this->post( engine.name(),mm,opts.id ) ;

    // Materialize the complete archive away from the live engine tree. No
    // existing executable or folder is removed before extraction succeeds.
    opts.updateStagePath = QDir( opts.tempPath ).filePath( ".mdps-update-stage-" + QUuid::createUuid().toString( QUuid::WithoutBraces ) ) ;
    if( !QDir().mkpath( opts.updateStagePath ) ){
        this->post( engine.name(),QObject::tr( "Failed to create engine update staging directory: %1" ).arg( opts.updateStagePath ),opts.id ) ;
        engine.setBroken() ;
        this->printVersion( opts.move(),true ) ;
        return ;
    }
	QStringList extractorArgs ;
	QString extractorExe ;

	if( utility::platformIsWindows() ){

		extractorExe = m_ctx.Engines().findExecutable( "bsdtar.exe" ) ;
		extractorArgs = QStringList{ "-x","-f",opts.filePath,"-C",opts.updateStagePath } ;
	}else{
		extractorExe = m_ctx.Engines().findExecutable( "bsdtar" ) ;

		if( extractorExe.isEmpty() ){

			extractorExe = m_ctx.Engines().findExecutable( "unzip" ) ;

			if( !extractorExe.isEmpty() ){

				extractorArgs = QStringList{ opts.filePath,"-d",opts.updateStagePath } ;
			}
		}else{
			extractorArgs = QStringList{ "-x","-f",opts.filePath,"-C",opts.updateStagePath } ;
		}
	}

	if( extractorExe.isEmpty() ){

		auto m = QObject::tr( "Failed To Extract" ) ;

		auto mm = [](){

			if( utility::platformIsWindows() ){

				return QObject::tr( "Failed To Find \"bsdtar.exe\" Executable" ) ;
			}else{
				return QObject::tr( "Failed To Find \"bsdtar\" or \"unzip\" Executable" ) ;
			}
		}() ;

		this->post( engine.name(),m + ": " + mm,opts.id ) ;

        removeUpdatePath( opts.updateStagePath ) ;
		engine.setBroken() ;
		this->printVersion( opts.move(),true ) ;
	}else{
		const auto& exe = extractorExe ;
		const auto& args = extractorArgs ;

		opts.exeArgs = { exe,args } ;

		utils::qprocess::run( exe,args,opts.move(),this,&networkAccess::extractArchiveOuput ) ;
	}
}

void networkAccess::post( const QString& engineName,const QString& m,int id ) const
{	
	m_ctx.logger().add( [ engineName,&m ]( Logger::Data& s,int id,bool ){

		auto e = m.toUtf8() ;

		auto p = QObject::tr( "Downloading" ) + " " + engineName ;

		auto prefix = p.toUtf8() ;

		if( s.isEmpty() ){

			s.add( "[media-downloader] " + e,id ) ;

		}else if( e == "..." ){

			auto m = s.getData( id ) ;

			if( m ){

				m.replaceLast( m.lastText() + " ..." ) ;
			}

		}else if( e.startsWith( prefix ) ){

			auto m = s.getData( id ) ;

			if( m ){

				if( m.lastText().startsWith( "[media-downloader] " + prefix ) ){

					m.removeLast() ;
				}
			}

			s.add( "[media-downloader] " + e,id ) ;
		}else{
			s.add( "[media-downloader] " + e,id ) ;
		}
	},id ) ;
}

QString networkAccess::reportError( const utils::network::progress& p ) const
{
	auto mm = QObject::tr( "Download Failed" ) ;

	if( p.timeOut() ){

		return mm + ": " + this->timeOutErrorString() ;
	}else{
		return mm + ": " + p.errorString() ;
	}
}

networkAccess::status::~status()
{
}

QString networkAccess::File::rename( const QString& e )
{
	return utility::rename( m_path,e ) ;
}

networkAccess::report::~report()
{
}

void networkAccess::downloadSpeed::setInitialTimeStamp()
{
	m_initialTimeStamp = this->currentSecsSinceEpoch() ;
}

qint64 networkAccess::downloadSpeed::currentSecsSinceEpoch()
{
	auto now = std::chrono::system_clock::now() ;
	return static_cast< qint64 >( std::chrono::system_clock::to_time_t( now ) ) ;
}

qint64 networkAccess::downloadSpeed::elapsedTime()
{
	return this->currentSecsSinceEpoch() - m_initialTimeStamp ;
}

QString networkAccess::downloadSpeed::calculate( const utils::network::progress& p )
{
	auto received  = p.received() ;
	auto totalSize = p.total() ;

	auto e = this->elapsedTime() ;

	if( e > 0 ){

		auto m = received / e ;

		m_dataSpeed = m_locale.formattedDataSize( m ) + "/s" ;

		if( e < 60 ){

			m_dataSpeed += " in " + QString::number( e ) + "s" ;

		}else if( e < 60 * 60 ){

			auto a = QString::number( e / 60 ) ;
			auto b = QString::number( e % 60 ) ;

			m_dataSpeed += " in " + a + "m:" + b + "s" ;
		}
	}

	if( totalSize <= 0 ){

		auto current = m_locale.formattedDataSize( received ) ;

		return QString( "%1 at %2" ).arg( current,m_dataSpeed ) ;
	}else{
		// Keep diagnostic byte counts exact, but never present an impossible
		// progress percentage above 100 when a server revises/misreports length.
		const auto rawPerc = double( received ) * 100 / double( totalSize ) ;
		const auto perc = rawPerc > 100.0 ? 100.0 : rawPerc ;
		auto size       = m_locale.formattedDataSize( totalSize ) ;
		auto current    = m_locale.formattedDataSize( received ) ;
		auto percentage = QString::number( perc,'f',2 ) ;

		if( percentage == "100.00" || percentage == "100,00" ){

			percentage = "100" ;
		}

		return QString( "%1 / %2, %3% at %4" ).arg( current,size,percentage,m_dataSpeed ) ;
	}
}
