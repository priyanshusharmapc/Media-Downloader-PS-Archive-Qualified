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

#include "library.h"
#include "basicdownloader.h"
#include "tabmanager.h"
#include "tableWidget.h"
#include "mainwindow.h"

#include "utils/miscellaneous.hpp"

#include <QDir>
#include <QFileInfo>
#include <QFile>

#ifdef Q_OS_UNIX
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#endif

namespace
{
QString canonicalLibraryPath( const QString& path )
{
	QFileInfo info( QDir::cleanPath( path ) ) ;
	const auto canonical = info.canonicalFilePath() ;
	return canonical.isEmpty() ? QString() :
		QDir::cleanPath( QDir::fromNativeSeparators( canonical ) ) ;
}

bool pathWithinLibraryRoot( const QString& root,const QString& candidate )
{
	const auto rootCanonical = canonicalLibraryPath( root ) ;
	const auto candidateCanonical = canonicalLibraryPath( candidate ) ;
	if( rootCanonical.isEmpty() || candidateCanonical.isEmpty() ){
		return false ;
	}

#ifdef Q_OS_WIN
	if( rootCanonical.compare( candidateCanonical,Qt::CaseInsensitive ) == 0 ){
		return true ;
	}
#else
	if( rootCanonical == candidateCanonical ){
		return true ;
	}
#endif

	const auto prefix = rootCanonical.endsWith( '/' ) ? rootCanonical : rootCanonical + "/" ;
#ifdef Q_OS_WIN
	return candidateCanonical.startsWith( prefix,Qt::CaseInsensitive ) ;
#else
	return candidateCanonical.startsWith( prefix,Qt::CaseSensitive ) ;
#endif
}

bool parentWithinLibraryRoot( const QString& root,const QString& candidate )
{
	return pathWithinLibraryRoot( root,QFileInfo( candidate ).absolutePath() ) ;
}

#ifdef Q_OS_UNIX
QByteArray nativeChildPath( const QByteArray& parent,const QByteArray& name )
{
	if( parent.isEmpty() || name.isEmpty() || name.contains( '/' ) || name.contains( '\0' ) ){
		return {} ;
	}
	QByteArray path = parent ;
	if( !path.endsWith( '/' ) )path.append( '/' ) ;
	path.append( name ) ;
	return path ;
}

bool nativePathWithinLibraryRoot( const QByteArray& root,const QByteArray& candidate )
{
	if( root.isEmpty() || candidate.isEmpty() || root.contains( '\0' ) || candidate.contains( '\0' ) ){
		return false ;
	}
	if( candidate == root )return true ;
	QByteArray prefix = root ;
	if( !prefix.endsWith( '/' ) )prefix.append( '/' ) ;
	return candidate.startsWith( prefix ) ;
}

bool nativeDirectoryIsSafe( const QByteArray& root,const QByteArray& path )
{
	if( !nativePathWithinLibraryRoot( root,path ) )return false ;
	struct stat st{} ;
	if( ::lstat( path.constData(),&st ) != 0 )return false ;
	return S_ISDIR( st.st_mode ) && !S_ISLNK( st.st_mode ) ;
}

bool nativePathStillExists( const QByteArray& path )
{
	struct stat st{} ;
	return ::lstat( path.constData(),&st ) == 0 ;
}

QString renameNativeEntry( const QByteArray& root,
				 const QByteArray& parent,
				 const QByteArray& oldName,
				 const QString& newName,
				 QByteArray& newNativeName )
{
	if( newName.isEmpty() || newName == "." || newName == ".." ||
		newName.contains( '/' ) || newName.contains( '\\' ) ){
		return QObject::tr( "Rename requires a file or folder name, not a path." ) ;
	}
	if( !nativeDirectoryIsSafe( root,parent ) ){
		return QObject::tr( "Current Library directory is no longer safe." ) ;
	}

	newNativeName = QFile::encodeName( newName ) ;
	if( newNativeName.isEmpty() || newNativeName.contains( '/' ) || newNativeName.contains( '\0' ) ){
		return QObject::tr( "Rename destination cannot be represented safely on this filesystem." ) ;
	}

	const auto oldPath = nativeChildPath( parent,oldName ) ;
	const auto newPath = nativeChildPath( parent,newNativeName ) ;
	if( oldPath.isEmpty() || newPath.isEmpty() ||
		!nativePathWithinLibraryRoot( root,oldPath ) ||
		!nativePathWithinLibraryRoot( root,newPath ) ){
		return QObject::tr( "Rename path is outside the Library root." ) ;
	}

	struct stat existing{} ;
	if( ::lstat( newPath.constData(),&existing ) == 0 ){
		return QObject::tr( "Rename destination already exists." ) ;
	}
	if( errno != ENOENT ){
		return QObject::tr( "Unable to validate rename destination: %1" )
			.arg( QString::fromLocal8Bit( std::strerror( errno ) ) ) ;
	}

	if( ::rename( oldPath.constData(),newPath.constData() ) != 0 ){
		return QObject::tr( "Renaming failed: %1" )
			.arg( QString::fromLocal8Bit( std::strerror( errno ) ) ) ;
	}
	return {} ;
}
#endif

bool deleteLibraryPath( const QString& root,const QString& path,std::atomic_bool& keepGoing )
{
	if( !keepGoing.load() ){
		return true ;
	}

	QFileInfo info( path ) ;

	// A link itself may live safely inside the Library even when its target does
	// not. Delete only the link leaf and never follow it.
	if( info.isSymLink() ){
		if( !parentWithinLibraryRoot( root,path ) ){
			return true ;
		}
		QFile::remove( path ) ;
		info.refresh() ;
		return info.exists() || info.isSymLink() ;
	}

	if( !pathWithinLibraryRoot( root,path ) ){
		return true ;
	}

	if( info.isDir() ){
		directoryManager::removeDirectory( path,keepGoing ) ;
	}else{
		QFile::remove( path ) ;
	}

	info.refresh() ;
	return info.exists() ;
}
}

