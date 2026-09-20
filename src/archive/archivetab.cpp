#include "ui_mainwindow.h"
#include "archivetab.h"
#include "archivesettings.h"

#include "../context.hpp"

#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFuture>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QMetaObject>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSet>
#include <QSplitter>
#include <QStorageInfo>
#include <QTableWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QtConcurrent/QtConcurrent>
#include <QUrl>

#include <algorithm>

namespace
{
QString readText(const QString& path)
{
    QFile f(path); if(!f.open(QIODevice::ReadOnly|QIODevice::Text)) return {}; return QString::fromUtf8(f.readAll());
}
QString pretty(const QString& s){return s.isEmpty()?QStringLiteral("-"):s;}

QString humanBytes(qint64 bytes)
{
    const char* units[]={"B","KB","MB","GB","TB"};
    double value=bytes;int unit=0;
    while(value>=1024.0&&unit<4){value/=1024.0;++unit;}
    return unit==0?QString("%1 %2").arg(bytes).arg(units[unit]):QString("%1 %2").arg(value,0,'f',1).arg(units[unit]);
}

int childDirectoryCount(const QString& path)
{
    return QDir(path).entryList(QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name).size();
}

bool pathInside(const QString& baseDir,const QString& path)
{
    if(baseDir.isEmpty()||path.isEmpty())return false;
    const auto base=QDir::cleanPath(QFileInfo(baseDir).absoluteFilePath());
    const auto child=QDir::cleanPath(QFileInfo(path).absoluteFilePath());
#ifdef Q_OS_WIN
    const auto cs=Qt::CaseInsensitive;
#else
    const auto cs=Qt::CaseSensitive;
#endif
    return child.compare(base,cs)==0||child.startsWith(base+QDir::separator(),cs);
}

struct RepresentationHealth
{
    bool present=false;
    bool verified=false;
    QString label;
};

RepresentationHealth representationHealth(const archive::Paths& paths,const archive::Representation& representation)
{
    if(representation.state!="complete")return {false,false,QStringLiteral("Not complete")};
    if(representation.path.isEmpty()||!paths.isSafeRelative(representation.path))
        return {false,false,QStringLiteral("Missing")};
    const QFileInfo file(paths.absoluteFromRelative(representation.path));
    if(!file.isFile()||file.size()<=0)return {false,false,QStringLiteral("Missing")};

    // verifiedAt is durable proof that a full verifier pass completed. Treat it
    // as current only while the exact stored path remains a safe, non-empty file
    // whose modification time has not advanced since that pass. This keeps GUI
    // refresh cheap while failing closed after ordinary deletion/truncation/edit.
    auto verifiedAt=QDateTime::fromString(representation.verifiedAt,Qt::ISODateWithMs);
    if(!verifiedAt.isValid())verifiedAt=QDateTime::fromString(representation.verifiedAt,Qt::ISODate);
    if(!verifiedAt.isValid()||file.lastModified().toUTC()>verifiedAt.toUTC())
        return {true,false,QStringLiteral("Verification stale")};
    return {true,true,QStringLiteral("Verified")};
}

QString representationDisplay(const archive::Representation& representation,const RepresentationHealth& health)
{
    return representation.state=="complete"
        ? QStringLiteral("complete · %1").arg(health.label)
        : representation.state;
}

QString healthAdjustedStatus(const archive::PlaylistItem& playlist,const archive::CanonicalItem* canonical,const archive::Paths& paths)
{
    const auto durable=archive::derivedStatus(playlist,canonical);
    if(!canonical)return durable;
    const auto video=representationHealth(paths,canonical->video);
    const auto audio=representationHealth(paths,canonical->audio);
    const bool bothPresent=video.present&&audio.present;
    const bool bothVerified=video.verified&&audio.verified;
    if(durable=="Protected"||durable=="Unavailable · Archived"){
        if(!bothPresent)return QStringLiteral("Missing");
        if(!bothVerified)return QStringLiteral("Verification Stale");
    }else if(durable=="Removed · Archived"&&!bothVerified){
        return QStringLiteral("Removed");
    }
    return durable;
}
}

ArchiveTab::ArchiveTab(const Context& ctx):ArchiveTab(*ctx.Ui().tabWidget,&ctx.mainWidget()) {}

ArchiveTab::ArchiveTab(QTabWidget& hostTabs,QWidget* owner):QObject(owner?owner:&hostTabs),m_hostTabs(hostTabs)
{
    buildUi(); wireUi(); updateActionState();
}

ArchiveTab::~ArchiveTab(){exiting();}

