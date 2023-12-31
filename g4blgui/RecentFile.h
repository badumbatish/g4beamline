//	RecentFile.h

#ifndef RECENTFILE_H
#define RECENTFILE_H

#include <QString>
#include <QStringList>
#include <QObject>
#include <QSettings>
#include <QMenu>
#include <QAction>

class G4blGuiWindow;

/**	class RecentFile manages the menu of recently opened files.
 *
 *	The list is maintained in QSettings so it is persistent.
 **/
class RecentFile : public QObject {
	Q_OBJECT
	enum { MAXRECENT=10 };
	G4blGuiWindow *window;
	QMenu *menu;
	QStringList recentList;
	QSettings *settings;
public:
	RecentFile(G4blGuiWindow *_window, QMenu *_menu);
	void fileOpened(QString filename);
private slots:
	void openRecentFile(QAction *action);
};

#endif // RECENTFILE_H