library::library( const Context& ctx ) :
	m_ctx( ctx ),
	m_settings( m_ctx.Settings() ),
	m_ui( m_ctx.Ui() ),
	m_table( *m_ui.tableWidgetLibrary,0,m_ctx.mainWidget().font() ),
	m_downloadFolder( QDir::fromNativeSeparators( m_settings.downloadFolder() ) ),
	m_currentPath( m_downloadFolder ),
	m_downloadNativePath( QFile::encodeName( canonicalLibraryPath( m_downloadFolder ) ) ),
	m_currentNativePath( m_downloadNativePath ),
	m_folderIcon( m_settings.getIcon( "folder" ).pixmap( 30,40 ) ),
	m_videoIcon( m_settings.getIcon( "video" ).pixmap( 30,40 ) )
{
	qRegisterMetaType< directoryEntries::iter >() ;

	this->setRenameUiVisible( false ) ;

	connect( m_ui.pbLibraryCancel,&QPushButton::clicked,[ this ](){

		m_continue = false ;
		if( m_scanContinue ){
			*m_scanContinue = false ;
		}
		if( m_deleteContinue ){
			*m_deleteContinue = false ;
		}
	} ) ;

	connect( m_ui.pbLibraryCancelRename,&QPushButton::clicked,[ this ](){

		this->setRenameUiVisible( false ) ;
		this->clearPendingAction() ;
	} ) ;

	connect( m_ui.pbLibrarySetNewFileName,&QPushButton::clicked,[ this ](){

		this->setRenameUiVisible( false ) ;

		const auto action = m_ui.pbLibrarySetNewFileName->objectName() ;
		const auto rows = this->pendingRows() ;
		const auto expectedCount = m_pendingActionNames.size() ;
		const auto directoryMatches = !m_pendingActionDirectory.isEmpty() &&
			QDir::cleanPath( m_pendingActionDirectory ) == QDir::cleanPath( m_currentPath ) ;

		// Confirmation is valid only for the exact view and item identities that
		// were displayed when the action was opened. Selection/current-row drift
		// can never redirect a destructive operation to another Library entry.
		if( action == "Rename" ){

			if( directoryMatches && expectedCount == 1 && rows.size() == 1 ){
				this->renameFile( rows.front() ) ;
			}

		}else if( action == "Delete" ){

			if( directoryMatches && expectedCount == 1 && rows.size() == 1 ){
				this->deleteEntry( rows.front() ) ;
			}

		}else if( action == "DeleteAll" ){

			if( directoryMatches ){
				this->deleteAll() ;
			}

		}else if( action == "DeleteSelectedItems" ){

			if( directoryMatches && expectedCount > 0 && rows.size() == static_cast< size_t >( expectedCount ) ){
				this->disableAll() ;
				m_ui.pbLibraryCancel->setEnabled( true ) ;
				m_continue = true ;
				m_deleteContinue = std::make_shared< std::atomic_bool >( true ) ;
				this->deleteEntries( rows ) ;
			}
		}

		this->clearPendingAction() ;
	} ) ;

	connect( this,&library::addEntrySignal,this,&library::addEntrySlot,Qt::QueuedConnection ) ;

	m_table.setUpHeaderMenu( [ this ]( int column ){

		this->arrangeEntries( column ) ;
	} ) ;

	/*
	connect( m_ui.cbLibraryTabEnable,&QCheckBox::clicked,[ this ]( bool e ){

		if( e ){

			this->enableAll() ;
			this->showContents( m_currentPath ) ;
		}else{
			m_table.clear() ;
			this->disableAll() ;
			m_ui.pbLibraryQuit->setEnabled( true ) ;
			m_ui.pbLibraryDowloadFolder->setEnabled( true ) ;
		}
	} ) ;
	*/

	auto cc = &QTableWidget::currentItemChanged ;

	m_table.connect( cc,[ this ]( QTableWidgetItem * c,QTableWidgetItem * p ){

		m_table.selectRow( c,p,1 ) ;
	} ) ;

	m_table.connect( &QTableWidget::customContextMenuRequested,this,&library::cxMenuRequested ) ;

	connect( m_ui.pbLibraryQuit,&QPushButton::clicked,[ this ](){

		m_ctx.mainWindow().quitApp() ;
	} ) ;

	connect( m_ui.pbLibraryDowloadFolder,&QPushButton::clicked,[ this ](){

		utility::openDownloadFolderPath( m_currentPath ) ;
	} ) ;

	connect( m_ui.pbLibraryHome,&QPushButton::clicked,[ this ](){

		auto m = m_settings.downloadFolder() ;

		m_downloadFolder = QDir::fromNativeSeparators( m ) ;
		m_downloadNativePath = QFile::encodeName( canonicalLibraryPath( m_downloadFolder ) ) ;

		if( m_downloadFolder != m_currentPath ){

			m_currentPath = m_downloadFolder ;
			m_currentNativePath = m_downloadNativePath ;

			this->showContents( m_currentPath,m_currentNativePath ) ;
		}
	} ) ;

	connect( m_ui.pbLibraryUp,&QPushButton::clicked,[ this ](){

		this->moveUp() ;
	} ) ;

	connect( m_ui.pbLibraryRefresh,&QPushButton::clicked,[ this ](){

		this->showContents( m_currentPath ) ;
	} ) ;

	m_table.connect( &QTableWidget::cellDoubleClicked,[ this ]( int row,int column ){
		Q_UNUSED( column )

		auto s = m_table.item( row,1 ).text() ;

#ifdef Q_OS_UNIX
		const auto nativeName = this->nativeNameAt( row ) ;
		if( !nativeName.isEmpty() ){
			const auto nativeCandidate = this->nativePathAt( row ) ;
			if( nativeCandidate.isEmpty() )return ;

			if( m_table.stuffAt( row ) == directoryEntries::ICON::FOLDER ){
				if( !nativeDirectoryIsSafe( m_downloadNativePath,nativeCandidate ) )return ;
				const auto displayCandidate = QDir::cleanPath( m_currentPath + "/" + s ) ;
				this->showContents( displayCandidate,nativeCandidate ) ;
			}else{
				// settings::openUrl(QByteArray) constructs a percent-encoded file URL
				// from these exact bytes instead of reconstructing a QString path.
				m_settings.openUrl( nativeCandidate ) ;
			}
			return ;
		}
#endif

		const auto candidate = QDir::cleanPath( m_currentPath + "/" + s ) ;

		if( !pathWithinLibraryRoot( m_downloadFolder,candidate ) ){
			return ;
		}

		if( m_table.stuffAt( row ) == directoryEntries::ICON::FOLDER ){

			m_currentPath = candidate ;

			this->showContents( m_currentPath ) ;
		}else{
			m_ctx.Engines().openUrls( candidate ) ;
		}
	} ) ;;
}