void ArchiveTab::buildUi()
{
    m_page=new QWidget(&m_hostTabs);
    m_page->setObjectName("archiveModeTab");
    auto* rootLayout=new QVBoxLayout(m_page);
    rootLayout->setContentsMargins(8,8,8,8); rootLayout->setSpacing(7);

    auto* rootRow=new QHBoxLayout;
    m_rootTitle=new QLabel(tr("Archive root:"),m_page); m_rootLabel=new QLabel(m_page); m_rootLabel->setObjectName("archiveRootPath"); m_rootLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_browse=new QPushButton(tr("Browse…"),m_page); m_browse->setObjectName("archiveBrowseRoot");
    rootRow->addWidget(m_rootTitle); rootRow->addWidget(m_rootLabel,1); rootRow->addWidget(m_browse); rootLayout->addLayout(rootRow);
    QObject::connect(m_browse,&QPushButton::clicked,this,&ArchiveTab::browseRoot);

    auto* overviewRow=new QHBoxLayout;
    m_systemGroup=new QGroupBox(tr("SYSTEM HEALTH"),m_page);
    auto* systemGrid=new QGridLayout(m_systemGroup);
    systemGrid->setColumnStretch(1,1);
    auto addSystemRow=[&](int row,const QString& name,QLabel*& value){
        auto* label=new QLabel(name,m_systemGroup); m_systemNameLabels.append(label); value=new QLabel(tr("Not checked"),m_systemGroup); value->setTextInteractionFlags(Qt::TextSelectableByMouse); value->setWordWrap(true);
        systemGrid->addWidget(label,row,0);systemGrid->addWidget(value,row,1);
    };
    addSystemRow(0,tr("Overall"),m_systemOverall);
    addSystemRow(1,tr("Archive root"),m_systemRoot);
    addSystemRow(2,tr("Canonical state"),m_systemState);
    addSystemRow(3,tr("Runtime tools"),m_systemTools);
    addSystemRow(4,tr("Disk"),m_systemDisk);
    addSystemRow(5,tr("Writer lock"),m_systemLock);
    addSystemRow(6,tr("Recovery imports"),m_systemImports);
    addSystemRow(7,tr("Latest scan"),m_systemLastScan);
    QFont overallFont=m_systemOverall->font();overallFont.setBold(true);m_systemOverall->setFont(overallFont);
    m_systemAttention=new QLabel(tr("Attention: Not checked"),m_systemGroup);m_systemAttention->setWordWrap(true);
    m_systemWorkload=new QLabel(tr("Archive workload: Not checked"),m_systemGroup);m_systemWorkload->setWordWrap(true);
    systemGrid->addWidget(m_systemAttention,8,0,1,2);systemGrid->addWidget(m_systemWorkload,9,0,1,2);

    m_operationGroup=new QGroupBox(tr("CURRENT OPERATION"),m_page);
    auto* operationLayout=new QVBoxLayout(m_operationGroup);
    m_operationName=new QLabel(tr("Idle"),m_operationGroup);QFont opFont=m_operationName->font();opFont.setBold(true);m_operationName->setFont(opFont);
    m_operationStage=new QLabel(tr("No operation running"),m_operationGroup);
    m_operationDetail=new QLabel(tr("Archive Mode is idle."),m_operationGroup);m_operationDetail->setWordWrap(true);
    m_operationProgress=new QProgressBar(m_operationGroup);m_operationProgress->setRange(0,1);m_operationProgress->setValue(0);m_operationProgress->setFormat(tr("Idle"));
    m_operationFailures=new QLabel(tr("Failures: 0"),m_operationGroup);
    operationLayout->addWidget(m_operationName);operationLayout->addWidget(m_operationStage);operationLayout->addWidget(m_operationDetail);operationLayout->addWidget(m_operationProgress);operationLayout->addWidget(m_operationFailures);operationLayout->addStretch();
    overviewRow->addWidget(m_systemGroup,3);overviewRow->addWidget(m_operationGroup,2);rootLayout->addLayout(overviewRow);

    auto* actions=new QHBoxLayout;
    m_add=new QPushButton(tr("Add Playlist"),m_page); m_remove=new QPushButton(tr("Remove"),m_page); m_scan=new QPushButton(tr("Scan"),m_page);
    m_syncSelected=new QPushButton(tr("Sync Selected"),m_page); m_syncAll=new QPushButton(tr("Sync All"),m_page);
    m_stop=new QPushButton(tr("Stop After Current"),m_page); m_retry=new QPushButton(tr("Retry Failed"),m_page);
    m_more=new QToolButton(m_page); m_more->setText(tr("More")); m_more->setPopupMode(QToolButton::InstantPopup);
    for(auto* b:{m_add,m_remove,m_scan,m_syncSelected,m_syncAll,m_stop,m_retry}) actions->addWidget(b);
    actions->addStretch(); actions->addWidget(m_more); rootLayout->addLayout(actions);

    auto* splitter=new QSplitter(Qt::Horizontal,m_page);
    auto* left=new QWidget(splitter); auto* leftLayout=new QVBoxLayout(left); leftLayout->setContentsMargins(0,0,0,0);
    m_playlistLabel=new QLabel(tr("PLAYLISTS"),left); m_sources=new QListWidget(left); m_sources->setMinimumWidth(210);
    leftLayout->addWidget(m_playlistLabel); leftLayout->addWidget(m_sources,1);

    auto* right=new QWidget(splitter); auto* rightLayout=new QVBoxLayout(right); rightLayout->setContentsMargins(0,0,0,0);
    m_healthLabel=new QLabel(tr("No playlist selected"),right); m_healthLabel->setWordWrap(true); rightLayout->addWidget(m_healthLabel);
    auto* filterRow=new QHBoxLayout; m_search=new QLineEdit(right); m_search->setPlaceholderText(tr("Search archive items…"));
    m_filter=new QComboBox(right);
    m_filter->addItem(tr("All"),QStringLiteral("all"));
    m_filter->addItem(tr("Protected"),QStringLiteral("protected"));
    m_filter->addItem(tr("Needs Sync"),QStringLiteral("needs_sync"));
    m_filter->addItem(tr("Missing"),QStringLiteral("missing"));
    m_filter->addItem(tr("Unavailable"),QStringLiteral("unavailable"));
    m_filter->addItem(tr("Removed"),QStringLiteral("removed"));
    m_filter->addItem(tr("Failed"),QStringLiteral("failed"));
    m_filter->addItem(tr("Interrupted"),QStringLiteral("interrupted"));
    filterRow->addWidget(m_search,1); filterRow->addWidget(m_filter); rightLayout->addLayout(filterRow);
    m_table=new QTableWidget(right); m_table->setColumnCount(6); m_table->setHorizontalHeaderLabels({tr("#"),tr("Title"),tr("Availability"),tr("Video"),tr("Audio"),tr("Status")});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows); m_table->setSelectionMode(QAbstractItemView::SingleSelection); m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false); m_table->horizontalHeader()->setStretchLastSection(false); m_table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch); for(int c=2;c<6;++c)m_table->horizontalHeader()->setSectionResizeMode(c,QHeaderView::ResizeToContents);
    rightLayout->addWidget(m_table,3);

    m_detailsTabs=new QTabWidget(right); m_sourceDetails=new QTextEdit; m_archiveDetails=new QTextEdit; m_historyDetails=new QTextEdit; m_recoveryDetails=new QTextEdit;
    for(auto* e:{m_sourceDetails,m_archiveDetails,m_historyDetails,m_recoveryDetails}) e->setReadOnly(true);
    m_detailsTabs->addTab(m_sourceDetails,tr("Source")); m_detailsTabs->addTab(m_archiveDetails,tr("Archive")); m_detailsTabs->addTab(m_historyDetails,tr("History")); m_detailsTabs->addTab(m_recoveryDetails,tr("Recovery"));
    m_detailsTabs->setMaximumHeight(210); rightLayout->addWidget(m_detailsTabs,1);
    splitter->addWidget(left); splitter->addWidget(right); splitter->setStretchFactor(0,0); splitter->setStretchFactor(1,1); splitter->setSizes({230,850}); rootLayout->addWidget(splitter,1);

    auto* statusRow=new QHBoxLayout; m_statusLabel=new QLabel(tr("Idle"),m_page); m_activityToggle=new QPushButton(tr("▸ Activity"),m_page); m_activityToggle->setCheckable(true);
    statusRow->addWidget(m_statusLabel,1); statusRow->addWidget(m_activityToggle); rootLayout->addLayout(statusRow);
    m_activity=new QPlainTextEdit(m_page); m_activity->setReadOnly(true); m_activity->setMaximumBlockCount(1000); m_activity->setMaximumHeight(180); m_activity->hide(); rootLayout->addWidget(m_activity);

    auto* menu=new QMenu(m_more);
    m_openRootAction=menu->addAction(tr("Open Archive Folder")); m_openPlaylistAction=menu->addAction(tr("Open Playlist Folder"));
    menu->addSeparator(); m_openCatalogAction=menu->addAction(tr("Open Catalog")); m_openMissingAction=menu->addAction(tr("Open Missing Report"));
    menu->addSeparator(); m_importsAction=menu->addAction(tr("Process External Imports")); m_openLogsAction=menu->addAction(tr("Open Logs"));
    m_more->setMenu(menu);
    QObject::connect(m_openRootAction,&QAction::triggered,[this]{openPath(m_root);});
    QObject::connect(m_openPlaylistAction,&QAction::triggered,this,&ArchiveTab::openSelectedPlaylistFolder);
    QObject::connect(m_openCatalogAction,&QAction::triggered,[this]{openProjection("catalog.csv");});
    QObject::connect(m_openMissingAction,&QAction::triggered,[this]{openProjection("missing.csv");});
    QObject::connect(m_importsAction,&QAction::triggered,this,&ArchiveTab::processImports);
    QObject::connect(m_openLogsAction,&QAction::triggered,[this]{if(!m_root.isEmpty())openPath(archive::Paths(m_root).activityLogs());});

    m_hostTabs.addTab(m_page,tr("Archive"));
}

void ArchiveTab::wireUi()
{
    QObject::connect(m_add,&QPushButton::clicked,this,&ArchiveTab::addPlaylist);
    QObject::connect(m_remove,&QPushButton::clicked,this,&ArchiveTab::removePlaylist);
    QObject::connect(m_scan,&QPushButton::clicked,this,&ArchiveTab::scanSelected);
    QObject::connect(m_syncSelected,&QPushButton::clicked,this,&ArchiveTab::syncSelected);
    QObject::connect(m_syncAll,&QPushButton::clicked,this,&ArchiveTab::syncAll);
    QObject::connect(m_retry,&QPushButton::clicked,this,&ArchiveTab::retryFailed);
    QObject::connect(m_stop,&QPushButton::clicked,this,&ArchiveTab::stopAfterCurrent);
    QObject::connect(m_sources,&QListWidget::currentRowChanged,[this]{updateActionState();refreshTable();refreshDetails();});
    QObject::connect(m_table,&QTableWidget::itemSelectionChanged,this,&ArchiveTab::refreshDetails);
    QObject::connect(m_search,&QLineEdit::textChanged,this,&ArchiveTab::refreshTable);
    QObject::connect(m_filter,QOverload<int>::of(&QComboBox::currentIndexChanged),[this](int){refreshTable();});
    QObject::connect(m_activityToggle,&QPushButton::toggled,[this](bool checked){m_activity->setVisible(checked);m_activityToggle->setText(checked?tr("▾ Activity"):tr("▸ Activity"));if(checked)refreshActivity();});
}

QString ArchiveTab::configuredRoot() const
{
    return archive::ui::configuredRoot(QCoreApplication::applicationDirPath(),{});
}

archive::RuntimeConfig ArchiveTab::runtimeConfig() const{return {m_root,QCoreApplication::applicationDirPath()};}

