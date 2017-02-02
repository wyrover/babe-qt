#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include<QString>
#include<QStringList>
#include "collectionDB.h"
#include<QDebug>
#include <QThread>
#include "playlist.h"
#include <about.h>
#include <QDir>
#include <QFileSystemWatcher>
#include <QLabel>
#include <QMovie>

namespace Ui {
class settings;
}

class settings : public QWidget
{
    Q_OBJECT

public:

    explicit settings(QWidget *parent = 0);
    ~settings();
    bool checkCollection();
    CollectionDB &getCollectionDB();
    int getToolbarIconSize()  {return iconSize;}
    QStringList getCollectionPath() {return collectionPaths;}
    void setSettings(QStringList setting);
    void readSettings();
    void removeSettings(QStringList setting);
    void refreshCollectionPaths();
    void collectionWatcher();
    void addToWatcher(QStringList paths);
    CollectionDB collection_db;
    enum iconSizes
    {
        s16,s22,s24
    };
    //enum albums { ALBUM_TITLE, ARTIST, ART};
   // enum artists { ARTIST_TITLE, ART};

private slots:

    void on_open_clicked();
    void on_toolbarIconSize_activated(const QString &arg1);
    void finishedAddingTracks(bool state);
    void on_pushButton_clicked();
    void handleFileChanged(QString file);
    void handleDirectoryChanged(QString dir);
    void on_collectionPath_clicked(const QModelIndex &index);
    void on_remove_clicked();

public slots:

    void populateDB(QString path);
    void fetchArt();

private:
    Ui::settings *ui;
    const QString settingPath= QDir().homePath()+"/.config/babe/";
    const QString collectionDBPath=QDir().homePath()+"/.local/share/babe/";
    const QString cachePath=QDir().homePath()+"/.cache/babe/";
    const QString collectionDBName = "collection.db";
    const QString settingsName = "settings.conf";
    int iconSize = 16;
    QStringList collectionPaths={};
    QLabel *artFetcherNotice;
    QMovie *movie;
    QString pathToRemove;
   // QFileSystemWatcher watcher;
    QThread* thread;
    About *about_ui;

signals:

    void toolbarIconSizeChanged(int newSize);
    void collectionPathChanged(QString newPath);
    void collectionDBFinishedAdding(bool state);
    void fileChanged(QString url);
    void dirChanged(QString url);
    void collectionPathRemoved(QString url);
    void refreshTables();

};

#endif // SETTINGS_H
