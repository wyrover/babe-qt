/* Copyright 2017 Camilo Higuita(milohr)
*
* This file is part of Babe Qt Music Player.
*
* Babe is free software: you can redistribute it
* and/or modify it under the terms of version 3 of the
* GNU General Public License as published by the Free Software Foundation.
*
* Babe is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with Babe. If not, see http://www.gnu.org/licenses/.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QWidget>
#include <QToolBar>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QStatusBar>
#include <QStringList>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QDir>
#include <QDirIterator>
#include <QStringList>
#include "collectionDB.h"
#include<QSqlQuery>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QMimeData>
#include <QMenu>
#include <QWidgetAction>
#include <QButtonGroup>
#include<QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QTimer>
#include <scrolltext.h>
#include <QBitmap>
#include <QPainter>
#include <album.h>
#include "artwork.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //setWindowFlags(Qt::WindowStaysOnTopHint);

    ui->setupUi(this);
    this->setWindowTitle(" Babe ... \xe2\x99\xa1  \xe2\x99\xa1 \xe2\x99\xa1 ");
    this->setAcceptDrops(true);
    this->setWindowIcon(QIcon(":Data/data/babe_48.svg"));
    this->setWindowIconText("Babe...");

    connect(this, SIGNAL(finishedPlayingSong(QString)),this,SLOT(addToPlayed(QString)));
    connect(this,SIGNAL(getCover(QString,QString)),this,SLOT(setCoverArt(QString,QString)));
//this->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    /*THE VIEWS*/
    frame = new QFrame();
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);

    settings_widget = new settings(); //this needs to go fist

    collectionTable = new BabeTable();
    collectionTable->passCollectionConnection(&settings_widget->getCollectionDB());
    connect(collectionTable,SIGNAL(tableWidget_doubleClicked(QStringList)),this,SLOT(addToPlaylist(QStringList)));
    connect(collectionTable,SIGNAL(songRated(QStringList)),this,SLOT(addToFavorites(QStringList)));
    connect(collectionTable,SIGNAL(enteredTable()),this,SLOT(hideControls()));
    connect(collectionTable,SIGNAL(leftTable()),this,SLOT(showControls()));
    connect(collectionTable,SIGNAL(finishedPopulating()),this,SLOT(orderTables()));
    connect(collectionTable,SIGNAL( babeIt_clicked(QStringList)),this,SLOT(addToPlaylist(QStringList)));


    favoritesTable =new BabeTable();
    connect(favoritesTable,SIGNAL(tableWidget_doubleClicked(QStringList)),this,SLOT(addToPlaylist(QStringList)));
    connect(favoritesTable,SIGNAL(enteredTable()),this,SLOT(hideControls()));
    connect(favoritesTable,SIGNAL(enteredTable()),this,SLOT(hideControls()));
    connect(favoritesTable,SIGNAL(leftTable()),this,SLOT(showControls()));
    connect(favoritesTable,SIGNAL( babeIt_clicked(QStringList)),this,SLOT(addToPlaylist(QStringList)));
     favoritesTable->setVisibleColumn(BabeTable::STARS);
    resultsTable=new BabeTable();
    //resultsTable->passStyle("QHeaderView::section { background-color:#474747; }");
    resultsTable->setVisibleColumn(BabeTable::STARS);
    resultsTable->showColumn(BabeTable::GENRE);
    connect(resultsTable,SIGNAL(songRated(QStringList)),this,SLOT(addToFavorites(QStringList)));
    connect(resultsTable,SIGNAL(tableWidget_doubleClicked(QStringList)),this,SLOT(addToPlaylist(QStringList)));
    //connect(resultsTable,SIGNAL(enteredTable()),this,SLOT(hideControls()));
    connect(resultsTable,SIGNAL(enteredTable()),this,SLOT(hideControls()));
    connect(resultsTable,SIGNAL(leftTable()),this,SLOT(showControls()));
    connect(resultsTable,SIGNAL( babeIt_clicked(QStringList)),this,SLOT(addToPlaylist(QStringList)));
    //connect(resultsTable,SIGNAL(createPlaylist_clicked()),this,SLOT(playlistsView()));

    albumsTable = new AlbumsView();
    //connect(albumsTable,SIGNAL(songClicked(QStringList)),this,SLOT(addToPlaylist(QStringList)));
   // connect(albumsTable,SIGNAL(songRated(QStringList)),this,SLOT(addToFavorites(QStringList)));
    //connect(albumsTable,SIGNAL(songBabeIt(QStringList)),this,SLOT(addToPlaylist(QStringList)));
    connect(albumsTable,SIGNAL(albumOrderChanged(QString)),this,SLOT(AlbumsViewOrder(QString)));
    connect(albumsTable->albumTable,SIGNAL(tableWidget_doubleClicked(QStringList)),this,SLOT(addToPlaylist(QStringList)));
    connect(albumsTable->albumTable,SIGNAL( babeIt_clicked(QStringList)),this,SLOT(addToPlaylist(QStringList)));
    connect(albumsTable->albumTable,SIGNAL(songRated(QStringList)),this,SLOT(addToFavorites(QStringList)));


    playlistTable = new PlaylistsView();
    connect(playlistTable,SIGNAL(playlistCreated(QString)),&settings_widget->getCollectionDB(),SLOT(insertPlaylist(QString)));
    //connect(playlistTable,SIGNAL(songClicked(QStringList)),this,SLOT(addToPlaylist(QStringList)));
    connect(playlistTable->table,SIGNAL(tableWidget_doubleClicked(QStringList)),this,SLOT(addToPlaylist(QStringList)));
    connect(playlistTable->table,SIGNAL( babeIt_clicked(QStringList)),this,SLOT(addToPlaylist(QStringList)));
    connect(playlistTable->table,SIGNAL(createPlaylist_clicked()),this,SLOT(playlistsView()));
    connect(playlistTable->table,SIGNAL(songRated(QStringList)),this,SLOT(addToFavorites(QStringList)));


    infoTable = new InfoView();


    //playback = new QToolBar();
    utilsBar = new QToolBar();

    settings_widget->readSettings();
    setToolbarIconSize(settings_widget->getToolbarIconSize());
    connect(settings_widget, SIGNAL(toolbarIconSizeChanged(int)), this, SLOT(setToolbarIconSize(int)));
    connect(settings_widget, SIGNAL(collectionDBFinishedAdding(bool)), this, SLOT(collectionDBFinishedAdding(bool)));
    connect(settings_widget, SIGNAL(dirChanged(QString)),this, SLOT(scanNewDir(QString)));
    connect(settings_widget, SIGNAL(collectionPathRemoved(QString)),&settings_widget->getCollectionDB(), SLOT(removePath(QString)));
    connect(settings_widget, SIGNAL(refreshTables()),this, SLOT(refreshTables()));


    if(settings_widget->checkCollection())
    {
        //collectionWatcher();
        settings_widget->collection_db.setCollectionLists();

       refreshTables();
       QStringList list =settings_widget->getCollectionDB().getPlaylists();
       playlistTable->setPlaylists(list);

       populateMainList();
       emit collectionChecked();
    }
    else
    {
        //views->setCurrentIndex();
    }

   //
    //babes_widget= new babes();
   //


    /*THE MAIN WIDGETS*/




    /*THE STREAMING / PLAYLIST*/
    connect(updater, SIGNAL(timeout()), this, SLOT(update()));
    player->setVolume(100);
    addMusicImg = new QLabel(ui->listWidget);
    addMusicImg->setPixmap(QPixmap(":Data/data/add.png").scaled(120,120,Qt::KeepAspectRatio));
    addMusicImg->setGeometry(45,40,120,120);
    connect(ui->listWidget->model() ,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(on_rowInserted(QModelIndex,int,int)));
    addMusicImg->hide();



    //playback->setMovable(false);

    //this->adjustSize();
   // this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    //this->setMinimumSize (200, 250);

    //checkCollection();


    //settings_widget->setContentsMargins(0,0,0,0);
    //settings_widget->setStyleSheet("background:red;");
    //collection_db.openCollection("../player/collection.db");




   // setUpViews();



    QAction *babe, *remove;
    babe = new QAction("Babe it");
    remove = new QAction("Remove from list");
    ui->listWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->listWidget->addAction(babe);
    ui->listWidget->addAction(remove);




    /*SETUP MAIN TOOLBAR*/

    auto *left_spacer = new QWidget();
    left_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *right_spacer = new QWidget();
    right_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->tracks_view->setToolTip("Search...");
    ui->mainToolBar->addWidget(ui->utilsBar);
    ui->utilsBar->setChecked(true);

    ui->mainToolBar->addWidget(left_spacer);

    ui->tracks_view->setToolTip("Collection");
    ui->mainToolBar->addWidget(ui->tracks_view);
    //ui->tracks_view->setChecked(true);

    ui->albums_view->setToolTip("Albums");
    ui->mainToolBar->addWidget(ui->albums_view);

    ui->babes_view->setToolTip("Favorites");
    ui->mainToolBar->addWidget(ui->babes_view);

    ui->playlists_view->setToolTip("Playlists");
    ui->mainToolBar->addWidget(ui->playlists_view);

    ui->queue_view->setToolTip("Queue");
    ui->mainToolBar->addWidget(ui->queue_view);

    ui->info_view->setToolTip("Info");
    ui->mainToolBar->addWidget(ui->info_view);

    ui->settings_view->setToolTip("Setings");
    ui->mainToolBar->addWidget(ui->settings_view);

    ui->mainToolBar->addWidget(right_spacer);

   /* ui->open_btn->setToolTip("Open...");
    ui->mainToolBar->addWidget(ui->open_btn);*/


    this->addToolBar(Qt::BottomToolBarArea, ui->mainToolBar);
   // this->setCentralWidget(ui->listView);



    //playback->addWidget();
    //playback->addWidget(ui->horizontalSlider);

    //playback->setIconSize(QSize(16, 16));
    //this->addToolBar(Qt::BottomToolBarArea, playback);


    /*
    status = new QToolBar();
    info=new QLabel(" Babe ... \xe2\x99\xa1  \xe2\x99\xa1 \xe2\x99\xa1 ");
    //info->setStyleSheet("color:white;");

    status->addWidget(ui->shuffle_btn);
    auto *sp = new QWidget();
    sp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    status->addWidget(sp);
    status->addWidget(info);
    auto *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    status->addWidget(spacer);
    status->addWidget(ui->hide_sidebar_btn);    
    status->setIconSize(QSize(16, 16));
    status->setMovable(false);

    this->addToolBar(Qt::BottomToolBarArea,status);
    */

    /* SEARCH BAR & UTILS BAR*/

    /*auto clearSearch = new QToolButton(ui->search);
    clearSearch->setIcon(QIcon::fromTheme("clearSearch"));
    clearSearch->setAutoRaise(false);
    clearSearch->setGeometry(50, ui->search->sizeHint().height()/2,16,16);*/



    ui->search->setClearButtonEnabled(true);

    utilsBar->setMovable(false);
    utilsBar->setContentsMargins(0,0,0,0);

    utilsBar->setStyleSheet("margin:0;");

    utilsBar->addWidget(infoTable->infoUtils);
    utilsBar->addWidget(playlistTable->btnContainer);
    utilsBar->addWidget(ui->searchFrame);
    utilsBar->addWidget(albumsTable->utilsFrame);
    utilsBar->addWidget(ui->collectionUtils);


     //utilsBar->actions().at(SEARCH_UB)->setVisible(false);
    // utilsBar->actions().at(2)->setVisible(false);
     utilsBar->actions().at(PLAYLISTS_UB)->setVisible(false); ui->frame_3->hide();
     utilsBar->actions().at(INFO_UB)->setVisible(false);
     hideAlbumViewUtils();


    ui->saveResults->setEnabled(false);



    ui->search->setPlaceholderText("Search...");

    //this->addToolBar(Qt::BottomToolBarArea,utilsBar);

    /*COMPOSE THE VIEWS*/

    views = new QStackedWidget;
    views->setFrameShape(QFrame::NoFrame);
    views->addWidget(collectionTable);
    views->addWidget(albumsTable);
    views->addWidget(favoritesTable);
    views->addWidget(playlistTable);
    views->addWidget(new BabeTable());
    views->addWidget(infoTable);
    views->addWidget(settings_widget);
    views->addWidget(resultsTable);


    connect(ui->tracks_view, SIGNAL(clicked()), this, SLOT(collectionView()));
    connect(ui->albums_view, SIGNAL(clicked()), this, SLOT(albumsView()));
    connect(ui->babes_view, SIGNAL(clicked()), this, SLOT(favoritesView()));
    connect(ui->playlists_view, SIGNAL(clicked()), this, SLOT(playlistsView()));
    connect(ui->queue_view, SIGNAL(clicked()), this, SLOT(queueView()));
    connect(ui->info_view, SIGNAL(clicked()), this, SLOT(infoView()));
    connect(ui->settings_view, SIGNAL(clicked()), this, SLOT(settingsView()));


    /*MAIN WINDOW*/
    frame_layout = new QVBoxLayout();
    frame_layout->setContentsMargins(0,0,0,0);
    layout = new QGridLayout();
    layout->setContentsMargins(6,0,6,0);
    main_widget= new QWidget();
    main_widget->setLayout(layout);
    this->setCentralWidget(main_widget);



    /*album view*/
    auto *album_widget= new QWidget();
  //  album_widget->setStyleSheet("background-color:red;");

    auto *album_view = new QGridLayout();
    album_art_frame=new QFrame();
    album_art_frame->setFrameShadow(QFrame::Raised);
    album_art_frame->setFrameShape(QFrame::StyledPanel);

    //album_art_frame->setStyleSheet("background-color:transparent;");
    //album_art_frame->setFixedSize(210,210);
    //album_art->setGeometry(0,0,100,100);

    album_art = new Album(":Data/data/cover.jpg",200,0,true,album_art_frame);
    album_art->setFixedSize(200,200);
    //connect(album_art,SIGNAL(albumCoverLeft()),this,SLOT(hide ui->controls()));
   // connect(album_art,SIGNAL(albumCoverEnter()),this,SLOT(show ui->controls()));

    album_art->titleVisible(false);
    //album_art->setTitleGeometry(0,0,200,30);
    album_art->widget->setGeometry(0,0,200,30);
    album_art->widget->setStyleSheet( QString("background-color: rgba(0,0,0,150); border: none;"));

    //album_art->setFixedSize(200,200);

   /* album_art->border_radius=2;
    album_art->size=200;
    album_art->image.load(":Data/data/cover.jpg");
    //album_art->sizeHint(QSize( 200, 200));*/
    //album_art->setStyleSheet("border:1px solid #333");
    //this->setCoverArt(":/Data/data/cover.jpg");
   // qDebug()<< QDir::current()<<"/cover.jpg";
    //connect(album_art,SIGNAL(clicked()),this,SLOT(labelClicked()));
    /* PLAYBACK CONTROL BOX*/





 // ui->hide_sidebar_btn->setBackgroundRole(QPalette :: Dark);
    ui->hide_sidebar_btn->setToolTip("Go Mini");
    //playback->addWidget(ui->hide_sidebar_btn);

   // playback->addWidget(ui->backward_btn);
   // playback->addWidget(ui->fav_btn);
    //playback->addWidget(ui->play_btn);