void library::moveUp()
{
#ifdef Q_OS_UNIX
	if( !m_currentNativePath.isEmpty() && m_currentNativePath != m_downloadNativePath ){
		auto nativeSlash = m_currentNativePath.lastIndexOf( '/' ) ;
		if( nativeSlash > 0 )m_currentNativePath.truncate( nativeSlash ) ;

		auto displaySlash = m_currentPath.lastIndexOf( '/' ) ;
		if( displaySlash > 0 )m_currentPath.truncate( displaySlash ) ;

		if( nativePathWithinLibraryRoot( m_downloadNativePath,m_currentNativePath ) ){
			this->showContents( m_currentPath,m_currentNativePath ) ;
		}else{
			m_currentPath = m_downloadFolder ;
			m_currentNativePath = m_downloadNativePath ;
			this->showContents( m_currentPath,m_currentNativePath ) ;
		}
		return ;
	}
#endif

	if( m_currentPath != m_downloadFolder ){

		auto m = m_currentPath.lastIndexOf( '/' ) ;

		if( m != -1 ){

			m_currentPath.truncate( m ) ;
		}

		this->showContents( m_currentPath ) ;
	}
}

void library::init_done()
{
	if( m_settings.enableLibraryTab() ){

		this->enableAll() ;
	}else{
		this->disableAll() ;
		m_ui.pbLibraryQuit->setEnabled( true ) ;
		m_ui.pbLibraryDowloadFolder->setEnabled( true ) ;
	}
}

void library::resetMenu()
{
}

void library::exiting()
{
	m_continue = false ;
	if( m_scanContinue ){
		*m_scanContinue = false ;
		m_scanContinue.reset() ;
	}
	if( m_deleteContinue ){
		*m_deleteContinue = false ;
		m_deleteContinue.reset() ;
	}
}