bool ArchiveTab::ensureReady(QString* error)
{
    if(error)error->clear();
    if(m_root.isEmpty()){
        m_root=archive::ui::configuredRoot(QCoreApplication::applicationDirPath(),{},error);
        if(error&&!error->isEmpty()){m_ready=false;updateActionState();return false;}
    }
    // Display the configured identity even when the volume is offline. Never
    // initialize a guessed root or recreate an absent mount point from the UI.
    m_rootLabel->setText(m_root.isEmpty()?tr("Not configured"):QDir::toNativeSeparators(m_root));
    m_ready=archive::ui::initializeRoot(m_root,error);
    updateActionState();
    return m_ready;
}

void ArchiveTab::init_done(){m_root=configuredRoot();refreshAll();}
void ArchiveTab::enableAll(){m_controlsEnabled=true;updateActionState();}
void ArchiveTab::disableAll(){m_controlsEnabled=false;updateActionState();}
void ArchiveTab::resetMenu(){}
void ArchiveTab::exiting(){m_stopRequested=true;if(m_watcher&&m_watcher->isRunning())m_watcher->waitForFinished();}
void ArchiveTab::retranslateUi()
{
    if(!m_page)return;

    m_rootTitle->setText(tr("Archive root:"));
    m_systemGroup->setTitle(tr("SYSTEM HEALTH"));
    m_operationGroup->setTitle(tr("CURRENT OPERATION"));
    m_playlistLabel->setText(tr("PLAYLISTS"));

    const QStringList systemNames={tr("Overall"),tr("Archive root"),tr("Canonical state"),tr("Runtime tools"),
                                   tr("Disk"),tr("Writer lock"),tr("Recovery imports"),tr("Latest scan")};
    for(int i=0;i<m_systemNameLabels.size()&&i<systemNames.size();++i)
        m_systemNameLabels[i]->setText(systemNames[i]);

    m_browse->setText(tr("Browse…"));
    m_add->setText(tr("Add Playlist"));
    m_remove->setText(tr("Remove"));
    m_scan->setText(tr("Scan"));
    m_syncSelected->setText(tr("Sync Selected"));
    m_syncAll->setText(tr("Sync All"));
    m_stop->setText(tr("Stop After Current"));
    m_retry->setText(tr("Retry Failed"));
    m_more->setText(tr("More"));
    m_search->setPlaceholderText(tr("Search archive items…"));

    const QStringList filters={tr("All"),tr("Protected"),tr("Needs Sync"),tr("Missing"),tr("Unavailable"),tr("Removed"),tr("Failed"),tr("Interrupted")};
    for(int i=0;i<m_filter->count()&&i<filters.size();++i)m_filter->setItemText(i,filters[i]);

    m_table->setHorizontalHeaderLabels({tr("#"),tr("Title"),tr("Availability"),tr("Video"),tr("Audio"),tr("Status")});
    m_detailsTabs->setTabText(0,tr("Source"));
    m_detailsTabs->setTabText(1,tr("Archive"));
    m_detailsTabs->setTabText(2,tr("History"));
    m_detailsTabs->setTabText(3,tr("Recovery"));
    m_activityToggle->setText(m_activityToggle->isChecked()?tr("▾ Activity"):tr("▸ Activity"));

    m_openRootAction->setText(tr("Open Archive Folder"));
    m_openPlaylistAction->setText(tr("Open Playlist Folder"));
    m_openCatalogAction->setText(tr("Open Catalog"));
    m_openMissingAction->setText(tr("Open Missing Report"));
    m_importsAction->setText(tr("Process External Imports"));
    m_openLogsAction->setText(tr("Open Logs"));

    const int tabIndex=m_hostTabs.indexOf(m_page);
    if(tabIndex>=0)m_hostTabs.setTabText(tabIndex,tr("Archive"));

    // Dynamic health text is also translated presentation state. Recompute it
    // under the newly installed translator instead of leaving old-language
    // status values beside freshly translated static labels.
    refreshSystemHealth();
}
void ArchiveTab::tabEntered(){refreshAll();}
void ArchiveTab::tabExited(){}
void ArchiveTab::keyPressed(utility::mainWindowKeyCombo){}
void ArchiveTab::textAlignmentChanged(Qt::LayoutDirection d){m_page->setLayoutDirection(d);}

void ArchiveTab::refreshAll()
{
    if(m_root.isEmpty())m_root=configuredRoot();
    if(m_busy){refreshSystemHealth();return;}
    QString e;if(!ensureReady(&e)){m_statusLabel->setText(e);refreshSystemHealth();return;}
    refreshSystemHealth();refreshSources();refreshTable();refreshDetails();if(m_activityToggle->isChecked())refreshActivity();
}