//    playback->addWidget(ui->foward_btn);

    ui->shuffle_btn->setToolTip("Shuffle");
  //  playback->addWidget(ui->shuffle_btn);



    // ui->controls = new QWidget(album_art);
    ui->controls->setParent(album_art);
    //ui->controls->setBackgroundRole(QPalette::Dark);
    seekBar = new QSlider();
    seekBar->setMaximum(1000);
    seekBar->setOrientation(Qt::Horizontal);
    seekBar->setContentsMargins(0,0,0,0);
    seekBar->setFixedHeight(5);
   // seekBar->setGeometry(0,195,200,5);
    seekBar->setStyleSheet("QSlider\n{\nbackground:transparent;}\nQSlider::groove:horizontal {border: none; background: transparent; height: 5px; border-radius: 0; } QSlider::sub-page:horizontal {\nbackground: #f85b79;border: none; height: 5px;border-radius: 0;} QSlider::add-page:horizontal {\nbackground: transparent; border: none; height: 5px; border-radius: 0; } QSlider::handle:horizontal {background: #f85b79; width: 8px; } QSlider::handle:horizontal:hover {background: qlineargradient(x1:0, y1:0, x2:1, y2:1,stop:0 #fff, stop:1 #ddd);border: 1px solid #444;border-radius: 4px;}QSlider::sub-page:horizontal:disabled {background: #bbb;border-color: #999;}QSlider::add-page:horizontal:disabled {background: #eee;border-color: #999;}QSlider::handle:horizontal:disabled {background: #eee;border: 1px solid #aaa;border-radius: 4px;}");
    connect(seekBar,SIGNAL(sliderMoved(int)),this,SLOT(on_seekBar_sliderMoved(int)));


    //auto  ui->controls_layout = new QGridLayout();
   //  ui->controls->setLayout( ui->controls_layout);
     ui->controls->setGeometry(100-75,75,150,50);
    // ui->controls->setStyleSheet(" QToolButton {background-color:transparent; }QWidget{background-color: rgba(255, 255, 255, 230); border-radius:6px;} QWidget:hover{background-color:white;} QToolTip{background-color:#545454; border: 1px solid #333; border-radius:2px;} ");