void library::retranslateUi()
{
}

void library::tabEntered()
{
	if( m_settings.enableLibraryTab() ){

		// A previous tab exit may have cancelled population and left a partial
		// table plus m_continue == false. Always start a fresh directory read
		// when the Library becomes active instead of treating rowCount as a
		// completion marker.
		this->showContents( m_currentPath ) ;
	}
}

void library::tabExited()
{
	m_continue = false ;
	++m_populationGeneration ;
	if( m_scanContinue ){
		*m_scanContinue = false ;
	}
	if( m_deleteContinue ){
		*m_deleteContinue = false ;
	}
}

void library::textAlignmentChanged( Qt::LayoutDirection )
{
}

void library::capturePendingRows( const std::vector< int >& rows )
{
	m_pendingActionDirectory = m_currentPath ;
	m_pendingActionNames.clear() ;
	m_pendingActionNativeNames.clear() ;

	for( const auto row : rows ){
		if( row >= 0 && row < m_table.rowCount() ){
			m_pendingActionNames.append( m_table.item( row,1 ).text() ) ;
			m_pendingActionNativeNames.append( this->nativeNameAt( row ) ) ;
		}
	}
}

void library::capturePendingRow( int row )
{
	this->capturePendingRows( { row } ) ;
}

void library::capturePendingDirectory()
{
	m_pendingActionDirectory = m_currentPath ;
	m_pendingActionNames.clear() ;
	m_pendingActionNativeNames.clear() ;
}

std::vector< int > library::pendingRows()
{
	std::vector< int > rows ;

	if( m_pendingActionDirectory.isEmpty() ||
		QDir::cleanPath( m_pendingActionDirectory ) != QDir::cleanPath( m_currentPath ) ){
		return rows ;
	}

	for( int index = 0 ; index < m_pendingActionNames.size() ; ++index ){
		const auto& name = m_pendingActionNames.at( index ) ;
		const auto nativeName = index < m_pendingActionNativeNames.size() ?
			m_pendingActionNativeNames.at( index ) : QByteArray() ;
		bool found = false ;
		for( int row = 0 ; row < m_table.rowCount() ; row++ ){
			if( m_table.item( row,1 ).text() == name &&
			    ( nativeName.isEmpty() || this->nativeNameAt( row ) == nativeName ) ){
				rows.emplace_back( row ) ;
				found = true ;
				break ;
			}
		}
		if( !found ){
			rows.clear() ;
			return rows ;
		}
	}

	return rows ;
}

void library::clearPendingAction()
{
	m_pendingActionDirectory.clear() ;
	m_pendingActionNames.clear() ;
	m_pendingActionNativeNames.clear() ;
}

bool library::hasMultipleSelections()
{
	int multipleSelections = 0 ;

	for( int row = 0 ; row < m_table.rowCount() ; row++ ){

		if( m_table.isSelected( row ) ){

			multipleSelections++ ;
		}
	}

	return multipleSelections > 1 ;
}

QByteArray library::nativeNameAt( int row ) const
{
	if( row < 0 || row >= m_table.rowCount() )return {} ;
	return m_table.item( row,1 ).data( Qt::UserRole ).toByteArray() ;
}

QByteArray library::nativePathAt( int row ) const
{
#ifdef Q_OS_UNIX
	const auto name = this->nativeNameAt( row ) ;
	if( name.isEmpty() )return {} ;
	const auto path = nativeChildPath( m_currentNativePath,name ) ;
	return nativePathWithinLibraryRoot( m_downloadNativePath,path ) ? path : QByteArray() ;
#else
	Q_UNUSED( row )
	return {} ;
#endif
}

bool library::deletePath( const QString& m )
{
	return deleteLibraryPath( m_downloadFolder,m,m_continue ) ;
}