void ArchiveTab::refreshSystemHealth()
{
    if(!m_systemOverall)return;
    if(m_root.isEmpty()){
        m_systemOverall->setText(tr("NOT CONFIGURED"));m_systemRoot->setText(tr("No Archive Root selected"));m_systemState->setText(tr("Not checked"));
        m_systemTools->setText(tr("Not checked"));m_systemDisk->setText(tr("Not checked"));m_systemLock->setText(tr("Not checked"));m_systemImports->setText(tr("Not checked"));m_systemLastScan->setText(tr("Never"));
        m_systemAttention->setText(tr("Attention: Select an Archive Root."));m_systemWorkload->setText(tr("Archive workload: unavailable until a root is selected."));return;
    }

    // Store reads take a lock that can create layout directories. Availability
    // must therefore be checked before constructing/reading a Store, even in
    // this nominally read-only health refresh and while a worker is running.
    QString availabilityError;
    if(!archive::ui::rootAvailable(m_root,&availabilityError)){
        m_systemOverall->setText(tr("UNAVAILABLE"));m_systemRoot->setText(availabilityError);
        m_systemState->setText(tr("Not checked"));m_systemTools->setText(tr("Not checked"));
        m_systemDisk->setText(tr("Not checked"));m_systemLock->setText(tr("Not checked"));
        m_systemImports->setText(tr("Not checked"));m_systemLastScan->setText(tr("Not checked"));
        m_systemAttention->setText(tr("Attention: Reconnect the configured root or use Browse."));
        m_systemWorkload->setText(tr("Archive workload: unavailable; no fallback archive opened."));
        updateActionState();return;
    }
    archive::Paths paths(m_root);archive::Store store(paths);QStringList alerts;
    const QFileInfo rootInfo(m_root);const bool rootExists=rootInfo.exists()&&rootInfo.isDir();const bool rootWritable=rootExists&&rootInfo.isWritable();
    m_systemRoot->setText(rootExists?(rootWritable?tr("Ready and writable"):tr("Readable but not writable")):tr("Missing or inaccessible"));
    if(!rootExists)alerts<<tr("Archive Root is missing or inaccessible");else if(!rootWritable)alerts<<tr("Archive Root is not writable");

    QString error;const auto sources=store.loadSources(&error);bool stateOk=error.isEmpty();if(!error.isEmpty())alerts<<tr("sources.json: %1").arg(error);
    error.clear();const auto canonical=store.loadCanonicalItems(&error);if(!error.isEmpty()){stateOk=false;alerts<<tr("items.json: %1").arg(error);}
    QSet<QString> canonicalKeys;int duplicateCanonical=0;int incomplete=0;int failed=0;int interrupted=0;int staleRunning=0;int unrecovered=0;int unavailable=0;
    for(const auto& c:canonical){
        if(canonicalKeys.contains(c.key))++duplicateCanonical;else canonicalKeys.insert(c.key);
        if(c.video.state!="complete"||c.audio.state!="complete")++incomplete;
        if(c.video.state=="failed"||c.audio.state=="failed")++failed;
        if(c.video.state=="interrupted"||c.audio.state=="interrupted")++interrupted;
        if(!m_busy&&(c.video.state=="running"||c.audio.state=="running"))++staleRunning;
        if(c.recoveryStatus=="unrecovered")++unrecovered;
        if(c.availability!="public")++unavailable;
    }
    if(duplicateCanonical>0){stateOk=false;alerts<<tr("%1 duplicate canonical key(s)").arg(duplicateCanonical);}

    int occurrenceRefs=0;int missingRefs=0;int invalidEntryKeys=0;int failedSources=0;QDateTime latestScan;
    for(const auto& source:sources){
        if(!source.lastScanAt.isEmpty()){
            auto dt=QDateTime::fromString(source.lastScanAt,Qt::ISODateWithMs);if(!dt.isValid())dt=QDateTime::fromString(source.lastScanAt,Qt::ISODate);if(dt.isValid()&&(!latestScan.isValid()||dt>latestScan))latestScan=dt;
            if(!QFileInfo::exists(paths.playlistItemsFile(source.key))){stateOk=false;alerts<<tr("Managed source %1 is missing items.json").arg(source.title.isEmpty()?source.key:source.title);}
        }
        const auto status=source.lastScanStatus.toLower();if(!source.lastError.trimmed().isEmpty()||status.contains("fail")||status.contains("error")||status.contains("incomplete"))++failedSources;
        QString playlistError;const auto playlist=store.loadPlaylistItems(source.key,&playlistError);if(!playlistError.isEmpty()){stateOk=false;alerts<<tr("Playlist state %1: %2").arg(source.key,playlistError);continue;}
        QSet<QString> entryKeys;
        for(const auto& p:playlist){
            ++occurrenceRefs;if(!canonicalKeys.contains(p.itemKey))++missingRefs;
            if(p.entryKey.isEmpty()||entryKeys.contains(p.entryKey))++invalidEntryKeys;else entryKeys.insert(p.entryKey);
        }
    }
    if(missingRefs>0){stateOk=false;alerts<<tr("%1 playlist reference(s) have no canonical item").arg(missingRefs);}
    if(invalidEntryKeys>0){stateOk=false;alerts<<tr("%1 missing or duplicate occurrence key(s)").arg(invalidEntryKeys);}
    if(failed>0)alerts<<tr("%1 canonical item(s) contain failed representations").arg(failed);
    if(interrupted>0)alerts<<tr("%1 canonical item(s) contain interrupted representations").arg(interrupted);
    if(staleRunning>0)alerts<<tr("%1 representation(s) still say running while the GUI is idle").arg(staleRunning);
    if(failedSources>0)alerts<<tr("%1 source(s) report a failed or incomplete latest scan").arg(failedSources);
    m_systemState->setText(stateOk?tr("OK | %1 sources | %2 canonical items | %3 occurrence refs").arg(sources.size()).arg(canonical.size()).arg(occurrenceRefs):tr("INTEGRITY ATTENTION | %1 sources | %2 canonical items").arg(sources.size()).arg(canonical.size()));

    archive::ToolResolver tools(runtimeConfig());const QStringList toolPaths={tools.ytDlp(),tools.ffmpeg(),tools.ffprobe(),tools.deno()};int resolvedTools=0;int packagedTools=0;
    for(const auto& tool:toolPaths){if(!tool.isEmpty()&&QFileInfo::exists(tool)){++resolvedTools;if(pathInside(QCoreApplication::applicationDirPath(),tool))++packagedTools;}}
    const bool toolsOk=resolvedTools==toolPaths.size();m_systemTools->setText(tr("%1/4 resolved | %2 packaged").arg(resolvedTools).arg(packagedTools));
    if(!toolsOk)alerts<<tr("One or more required runtimes are missing");else if(packagedTools<resolvedTools)alerts<<tr("%1 runtime tool(s) resolve outside the application package").arg(resolvedTools-packagedTools);

    QStorageInfo storage(m_root);if(storage.isValid()&&storage.isReady()){
        const auto available=storage.bytesAvailable();const auto total=storage.bytesTotal();const double pct=total>0?(100.0*double(available)/double(total)):0.0;
        m_systemDisk->setText(tr("%1 free | %2% available").arg(humanBytes(available)).arg(pct,0,'f',1));
        const qint64 fiveGiB=5LL*1024LL*1024LL*1024LL;if(available<fiveGiB||(total>0&&pct<5.0))alerts<<tr("Disk headroom is low");
    }else{m_systemDisk->setText(tr("Storage status unavailable"));alerts<<tr("Storage status could not be read");}

    if(m_busy)m_systemLock->setText(tr("Held by this GUI operation"));
    else if(rootExists&&rootWritable){archive::SyncLock lock(paths);if(lock.tryLock(0)){m_systemLock->setText(tr("Available"));lock.unlock();}else{m_systemLock->setText(tr("Held by another writer"));alerts<<tr("Another archive writer currently owns the lock");}}
    else m_systemLock->setText(tr("Unavailable"));

    const int pending=childDirectoryCount(paths.importsPending());const int accepted=childDirectoryCount(paths.importsAccepted());const int rejected=childDirectoryCount(paths.importsRejected());
    m_systemImports->setText(tr("Pending %1 | Accepted %2 | Rejected %3").arg(pending).arg(accepted).arg(rejected));
    if(sources.isEmpty())m_systemLastScan->setText(tr("No sources registered"));else if(latestScan.isValid())m_systemLastScan->setText(tr("%1 | %2 source issue(s)").arg(latestScan.toLocalTime().toString(Qt::ISODate)).arg(failedSources));else m_systemLastScan->setText(tr("Never"));

    const bool hardError=!rootExists||!rootWritable||!stateOk||!toolsOk;
    m_systemOverall->setText(hardError?tr("ERROR"):(!alerts.isEmpty()?tr("ATTENTION"):tr("HEALTHY")));
    m_systemAttention->setText(alerts.isEmpty()?tr("Attention: No system-level alerts detected."):tr("Attention: %1").arg(alerts.join(tr(" | "))));
    m_systemWorkload->setText(tr("Archive workload: %1 incomplete media item(s) | %2 unrecovered | %3 unavailable | %4 pending import(s) | %5 rejected import(s)").arg(incomplete).arg(unrecovered).arg(unavailable).arg(pending).arg(rejected));
}

void ArchiveTab::refreshSources()
{
    if(!m_ready||!archive::ui::rootAvailable(m_root))return;
    const auto current=selectedSourceKey(); archive::Store store{archive::Paths(m_root)}; const auto sources=store.loadSources(); m_sources->blockSignals(true);m_sources->clear();int selected=-1;
    for(int i=0;i<sources.size();++i){auto* item=new QListWidgetItem(sources[i].title.isEmpty()?sources[i].key:sources[i].title,m_sources);item->setData(Qt::UserRole,sources[i].key);item->setToolTip(sources[i].url);if(sources[i].key==current)selected=i;}
    if(selected<0&&m_sources->count()>0)selected=0;if(selected>=0)m_sources->setCurrentRow(selected);m_sources->blockSignals(false);
    updateActionState();
}

archive::Source ArchiveTab::selectedSource() const
{
    if(!m_ready||!archive::ui::rootAvailable(m_root))return {};
    const auto key=selectedSourceKey(); archive::Store store{archive::Paths(m_root)};for(const auto& s:store.loadSources())if(s.key==key)return s;return {};
}
QString ArchiveTab::selectedSourceKey() const{auto* i=m_sources?m_sources->currentItem():nullptr;return i?i->data(Qt::UserRole).toString():QString();}
QString ArchiveTab::selectedItemKey() const{const auto rows=m_table?m_table->selectionModel()->selectedRows():QModelIndexList{};if(rows.isEmpty())return {};auto* i=m_table->item(rows.first().row(),0);return i?i->data(Qt::UserRole+1).toString():QString();}
QString ArchiveTab::selectedEntryKey() const{const auto rows=m_table?m_table->selectionModel()->selectedRows():QModelIndexList{};if(rows.isEmpty())return {};auto* i=m_table->item(rows.first().row(),0);return i?i->data(Qt::UserRole).toString():QString();}

QVector<archive::CanonicalItem> ArchiveTab::itemsForSource(const archive::Source& source) const
{
    if(!m_ready||!archive::ui::rootAvailable(m_root))return {};
    archive::Store store{archive::Paths(m_root)};const auto playlist=store.loadPlaylistItems(source.key);const auto all=store.loadCanonicalItems();QHash<QString,archive::CanonicalItem> map;for(const auto& c:all)map[c.key]=c;QVector<archive::CanonicalItem> out;for(const auto& p:playlist)if(p.membership=="active"&&map.contains(p.itemKey))out.append(map[p.itemKey]);return out;
}

