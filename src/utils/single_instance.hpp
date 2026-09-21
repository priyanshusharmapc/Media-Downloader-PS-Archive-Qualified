/*
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

#include <memory>
#include <iostream>

#include <QTimer>
#include <QApplication>
#include <QByteArray>
#include <QString>
#include <QFile>
#include <QDir>
#include <QLockFile>
#include <QLocalServer>
#include <QLocalSocket>
#include <QElapsedTimer>

namespace utils
{
	namespace app
	{
		namespace details
		{
			class exec : public QObject
			{
				Q_OBJECT
			public:
				template< typename Function >
				exec( Function function ) : m_function( function )
				{
					connect( this,&exec::run,this,&exec::meaw,Qt::QueuedConnection ) ;

					emit this->run() ;
				}
			private:
				void meaw()
				{
					m_function() ;
				}
			signals:
				void run() ;
			private:
				std::function< void() > m_function ;
			} ;

			template< typename Args >
			auto engineLockPath( const Args& args,int ) -> decltype( args.ePaths.socketLockPath() )
			{
				return args.ePaths.socketLockPath() ;
			}

			template< typename Args >
			QString engineLockPath( const Args&,long )
			{
				return QString() ;
			}

			inline QString fallbackLockPath( const QString& socketPath )
			{
				if( socketPath.startsWith( "\\\\.\\pipe\\" ) || socketPath.startsWith( "//./pipe/" ) ){
					return QDir::tempPath() + "/MediaDownloaderIPC.lock" ;
				}
				return socketPath + ".lock" ;
			}

			template< typename Args >
			QString lockPath( const Args& args,const QString& socketPath )
			{
				const auto fromEngine = engineLockPath( args,0 ) ;
				if( !fromEngine.isEmpty() ){
					return fromEngine ;
				}
				return fallbackLockPath( socketPath ) ;
			}
		} ;
		template< typename Type,typename TypeArgs >
		struct appInfo
		{
			appInfo( TypeArgs t,const QString& s,QApplication& a,QByteArray d = QByteArray() ) :
				args( std::move( t ) ),socketPath( s ),app( a ),data( std::move( d ) )
			{
			}
			using appType = Type ;
			TypeArgs args ;
			QString socketPath ;
			QApplication& app ;
			QByteArray data ;
		} ;

		template< typename AppInfo >
		class multipleInstance
		{
		public:
			multipleInstance( AppInfo info ) :
				m_info( std::move( info ) ),
				m_exec( [ this ](){ this->run() ; } )
			{
			}
			void run()
			{
				m_mainApp = std::make_unique< typename AppInfo::appType >( std::move( m_info.args ) ) ;
				m_mainApp->start( m_info.data ) ;
			}
			int exec()
			{
				return m_info.app.exec() ;
			}
		private:
			AppInfo m_info ;
			details::exec m_exec ;
			std::unique_ptr< typename AppInfo::appType > m_mainApp ;
		} ;

		template< typename AppInfo >
		int runMultiInstances( AppInfo info )
		{
			return multipleInstance< AppInfo >( std::move( info ) ).exec() ;
		}

		template< typename OIR,typename PIC >
		struct instanceArgs
		{
			OIR otherInstanceRunning ;
			PIC otherInstanceCrashed ;
		} ;

		template< typename OIR,typename PIC >
		auto make_oneinstance_args( OIR r,PIC c )
		{
			return instanceArgs< OIR,PIC >{ std::move( r ),std::move( c ) } ;
		}

		template< typename AppInfo,typename InstanceArgs >
		class oneinstance
		{
		public:
			oneinstance( AppInfo info,InstanceArgs iargs ) :
				m_info( std::move( info ) ),
				m_iargs( std::move( iargs ) ),
				m_exec( [ this ](){ this->run() ; } ),
				m_lockFile( details::lockPath( m_info.args,m_info.socketPath ) )
			{
				m_lockFile.setStaleLockTime( 30000 ) ;
				// The primary owns this lock for its entire lifetime. Failure
				// to acquire it means this process is a secondary candidate.
				m_lockOwned = m_lockFile.tryLock( 0 ) ;
			}
			~oneinstance()
			{
				if( m_localServer.isListening() )m_localServer.close() ;
				if( m_lockOwned ){
					QLocalServer::removeServer( m_info.socketPath ) ;
					m_lockFile.unlock() ;
				}
			}
			int exec()
			{
				return m_info.app.exec() ;
			}
		private:
			void run()
			{
				if( m_lockOwned ){
					// A lock acquired while an endpoint exists can happen during
					// upgrade from an older primary. Probe once before cleanup.
					if( QFile::exists( m_info.socketPath ) ){
						QLocalSocket probe ;
						probe.connectToServer( m_info.socketPath ) ;
						if( probe.waitForConnected( 500 ) ){
							probe.abort() ;
							m_lockFile.unlock() ;
							m_lockOwned = false ;
							this->connectToPrimary() ;
							return ;
						}
						m_iargs.otherInstanceCrashed() ;
						QLocalServer::removeServer( m_info.socketPath ) ;
					}
					this->start() ;
					return ;
				}
				this->connectToPrimary() ;
			}
			bool staleEndpointError( QLocalSocket::LocalSocketError error ) const
			{
				return error == QLocalSocket::ServerNotFoundError ||
				       error == QLocalSocket::ConnectionRefusedError ;
			}
			void socketError( QLocalSocket::LocalSocketError error )
			{
				if( m_handoffComplete )return ;
				m_localSocket.abort() ;
				// A transient error never promotes this process. Stale cleanup
				// requires a stale-class socket error and ownership of the
				// lifetime lock, followed by a second failed endpoint probe.
				if( !this->staleEndpointError( error ) || !m_lockFile.tryLock( 0 ) ){
					std::cerr << "Single-instance handoff failed while primary ownership remains live" << std::endl ;
					m_info.app.exit( 1 ) ;
					return ;
				}
				QLocalSocket probe ;
				probe.connectToServer( m_info.socketPath ) ;
				if( probe.waitForConnected( 500 ) ){
					probe.abort() ;
					m_lockFile.unlock() ;
					std::cerr << "Single-instance endpoint is still live; refusing stale cleanup" << std::endl ;
					m_info.app.exit( 1 ) ;
					return ;
				}
				if( !this->staleEndpointError( probe.error() ) ){
					m_lockFile.unlock() ;
					std::cerr << "Single-instance endpoint state is ambiguous; refusing stale cleanup" << std::endl ;
					m_info.app.exit( 1 ) ;
					return ;
				}
				m_lockOwned = true ;
				m_iargs.otherInstanceCrashed() ;
				QLocalServer::removeServer( m_info.socketPath ) ;
				this->start() ;
			}
			void connectToPrimary()
			{
				QObject::connect( &m_localSocket,&QLocalSocket::connected,[ this ](){
					const auto payload = m_info.data ;
					const auto frame = QByteArray::number( payload.size() ) + "\n" + payload ;
					qint64 offset = 0 ;
					while( offset < frame.size() ){
						const auto written = m_localSocket.write( frame.constData() + offset,frame.size() - offset ) ;
						if( written <= 0 ){
							std::cerr << "Failed to deliver complete single-instance event" << std::endl ;
							m_localSocket.abort() ;
							m_info.app.exit( 1 ) ;
							return ;
						}
						offset += written ;
					}
					if( !m_localSocket.waitForBytesWritten( 5000 ) ){
						std::cerr << "Timed out delivering single-instance event" << std::endl ;
						m_localSocket.abort() ;
						m_info.app.exit( 1 ) ;
						return ;
					}
					QByteArray acknowledgement ;
					QElapsedTimer deadline ;
					deadline.start() ;
					while( acknowledgement.size() < 3 && deadline.elapsed() < 5000 ){
						if( m_localSocket.bytesAvailable() == 0 )m_localSocket.waitForReadyRead( 250 ) ;
						acknowledgement += m_localSocket.readAll() ;
					}
					if( !acknowledgement.startsWith( "OK\n" ) ){
						std::cerr << "Primary instance did not acknowledge the complete event" << std::endl ;
						m_localSocket.abort() ;
						m_info.app.exit( 1 ) ;
						return ;
					}
					m_handoffComplete = true ;
					m_localSocket.disconnectFromServer() ;
					m_iargs.otherInstanceRunning() ;
					m_info.app.quit() ;
				} ) ;
#if QT_VERSION < QT_VERSION_CHECK( 5,15,0 )
				using cs = void( QLocalSocket::* )( QLocalSocket::LocalSocketError ) ;
				QObject::connect( &m_localSocket,static_cast< cs >( &QLocalSocket::error ),
					[ this ]( QLocalSocket::LocalSocketError error ){ this->socketError( error ) ; } ) ;
#else
				QObject::connect( &m_localSocket,&QLocalSocket::errorOccurred,
					[ this ]( QLocalSocket::LocalSocketError error ){ this->socketError( error ) ; } ) ;
#endif
				m_localSocket.connectToServer( m_info.socketPath ) ;
			}
			void start()
			{
				QObject::connect( &m_localServer,&QLocalServer::newConnection,[ this ](){

					auto s = m_localServer.nextPendingConnection() ;
					auto bytes = std::make_shared< QByteArray >() ;
					auto expected = std::make_shared< qint64 >( -1 ) ;
					auto delivered = std::make_shared< bool >( false ) ;
					const qint64 maxEventBytes = 1024 * 1024 ;

					auto consume = [ this,s,bytes,expected,delivered,maxEventBytes ](){
						if( *delivered )return ;

						bytes->append( s->readAll() ) ;

						if( *expected < 0 ){
							const auto separator = bytes->indexOf( '\n' ) ;
							if( separator < 0 ){
								if( bytes->size() > 32 )s->abort() ;
								return ;
							}

							bool ok = false ;
							const auto value = bytes->left( separator ).toLongLong( &ok ) ;
							if( !ok || value < 0 || value > maxEventBytes ){
								s->abort() ;
								return ;
							}
							*expected = value ;
							bytes->remove( 0,separator + 1 ) ;
						}

						if( bytes->size() > *expected ){
							s->abort() ;
							return ;
						}

						if( bytes->size() == *expected ){
							if( m_mainApp && *expected > 0 ){
								m_mainApp->hasEvent( *bytes ) ;
							}
							*delivered = true ;
							s->write( "OK\n",3 ) ;
							s->flush() ;
							s->disconnectFromServer() ;
						}
					} ;

					QObject::connect( s,&QLocalSocket::readyRead,consume ) ;
					QObject::connect( s,&QLocalSocket::disconnected,[ s,consume,delivered ](){
						consume() ;
						if( !*delivered ){
							// Incomplete frames are discarded and never surfaced as events.
						}
						s->deleteLater() ;
					} ) ;
				} ) ;
				// The single-instance contract is not established until the IPC
				// endpoint is actually listening. Keep the startup lock held and
				// fail startup rather than exposing a full GUI with no listener.
				if( !m_localServer.listen( m_info.socketPath ) ){
					std::cerr << "Failed to establish single-instance listener: "
						  << m_localServer.errorString().toStdString() << std::endl ;
					m_info.app.exit( 1 ) ;
					return ;
				}

				m_mainApp = std::make_unique< typename AppInfo::appType >( std::move( m_info.args ) ) ;
				m_mainApp->start( std::move( m_info.data ) ) ;

				// Keep the ownership lock for the full primary lifetime.
			}
			QLocalServer m_localServer ;
			QLocalSocket m_localSocket ;
			std::unique_ptr< typename AppInfo::appType > m_mainApp ;
			AppInfo m_info ;
			InstanceArgs m_iargs ;
			details::exec m_exec ;
			QLockFile m_lockFile ;
			bool m_lockOwned = false ;
			bool m_handoffComplete = false ;
		} ;

		class AppTypeInterface
		{
		public:
			struct args
			{
				QApplication& app ;
			} ;
			AppTypeInterface( const AppTypeInterface::args& )
			{
			}
			void hasEvent( QByteArray )
			{
				//This method is called with data from another instance that failed
				//to start because this instance prevented it from starting
			}
			void start( QByteArray )
			{
				//This method is called when the first instance is started
			}
		} ;

		template< typename AppInfo,typename Err >
		int runOneInstance( AppInfo info,Err err )
		{
			return oneinstance< AppInfo,Err >( std::move( info ),std::move( err ) ).exec() ;
		}

		template< typename AppInfo >
		int runOneInstance( AppInfo info )
		{
			auto err = make_oneinstance_args( [](){

				std::cout << "There seem to be another instance running,exiting this one" << std::endl ;
			},[](){
				std::cout << "Previous instance seem to have crashed,trying to clean up before starting" << std::endl ;
			} ) ;

			return runOneInstance( std::move( info ),std::move( err ) ) ;
		}
	}
}