//ui->seekBar->setStyleSheet("background:transparent; ");
    album_view->addWidget(album_art, 0,0,Qt::AlignTop);
    album_view->addWidget(ui->frame_5,1,0);
album_view->addWidget(seekBar,2,0);

 album_view->addWidget(ui->frame_6,3,0);
    album_view->addWidget(ui->listWidget,4,0);
    album_view->addWidget(ui->frame_4,5,0);
    album_view->addWidget(ui->playlistUtils,6,0);



    ui->tracks_view_2->hide();


    /*auto playlistUtilsFrame = new QFrame();
    auto playlistUtilsFrameLayout = new QHBoxLayout();
    auto playlistUtilsFrame_spacer = new QWidget();
    left_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto playlistUtilsFrame_spacer_line = new QFrame();
   playlistUtilsFrame_spacer_line->setFrameShape(QFrame::HLine);
   playlistUtilsFrame_spacer_line->setFrameShadow(QFrame::Plain);
   playlistUtilsFrame_spacer_line->setMaximumHeight(1);*/


    //


    album_view->setContentsMargins(0,0,0,0);
    album_view->setSpacing(0);

    // ui->controls_layout->addWidget(playback,0,0,Qt::AlignHCenter);
    // ui->controls_layout->addWidget(ui->seekBar,1,0,Qt::AlignTop);



   album_widget->setStyleSheet("QWidget { padding:0; margin:0;  }");
    //album_art->setStyleSheet("background-color:red; padding:0; margin:0;");
   // album_art->setStyleSheet("border: 1px solid #333;");
    //playback->setStyleSheet(" QToolButton {background-color:transparent; } QToolBar {background:transparent; border:none;}");


    //album_widget->setLayout(album_view);
    album_art_frame->setLayout(album_view);
    album_widget->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding  );

     line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setMaximumHeight(1);
    frame_layout->setSpacing(0);
    frame_layout->addWidget(views);
    frame_layout->addWidget(line);
    frame_layout->addWidget(utilsBar);

    frame->setLayout(frame_layout);


    layout->addWidget(frame, 0,0 );
    layout->addWidget(album_art_frame,0,1, Qt::AlignRight);
    //this->setStyle();
   // ui->saveResults->setContextMenuPolicy(Qt::ActionsContextMenu);
    saveResults_menu = new QMenu();
    //saveResults_actions();
    //collectionTable->setUpContextMenu();
    //connect(collectionTable,SIGNAL(refreshPlaylistsMenu(QStringList)),this,SLOT(saveResults_actions(QStringList)));
    ui->saveResults->setMenu(saveResults_menu);
    ui->saveResults->setStyleSheet("QToolButton::menu-indicator { image: none; }");
    connect(saveResults_menu, SIGNAL(triggered(QAction*)), this, SLOT(saveResultsTo(QAction*)));

    // ui->saveResults->setPopupMode(QToolButton::InstantPopup);



     //connect(ui->saveResults,SIGNAL(clicked()),ui->saveResults,SLOT(showMenu()));
    ui->listWidget->setCurrentRow(0);
    if(ui->listWidget->count() != 0)
    {

        loadTrack();
        //player->pause();
        updater->start();
        go_mini();


    }else
    {
        addMusicImg->show();
        ui->tracks_view->setChecked(true);
    }




}





MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::addToPlayed(QString url)
{
    if(settings_widget->getCollectionDB().checkQuery("SELECT * FROM tracks WHERE location = \""+url+"\""))
    {
               //ui->fav_btn->setIcon(QIcon::fromTheme("face-in-love"));
        qDebug()<<"Song totally played"<<url;


        QSqlQuery query = settings_widget->getCollectionDB().getQuery("SELECT * FROM tracks WHERE location = \""+url+"\"");

        int played;
        while (query.next()) played = query.value(BabeTable::PLAYED).toInt();
        qDebug()<<played;

        if(settings_widget->getCollectionDB().insertInto("tracks","played",url,played+1))
        {
                    //ui->fav_btn->setIcon(QIcon(":Data/data/love-amarok.svg"));
            qDebug()<<played;

        }

    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
   QMainWindow::resizeEvent(event);
   qDebug()<<event->size().width()<<"x"<<event->size().height();
  if(mini_mode==0 && event->size().width()==this->minimumSize().width())
  {
      //this->setMaximumWidth(200);
     // this->setFixedWidth(200);
      int oldHeight = this->size().height();
 this->resize(200,oldHeight);
      go_mini();
  }else if(mini_mode!=0 && event->size().width()!=200)
  {
      int oldHeight = this->size().height();
      this->resize(700,oldHeight);
      expand();
  }
}

void MainWindow::refreshTables()
{

    collectionTable->flushTable();
    collectionTable->populateTableView("SELECT * FROM tracks");
    favoritesTable->flushTable();
    favoritesTable->populateTableView("SELECT * FROM tracks WHERE stars > \"0\" OR babe =  \"1\"");
    albumsTable->flushGrid();
    albumsTable->populateTableView(settings_widget->getCollectionDB().getQuery("SELECT * FROM albums ORDER by title asc"));

}

void MainWindow::labelClicked()
{
    qDebug()<<"the label got clicked";
}

void MainWindow::AlbumsViewOrder(QString order)
{
    albumsTable->flushGrid();
    albumsTable->populateTableView(settings_widget->getCollectionDB().getQuery("SELECT * FROM albums ORDER by "+order.toLower()+" asc"));

}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return :
    {
        lCounter = getIndex();
        if(lCounter != -1)
        {
            //ui->play->setChecked(false);
             ui->play_btn->setIcon(QIcon::fromTheme("media-playback-pause"));
            ui->search->clear();

           loadTrack();

        }
        break;
    }
    case Qt::Key_Up :
    {
        int ind = getIndex() - 1;if(ind < 0)ind = ui->listWidget->count() - 1;
        ui->listWidget->setCurrentRow(ind);
        break;
    }
    case Qt::Key_Down :
    {
        int ind = getIndex() + 1;if(ind >= ui->listWidget->count())ind = 0;
        ui->listWidget->setCurrentRow(ind);
        break;
    }
    default :
    {
                        ui->search->setFocusPolicy(Qt::StrongFocus);
                 ui->search->setFocus();
        qDebug()<<"trying to focus the serachbar";

        break;
    }
    }
}

 void MainWindow::enterEvent(QEvent *event)
{
    //qDebug()<<"entered the window";
     Q_UNUSED(event);
     showControls();
   //if (mini_mode==2) this->setWindowState(Qt::WindowActive);



}


 void MainWindow::leaveEvent(QEvent *event)
{
    //qDebug()<<"left the window";
    hideControls();

    //timer = new QTimer(this);
      /*connect(timer, SIGNAL(timeout()), this, SLOT(hide ui->controls()));

      connect(timer,SIGNAL(timeout()), this, [&timer, this]() {
          qDebug()<<"ime is up";
          timer->stop();
      });*/

        //timer->start(3000);


}

 void MainWindow::hideControls()
 {
     //qDebug()<<"ime is up";
      ui->controls->hide();
      album_art->titleVisible(false);
    // timer->stop();*/
 }

 void MainWindow::showControls()
 {
     //qDebug()<<"ime is up";
      ui->controls->show();
     // if (mini_mode==2)album_art->titleVisible(true);
    // timer->stop();*/
 }
  void	MainWindow::dragEnterEvent(QDragEnterEvent *event)
 {
     event->accept();
 }

  void	MainWindow::dragLeaveEvent(QDragLeaveEvent *event){
     event->accept();
 }

  void	MainWindow::dragMoveEvent(QDragMoveEvent *event)
 {
     event->accept();
 }

  void	MainWindow::dropEvent(QDropEvent *event)
 {
     QList<QUrl> urls;
     urls = event->mimeData()->urls();
    QStringList list;
     for( auto url  : urls)
     {
         //qDebug()<<url.path();

         if(QFileInfo(url.path()).isDir())
         {
             //QDir dir = new QDir(url.path());
                 QDirIterator it(url.path(), QStringList() << "*.mp4" << "*.mp3" << "*.wav" <<"*.flac" <<"*.ogg" << "*.m4a", QDir::Files, QDirIterator::Subdirectories);
                 while (it.hasNext())
                 {
                     list<<it.next();

                     //qDebug() << it.next();
                 }

         }else if(QFileInfo(url.path()).isFile())
         {
         list<<url.path();
         }


     }

     playlist.add(list);
     updateList();
     //populateTableView();
     //ui->save->setChecked(false);
     if(shuffle) shufflePlaylist();




 }
  void MainWindow::dummy()
  {
      qDebug()<<"TTTTTTTTTTTTTTTTTS on DUMMYT";

  }

  void MainWindow::setLyrics(QString artist,QString title)
  {

      lyrics = new Lyrics();
      connect(lyrics,SIGNAL(lyricsReady(QString)),infoTable,SLOT(setLyrics(QString)));
      lyrics->setData(artist,title);
  }

  void MainWindow::setCoverArt(QString artist, QString album)
  {
      qDebug()<<"Going to try and get the cover for: "<< album <<"by"<<artist;
         auto coverArt = new ArtWork();
          auto artistHead = new ArtWork;

          connect(coverArt, SIGNAL(coverReady(QByteArray)), this, SLOT(putPixmap(QByteArray)));
          connect(artistHead, SIGNAL(headReady(QByteArray)), infoTable, SLOT(setArtistArt(QByteArray)));


          coverArt->setDataCover(artist,album);
          artistHead->setDataHead(artist);

  }


 void MainWindow::putPixmap(QByteArray array)
 {
    album_art->putPixmap(array);
     //infoTable->setAlbumInfo(coverArt->info);

     //delete artwork;

 }


void MainWindow::addToFavorites(QStringList list)
{
    favoritesTable->addRow(list.at(0),list.at(1),list.at(2),list.at(3),list.at(4),list.at(5));
    qDebug()<<list.at(0)<<list.at(1)<<list.at(2)<<list.at(3)<<list.at(4)<<list.at(5);
}

void MainWindow::addToCollection(QStringList list)
{
    collectionTable->addRow(list.at(0),list.at(1),list.at(2),list.at(3),list.at(4),list.at(5));
    qDebug()<<list.at(0)<<list.at(1)<<list.at(2)<<list.at(3)<<list.at(4)<<list.at(5);
}

void MainWindow::setToolbarIconSize(int iconSize)
{
    qDebug()<< "Toolbar icons size changed";
    ui->mainToolBar->setIconSize(QSize(iconSize,iconSize));
    //playback->setIconSize(QSize(iconSize,iconSize));
    utilsBar->setIconSize(QSize(iconSize,iconSize));
    ui->mainToolBar->update();
    //playback->update();
   // this->update();
}



void MainWindow::setUpViews()
{

}


void MainWindow::collectionView()
{
    qDebug()<< "All songs view";
    views->setCurrentIndex(COLLECTION);
    if(mini_mode!=0) expand();

   hideAlbumViewUtils();
   utilsBar->actions().at(COLLECTION_UB)->setVisible(true);

   utilsBar->actions().at(PLAYLISTS_UB)->setVisible(false); ui->frame_3->hide();
   utilsBar->actions().at(INFO_UB)->setVisible(false);

    prevIndex=views->currentIndex();
}