void ArchiveTab::refreshTable()
{
    if(!m_ready||!m_table||!archive::ui::rootAvailable(m_root))return;

    const auto selectedEntry=selectedEntryKey();
    const auto source=selectedSource();
    if(source.key.isEmpty()){
        m_stateReadable=true;
        m_table->setRowCount(0);
        m_healthLabel->setText(tr("No playlist selected"));
        updateActionState();
        return;
    }

    const archive::Paths paths(m_root);
    archive::Store store{paths};

    QString playlistError;
    const auto playlist=store.loadPlaylistItems(source.key,&playlistError);
    if(!playlistError.isEmpty()){
        m_stateReadable=false;
        m_table->setRowCount(0);
        m_sourceDetails->clear();m_archiveDetails->clear();m_historyDetails->clear();m_recoveryDetails->clear();
        const auto message=tr("Playlist state error: %1").arg(playlistError);
        m_healthLabel->setText(message);m_statusLabel->setText(message);
        updateActionState();
        return;
    }

    QString canonicalError;
    const auto all=store.loadCanonicalItems(&canonicalError);
    if(!canonicalError.isEmpty()){
        m_stateReadable=false;
        m_table->setRowCount(0);
        m_sourceDetails->clear();m_archiveDetails->clear();m_historyDetails->clear();m_recoveryDetails->clear();
        const auto message=tr("Canonical state error: %1").arg(canonicalError);
        m_healthLabel->setText(message);m_statusLabel->setText(message);
        updateActionState();
        return;
    }

    m_stateReadable=true;
    QHash<QString,archive::CanonicalItem> map;for(const auto& item:all)map[item.key]=item;
    const auto search=m_search->text().trimmed();
    auto filter=m_filter->currentData().toString();
    static const QSet<QString> knownFilters={"all","protected","needs_sync","missing","unavailable","removed","failed","interrupted"};
    if(!knownFilters.contains(filter))filter="all";

    int protectedCount=0,needs=0,unavailable=0,removed=0,selectedRow=-1;
    m_table->setRowCount(0);
    for(const auto& p:playlist){
        const bool has=map.contains(p.itemKey);
        const auto item=has?map[p.itemKey]:archive::CanonicalItem{};
        const auto video=has?representationHealth(paths,item.video):RepresentationHealth{};
        const auto audio=has?representationHealth(paths,item.audio):RepresentationHealth{};
        const auto status=healthAdjustedStatus(p,has?&item:nullptr,paths);

        if(status=="Protected")++protectedCount;
        if(status.contains("Needs")||status=="Missing"||status=="Failed"||status=="Interrupted"||status=="Verification Stale")++needs;
        if(p.availability!="public")++unavailable;
        if(p.membership=="removed")++removed;

        if(!search.isEmpty()&&!p.title.contains(search,Qt::CaseInsensitive)&&!p.providerId.contains(search,Qt::CaseInsensitive))continue;

        if(filter!="all"){
            bool match=false;
            if(filter=="protected")match=status=="Protected";
            else if(filter=="needs_sync")match=status=="Needs Sync"||status=="Verification Stale";
            else if(filter=="missing")match=!has||status=="Missing"||video.label=="Missing"||audio.label=="Missing";
            else if(filter=="unavailable")match=p.availability!="public";
            else if(filter=="removed")match=p.membership=="removed";
            else if(filter=="failed")match=status=="Failed"||item.recoveryStatus=="failed";
            else if(filter=="interrupted")match=status=="Interrupted"||item.recoveryStatus=="interrupted";
            if(!match)continue;
        }

        const int row=m_table->rowCount();
        m_table->insertRow(row);
        auto* pos=new QTableWidgetItem(p.position<0?QStringLiteral("-"):QString::number(p.position));
        pos->setData(Qt::UserRole,p.entryKey);
        pos->setData(Qt::UserRole+1,p.itemKey);
        m_table->setItem(row,0,pos);
        m_table->setItem(row,1,new QTableWidgetItem(p.title));
        m_table->setItem(row,2,new QTableWidgetItem(p.availability));
        m_table->setItem(row,3,new QTableWidgetItem(has?representationDisplay(item.video,video):"missing"));
        m_table->setItem(row,4,new QTableWidgetItem(has?representationDisplay(item.audio,audio):"missing"));
        m_table->setItem(row,5,new QTableWidgetItem(status));
        if(!selectedEntry.isEmpty()&&p.entryKey==selectedEntry)selectedRow=row;
    }

    if(selectedRow>=0)m_table->selectRow(selectedRow);
    m_healthLabel->setText(tr("%1  |  All %2  |  Protected %3  |  Needs Work %4  |  Unavailable %5  |  Removed %6  |  Last scan %7")
        .arg(source.title.isEmpty()?source.key:source.title).arg(playlist.size()).arg(protectedCount).arg(needs).arg(unavailable).arg(removed).arg(pretty(source.lastScanAt)));
    updateActionState();
}

void ArchiveTab::refreshDetails()
{
    if(!m_ready||!archive::ui::rootAvailable(m_root))return;

    const auto source=selectedSource();
    const auto itemKey=selectedItemKey();
    const auto entryKey=selectedEntryKey();
    if(source.key.isEmpty()||itemKey.isEmpty()||entryKey.isEmpty()){
        m_sourceDetails->clear();m_archiveDetails->clear();m_historyDetails->clear();m_recoveryDetails->clear();
        return;
    }

    archive::Store store{archive::Paths(m_root)};
    QString playlistError;
    const auto playlist=store.loadPlaylistItems(source.key,&playlistError);
    QString canonicalError;
    const auto canonical=store.loadCanonicalItems(&canonicalError);
    if(!playlistError.isEmpty()||!canonicalError.isEmpty()){
        m_stateReadable=false;
        const auto message=tr("Archive state error: %1").arg(!playlistError.isEmpty()?playlistError:canonicalError);
        m_sourceDetails->setPlainText(message);m_archiveDetails->clear();m_historyDetails->clear();m_recoveryDetails->clear();
        m_statusLabel->setText(message);updateActionState();return;
    }

    m_stateReadable=true;
    archive::PlaylistItem occurrence;archive::CanonicalItem item;bool haveOccurrence=false,haveItem=false;
    for(const auto& candidate:playlist)if(candidate.entryKey==entryKey){occurrence=candidate;haveOccurrence=true;break;}
    for(const auto& candidate:canonical)if(candidate.key==itemKey){item=candidate;haveItem=true;break;}

    if(!haveOccurrence||occurrence.itemKey!=itemKey){
        m_sourceDetails->clear();m_archiveDetails->clear();m_historyDetails->clear();m_recoveryDetails->clear();
        if(haveOccurrence)m_statusLabel->setText(tr("Archive selection identity mismatch; refresh the view."));
        updateActionState();
        return;
    }

    m_sourceDetails->setPlainText(tr("Title: %1\nYouTube ID: %2\nOriginal URL: %3\nPlaylist position: %4\nMembership: %5\nAvailability: %6\nFirst seen: %7\nLast seen: %8")
        .arg(occurrence.title,pretty(occurrence.providerId),pretty(occurrence.url)).arg(occurrence.position)
        .arg(occurrence.membership,occurrence.availability,pretty(occurrence.firstSeen),pretty(occurrence.lastSeen)));

    const archive::Paths paths(m_root);
    const auto video=haveItem?representationHealth(paths,item.video):RepresentationHealth{};
    const auto audio=haveItem?representationHealth(paths,item.audio):RepresentationHealth{};
    if(haveItem)m_archiveDetails->setPlainText(tr("Canonical key: %1\nVideo: %2\nVideo path: %3\nVideo origin: %4\nAudio: %5\nAudio path: %6\nAudio origin: %7\nMetadata: %8")
        .arg(item.key,representationDisplay(item.video,video),pretty(item.video.path),pretty(item.video.origin),
             representationDisplay(item.audio,audio),pretty(item.audio.path),pretty(item.audio.origin),pretty(item.metadataPath)));

    const auto history=readText(paths.playlistHistoryFile(source.key));
    QStringList matching;
    for(const auto& line:history.split('\n')){
        if(line.trimmed().isEmpty())continue;
        QJsonParseError pe;
        const auto doc=QJsonDocument::fromJson(line.toUtf8(),&pe);
        if(pe.error!=QJsonParseError::NoError||!doc.isObject())continue;
        const auto object=doc.object();
        const auto eventEntry=object.value("entry_key").toString();
        if((!eventEntry.isEmpty()&&eventEntry==entryKey)||(eventEntry.isEmpty()&&object.value("item_key").toString()==itemKey))
            matching<<line;
    }
    m_historyDetails->setPlainText(matching.join("\n"));

    if(haveItem)m_recoveryDetails->setPlainText(tr("Recovery status: %1\nCurrent availability: %2\nVideo integrity: %3\nAudio integrity: %4\nExternal recovery is submitted through State/ArchiveMode/Imports/Pending according to ARCHIVE_AGENT.md.")
        .arg(item.recoveryStatus,item.availability,video.label,audio.label));
    updateActionState();
}