void library::deleteEntries( library::iter items )
{
	if( !m_continue || !m_deleteContinue || !m_deleteContinue->load() || items.empty() ){

		m_deleteContinue.reset() ;
		// Successful filesystem mutations invalidate the cached directory
		// snapshot used by sorting. Re-read it before re-enabling the view.
		return this->showContents( m_currentPath ) ;
	}

	auto row = items.next() ;
	if( row < 0 || row >= m_table.rowCount() ){
		m_deleteContinue.reset() ;
		return this->showContents( m_currentPath ) ;
	}

	const auto path = QDir::cleanPath( m_currentPath + "/" + m_table.item( row,1 ).text() ) ;
	const auto root = m_downloadFolder ;
	const auto nativePath = this->nativePathAt( row ) ;
	const auto nativeRoot = m_downloadNativePath ;
	auto keepGoing = m_deleteContinue ;

	class meaw
	{
	public:
		meaw( library * parent,library::iter items,int row,QString root,QString path,
		      QByteArray nativeRoot,QByteArray nativePath,
		      std::shared_ptr< std::atomic_bool > keepGoing ) :
			m_parent( parent ),m_items( items.move() ),m_row( row ),
			m_root( std::move( root ) ),m_path( std::move( path ) ),
			m_nativeRoot( std::move( nativeRoot ) ),m_nativePath( std::move( nativePath ) ),
			m_continue( std::move( keepGoing ) )
		{
		}
		bool bg()
		{
#ifdef Q_OS_UNIX
			if( !m_nativePath.isEmpty() ){
				if( !nativePathWithinLibraryRoot( m_nativeRoot,m_nativePath ) )return true ;
				struct stat st{} ;
				if( ::lstat( m_nativePath.constData(),&st ) != 0 )return false ;
				if( S_ISDIR( st.st_mode ) && !S_ISLNK( st.st_mode ) ){
					directoryManager::removeDirectoryNative( m_nativePath,*m_continue ) ;
				}else{
					::unlink( m_nativePath.constData() ) ;
				}
				return nativePathStillExists( m_nativePath ) ;
			}
#endif
			return deleteLibraryPath( m_root,m_path,*m_continue ) ;
		}
		void fg( bool stillExists )
		{
			if( !m_continue->load() || m_parent->m_deleteContinue != m_continue ){
				m_parent->enableAll() ;
				return ;
			}

			if( !stillExists ){
				m_parent->m_table.removeRow( m_row ) ;
			}

			m_parent->deleteEntries( m_items.move() ) ;
		}
	private:
		library * m_parent ;
		library::iter m_items ;
		int m_row ;
		QString m_root ;
		QString m_path ;
		QByteArray m_nativeRoot ;
		QByteArray m_nativePath ;
		std::shared_ptr< std::atomic_bool > m_continue ;
	} ;

	// The background phase owns only value state and a shared cancellation token.
	// Foreground publication is automatically suppressed if Library is destroyed.
	utils::qthread::run( this,meaw( this,items.move(),row,root,path,nativeRoot,nativePath,std::move( keepGoing ) ) ) ;
}

void library::setRenameUiVisible( bool e )
{
	m_ui.labelLibrarySetNewFileName->setVisible( e ) ;
	m_ui.pbLibraryCancelRename->setVisible( e ) ;
	m_ui.pbLibrarySetNewFileName->setVisible( e ) ;
	m_ui.plainTextLibrarySetNewName->setVisible( e ) ;
	m_ui.labelLibraryWidgetOverMainTable->setVisible( e ) ;
}

void library::renameFile( int row )
{
	auto nn = m_ui.plainTextLibrarySetNewName->toPlainText() ;

	auto& item = m_table.item( row,1 ) ;

#ifdef Q_OS_UNIX
	const auto oldNativeName = this->nativeNameAt( row ) ;
	if( !oldNativeName.isEmpty() ){
		QByteArray newNativeName ;
		const auto error = renameNativeEntry(
			m_downloadNativePath,m_currentNativePath,oldNativeName,nn,newNativeName ) ;
		if( error.isEmpty() ){
			item.setText( nn ) ;
			item.setData( Qt::UserRole,newNativeName ) ;
		}else{
			m_ctx.logger().add( error,utility::loggerID() ) ;
			this->showContents( m_currentPath,m_currentNativePath ) ;
		}
		return ;
	}
#endif

	if( !pathWithinLibraryRoot( m_downloadFolder,m_currentPath ) ){
		this->showContents( m_downloadFolder ) ;
		return ;
	}

	if( !utility::rename( m_ctx,item,m_currentPath,nn,item.text() ).isEmpty() ){

		this->showContents( m_currentPath ) ;
	}
}

void library::keyPressed( utility::mainWindowKeyCombo m )
{
	if( m == utility::mainWindowKeyCombo::CTRL_D ){

		auto a = tr( "Are You Sure You Want To Delete Selected Items?" ) ;

		m_ui.labelLibrarySetNewFileName->setText( a ) ;

		m_ui.pbLibrarySetNewFileName->setObjectName( "DeleteSelectedItems" ) ;
		this->capturePendingRows( m_table.selectedRows() ) ;

		m_ui.pbLibrarySetNewFileName->setText( tr( "Yes" ) ) ;

		m_ui.pbLibraryCancelRename->setText( tr( "No" ) ) ;

		this->setRenameUiVisible( true ) ;

		m_ui.plainTextLibrarySetNewName->setVisible( false ) ;
	}else{
		utility::keyPressed( m_table,m ) ;
	}
}

void library::deleteEntry( int row )
{
	if( m_table.isSelected( row ) ){

		this->disableAll() ;

		m_ui.pbLibraryCancel->setEnabled( true ) ;
		m_continue = true ;
		m_deleteContinue = std::make_shared< std::atomic_bool >( true ) ;

		this->deleteEntries( row ) ;
	}
}