void MainWindow::albumsView()
{
    views->setCurrentIndex(ALBUMS);
    //if(hideSearch)utilsBar->show(); line->show();
    if(mini_mode!=0) expand();
    showAlbumViewUtils();
    utilsBar->actions().at(COLLECTION_UB)->setVisible(true);
    utilsBar->actions().at(PLAYLISTS_UB)->setVisible(false); ui->frame_3->hide();
    utilsBar->actions().at(INFO_UB)->setVisible(false);

    prevIndex=views->currentIndex();
}
void MainWindow::playlistsView()
{
    views->setCurrentIndex(PLAYLISTS);
    if(mini_mode!=0) expand();
     hideAlbumViewUtils();
     utilsBar->actions().at(COLLECTION_UB)->setVisible(true);
     utilsBar->actions().at(PLAYLISTS_UB)->setVisible(true); ui->frame_3->show();
     utilsBar->actions().at(INFO_UB)->setVisible(false);


    prevIndex=views->currentIndex();
}
void MainWindow::queueView()
{
    views->setCurrentIndex(QUEUE);
    if(mini_mode!=0) expand();
     hideAlbumViewUtils();
     utilsBar->actions().at(COLLECTION_UB)->setVisible(false);
     utilsBar->actions().at(PLAYLISTS_UB)->setVisible(false); ui->frame_3->hide();
     utilsBar->actions().at(INFO_UB)->setVisible(false);


    prevIndex=views->currentIndex();
}
void MainWindow::infoView()
{

    views->setCurrentIndex(INFO);

    if(mini_mode!=0) expand();
    hideAlbumViewUtils();
    utilsBar->actions().at(COLLECTION_UB)->setVisible(true);
    utilsBar->actions().at(PLAYLISTS_UB)->setVisible(false);
    utilsBar->actions().at(INFO_UB)->setVisible(true); ui->frame_3->show();

    prevIndex=views->currentIndex();
    //if(!hideSearch)utilsBar->hide(); line->hide();
}
void MainWindow::favoritesView()
{
    views->setCurrentIndex(FAVORITES);
    if(mini_mode!=0) expand();
     hideAlbumViewUtils();
     utilsBar->actions().at(COLLECTION_UB)->setVisible(true);
     utilsBar->actions().at(PLAYLISTS_UB)->setVisible(false); ui->frame_3->hide();
     utilsBar->actions().at(INFO_UB)->setVisible(false);


    prevIndex=views->currentIndex();

   /* QString url= QFileDialog::getExistingDirectory();

qDebug()<<url;

    QStringList urlCollection;
//QDir dir = new QDir(url);
    QDirIterator it(url, QStringList() << "*.mp4" << "*.mp3" << "*.wav" <<"*.flac" <<"*.ogg", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        urlCollection<<it.next();

        //qDebug() << it.next();
    }

   // collection.add(urlCollection);
    //updateList();
    populateTableView();*/
}
void MainWindow::settingsView()
{
    views->setCurrentIndex(SETTINGS);
    if(mini_mode!=0) expand();
    //if(!hideSearch) utilsBar->hide(); line->hide();
     hideAlbumViewUtils();
     utilsBar->actions().at(COLLECTION_UB)->setVisible(true);
     utilsBar->actions().at(PLAYLISTS_UB)->setVisible(false); ui->frame_3->hide();
     utilsBar->actions().at(INFO_UB)->setVisible(false);


    prevIndex=views->currentIndex();

}

void MainWindow::expand()
{
    ui->tracks_view_2->hide();
    views->show(); frame->show();
    ui->mainToolBar->show();
    utilsBar->show(); line->show();
    ui->utilsBar->setChecked(true);
    hideSearch=false;
    album_art_frame->setFrameShadow(QFrame::Raised);
    album_art_frame->setFrameShape(QFrame::StyledPanel);
    layout->setContentsMargins(6,0,6,0);
    this->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    //this->setMinimumSize(750,500);
    //qDebug()<<this->minimumWidth()<<this->minimumHeight();

    this->resize(700,500);
    this->setMinimumSize(0,0);
   // this->adjustSize();
    ui->hide_sidebar_btn->setToolTip("Go Mini");

    ui->hide_sidebar_btn->setIcon(QIcon::fromTheme("object-columns"));
    ui->mainToolBar->actions().at(0)->setVisible(true);
    ui->mainToolBar->actions().at(8)->setVisible(true);

     mini_mode=0;
//keepOnTop(false);
}

void MainWindow::go_mini()
{
    //this->setMaximumSize (0, 0);
    /*this->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
this->show();*/
    ui->tracks_view_2->show();

    QString icon;

    switch(prevIndex)
    {
        case COLLECTION: icon="filename-filetype-amarok"; break;
        case ALBUMS:  icon="media-album-track"; break;
        case FAVORITES:  icon="draw-star"; break;
        case PLAYLISTS:  icon="amarok_lyrics"; break;
        case QUEUE:  icon="amarok_clock"; break;
        case INFO: icon="internet-amarok"; break;
        case SETTINGS:  icon="games-config-options"; break;
        default:  icon="search";
    }




ui->tracks_view_2->setIcon(QIcon::fromTheme(icon));



    views->hide(); frame->hide();
    ui->mainToolBar->hide();
    //playlistTable->line_v->hide();
    utilsBar->hide(); line->hide();
    ui->utilsBar->setChecked(false);
   // hideSearch=true;
   // ui->mainToolBar->actions().at(0)->setVisible(false);
   // ui->mainToolBar->actions().at(8)->setVisible(false);
    album_art_frame->setFrameShadow(QFrame::Plain);
    album_art_frame->setFrameShape(QFrame::NoFrame);
    layout->setContentsMargins(0,0,0,0);
   // ui->utilsBar->setVisible(false);
    //this->setMinimumSize(200,400);
    int oldHeigh = this->size().height();
    this->resize(200,oldHeigh);
    this->adjustSize();
    ui->hide_sidebar_btn->setToolTip("Go Extra-Mini");
    ui->hide_sidebar_btn->setIcon(QIcon::fromTheme("show_table_column"));
    mini_mode=1;
//keepOnTop(true);

}

void MainWindow::keepOnTop(bool state)
{

    if (state)
        {//Qt::WindowFlags flags = windowFlags();
setWindowFlags(Qt::WindowStaysOnTopHint);
show();
    }else
    {
        //setWindowFlags(~ Qt::WindowStaysOnTopHint);
       //show();
    }
}

void MainWindow::setStyle()
{

   /* ui->mainToolBar->setStyleSheet(" QToolBar { border-right: 1px solid #575757; } QToolButton:hover { background-color: #d8dfe0; border-right: 1px solid #575757;}");
    playback->setStyleSheet("QToolBar { border:none;} QToolBar QToolButton { border:none;} QToolBar QSlider { border:none;}");
    this->setStyleSheet("QToolButton { border: none; padding: 5px; }  QMainWindow { border-top: 1px solid #575757; }");*/
    //status->setStyleSheet("QToolButton { color:#fff; } QToolBar {background-color:#575757; color:#fff; border:1px solid #575757;} QToolBar QLabel { color:#fff;}" );

}