void ArchiveTab::refreshActivity()
{
    if(!archive::ui::rootAvailable(m_root))return;
    const auto base=archive::Paths(m_root).activityLogs();
    QDir daysRoot(base);
    const auto days=daysRoot.entryList(QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name|QDir::Reversed);
    QStringList lines;

    for(const auto& day:days){
        QDir dayDir(daysRoot.filePath(day));
        const auto files=dayDir.entryList(QDir::Files,QDir::Name|QDir::Reversed);
        for(const auto& file:files){
            const auto fileLines=readText(dayDir.filePath(file)).split('\n');
            for(int i=fileLines.size()-1;i>=0&&lines.size()<200;--i){
                const auto& line=fileLines[i];
                if(line.trimmed().isEmpty())continue;
                QJsonParseError pe;
                const auto doc=QJsonDocument::fromJson(line.toUtf8(),&pe);
                if(pe.error==QJsonParseError::NoError&&doc.isObject()){
                    const auto object=doc.object();
                    lines<<QString("%1  %2  %3").arg(object.value("timestamp").toString(),object.value("event").toString(),
                        QString::fromUtf8(QJsonDocument(object.value("details").toObject()).toJson(QJsonDocument::Compact)));
                }else{
                    lines<<line;
                }
            }
            if(lines.size()>=200)break;
        }
        if(lines.size()>=200)break;
    }

    std::reverse(lines.begin(),lines.end());
    m_activity->setPlainText(lines.join("\n"));
}

void ArchiveTab::updateActionState()
{
    // Root/readability and worker ownership are independent gates. Global
    // controls are available for a healthy Archive, while source-scoped
    // actions require an actual selected source.
    const bool ready=m_controlsEnabled&&!m_busy&&m_ready&&m_stateReadable&&archive::ui::rootAvailable(m_root);
    const bool sourceSelected=ready&&!selectedSourceKey().isEmpty();

    const QList<QWidget*> globalControls={m_add,m_syncAll,m_more,m_sources,m_search,m_filter};
    for(auto* widget:globalControls)widget->setEnabled(ready);

    const QList<QWidget*> sourceControls={m_remove,m_scan,m_syncSelected,m_retry};
    for(auto* widget:sourceControls)widget->setEnabled(sourceSelected);

    if(m_openRootAction)m_openRootAction->setEnabled(ready);
    if(m_openPlaylistAction)m_openPlaylistAction->setEnabled(sourceSelected);
    if(m_openCatalogAction)m_openCatalogAction->setEnabled(ready);
    if(m_openMissingAction)m_openMissingAction->setEnabled(ready);
    if(m_importsAction)m_importsAction->setEnabled(ready);
    if(m_openLogsAction)m_openLogsAction->setEnabled(ready);

    m_browse->setEnabled(m_controlsEnabled&&!m_busy);
    m_stop->setEnabled(m_busy);
}

void ArchiveTab::setBusy(bool busy,const QString& text)
{
    m_busy=busy;updateActionState();
    if(!text.isEmpty())m_statusLabel->setText(text);
    else if(!busy)m_statusLabel->setText(tr("Idle"));
    refreshSystemHealth();
}

void ArchiveTab::postOperationProgress(const QString& stage,const QString& detail,int current,int total,int failures)
{
    QMetaObject::invokeMethod(this,[this,stage,detail,current,total,failures]{
        if(!m_operationStage)return;m_operationStage->setText(stage);m_operationDetail->setText(detail);m_operationFailures->setText(tr("Failures: %1").arg(failures));
        if(total>0){m_operationProgress->setRange(0,total);m_operationProgress->setValue(qBound(0,current,total));m_operationProgress->setFormat(QStringLiteral("%v / %m"));}
        else{m_operationProgress->setRange(0,0);m_operationProgress->setFormat(tr("Working…"));}
    },Qt::QueuedConnection);
}

bool ArchiveTab::setRoot(const QString& candidate,QString* error)
{
    if(m_busy){if(error)*error=tr("An Archive operation is running. Stop it before changing the root.");return false;}
    // selectRoot owns the validate/persist/switch ordering. Rejected selections
    // leave the old root and view intact; there is no destructive rollback.
    if(!archive::ui::selectRoot(candidate,m_root,error))return false;
    refreshAll();
    return true;
}

void ArchiveTab::browseRoot()
{
    if(m_busy)return;
    const auto candidate=QFileDialog::getExistingDirectory(m_page,tr("Select Archive Root"),m_root,QFileDialog::ShowDirsOnly);
    if(candidate.isEmpty())return;
    QString error;
    if(!setRoot(candidate,&error))QMessageBox::critical(m_page,tr("Archive Root"),error);
}

void ArchiveTab::addPlaylist()
{
    if(m_busy)return;
    QString error;if(!ensureReady(&error)){QMessageBox::critical(m_page,tr("Archive"),error);return;}
    bool ok=false;const auto url=QInputDialog::getText(m_page,tr("Add Playlist"),tr("YouTube playlist URL:"),QLineEdit::Normal,{},&ok).trimmed();if(!ok||url.isEmpty())return;
    const auto key=archive::sourceKeyFromUrl(url);if(key.isEmpty()){QMessageBox::warning(m_page,tr("Add Playlist"),tr("Enter a valid YouTube playlist URL containing a list ID."));return;}
    const auto canonicalUrl=QStringLiteral("https://www.youtube.com/playlist?list=")+key;
    const auto title=QInputDialog::getText(m_page,tr("Add Playlist"),tr("Display name:"),QLineEdit::Normal,key,&ok).trimmed();if(!ok)return;
    if(!archive::ui::rootAvailable(m_root)){refreshAll();return;}
    archive::Paths paths(m_root);archive::SyncLock lock(paths);if(!lock.tryLock()){QMessageBox::warning(m_page,tr("Archive"),lock.errorString());return;}
    archive::Store store(paths);auto sources=store.loadSources(&error);if(!error.isEmpty()){QMessageBox::critical(m_page,tr("Archive"),error);return;}
    for(const auto& existing:sources)if(existing.key==key){
        for(int i=0;i<m_sources->count();++i){
            if(m_sources->item(i)->data(Qt::UserRole).toString()==key){
                m_sources->setCurrentRow(i);
                break;
            }
        }
        QMessageBox::information(m_page,tr("Add Playlist"),tr("This playlist is already registered."));
        return;
    }
    archive::Source source;source.key=key;source.url=canonicalUrl;source.title=title.isEmpty()?key:title;source.addedAt=QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    lock.unlock();

    // A syntactically valid URL is only a candidate until provider discovery
    // proves it is an authoritative complete playlist snapshot. Failed first
    // discovery must leave no durable active source behind.
    m_stopRequested=false;
    runAsync(tr("Adding playlist"),[this,source]{
        return operationScanOrSync({source},true,true);
    });
}
void ArchiveTab::removePlaylist()
{
    const auto source=selectedSource();if(source.key.isEmpty()||m_busy)return;
    if(QMessageBox::question(m_page,tr("Remove Playlist"),tr("Stop managing '%1'? Archived media and historical playlist files will not be deleted.").arg(source.title))!=QMessageBox::Yes)return;
    if(!archive::ui::rootAvailable(m_root)){refreshAll();return;}
    archive::Paths paths(m_root);archive::SyncLock lock(paths);if(!lock.tryLock()){QMessageBox::warning(m_page,tr("Archive"),lock.errorString());return;}
    archive::Store store(paths);QString error;auto sources=store.loadSources(&error);if(!error.isEmpty()){QMessageBox::critical(m_page,tr("Archive"),error);return;}
    for(int i=sources.size()-1;i>=0;--i)if(sources[i].key==source.key)sources.remove(i);
    if(!store.saveSources(sources,&error)){QMessageBox::critical(m_page,tr("Remove Playlist"),error);return;}
    archive::ActivityLogger logger(paths);logger.event("INFO","source","playlist_unregistered",{{"source_key",source.key}});lock.unlock();refreshAll();
}

