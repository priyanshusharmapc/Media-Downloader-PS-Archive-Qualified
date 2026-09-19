#ifndef MDPS_ARCHIVESETTINGS_H
#define MDPS_ARCHIVESETTINGS_H

#include <QString>

namespace archive { namespace ui {
// Archive UI configuration is independent of the portable package and of the
// general downloader folder. Merely resolving a setting never creates an archive.
QString settingsFile();

// Return only an explicitly configured root, including an offline one. Legacy
// relative roots resolve against applicationDir; downloadFolder is retained for
// source compatibility, not used as an implicit archive. Read failures return an
// empty result with an error and never select a fallback directory.
QString configuredRoot(const QString& applicationDir,const QString& downloadFolder,QString* error=nullptr);

// Non-mutating admission check for GUI reads and operations. A missing root may
// be an offline volume, so callers must not create it as an attempted recovery.
bool rootAvailable(const QString& root,QString* error=nullptr);

// Initialize/validate the layout only inside an explicitly selected, existing
// directory. CLI initialization retains its separate create-new-root contract.
bool initializeRoot(const QString& root,QString* error=nullptr);

// Commit an accepted absolute path atomically, preserving unrelated INI values.
// On failure the old settings bytes survive; no QSettings destructor can retry a
// rejected write to the live file. Does not create or initialize an archive.
bool persistRoot(const QString& root,QString* error=nullptr);

// Two-phase GUI selection: initialize the candidate, commit its setting, then
// switch currentRoot. Failure preserves currentRoot and its durable setting.
// Initialized candidate data is retained on settings failure, never deleted.
bool selectRoot(const QString& candidate,QString& currentRoot,QString* error=nullptr);
} }

#endif
