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

library::library( const Context& ctx ) :
	m_ctx( ctx ),
	m_settings( m_ctx.Settings() ),
	m_ui( m_ctx.Ui() ),
	m_table( *m_ui.tableWidgetLibrary,0,m_ctx.mainWidget().font() ),
	m_downloadFolder( QDir::fromNativeSeparators( m_settings.downloadFolder() ) ),
	m_currentPath( m_downloadFolder ),
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

		if( m_downloadFolder != m_currentPath ){

			m_currentPath = m_downloadFolder ;

			this->showContents( m_currentPath ) ;
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

		if( m_table.stuffAt( row ) == directoryEntries::ICON::FOLDER ){

			m_currentPath +=  "/" + s ;

			this->showContents( m_currentPath ) ;
		}else{
			m_ctx.Engines().openUrls( m_currentPath + "/" + s ) ;
		}
	} ) ;
}

void library::moveUp()
{
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
}

void library::retranslateUi()
{
}

void library::tabEntered()
{
	if( m_settings.enableLibraryTab() && m_table.rowCount() == 0 ){

		this->showContents( m_currentPath ) ;
	}
}

void library::tabExited()
{
	m_continue = false ;
	if( m_scanContinue ){
		*m_scanContinue = false ;
	}
}

void library::textAlignmentChanged( Qt::LayoutDirection )
{
}

void library::capturePendingRows( const std::vector< int >& rows )
{
	m_pendingActionDirectory = m_currentPath ;
	m_pendingActionNames.clear() ;

	for( const auto row : rows ){
		if( row >= 0 && row < m_table.rowCount() ){
			m_pendingActionNames.append( m_table.item( row,1 ).text() ) ;
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
}

std::vector< int > library::pendingRows()
{
	std::vector< int > rows ;

	if( m_pendingActionDirectory.isEmpty() ||
		QDir::cleanPath( m_pendingActionDirectory ) != QDir::cleanPath( m_currentPath ) ){
		return rows ;
	}

	for( const auto& name : m_pendingActionNames ){
		bool found = false ;
		for( int row = 0 ; row < m_table.rowCount() ; row++ ){
			if( m_table.item( row,1 ).text() == name ){
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

bool library::deletePath( const QString& m )
{
	QFileInfo mm( m ) ;

	if( mm.isSymLink() ){

		QFile::remove( m ) ;

	}else if( mm.isDir() ){

		directoryManager::removeDirectory( m,m_continue ) ;
	}else{
		QFile::remove( m ) ;
	}

	mm.refresh() ;

	return mm.exists() ;
}

void library::deleteEntries( library::iter items )
{
	if( items.empty() ){

		return this->enableAll() ;
	}

	auto row = items.next() ;

	class meaw
	{
	public:
		meaw( library& library,library::iter items,int row ) :
			m_parent( library ),
			m_items( items.move() ),
			m_row( row ),
			m_path( this->path() )
		{
		}
		bool bg()
		{
			return m_parent.deletePath( m_path ) ;
		}
		void fg( bool s )
		{
			if( !s ){

				m_parent.m_table.removeRow( m_row ) ;
			}

			m_parent.deleteEntries( m_items.move() ) ;
		}
	private:
		QString path() const
		{
			auto s = m_parent.m_table.item( m_row,1 ).text() ;
			return m_parent.m_currentPath + "/" + s ;
		}
		library& m_parent ;
		library::iter m_items ;
		int m_row ;
		QString m_path ;
	} ;

	utils::qthread::run( meaw( *this,items.move(),row ) ) ;
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

	utility::rename( m_ctx,item,m_currentPath,nn,item.text() ) ;
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

		this->deleteEntries( row ) ;
	}
}

void library::deleteAll()
{
	this->disableAll() ;

	m_ui.pbLibraryCancel->setEnabled( true ) ;

	class meaw
	{
	public:
		meaw( library& library ) : m_parent( library )
		{
		}
		void bg()
		{
			const auto& a = m_parent.m_currentPath ;
			auto& b = m_parent.m_continue ;

			directoryManager::removeDirectoryContents( a,b ) ;
		}
		void fg()
		{
			m_parent.showContents( m_parent.m_currentPath ) ;
		}
	private:
		library& m_parent ;
	} ;

	utils::qthread::run( meaw( *this ) ) ;
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
	item.setTextAlignment( Qt::AlignCenter ) ;
	item.setFont( m_ctx.mainWidget().font() ) ;
}

void library::addEntrySlot( const directoryEntries::iter& s )
{
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

	this->addEntrySlot( m_directoryEntries.Iter() ) ;
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

void library::showContents( const QString& path )
{
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
		meaw( library * library,const QString& path,std::shared_ptr< std::atomic_bool > keepGoing ) :
			m_parent( library ),
			m_path( path ),
			m_continue( std::move( keepGoing ) )
		{
		}
		directoryEntries bg()
		{
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
		std::shared_ptr< std::atomic_bool > m_continue ;
	} ;

	utils::qthread::run( meaw( this,path,std::move( scanContinue ) ) ) ;
}