void ArchiveTab::scanSelected(){const auto s=selectedSource();if(!s.key.isEmpty())runSources({s},false,tr("Scanning %1").arg(s.title));}
void ArchiveTab::syncSelected(){const auto s=selectedSource();if(!s.key.isEmpty())runSources({s},true,tr("Syncing %1").arg(s.title));}
void ArchiveTab::retryFailed()
{
    const auto source=selectedSource();
    if(source.key.isEmpty()||m_busy)return;

    QString error;
    if(!ensureReady(&error)){m_statusLabel->setText(error);return;}

    m_stopRequested=false;
    const auto name=tr("Retrying failed work for %1").arg(source.title.isEmpty()?source.key:source.title);
    runAsync(name,[this,source]{return operationRetryFailed(source);});
}
void ArchiveTab::syncAll()
{
    if(m_busy)return;
    QString error;
    if(!ensureReady(&error)){m_statusLabel->setText(error);return;}
    archive::Store store{archive::Paths(m_root)};
    const auto sources=store.loadSources();
    if(!sources.isEmpty())runSources(sources,true,tr("Syncing all playlists"));
}
void ArchiveTab::stopAfterCurrent(){m_stopRequested=true;m_statusLabel->setText(tr("Stop requested. The current item will finish safely."));if(m_operationDetail)m_operationDetail->setText(tr("Stop requested. The current item will finish safely, then the queue will stop."));refreshSystemHealth();}

void ArchiveTab::processImports()
{
    if(m_busy)return;
    QString error;
    if(!ensureReady(&error)){m_statusLabel->setText(error);return;}
    m_stopRequested=false;
    runAsync(tr("Processing external imports"),[this]{
        // Recheck at worker entry: the selected volume may disappear after
        // the UI accepted the command but before the worker starts running.
        QString rootError;
        if(!archive::ui::initializeRoot(m_root,&rootError))return QString("ERROR:")+rootError;
        postOperationProgress(tr("Recovery imports"),tr("Validating and ingesting Pending recovery packages"),0,0,0);
        archive::Paths paths(m_root);archive::Store store(paths);archive::ActivityLogger logger(paths);
        archive::RecoveryImporter importer(runtimeConfig(),store,logger);
        QStringList failures,warnings;
        const auto accepted=importer.ingestPending(&failures,[this]{return m_stopRequested.load();},&warnings);
        const QJsonObject result{{"accepted",accepted},{"failure_count",failures.size()},
                                 {"warning_count",warnings.size()},{"warnings",QJsonArray::fromStringList(warnings)},
                                 {"stopped",m_stopRequested.load()},{"failures",QJsonArray::fromStringList(failures)}};
        return QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
    });
}

void ArchiveTab::runSources(const QVector<archive::Source>& sources,bool downloads,const QString& name)
{
    if(m_busy||sources.isEmpty())return;
    QString error;
    if(!ensureReady(&error)){m_statusLabel->setText(error);return;}
    m_stopRequested=false;
    runAsync(name,[this,sources,downloads]{return operationScanOrSync(sources,downloads);});
}

QString ArchiveTab::operationScanOrSync(QVector<archive::Source> sources,bool doDownloads,bool requireCompleteBeforeFirstCommit)
{
    // Admission is repeated in the worker; UI enablement is not a filesystem
    // guarantee, and an offline configured root must never be recreated here.
    QString error;
    if(!archive::ui::initializeRoot(m_root,&error))return QString("ERROR:")+error;
    archive::Paths paths(m_root);archive::Store store(paths);archive::ActivityLogger logger(paths);
    archive::SyncLock lock(paths);
    if(!lock.tryLock())return "ERROR:"+lock.errorString();
    QJsonObject result;int totalObserved=0,totalFailures=0,totalDownloaded=0;QStringList failures,warnings;
    logger.event("INFO","application",doDownloads?"sync_session_started":"scan_session_started",{{"source_count",sources.size()}});
    int sourceIndex=0;
    for(auto& source:sources){
        if(m_stopRequested.load())break;++sourceIndex;const auto sourceName=source.title.isEmpty()?source.key:source.title;
        postOperationProgress(tr("Discovery"),tr("Source %1/%2: %3").arg(sourceIndex).arg(sources.size()).arg(sourceName),sourceIndex-1,sources.size(),totalFailures);
        archive::PlaylistDiscovery discovery(runtimeConfig(),logger);auto snapshot=discovery.discover(source);
        if(requireCompleteBeforeFirstCommit&&!snapshot.complete){
            ++totalFailures;
            failures<<source.key+": "+(snapshot.error.isEmpty()?tr("Initial discovery did not produce a complete authoritative playlist snapshot"):snapshot.error);
            logger.event("WARNING","source","playlist_admission_rejected",{{"source_key",source.key},{"error",snapshot.error}});
            postOperationProgress(tr("Add rejected"),snapshot.error,sourceIndex,sources.size(),totalFailures);
            continue;
        }
        postOperationProgress(tr("Reconcile"),tr("Reconciling %1 observed item(s) for %2").arg(snapshot.items.size()).arg(sourceName),sourceIndex-1,sources.size(),totalFailures);
        auto sum=store.reconcile(source,snapshot,&logger);
        if(requireCompleteBeforeFirstCommit&&sum.committed)
            logger.event("INFO","source","playlist_added",{{"source_key",source.key},{"url",source.url},{"title",source.title}});if(!sum.committed){failures<<source.key+": "+sum.error;++totalFailures;postOperationProgress(tr("Reconcile failed"),sum.error,sourceIndex,sources.size(),totalFailures);continue;}
        if(!sum.projectionWarning.isEmpty())warnings<<source.key+": "+sum.projectionWarning;
        totalObserved+=sum.observed;if(!snapshot.complete){++totalFailures;failures<<source.key+": "+snapshot.error;postOperationProgress(tr("Discovery incomplete"),snapshot.error,sourceIndex,sources.size(),totalFailures);continue;}
        if(!doDownloads){postOperationProgress(tr("Source complete"),tr("%1 scanned successfully").arg(sourceName),sourceIndex,sources.size(),totalFailures);continue;}

        auto playlist=store.loadPlaylistItems(source.key);auto canonical=store.loadCanonicalItems();QHash<QString,archive::CanonicalItem> map;for(const auto& c:canonical)map[c.key]=c;
        int eligible=0;for(const auto& p:playlist){if(p.membership!="active"||!map.contains(p.itemKey))continue;++eligible;}
        int itemIndex=0;
        for(const auto& p:playlist){
            if(m_stopRequested.load())break;if(p.membership!="active"||!map.contains(p.itemKey))continue;auto c=map[p.itemKey];++itemIndex;
            const auto itemName=p.title.isEmpty()?p.itemKey:p.title;postOperationProgress(tr("Media sync + verify + commit"),tr("%1 | source %2/%3 | item %4/%5").arg(itemName).arg(sourceIndex).arg(sources.size()).arg(itemIndex).arg(eligible),itemIndex-1,eligible,totalFailures);
            archive::MediaExecutor executor(runtimeConfig(),store,logger);QString e;if(executor.syncItem(c,true,true,&e))++totalDownloaded;else{++totalFailures;failures<<p.itemKey+": "+e;}
            postOperationProgress(tr("Media sync + verify + commit"),tr("Processed %1/%2 for %3").arg(itemIndex).arg(eligible).arg(sourceName),itemIndex,eligible,totalFailures);
        }
        postOperationProgress(tr("Projection"),tr("Regenerating reports for %1").arg(sourceName),sourceIndex-1,sources.size(),totalFailures);
        QString projectionError;if(!store.writeAllProjections(&projectionError))
            warnings<<source.key+": State committed, projection rebuild required: "+projectionError;
        postOperationProgress(tr("Source complete"),tr("Completed %1").arg(sourceName),sourceIndex,sources.size(),totalFailures);
    }
    postOperationProgress(tr("Finalizing"),tr("Writing final session result"),sources.size(),sources.size(),totalFailures);
    lock.unlock();warnings.removeDuplicates();
    result["observed"]=totalObserved;result["completed_items"]=totalDownloaded;
    result["failure_count"]=totalFailures;result["stopped"]=m_stopRequested.load();
    result["failures"]=QJsonArray::fromStringList(failures);
    result["warning_count"]=warnings.size();result["warnings"]=QJsonArray::fromStringList(warnings);
    logger.event(totalFailures||!warnings.isEmpty()?"WARNING":"INFO","application",doDownloads?"sync_session_completed":"scan_session_completed",result);
    return QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
}