void library::deleteAll()
{
	this->disableAll() ;

	m_ui.pbLibraryCancel->setEnabled( true ) ;
	m_continue = true ;
	m_deleteContinue = std::make_shared< std::atomic_bool >( true ) ;

	const auto root = m_downloadFolder ;
	const auto path = m_currentPath ;
	const auto nativeRoot = m_downloadNativePath ;
	const auto nativePath = m_currentNativePath ;
	auto keepGoing = m_deleteContinue ;

	class meaw
	{
	public:
		meaw( library * parent,QString root,QString path,QByteArray nativeRoot,QByteArray nativePath,
		      std::shared_ptr< std::atomic_bool > keepGoing ) :
			m_parent( parent ),m_root( std::move( root ) ),m_path( std::move( path ) ),
			m_nativeRoot( std::move( nativeRoot ) ),m_nativePath( std::move( nativePath ) ),
			m_continue( std::move( keepGoing ) )
		{
		}
		void bg()
		{
#ifdef Q_OS_UNIX
			if( !m_nativePath.isEmpty() &&
			    nativeDirectoryIsSafe( m_nativeRoot,m_nativePath ) ){
				directoryManager::removeDirectoryContentsNative( m_nativePath,*m_continue ) ;
				return ;
			}
#endif
			if( pathWithinLibraryRoot( m_root,m_path ) ){
				directoryManager::removeDirectoryContents( m_path,*m_continue ) ;
			}
		}
		void fg()
		{
			if( m_parent->m_deleteContinue == m_continue ){
				const auto completed = m_continue->load() ;
				m_parent->m_deleteContinue.reset() ;
				if( completed ){
					m_parent->showContents( m_parent->m_currentPath ) ;
				}else{
					m_parent->enableAll() ;
				}
			}
		}
	private:
		library * m_parent ;
		QString m_root ;
		QString m_path ;
		QByteArray m_nativeRoot ;
		QByteArray m_nativePath ;
		std::shared_ptr< std::atomic_bool > m_continue ;
	} ;

	utils::qthread::run( this,meaw( this,root,path,nativeRoot,nativePath,std::move( keepGoing ) ) ) ;
}

void library::enableAll()
{
	m_table.setEnabled( true ) ;
	m_ui.pbLibraryQuit->setEnabled( true ) ;
	m_ui.pbLibraryCancel->setEnabled( true ) ;
	m_ui.pbLibraryHome->setEnabled( true ) ;
	m_ui.pbLibraryDowloadFolder->setEnabled( true ) ;
	m_ui.pbLibraryRefresh->setEnabled( true ) ;
	m_ui.pbLibraryUp->setEnabled( true ) ;
}

void library::disableAll()
{
	m_table.setEnabled( false ) ;
	m_ui.pbLibraryQuit->setEnabled( false ) ;
	m_ui.pbLibraryCancel->setEnabled( false ) ;
	m_ui.pbLibraryHome->setEnabled( false ) ;
	m_ui.pbLibraryDowloadFolder->setEnabled( false ) ;
	m_ui.pbLibraryRefresh->setEnabled( false ) ;
	m_ui.pbLibraryUp->setEnabled( false ) ;
}

void library::addItem( const directoryEntries::iter& s )
{
	auto icon = s.icon() ;

	auto row = m_table.addRow( icon ) ;

	auto label = new QLabel() ;

	if( icon == directoryEntries::ICON::FILE ){

		label->setPixmap( m_videoIcon ) ;
	}else{
		label->setPixmap( m_folderIcon ) ;
	}

	label->setAlignment( Qt::AlignCenter ) ;

	m_table.get().setCellWidget( row,0,label ) ;

	auto& item = m_table.item( row,1 ) ;

	item.setText( s.value() ) ;
	item.setData( Qt::UserRole,s.nativeName() ) ;
	item.setTextAlignment( Qt::AlignCenter ) ;
	item.setFont( m_ctx.mainWidget().font() ) ;
}

void library::addEntrySlot( const directoryEntries::iter& s )
{
	// A queued event from an older scan/sort must never be re-armed merely
	// because m_continue became true for a replacement population.
	if( s.generation() != m_populationGeneration ){
		return ;
	}

	if( s.hasNext() && m_continue ){

		this->addItem( s ) ;

		m_table.setLastRow() ;

		emit this->addEntrySignal( s.next() ) ;
	}else{
		m_table.setLastRow() ;

		this->enableAll() ;
	}
}

