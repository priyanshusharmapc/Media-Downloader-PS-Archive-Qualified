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

#ifndef LIBRARY_H
#define LIBRARY_H

#include "context.hpp"
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QList>
#include <QDir>

#include "settings.h"
#include "utility.h"
#include "tableWidget.h"
#include "directoryEntries.h"

class tabManager ;

#include <QObject>
#include <QPointer>

#include <memory>

class library : public QObject
{
	Q_OBJECT
public:
	library( const Context& ) ;
	void keyPressed( utility::mainWindowKeyCombo ) ;
	void init_done() ;
	void enableAll() ;
	void disableAll() ;
	void resetMenu() ;
	void exiting() ;
	void retranslateUi() ;
	void tabEntered() ;
	void tabExited() ;
	void textAlignmentChanged( Qt::LayoutDirection ) ;
#ifdef MDPS_LIBRARY_TEST_HOOKS
	static bool testPendingDirectoryMatches( const QString&,const QString&,const QByteArray&,const QByteArray& ) ;
	static bool testRemoveNativeEntry( const QByteArray&,const QByteArray&,const QByteArray&,std::atomic_bool& ) ;
	static bool testRemoveNativeDirectoryContents( const QByteArray&,const QByteArray&,std::atomic_bool& ) ;
#endif
private:
signals:
	void addEntrySignal( const directoryEntries::iter& ) ;
private:
	class iter
	{
	public:
		iter( int s )
		{
			m_entries.emplace_back( s ) ;
		}
		iter( std::vector< int > s ) : m_entries( std::move( s ) )
		{
		}
		bool empty() const
		{
			return m_entries.size() == 0 ;
		}
		int next()
		{
			auto m = m_entries.back() ;

			m_entries.pop_back() ;

			return m ;
		}
		iter move()
		{
			return std::move( *this ) ;
		}
	private:
		std::vector< int > m_entries ;
	} ;
	void deleteEntries( library::iter ) ;
	bool hasMultipleSelections() ;
	void capturePendingRows( const std::vector< int >& ) ;
	void capturePendingRow( int ) ;
	void capturePendingDirectory() ;
	std::vector< int > pendingRows() ;
	void clearPendingAction() ;
	bool deletePath( const QString& ) ;
	void setRenameUiVisible( bool ) ;
	void renameFile( int ) ;
	void deleteEntry( int ) ;
	void deleteAll( const QByteArray& confirmedNativePath = {} ) ;
	void addEntrySlot( const directoryEntries::iter& ) ;
	void cxMenuRequested( QPoint ) ;
	void arrangeAndShow() ;
	void arrangeEntries( int ) ;
	void showContents( const QString&,const QByteArray& nativePath = {} ) ;
	QByteArray nativeNameAt( int row ) const ;
	QByteArray nativePathAt( int row ) const ;
	void moveUp() ;
	void addItem( const directoryEntries::iter& ) ;
	const Context& m_ctx ;
	settings& m_settings ;
	std::atomic_bool m_continue ;
	std::shared_ptr< std::atomic_bool > m_scanContinue ;
	// Every queued row-population chain is stamped with this generation. A new
	// scan, sort or tab exit increments it before old queued events can resume.
	quint64 m_populationGeneration = 0 ;
	// Destructive workers own this cancellation token independently of the
	// Library QObject so shutdown never leaves a thread dereferencing m_continue.
	std::shared_ptr< std::atomic_bool > m_deleteContinue ;
	Ui::MainWindow& m_ui ;
	tableMiniWidget< directoryEntries::ICON,2 > m_table ;
	QString m_downloadFolder ;
	QString m_currentPath ;
	// Display paths remain QString, but POSIX filesystem authority is carried
	// independently as native bytes so undecodable names never round-trip
	// through Unicode before open/rename/delete/navigation.
	QByteArray m_downloadNativePath ;
	QByteArray m_currentNativePath ;
	// Confirmation actions are bound to this immutable view/identity snapshot,
	// never to the table's mutable current row at confirmation time.
	QString m_pendingActionDirectory ;
	QByteArray m_pendingActionNativeDirectory ;
	QStringList m_pendingActionNames ;
	QList< QByteArray > m_pendingActionNativeNames ;
	QPixmap m_folderIcon ;
	QPixmap m_videoIcon ;
	directoryEntries m_directoryEntries ;
};

#endif