QString ArchiveTab::operationRetryFailed(const archive::Source& source)
{
    QString error;
    if(!archive::ui::initializeRoot(m_root,&error))return QString("ERROR:")+error;

    archive::Paths paths(m_root);
    archive::Store store(paths);
    archive::ActivityLogger logger(paths);
    archive::SyncLock lock(paths);
    if(!lock.tryLock())return "ERROR:"+lock.errorString();

    const auto playlist=store.loadPlaylistItems(source.key,&error);
    if(!error.isEmpty())return "ERROR:"+error;
    const auto canonical=store.loadCanonicalItems(&error);
    if(!error.isEmpty())return "ERROR:"+error;

    QHash<QString,archive::CanonicalItem> map;
    for(const auto& item:canonical)map[item.key]=item;

    const auto retryable=[](const QString& state){
        return state=="failed"||state=="interrupted";
    };

    QSet<QString> eligibleKeys;
    for(const auto& occurrence:playlist){
        if(occurrence.membership!="active"||!map.contains(occurrence.itemKey))continue;
        const auto& item=map[occurrence.itemKey];
        if(retryable(item.video.state)||retryable(item.audio.state))eligibleKeys.insert(occurrence.itemKey);
    }

    const auto eligible=eligibleKeys.size();
    int processed=0,failuresCount=0;
    QStringList failures;
    logger.event("INFO","application","retry_failed_started",{{"source_key",source.key},{"eligible",eligible}});

    QSet<QString> processedKeys;
    for(const auto& occurrence:playlist){
        if(m_stopRequested.load())break;
        if(occurrence.membership!="active"||!map.contains(occurrence.itemKey)||processedKeys.contains(occurrence.itemKey))continue;

        const auto item=map[occurrence.itemKey];
        const bool retryVideo=retryable(item.video.state);
        const bool retryAudio=retryable(item.audio.state);
        if(!retryVideo&&!retryAudio)continue;

        processedKeys.insert(occurrence.itemKey);
        ++processed;
        const auto itemName=occurrence.title.isEmpty()?occurrence.itemKey:occurrence.title;
        postOperationProgress(tr("Retry failed media"),tr("%1 | item %2/%3").arg(itemName).arg(processed).arg(eligible),processed-1,eligible,failuresCount);

        archive::MediaExecutor executor(runtimeConfig(),store,logger);
        QString itemError;
        if(!executor.syncItem(item,retryVideo,retryAudio,&itemError)){
            ++failuresCount;
            failures<<occurrence.itemKey+": "+itemError;
        }

        postOperationProgress(tr("Retry failed media"),tr("Processed %1/%2").arg(processed).arg(eligible),processed,eligible,failuresCount);
    }

    lock.unlock();

    QJsonObject result;
    result["observed"]=eligible;
    result["completed_items"]=processed-failuresCount;
    result["failure_count"]=failuresCount;
    result["warning_count"]=0;
    result["stopped"]=m_stopRequested.load();
    result["failures"]=QJsonArray::fromStringList(failures);
    logger.event(failuresCount?"WARNING":"INFO","application","retry_failed_completed",result);
    return QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
}

void ArchiveTab::runAsync(const QString& operationName,const std::function<QString()>& fn)
{
    if(m_busy)return;
    m_operationName->setText(operationName);
    m_operationStage->setText(tr("Starting"));
    m_operationDetail->setText(tr("Preparing Archive Mode operation"));
    m_operationFailures->setText(tr("Failures: 0"));
    m_operationProgress->setRange(0,0);
    m_operationProgress->setFormat(tr("Working…"));
    setBusy(true,operationName);
    m_watcher=new QFutureWatcher<QString>(this);
    QObject::connect(m_watcher,&QFutureWatcher<QString>::finished,this,[this,operationName]{
        // Retrieve the worker's outcome before releasing its watcher. Rendering
        // stays on the owning UI thread, independently of filesystem refresh.
        const auto result=m_watcher->result();
        m_watcher->deleteLater();m_watcher=nullptr;setBusy(false);
        bool stopped=false;
        int failures=0,warnings=0;
        QString detail;
        if(result.startsWith("ERROR:")){
            failures=1;detail=result.mid(6);
            m_statusLabel->setText(result);
            QMessageBox::warning(m_page,tr("Archive Operation"),detail);
        }else{
            const auto report=QJsonDocument::fromJson(result.toUtf8()).object();
            failures=report.value("failure_count").toInt();
            warnings=report.value("warning_count").toInt();
            stopped=report.value("stopped").toBool();
            QStringList parts;
            if(report.contains("observed"))parts<<tr("Observed %1").arg(report.value("observed").toInt());
            if(report.contains("completed_items"))parts<<tr("Completed media %1").arg(report.value("completed_items").toInt());
            if(report.contains("accepted"))parts<<tr("Accepted imports %1").arg(report.value("accepted").toInt());
            detail=parts.isEmpty()?tr("Operation finished"):parts.join(tr(" | "));
            if(stopped){
                m_statusLabel->setText(tr("Stopped safely after the current item. Remaining work is not complete."));
            }else if(failures>0){
                m_statusLabel->setText(tr("Finished with %1 failure(s). Review Activity and retry.").arg(failures));
                QMessageBox box(QMessageBox::Warning,tr("Archive Operation"),m_statusLabel->text(),QMessageBox::Ok,m_page);
                box.setDetailedText(result);box.exec();
            }else if(warnings>0){
                // Durable work succeeded. Never tell the user to retry an
                // accepted import just because a generated report is blocked.
                m_statusLabel->setText(tr("State committed with %1 report warning(s). Repair projections; do not resubmit accepted packages.").arg(warnings));
                QMessageBox box(QMessageBox::Warning,tr("Archive Operation"),m_statusLabel->text(),QMessageBox::Ok,m_page);
                box.setDetailedText(result);box.exec();
            }else{
                m_statusLabel->setText(operationName+tr(" completed successfully"));
            }
            if(warnings>0)detail+=tr(" | Report warnings %1; committed work is preserved").arg(warnings);
        }
        // Post-commit report warnings are not admission failures. Preserve the
        // accepted counts and this outcome even when health refresh later
        // reports that the dirty projection still cannot be rebuilt.
        m_operationName->setText(operationName);
        m_operationStage->setText(result.startsWith("ERROR:")?tr("ERROR"):
            stopped?tr("STOPPED"):failures>0?tr("COMPLETED WITH FAILURES"):
            warnings>0?tr("COMPLETED WITH WARNINGS"):tr("COMPLETED"));
        m_operationDetail->setText(detail);
        m_operationFailures->setText(tr("Failures: %1 | Warnings: %2").arg(failures).arg(warnings));
        m_operationProgress->setRange(0,1);m_operationProgress->setValue(1);
        m_operationProgress->setFormat(stopped?tr("Stopped"):failures>0?tr("Finished with failures"):
            warnings>0?tr("Finished with warnings"):tr("Complete"));
        refreshAll();
    });
    m_watcher->setFuture(QtConcurrent::run([fn]{
        try{return fn();}
        catch(const std::exception& e){return QString("ERROR:")+QString::fromUtf8(e.what());}
        catch(...){return QString("ERROR:Unexpected archive worker exception");}
    }));
}

void ArchiveTab::openPath(const QString& path){if(!path.isEmpty())QDesktopServices::openUrl(QUrl::fromLocalFile(path));}
void ArchiveTab::openSelectedPlaylistFolder(){const auto key=selectedSourceKey();if(!key.isEmpty())openPath(archive::Paths(m_root).sourceDir(key));}
void ArchiveTab::openProjection(const QString& name){const auto key=selectedSourceKey();if(key.isEmpty())return;const auto p=QDir(archive::Paths(m_root).sourceDir(key)).filePath(name);if(QFileInfo::exists(p))openPath(p);else QMessageBox::information(m_page,tr("Archive"),tr("The projection has not been generated yet."));}