void library::cxMenuRequested( QPoint )
{
	auto row = m_table.currentRow() ;

	if( row == -1 ){

		return ;
	}

	QMenu m ;

	auto hasMultipleSelections = this->hasMultipleSelections() ;

	connect( m.addAction( tr( "Delete" ) ),&QAction::triggered,[ this,row,hasMultipleSelections ](){

		if( hasMultipleSelections ){

			m_ui.pbLibrarySetNewFileName->setObjectName( "DeleteSelectedItems" ) ;
			this->capturePendingRows( m_table.selectedRows() ) ;

			auto a = tr( "Are You Sure You Want To Delete Selected Items?" ) ;

			m_ui.labelLibrarySetNewFileName->setText( a ) ;

			this->setRenameUiVisible( true ) ;

			m_ui.plainTextLibrarySetNewName->setVisible( false ) ;
		}else{
			if( m_table.stuffAt( row ) == directoryEntries::ICON::FILE ){

				auto a = tr( "Are You Sure You Want To Delete Below File?" ) ;

				m_ui.labelLibrarySetNewFileName->setText( a ) ;
			}else{
				auto a = tr( "Are You Sure You Want To Delete Below Folder?" ) ;

				m_ui.labelLibrarySetNewFileName->setText( a ) ;
			}

			m_ui.pbLibrarySetNewFileName->setObjectName( "Delete" ) ;
			this->capturePendingRow( row ) ;

			auto m = m_table.item( row,1 ).text() ;

			m_ui.plainTextLibrarySetNewName->setPlainText( m ) ;

			m_ui.plainTextLibrarySetNewName->setReadOnly( true ) ;

			this->setRenameUiVisible( true ) ;
		}

		m_ui.pbLibrarySetNewFileName->setText( tr( "Yes" ) ) ;

		m_ui.pbLibraryCancelRename->setText( tr( "No" ) ) ;
	} ) ;

	auto ac = m.addAction( tr( "Delete All" ) ) ;

	ac->setEnabled( !hasMultipleSelections ) ;

	connect( ac,&QAction::triggered,[ this ](){

		auto a = tr( "Are You Sure You Want To Delete All Files And Folders?" ) ;

		m_ui.labelLibrarySetNewFileName->setText( a ) ;

		m_ui.pbLibrarySetNewFileName->setObjectName( "DeleteAll" ) ;
		this->capturePendingDirectory() ;

		m_ui.pbLibrarySetNewFileName->setText( tr( "Yes" ) ) ;

		m_ui.pbLibraryCancelRename->setText( tr( "No" ) ) ;

		this->setRenameUiVisible( true ) ;

		m_ui.plainTextLibrarySetNewName->setVisible( false ) ;
	} ) ;

	ac = m.addAction( tr( "Rename" ) ) ;

	ac->setEnabled( !hasMultipleSelections ) ;

	connect( ac,&QAction::triggered,[ this,row ](){

		if( m_table.stuffAt( row ) == directoryEntries::ICON::FILE ){

			auto a = tr( "Rename File To Below Text" ) ;

			m_ui.labelLibrarySetNewFileName->setText( a ) ;
		}else{
			auto a = tr( "Rename Folder To Below Text" ) ;

			m_ui.labelLibrarySetNewFileName->setText( a ) ;
		}

		m_ui.pbLibrarySetNewFileName->setObjectName( "Rename" ) ;
		this->capturePendingRow( row ) ;

		m_ui.pbLibrarySetNewFileName->setText( tr( "Rename" ) ) ;

		m_ui.pbLibraryCancelRename->setText( tr( "Cancel" ) ) ;

		auto m = m_table.item( row,1 ).text() ;

		m_ui.plainTextLibrarySetNewName->setPlainText( m ) ;

		m_ui.plainTextLibrarySetNewName->moveCursor( QTextCursor::End ) ;

		m_ui.plainTextLibrarySetNewName->setReadOnly( false ) ;

		m_ui.plainTextLibrarySetNewName->setFocus() ;

		this->setRenameUiVisible( true ) ;
	} ) ;

	m.exec( QCursor::pos() ) ;
}

void library::arrangeAndShow()
{
	if( m_settings.libraryArrangeAscending() ){

		if( m_settings.libraryArrangeByDate() ){

			m_directoryEntries.sortByDateAscending() ;
		}else{
			m_directoryEntries.sortByNameAscending() ;
		}
	}else{
		if( m_settings.libraryArrangeByDate() ){

			m_directoryEntries.sortByDateDescending() ;
		}else{
			m_directoryEntries.sortByNameDescending() ;
		}
	}

	m_table.clear() ;

	m_directoryEntries.join( m_settings.libraryShowFolderFirst() ) ;

	const auto generation = ++m_populationGeneration ;
	this->addEntrySlot( m_directoryEntries.Iter( generation ) ) ;
}

static void _set_option( QMenu& m,const QString& tr,const QString& utr,bool o )
{
	auto ac = m.addAction( tr ) ;
	ac->setObjectName( utr ) ;
	ac->setCheckable( true ) ;
	ac->setChecked( o ) ;
	m.addAction( ac ) ;
}