void MainWindow::on_hide_sidebar_btn_clicked()
{
    if(mini_mode==0)
    {
        go_mini();

    }else if(mini_mode==1)
    {

        ui->listWidget->hide();
        ui->mainToolBar->hide();
        ui->frame_4->hide();
        ui->playlistUtils->hide();
       // ui->mainToolBar->hide();
       // ui->tableWidget->hide();
        //this->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);

        this->setFixedSize(200,200);
//album_art->border_radius=5;
        album_art->borderColor=true;

       // album_art->setStyleSheet("QLabel{background-color:transparent;}");
       // album_art_frame->setStyleSheet("QFrame{border: 1px solid red;border-radius:5px;}");
       // this->setStyleSheet("QMainWindow{background-color:transparent;");
        //album_widget.
        layout->setContentsMargins(0,0,0,0);

        this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        this->show();
        ui->hide_sidebar_btn->setToolTip("Expand");

        mini_mode=2;

    }else if(mini_mode==2)
    {
        this->setWindowFlags(Qt::Window| Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        this->show();
        // ui->mainToolBar->show();
        ui->listWidget->show();
        ui->frame_4->show();
        ui->playlistUtils->show();

        ui->hide_sidebar_btn->setToolTip("Full View");
        //layout->setContentsMargins(6,0,6,0);
        album_art->titleVisible(false);
       // album_art->border_radius=2;
album_art->borderColor=false;
        //album_art->setStyleSheet("QLabel{border: none}");
        ui->hide_sidebar_btn->setIcon(QIcon::fromTheme("object-columns"));
ui->mainToolBar->hide();


//main_widget->resize(minimumSizeHint());

//this->setMinimumSize(0,0);

this->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
this->resize(700,500);
this->setMinimumSize(0,0);

//this->adjustSize();


//this->updateGeometry();
//this->setfix(minimumSizeHint());
//this->adjustSize();
        mini_mode=3;
    }else if (mini_mode==3)
    {
        expand();
    }


}

void MainWindow::on_shuffle_btn_clicked()
{
    /*state 0: media-playlist-consecutive-symbolic
            1: media-playlist-shuffle
            2:media-playlist-repeat-symbolic
    */
    if(shuffle_state==0)
    {
        shuffle = true;
        shufflePlaylist();
        ui->shuffle_btn->setIcon(QIcon::fromTheme("media-playlist-shuffle"));
        ui->shuffle_btn->setToolTip("Repeat");
        shuffle_state=1;

    }else if (shuffle_state==1)
    {

        repeat = true;
        ui->shuffle_btn->setIcon(QIcon::fromTheme("media-playlist-repeat"));
        ui->shuffle_btn->setToolTip("Consecutive");
        shuffle_state=2;


    }else if(shuffle_state==2)
    {
        repeat = false;
        shuffle = false;
        ui->shuffle_btn->setIcon(QIcon::fromTheme("view-media-playlist"));
        ui->shuffle_btn->setToolTip("Shuffle");
        shuffle_state=0;
    }



}

void MainWindow::on_open_btn_clicked()
{
    //bool startUpdater = false;

    //if(ui->listWidget->count() == 0) startUpdater = true;




      QStringList files = QFileDialog::getOpenFileNames(this, tr("Select Music Files"),QDir().homePath()+"/Music/", tr("Audio (*.mp3 *.wav *.mp4 *.flac *.ogg *.m4a)"));
    if(!files.empty())
    {



        playlist.add(files);
        updateList();
        //populateTableView();
        //ui->save->setChecked(false);
        if(shuffle) shufflePlaylist();
        //if(startUpdater) updater->start();
    }
}



void MainWindow::populateMainList()
{
    QSqlQuery query= settings_widget->getCollectionDB().getQuery("SELECT * FROM tracks WHERE babe = 1 ORDER by played desc");

    QStringList files;
       while (query.next())
       {



        files << query.value(BabeTable::LOCATION).toString();

       }

       playlist.add(files);
       updateList();
       //populateTableView();
       //ui->save->setChecked(false);
       if(shuffle) shufflePlaylist();
}

void MainWindow::updateList()
{
    ui->listWidget->clear();
     ui->listWidget->addItems(playlist.getTracksNameList());
    /*for(auto str : playlist.getTracksNameList())
    {
        auto label =new ScrollText();
        label->setText(str);
        //label->setFixedHeight(40);
        label->setMaxSize(200);
       // label->setStyleSheet("color:red;");
        auto item =new QListWidgetItem();

        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item,label);
    }*/


}

void MainWindow::on_listWidget_doubleClicked(const QModelIndex &index)
{
    lCounter = getIndex();

    //ui->play_btn->setChecked(false);
    //ui->searchBar->clear();
    loadTrack();

    updater->start();
    playing= true;
    ui->play_btn->setIcon(QIcon::fromTheme("media-playback-pause"));

}



void MainWindow::loadTrack()
{
    QString artist=QString::fromStdString(playlist.tracks[getIndex()].getArtist());
    QString album=QString::fromStdString(playlist.tracks[getIndex()].getAlbum());
    QString title=QString::fromStdString(playlist.tracks[getIndex()].getTitle());
    current_song_url = QString::fromStdString(playlist.tracks[getIndex()].getLocation());
    player->setMedia(QUrl::fromLocalFile(current_song_url));
    player->play();
    this->setWindowTitle(title+" \xe2\x99\xa1 "+artist);

    album_art->setArtist(artist);
    album_art->setAlbum(album);
    album_art->setTitle();


    //IF CURRENT SONG EXISTS IN THE COLLECTION THEN GET THE COVER FROM DB
    if(settings_widget->getCollectionDB().checkQuery("SELECT * FROM tracks WHERE location = \""+current_song_url+"\""))
    {
        QSqlQuery queryCover = settings_widget->collection_db.getQuery("SELECT * FROM albums WHERE title = \""+album+"\" AND artist = \""+artist+"\"");
        while (queryCover.next())
        {
            if(!queryCover.value(2).toString().isEmpty())album_art->image.load( queryCover.value(2).toString());

        }

        QSqlQuery queryHead = settings_widget->collection_db.getQuery("SELECT * FROM artists WHERE title = \""+artist+"\"");
        while (queryHead.next())
        {
            infoTable->artist->image.load( queryHead.value(1).toString());

        }


    }else
    {
        emit getCover(artist,album);
    }

    //CHECK IF THE SONG IS BABED IT OR IT ISN'T
    if(settings_widget->getCollectionDB().checkQuery("SELECT * FROM tracks WHERE location = \""+current_song_url+"\" AND babe = \"1\""))
    {
        ui->fav_btn->setIcon(QIcon(":Data/data/loved.svg"));
    }else
    {
        ui->fav_btn->setIcon(QIcon(":Data/data/love-amarok"));
    }

    //AND WHETHER THE SONG EXISTS OR NOT GET THE TRACK INFO
    getTrackInfo(artist,album,title);

     qDebug()<<"Current song playing is: "<< current_song_url;
}


 void MainWindow::getTrackInfo(QString artist, QString album,QString title)
 {
     auto coverInfo = new ArtWork();
     auto artistInfo = new ArtWork;
     connect(coverInfo, SIGNAL(infoReady(QString)), infoTable, SLOT(setAlbumInfo(QString)));
     connect(artistInfo, SIGNAL(bioReady(QString)), infoTable, SLOT(setArtistInfo(QString)));
     coverInfo->setDataCoverInfo(artist,album);
     artistInfo->setDataHeadInfo(artist);
      setLyrics(artist,title);
 }


int MainWindow::getIndex()
{
    return ui->listWidget->currentIndex().row();
}



void MainWindow::on_seekBar_sliderMoved(int position)
{
    player->setPosition(player->duration() / 1000 * position);
}






void MainWindow::update()
{

    if(!seekBar->isSliderDown())
        seekBar->setValue((double)player->position()/player->duration() * 1000);

    if(player->state() == QMediaPlayer::StoppedState)
    {
         QString prevSong = current_song_url;
          qDebug()<<"finished playing song: "<<prevSong;
        next();


       emit finishedPlayingSong(prevSong);

    }


}

void MainWindow::next()
{
    lCounter++;

    if(repeat)
    {
        lCounter--;
    }

    if(lCounter >= ui->listWidget->count())
        lCounter = 0;

    (!shuffle or repeat) ? ui->listWidget->setCurrentRow(lCounter) : ui->listWidget->setCurrentRow(shuffledPlaylist[lCounter]);

    //ui->play->setChecked(false);
    //ui->searchBar->clear();

    loadTrack();


}

void MainWindow::back()
{
     lCounter--;

     if(lCounter < 0)
        lCounter = ui->listWidget->count() - 1;


     (!shuffle) ? ui->listWidget->setCurrentRow(lCounter) : ui->listWidget->setCurrentRow(shuffledPlaylist[lCounter]);

     //ui->play->setChecked(false);
     //ui->searchBar->clear();

     loadTrack();

}

void MainWindow::shufflePlaylist()
{
    shuffledPlaylist.resize(0);

    for(int i = 0; i < ui->listWidget->count(); i++)
    {
        shuffledPlaylist.push_back(i);
    }

    random_shuffle(shuffledPlaylist.begin(), shuffledPlaylist.end());
}

void MainWindow::on_play_btn_clicked()
{
    if(ui->listWidget->count() != 0)
    {
        if(player->state() == QMediaPlayer::PlayingState)
        {
            player->pause();
            ui->play_btn->setIcon(QIcon::fromTheme("media-playback-start"));
        }
       else
       {
            player->play();
            updater->start();
            ui->play_btn->setIcon(QIcon::fromTheme("media-playback-pause"));
       }
      }
}

void MainWindow::on_backward_btn_clicked()
{
    if(ui->listWidget->count() != 0)
    {
        if(player->position() > 3000)
        {
           player->setPosition(0);
        }
        else
        {

            back();
            ui->play_btn->setIcon(QIcon::fromTheme("media-playback-pause"));
        }
     }
}

void MainWindow::on_foward_btn_clicked()
{
    if(ui->listWidget->count() != 0)
    {
        if(repeat)
        {
            repeat = !repeat;next();repeat = !repeat;
        }
        else
        {
            next();
            ui->play_btn->setIcon(QIcon::fromTheme("media-playback-pause"));
        }
     }
}


void MainWindow::collectionDBFinishedAdding(bool state)
{
    if(state)
    {
        qDebug()<<"now it i time to put the tracks in the table ;)";
        //settings_widget->getCollectionDB().closeConnection();
        collectionTable->flushTable();
        collectionTable->populateTableView( "SELECT * FROM tracks");
        albumsTable->flushGrid();
        albumsTable->populateTableView(settings_widget->getCollectionDB().getQuery("SELECT * FROM albums ORDER by title asc"));


    }
}

void MainWindow::orderTables()
{
    favoritesTable->setTableOrder(BabeTable::STARS,BabeTable::DESCENDING);
    collectionTable->setTableOrder(BabeTable::ARTIST,BabeTable::ASCENDING);
    qDebug()<<"finished populating tables, now ordering them";
}





void MainWindow::on_fav_btn_clicked()
{


    if(settings_widget->getCollectionDB().checkQuery("SELECT * FROM tracks WHERE location = \""+current_song_url+"\" AND babe = \"1\""))
    {
        //ui->fav_btn->setIcon(QIcon::fromTheme("face-in-love"));
        qDebug()<<"The song is already babed";
        if(settings_widget->getCollectionDB().insertInto("tracks","babe",current_song_url,0))
        {
            ui->fav_btn->setIcon(QIcon::fromTheme("love-amarok"));

        }

    }else
    {

                  if(settings_widget->getCollectionDB().check_existance("tracks","location",current_song_url))
                  {
                      if(settings_widget->getCollectionDB().insertInto("tracks","babe",current_song_url,1))
                      {
                          ui->fav_btn->setIcon(QIcon(":Data/data/loved.svg"));

                      }
                      qDebug()<<"trying to babe sth";
                  }else
                  {
                     qDebug()<<"Sorry but that song is not in the database";

                     addToCollectionDB({current_song_url},"1");


                       ui->fav_btn->setIcon(QIcon(":Data/data/loved.svg"));

                       //ui->tableWidget->insertRow(ui->tableWidget->rowCount());



                      //to-do: create a list and a tracks object and send it the new song and then write that track list into the database
                  }
                  addToFavorites({QString::fromStdString(playlist.tracks[getIndex()].getTitle()),QString::fromStdString(playlist.tracks[getIndex()].getArtist()),QString::fromStdString(playlist.tracks[getIndex()].getAlbum()),QString::fromStdString(playlist.tracks[getIndex()].getLocation()),"\xe2\x99\xa1","1"});

    }






}
  void MainWindow::scanNewDir(QString url)
{
     QStringList list;
     QDirIterator it(url, QStringList() << "*.mp4" << "*.mp3" << "*.wav" <<"*.flac" <<"*.ogg" <<"*.m4a", QDir::Files, QDirIterator::Subdirectories);
     while (it.hasNext())
     {
       QString song = it.next();
       if(!settings_widget->getCollectionDB().check_existance("tracks","location",song))
       {
         // qDebug()<<"New music files recently added: "<<it.next();
          list<<song;
       }
          //qDebug() << it.next();
     }
     addToCollectionDB_t(list);

 }
void MainWindow::addToCollectionDB(QStringList url, QString babe)
{
   /* Playlist song;
      song.add(url);
      for(auto ui: song.getTracks()) qDebug()<< QString::fromStdString(ui.getTitle());*/

 settings_widget->getCollectionDB().addTrack(url,babe.toInt());
 // addToCollection({QString::fromStdString(song.tracks[getIndex()].getTitle()),QString::fromStdString(song.tracks[getIndex()].getArtist()),QString::fromStdString(song.tracks[getIndex()].getAlbum()),QString::fromStdString(song.tracks[getIndex()].getLocation()),"\xe2\x99\xa1",babe});
 collectionTable->flushTable();
 collectionTable->populateTableView("SELECT * FROM tracks");
 albumsTable->flushGrid();
 albumsTable->populateTableView(settings_widget->getCollectionDB().getQuery("SELECT * FROM albums ORDER by title asc"));
}


//iterates through the paths of the modify folders tp search for new music and then refreshes the collection view
void MainWindow::addToCollectionDB_t(QStringList url)
{
    settings_widget->getCollectionDB().addTrack(url);
    collectionTable->flushTable();
    collectionTable->populateTableView("SELECT * FROM tracks");
    albumsTable->flushGrid();
    albumsTable->populateTableView(settings_widget->getCollectionDB().getQuery("SELECT * FROM albums ORDER by title asc"));

}

void MainWindow::on_utilsBar_clicked()
{

    if(hideSearch)
    {
        //utilsBar->hide(); line->hide();
        utilsBar->hide(); line->hide();
      //  utilsBar->actions().at(1)->setVisible(false);
       // utilsBar->actions().at(2)->setVisible(false);

        ui->utilsBar->setChecked(false);
        hideSearch=false;

    }else
    {
         utilsBar->show(); line->show();
        //utilsBar->actions().at(1)->setVisible(true);
       // utilsBar->actions().at(2)->setVisible(true);
       // utilsBar->show(); line->show();
        ui->utilsBar->setChecked(true);
        hideSearch=true;

    }

    if(mini_mode!=0)
    {
        expand();
        utilsBar->show(); line->show();
        ui->utilsBar->setChecked(true);
        hideSearch=true;
    }
}


void MainWindow::addToPlaylist(QStringList list)
{
    qDebug()<<"hayatuususu";

    playlist.add(list);
        updateList();

        if(shuffle) shufflePlaylist();
        ui->listWidget->scrollToBottom();
}

void MainWindow::on_search_returnPressed()
{

    if(ui->search->text().size()!=0) views->setCurrentIndex(RESULTS);
    else views->setCurrentIndex(prevIndex);


}

void MainWindow::on_search_textChanged(const QString &arg1)
{
    QString search=arg1;
    QStringList keys = {"location:","artist:","album:","title:","genre:" };
    QString key;


        for(auto k : keys)
        {

            if(search.contains(k))
            {

                key=k;
                qDebug()<<"search contains key: "<<key;
                search.replace(k,"");
            }
        }


      qDebug()<<"Searching for: "<<search;
    //int oldIndex = views->currentIndex();
    //qDebug()<<oldIndex;
     hideAlbumViewUtils();


    if(search.size()!=0)
    {
        views->setCurrentIndex(RESULTS);
        if(prevIndex==PLAYLISTS) {utilsBar->actions().at(PLAYLISTS_UB)->setVisible(false); ui->frame_3->hide();}


        resultsTable->flushTable();


            if(key=="location:")
            {


                resultsTable->populateTableView("SELECT * FROM tracks WHERE location LIKE \"%"+search+"%\"");
              ui->search->setBackgroundRole(QPalette :: Dark);
            }else if(key== "artist:")
            {


                resultsTable->populateTableView("SELECT * FROM tracks WHERE artist LIKE \"%"+search+"%\"");
               ui->search->setBackgroundRole(QPalette :: Dark);
            }else if(key== "album:")
            {

                resultsTable->populateTableView("SELECT * FROM tracks WHERE album LIKE \"%"+search+"%\"");
              ui->search->setBackgroundRole(QPalette :: Dark);

            }else if(key=="title:")
            {


                resultsTable->populateTableView("SELECT * FROM tracks WHERE title LIKE \"%"+search+"%\"");
                ui->search->setBackgroundRole(QPalette :: Dark);
            }else if(key==  "genre:")
            {

                resultsTable->populateTableView("SELECT * FROM tracks WHERE genre LIKE \"%"+search+"%\"");
                //ui->search->setStyleSheet("background-color:#e3f4d7;");
                ui->search->setBackgroundRole(QPalette :: Dark);
            }else
            {
                 resultsTable->populateTableView("SELECT * FROM tracks WHERE title LIKE \"%"+search+"%\" OR artist LIKE \"%"+search+"%\" OR album LIKE \"%"+search+"%\"OR genre LIKE \"%"+search+"%\"");
                     //ui->search->setStyleSheet("background-color:transparent;");
                    ui->search->setBackgroundRole(QPalette :: Light);
            }





        //utilsBar->actions().at(1)->setVisible(true);
        //utilsBar->actions().at(2)->setVisible(true);

        ui->saveResults->setEnabled(true);

        ui->refreshAll->setEnabled(false);
       // prevIndex= views->currentIndex();

    }else
    {

        views->setCurrentIndex(prevIndex);
        if(views->currentIndex()==ALBUMS) showAlbumViewUtils();
         if(views->currentIndex()==PLAYLISTS) {utilsBar->actions().at(PLAYLISTS_UB)->setVisible(true); ui->frame_3->show();}

        resultsTable->flushTable();
        //ui->tracks_view->setChecked(true);
       // utilsBar->actions().at(1)->setVisible(false);
       // utilsBar->actions().at(2)->setVisible(false);

        ui->saveResults->setEnabled(false);

        ui->refreshAll->setEnabled(true);
    }

}

void MainWindow::showAlbumViewUtils()
{
    utilsBar->actions().at(ALBUMS_UB)->setVisible(true);
    //utilsBar->actions().at(5)->setVisible(true);
}

void MainWindow::hideAlbumViewUtils()
{

    utilsBar->actions().at(ALBUMS_UB)->setVisible(false);
    //utilsBar->actions().at(5)->setVisible(false);
}



void MainWindow::on_settings_view_clicked()
{
    //setCoverArt(":Data/data/cover.png");
}



void MainWindow::on_rowInserted(QModelIndex model ,int x,int y)
{
    qDebug()<<"indexes moved";
    addMusicImg->hide();
}

void MainWindow::on_refreshBtn_clicked()
{
   playlist.removeAll();
   //QStringList
   populateMainList();
}

void MainWindow::on_tracks_view_2_clicked()
{
    expand();
}

void MainWindow::on_refreshAll_clicked()
{
    refreshTables();
}

void MainWindow::on_addAll_clicked()
{
    switch(views->currentIndex())
    {
        case COLLECTION: addToPlaylist(collectionTable->getTableContent(BabeTable::LOCATION)); break;
        case ALBUMS: addToPlaylist(albumsTable->albumTable->getTableContent(BabeTable::LOCATION)); break;
        case FAVORITES: addToPlaylist(favoritesTable->getTableContent(BabeTable::LOCATION)); break;
        case PLAYLISTS: addToPlaylist(playlistTable->table->getTableContent(BabeTable::LOCATION)); break;
        case QUEUE: addToPlaylist(collectionTable->getTableContent(BabeTable::LOCATION)); break;
         case INFO: addToPlaylist(collectionTable->getTableContent(BabeTable::LOCATION)); break;
    case SETTINGS:  addToPlaylist(collectionTable->getTableContent(BabeTable::LOCATION)); break;
    case RESULTS: addToPlaylist(resultsTable->getTableContent(BabeTable::LOCATION)); break;

    }
}

void MainWindow::on_saveResults_clicked()
{

qDebug()<<"on_saveResults_clicked";
saveResults_menu->clear();

for(auto action: collectionTable->getPlaylistMenus()) saveResults_menu->addAction(action);
ui->saveResults->showMenu();

}



void MainWindow::saveResultsTo(QAction *action)
{
     QString playlist=action->text().replace("&","");
    switch(views->currentIndex())
    {
        case COLLECTION: collectionTable->populatePlaylist(collectionTable->getTableContent(BabeTable::LOCATION),playlist); break;
        case ALBUMS: collectionTable->populatePlaylist(collectionTable->getTableContent(BabeTable::LOCATION),playlist); break;
        case FAVORITES: favoritesTable->populatePlaylist(favoritesTable->getTableContent(BabeTable::LOCATION),playlist); break;
        case PLAYLISTS: playlistTable->table->populatePlaylist(playlistTable->table->getTableContent(BabeTable::LOCATION),playlist); break;
        case QUEUE: collectionTable->populatePlaylist(collectionTable->getTableContent(BabeTable::LOCATION),playlist); break;
         case INFO: collectionTable->populatePlaylist(collectionTable->getTableContent(BabeTable::LOCATION),playlist); break;
    case SETTINGS:  collectionTable->populatePlaylist(collectionTable->getTableContent(BabeTable::LOCATION),playlist); break;
    case RESULTS: resultsTable->populatePlaylist(resultsTable->getTableContent(BabeTable::LOCATION),playlist); break;

    }
}