void library::arrangeEntries( int )
{
	QMenu m ;

	auto a = m_settings.libraryShowFolderFirst() ;

	_set_option( m,QObject::tr( "Show Folders First" ),"Show Folders First",a ) ;

	a = m_settings.libraryArrangeAscending() ;

	_set_option( m,QObject::tr( "Arrange In Ascending Order" ),"Arrange In Ascending Order",a ) ;

	a = m_settings.libraryArrangeByDate() ;

	_set_option( m,QObject::tr( "Arrange By Date" ),"Arrange By Date",a ) ;

	_set_option( m,QObject::tr( "Arrange By Name" ),"Arrange By Name",!a ) ;

	QObject::connect( &m,&QMenu::triggered,[ this ]( QAction * ac ){

		auto e = ac->objectName() ;

		auto checked = ac->isChecked() ;

		if( e == "Show Folders First" ){

			m_settings.setLibraryShowFolderFirst( checked ) ;

		}else if( e == "Arrange In Ascending Order" ){

			m_settings.setLibraryArrangeAscending( checked ) ;

		}else if( e == "Arrange By Date" ){

			m_settings.setLibraryArrangeByDate( checked ) ;

		}else if( e == "Arrange By Name" ){

			auto m = m_settings.libraryArrangeByDate() ;

			m_settings.setLibraryArrangeByDate( !m ) ;
		}

		this->disableAll() ;

		m_ui.pbLibraryCancel->setEnabled( true ) ;

		this->arrangeAndShow() ;
	} ) ;

	m.exec( QCursor::pos() ) ;
}

void library::showContents( const QString& path,const QByteArray& nativePath )
{
	// Invalidate already queued row events immediately, before the replacement
	// background scan has had time to publish its new snapshot.
	++m_populationGeneration ;
	m_continue = true ;

	auto safePath = QDir::cleanPath( path ) ;

#ifdef Q_OS_UNIX
	QByteArray safeNativePath = nativePath ;
	if( !safeNativePath.isEmpty() ){
		if( !nativeDirectoryIsSafe( m_downloadNativePath,safeNativePath ) ){
			safePath = QDir::cleanPath( m_downloadFolder ) ;
			safeNativePath = m_downloadNativePath ;
		}
	}else{
		if( !pathWithinLibraryRoot( m_downloadFolder,safePath ) ){
			safePath = QDir::cleanPath( m_downloadFolder ) ;
		}
		safeNativePath = QFile::encodeName( canonicalLibraryPath( safePath ) ) ;
	}
	if( safeNativePath.isEmpty() || !nativeDirectoryIsSafe( m_downloadNativePath,safeNativePath ) ){
		m_table.clear() ;
		this->enableAll() ;
		return ;
	}
	m_currentNativePath = safeNativePath ;
#else
	Q_UNUSED( nativePath )
	if( !pathWithinLibraryRoot( m_downloadFolder,safePath ) ){
		safePath = QDir::cleanPath( m_downloadFolder ) ;
		if( !pathWithinLibraryRoot( m_downloadFolder,safePath ) ){
			m_table.clear() ;
			this->enableAll() ;
			return ;
		}
	}
#endif
	m_currentPath = safePath ;
	m_table.get().setHorizontalHeaderItem( 1,new QTableWidgetItem( m_currentPath ) ) ;

	this->disableAll() ;

	m_ui.pbLibraryCancel->setEnabled( true ) ;

	// Supersede any earlier scan without leaving its worker tied to this
	// QObject's lifetime. The worker owns only its path and cancellation token.
	if( m_scanContinue ){
		*m_scanContinue = false ;
	}
	m_scanContinue = std::make_shared< std::atomic_bool >( true ) ;
	auto scanContinue = m_scanContinue ;

	class meaw
	{
	public:
		meaw( library * library,const QString& path,QByteArray nativePath,
		      std::shared_ptr< std::atomic_bool > keepGoing ) :
			m_parent( library ),
			m_path( path ),
			m_nativePath( std::move( nativePath ) ),
			m_continue( std::move( keepGoing ) )
		{
		}
		directoryEntries bg()
		{
#ifdef Q_OS_UNIX
			if( !m_nativePath.isEmpty() ){
				return directoryManager::readAllNative( m_nativePath,*m_continue ) ;
			}
#endif
			return directoryManager::readAll( m_path,*m_continue ) ;
		}
		void fg( directoryEntries&& entries )
		{
			// QPointer becomes null as soon as the QObject is destroyed. The token
			// also prevents an obsolete scan from publishing after a newer scan,
			// tab exit or application shutdown has superseded it.
			if( !m_parent || !m_continue->load() || m_parent->m_scanContinue != m_continue ){
				return ;
			}

			m_parent->m_directoryEntries = std::move( entries ) ;
			m_parent->arrangeAndShow() ;
		}
	private:
		QPointer< library > m_parent ;
		QString m_path ;
		QByteArray m_nativePath ;
		std::shared_ptr< std::atomic_bool > m_continue ;
	} ;

	utils::qthread::run( this,meaw( this,safePath,m_currentNativePath,std::move( scanContinue ) ) ) ;
}
