/*
	*
	* NBFolderView.cpp - NewBreeze FolderView Class
	*
*/

#include <NBFolderView.hpp>
#include <NBPluginManager.hpp>

NBFolderView::NBFolderView( QWidget *parent ) : QStackedWidget( parent ) {

	// ClipBoard
	clipBoard = QApplication::clipboard();

	// Set Data Model
	fsModel = new NBFileSystemModel();
	fsModel->setCategorizationEnabled( Settings->General.Grouping );

	// Setup the views
	IconView = new NBIconView( fsModel );
	ApplicationsView = new NBApplicationsView();
	CatalogView = new NBCatalogView();

	// Process Manager
	pMgr = NBProcessManager::instance();

	addWidget( IconView );
	addWidget( ApplicationsView );
	addWidget( CatalogView );

	// Minimum Width - 700px
	setMinimumWidth( 700 );

	// Init moveItems
	moveItems = false;

	// Create and setup actions like copy, move, rename, etc, etc
	createAndSetupActions();

	// Do not accept focus
	setFocusPolicy( Qt::NoFocus );
};

void NBFolderView::updateViewMode() {

	IconView->updateViewMode();
};

bool NBFolderView::hasSelection() {

	return ( IconView->selection().count() > 0 );
};

void NBFolderView::createAndSetupActions() {

	connect( IconView, SIGNAL( open( QModelIndex ) ), this, SLOT( doOpen( QModelIndex ) ) );

	connect( IconView, SIGNAL( contextMenuRequested( QPoint ) ), this, SLOT( showContextMenu( QPoint ) ) );

	connect(
		IconView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
		this, SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) )
	);

	connect( IconView, SIGNAL( link( QStringList, QString ) ), this, SLOT( link( QStringList, QString ) ) );

	// DragDrop copy
	connect( IconView, SIGNAL( copy( QStringList, QString ) ), this, SLOT( copy( QStringList, QString ) ) );

	// DragDrop move
	connect( IconView, SIGNAL( move( QStringList, QString ) ), this, SLOT( move( QStringList, QString ) ) );

	connect( fsModel, SIGNAL( loadFolders() ), this, SLOT( showFolders() ) );
	connect( fsModel, SIGNAL( loadApplications() ), this, SLOT( showApplications() ) );
	connect( fsModel, SIGNAL( loadCatalogs() ), this, SLOT( showCatalogs() ) );

	// Home
	actHomeDir = new QAction( QIcon( ":/icons/home.png" ), "&Home", this );
	actHomeDir->setShortcuts( Settings->Shortcuts.GoHome );

	connect( actHomeDir, SIGNAL( triggered() ), this, SLOT( doOpenHome() ) );
	addAction( actHomeDir );

	// Up
	actParDir = new QAction( QIcon( ":/icons/up.png" ), "&Up", this );
	actParDir->setShortcuts( Settings->Shortcuts.GoUp );

	connect( actParDir, SIGNAL( triggered() ), this, SLOT( goUp() ) );
	addAction( actParDir );

	// Back
	actPrevDir = new QAction( QIcon( ":/icons/prev.png" ), "&Back", this );
	actPrevDir->setShortcuts( Settings->Shortcuts.GoLeft );

	connect( actPrevDir, SIGNAL( triggered() ), this, SLOT( goBack() ) );
	addAction( actPrevDir );

	// Forward
	actNextDir = new QAction( QIcon( ":/icons/next.png" ), "&Forward", this );
	actNextDir->setShortcuts( Settings->Shortcuts.GoRight );

	connect( actNextDir, SIGNAL( triggered() ), this, SLOT( goForward() ) );
	addAction( actNextDir );

	// New Folder
	actNewDir = new QAction( QIcon::fromTheme( "folder-new" ), "New folder", this );
	actNewDir->setShortcuts( Settings->Shortcuts.NewFolder );

	connect( actNewDir, SIGNAL( triggered() ), this, SLOT( newFolder() ) );
	addAction( actNewDir );

	// New file
	actNewFile = new QAction( QIcon::fromTheme( "document-new" ), "New File", this );
	actNewFile->setShortcuts( Settings->Shortcuts.NewFile );

	connect( actNewFile, SIGNAL( triggered() ), this, SLOT( newFile() ) );
	addAction( actNewFile );

	// Peek
	peekAct = new QAction( QIcon( ":/icons/peek.png" ), "Pee&k", this );
	peekAct->setShortcuts( Settings->Shortcuts.Peek );

	connect( peekAct, SIGNAL( triggered() ), this, SLOT( doPeek() ) );
	addAction( peekAct );

	// Copy
	copyAct = new QAction( QIcon( ":/icons/copy.png" ), "&Copy", this );
	copyAct->setShortcuts( Settings->Shortcuts.Copy );

	connect( copyAct, SIGNAL( triggered() ), this, SLOT( prepareCopy() ) );
	addAction( copyAct );

	// Move
	moveAct = new QAction( QIcon( ":/icons/cut.png" ), "Cu&t", this );
	moveAct->setShortcuts( Settings->Shortcuts.Cut );

	connect( moveAct, SIGNAL( triggered() ), this, SLOT( prepareMove() ) );
	addAction( moveAct );

	// Paste
	pasteAct = new QAction( QIcon( ":/icons/paste.png" ), "&Paste", this );
	pasteAct->setShortcuts( Settings->Shortcuts.Paste );

	connect( pasteAct, SIGNAL( triggered() ), this, SLOT( prepareIO() ) );
	addAction( pasteAct );

	// Rename
	renameAct = new QAction( QIcon( ":/icons/rename.png" ), "&Rename", this );
	renameAct->setShortcuts( Settings->Shortcuts.Rename );

	connect( renameAct, SIGNAL( triggered() ), this, SLOT( doRename() ) );
	addAction( renameAct );

	// Reload
	reloadAct = new QAction( QIcon( ":/icons/reload.png" ), "Re&fresh", this );
	reloadAct->setShortcuts( Settings->Shortcuts.Reload );

	connect( reloadAct, SIGNAL( triggered() ), this, SLOT( doReload() ) );
	addAction( reloadAct );

	// showDotFiles
	showHideDotFiles = new QAction( QIcon( ":/icons/showDotFiles.png" ), "Show &Hidden", this );
	showHideDotFiles->setShortcuts( Settings->Shortcuts.ToggleHidden );

	connect( showHideDotFiles, SIGNAL( triggered() ), this, SLOT( doToggleHidden() ) );
	addAction( showHideDotFiles );

	// Trash
	trashAct = new QAction( QIcon( ":/icons/trash.png" ), "Move to trash", this );
	trashAct->setShortcuts( Settings->Shortcuts.Trash );

	connect( trashAct, SIGNAL( triggered() ), this, SLOT( doSendToTrash() ) );
	addAction( trashAct );

	// Delete
	delAct = new QAction( QIcon( ":/icons/delete.png" ), "Delete", this );
	delAct->setShortcuts( Settings->Shortcuts.Delete );

	connect( delAct, SIGNAL( triggered() ), this, SLOT( doDelete() ) );
	addAction( delAct );

	// Properties
	propertiesAct = new QAction( QIcon( ":/icons/props.png" ), "&Properties", this );
	propertiesAct->setShortcuts( Settings->Shortcuts.Properties );

	connect( propertiesAct, SIGNAL( triggered() ), this, SIGNAL( showProperties() ) );
	addAction( propertiesAct );

	// Permissions
	permissionsAct = new QAction( QIcon::fromTheme( "system-users" ), "P&ermissions", this );
	permissionsAct->setShortcuts( Settings->Shortcuts.Permissions );

	connect( permissionsAct, SIGNAL( triggered() ), this, SIGNAL( showPermissions() ) );
	addAction( permissionsAct );

	// Open a virtual terminal emulator
	openVTE = new QAction( "Open &VTE", this );
	openVTE->setShortcuts( Settings->Shortcuts.Terminal );

	connect( openVTE, SIGNAL( triggered() ), this, SLOT( openTerminal() ) );
	addAction( openVTE );

	// Select All
	QAction *selectAllAct = new QAction( "&Select All", this );
	selectAllAct->setShortcuts( Settings->Shortcuts.SelectAll );

	connect( selectAllAct, SIGNAL( triggered() ), this, SLOT( selectAll() ) );
	addAction( selectAllAct );

	// Sorting
	sortByNameAct = new QAction( QIcon::fromTheme( "format-text-underline" ), "&Name", this );
	sortByNameAct->setCheckable( true );
	connect( sortByNameAct, SIGNAL( triggered() ), this, SLOT( sortByName() ) );

	sortByTypeAct = new QAction( QIcon::fromTheme( "preferences-other" ), "&Type", this );
	sortByTypeAct->setCheckable( true );
	connect( sortByTypeAct, SIGNAL( triggered() ), this, SLOT( sortByType() ) );

	sortBySizeAct = new QAction( QIcon( ":/icons/size.png" ), "&Size", this );
	sortBySizeAct->setCheckable( true );
	connect( sortBySizeAct, SIGNAL( triggered() ), this, SLOT( sortBySize() ) );

	sortByDateAct = new QAction( QIcon::fromTheme( "office-calendar" ), "&Date", this );
	sortByDateAct->setCheckable( true );
	connect( sortByDateAct, SIGNAL( triggered() ), this, SLOT( sortByDate() ) );

	QActionGroup *sortGroup = new QActionGroup( this );
	sortGroup->addAction( sortByNameAct );
	sortGroup->addAction( sortByTypeAct );
	sortGroup->addAction( sortBySizeAct );
	sortGroup->addAction( sortByDateAct );

	switch( Settings->General.SortColumn ) {
		case 0: {
			sortByNameAct->setChecked( true );
			break;
		}
		case 1: {
			sortBySizeAct->setChecked( true );
			break;
		}
		case 2: {
			sortByTypeAct->setChecked( true );
			break;
		}
		case 4: {
			sortByDateAct->setChecked( true );
			break;
		}
	}

	groupsAct = new QAction( QIcon::fromTheme( "view-group", QIcon( ":/icons/groups.png" ) ), "Show in &Groups", this );
	groupsAct->setCheckable( true );
	groupsAct->setChecked( Settings->General.Grouping );
	connect( groupsAct, SIGNAL( triggered() ), this, SIGNAL( toggleGroups() ) );
};

QModelIndexList NBFolderView::getSelection() {

	QModelIndexList selectedList;
	selectedList << IconView->selection();
	foreach( QModelIndex idx, selectedList )
		if ( idx.column() )
			selectedList.removeAt( selectedList.indexOf( idx ) );

	return selectedList;
};

void NBFolderView::goUp() {

	fsModel->goUp();
};

void NBFolderView::goBack() {

	if ( fsModel->canGoBack() ) {
		fsModel->goBack();
	}
};

void NBFolderView::goForward() {

	if ( fsModel->canGoForward() ) {
		fsModel->goForward();
	}
};

void NBFolderView::doOpenHome() {

	NBDebugMsg( DbgMsgPart::ONESHOT, "Opening dir: %s ", NBXdg::home().toLocal8Bit().data() );
	if ( Settings->General.CombiHome )
		fsModel->loadCombiView();

	else
		fsModel->goHome();
};

void NBFolderView::newFile() {

	NBNewNodeDialog *newFile = new NBNewNodeDialog( "file", QDir( fsModel->currentDir() ), QString(), this );
	newFile->exec();
};

void NBFolderView::newFolder() {

	NBNewNodeDialog *newFolder = new NBNewNodeDialog( "dir", QDir( fsModel->currentDir() ), QString(), this );
	newFolder->exec();
};

void NBFolderView::doOpen( QString loc ) {

	if ( loc.startsWith( "NB://" ) ) {
		fsModel->setRootPath( loc );
		return;
	}

	if ( not isReadable( loc ) ) {
		QString title = tr( "Access Error" );
		QString text;
		text += tr( "<p>You do not have enough permissions to open the %1:</p><p><center><b>%2</b></center></p>" );
		text += tr(  "<p>Please change the permissions of the %1 to enter it." );
		if ( isDir( loc ) )
			text = text.arg( "directory" ).arg( loc );

		else
			text = text.arg( "file" ).arg( loc );

		if ( fsModel->rootPath().isEmpty() )
			text += tr( "<br>Instead, I will be opening your <b>home</b> folder for you.</p>" );

		NBMessageDialog::error( this, title, text );

		fsModel->setRootPath( NBXdg::home() );

		return;
	}

	if ( isDir( loc ) ) {
		NBDebugMsg( DbgMsgPart::ONESHOT, "Opening dir: %s ", loc.toLocal8Bit().data() );
		fsModel->setRootPath( loc );
	}

	else if ( isFile( loc ) ) {
		if ( isExec( loc ) and not isText( loc ) ) {
			/*
				*
				* We make sure that @v loc is really an executable file,
				* i.e it is one of x-exec or x-sharedlib or something
				* of the sort and not a shellscript or jpg file with exec
				* permissions
				*
			*/
			NBDebugMsg( DbgMsgPart::HEAD, "Executing: %s... ", loc.toLocal8Bit().data() );
			NBDebugMsg( DbgMsgPart::TAIL, ( QProcess::startDetached( loc ) ? "[DONE]" : "[FAILED]" ) );
		}

		else {
			NBDebugMsg( DbgMsgPart::HEAD, "Opening file: %s ", loc.toLocal8Bit().data() );
			NBAppFile app = NBAppEngine::instance()->xdgDefaultApp( mimeDb.mimeTypeForFile( loc ) );
			if ( not app.isValid() )
				doOpenWithCmd();

			QStringList exec = app.execArgs();

			// Prepare @v exec
			if ( app.takesArgs() ) {
				if ( app.multipleArgs() ) {
					int idx = exec.indexOf( "<#NEWBREEZE-ARG-FILES#>" );
					exec.removeAt( idx );
					exec.insert( idx, loc );
				}

				else {
					int idx = exec.indexOf( "<#NEWBREEZE-ARG-FILE#>" );
					exec.removeAt( idx );
					exec.insert( idx, loc );
				}
			}
			else {
				exec << loc;
			}

			NBDebugMsg( DbgMsgPart::TAIL, ( QProcess::startDetached( exec.takeFirst(), exec ) ? "[DONE]" : " [FAILED]" ) );
		}
	}

	else {
		QString title = QString( "Error opening file" );
		QString text = QString( "I really do not have any idea how to open <tt><b>%1</b></tt>" ).arg( loc );

		NBMessageDialog::error( this, title, text );
		return;
	}

	currentWidget()->setFocus();
};

void NBFolderView::doOpen( QModelIndex idx ) {
	/* This slot is triggered when the user double clicks or presses enter */

	Q_UNUSED( idx );
	QList<QModelIndex> selectedList = getSelection();

	foreach( QModelIndex index, selectedList ) {
		QString fileToBeOpened = fsModel->nodePath( index );

		if ( not isReadable( fileToBeOpened ) ) {
			QString title = tr( "Access Error" );
			QString text = tr( "You do not have enough permissions to open <b>%1</b>. " ).arg( baseName( fileToBeOpened ) );
			if ( isDir( fileToBeOpened ) )
				text += tr( "Please change the permissions of the directory to enter it." );

			else
				text += tr( "Please change the permissions of the file to edit/view it." );

			NBMessageDialog::error( this, title, text );
			return;
		}

		if ( isDir( fileToBeOpened ) ) {
			NBDebugMsg( DbgMsgPart::ONESHOT, "Opening dir: %s", fileToBeOpened.toLocal8Bit().data() );
			if ( index == idx )
				fsModel->setRootPath( fileToBeOpened );

			else
				emit newTab( fileToBeOpened );
		}

		else if ( isFile( fileToBeOpened ) ) {
			if ( isExec( fileToBeOpened ) and not isText( fileToBeOpened ) ) {
				/*
					*
					* We make sure that @v fileToBeOpened is really an executable file,
					* i.e it is one of shellscript, install file, or x-exec or x-sharedlib
					* or something of the sort and not a jpg file with exec perms
					*
				*/
				NBDebugMsg( DbgMsgPart::HEAD, "Executing: %s... ", fileToBeOpened.toLocal8Bit().data() );
				NBDebugMsg( DbgMsgPart::TAIL, ( QProcess::startDetached( fileToBeOpened ) ? "[DONE]" : "[FAILED]" ) );
			}

			else {
				NBDebugMsg( DbgMsgPart::HEAD, "Opening file: %s ", fileToBeOpened.toLocal8Bit().data() );
				NBAppFile app = NBAppEngine::instance()->xdgDefaultApp( mimeDb.mimeTypeForFile( fileToBeOpened ) );
				if ( not app.isValid() )
					doOpenWithCmd();

				QStringList exec = app.execArgs();

				// Prepare @v exec
				if ( app.takesArgs() ) {
					if ( app.multipleArgs() ) {
						int idx = exec.indexOf( "<#NEWBREEZE-ARG-FILES#>" );
						exec.removeAt( idx );
						exec.insert( idx, fileToBeOpened );
					}

					else {
						int idx = exec.indexOf( "<#NEWBREEZE-ARG-FILE#>" );
						exec.removeAt( idx );
						exec.insert( idx, fileToBeOpened );
					}
				}
				else {
					exec << fileToBeOpened;
				}

				NBDebugMsg( DbgMsgPart::TAIL, ( QProcess::startDetached( exec.takeFirst(), exec ) ? "[DONE]" : " [FAILED]" ) );
			}
		}

		else {
			QString title = QString( "Error" );
			QString text = QString( "I really do not have any idea how to open <b>%1</b>." ).arg( index.data().toString() );

			NBMessageDialog::error( this, title, text );
			return;
		}
	}

	currentWidget()->setFocus();
};

void NBFolderView::doOpenWith() {

	QStringList cmdList = qobject_cast<QAction *>( sender() )->data().toStringList();
	QString cmd = cmdList.takeFirst();

	QProcess::startDetached( cmd, cmdList );
};

void NBFolderView::doOpenInNewWindow() {

	emit newWindow( qobject_cast<QAction *>( sender() )->data().toString() );
};

void NBFolderView::doOpenInNewTab() {

	emit newTab( qobject_cast<QAction *>( sender() )->data().toString() );
};

void NBFolderView::doOpenWithCmd() {

	QStringList files;

	if ( qobject_cast<QAction *>( sender() ) ) {
		QAction *action = qobject_cast<QAction *>( sender() );
		files << action->data().toStringList();
	}

	else {
		Q_FOREACH( QModelIndex idx, getSelection() )
			files << fsModel->nodePath( idx );
	}

	NBRunCmdDialog *runCmd = new NBRunCmdDialog( QFileInfo( files[ 0 ] ).fileName() );
	runCmd->exec();

	if ( runCmd->canRun() ) {
		QStringList exec = runCmd->commandString().split( " " );

		// Prepare @v exec
		if ( exec.contains( "<#NEWBREEZE-ARG-FILES#>" ) ) {
			int idx = exec.indexOf( "<#NEWBREEZE-ARG-FILES#>" );
			exec.removeAt( idx );
			foreach( QString file, files )
				exec.insert( idx, file );
		}

		else if ( exec.contains( "<#NEWBREEZE-ARG-FILE#>" ) ) {
			int idx = exec.indexOf( "<#NEWBREEZE-ARG-FILE#>" );
			exec.removeAt( idx );
			exec.insert( idx, files[ 0 ] );
		}

		else
			exec << files;

		QString cmd = exec.takeFirst();
		QProcess::startDetached( cmd, exec );
	}
};

void NBFolderView::showApplications() {

	setCurrentIndex( 1 );
	currentWidget()->setFocus();

	emit hideStatusBar();
};

void NBFolderView::showCatalogs() {

	setCurrentIndex( 2 );
	currentWidget()->setFocus();

	emit hideStatusBar();
};

void NBFolderView::showFolders() {

	setCurrentIndex( 0 );
	currentWidget()->setFocus();

	emit showStatusBar();
};

void NBFolderView::doPeek() {

	QModelIndex curIndex = IconView->currentIndex();

	if ( !curIndex.isValid() ) {
		emit showProperties();
		return;
	}

	if ( getSelection().count() > 1 ) {
		emit showProperties();
		return;
	}

	QString currentNode = QDir( fsModel->currentDir() ).absoluteFilePath( curIndex.data().toString() );

	if ( not isReadable( currentNode ) ) {
		NBMessageDialog::error( this,
			QString( "Cannot Open Location" ),
			QString(
				"You do not have enough permissions to preview the location. "		\
				"Please change the permissions of the file/directory to see its contents."
			)
		);

		return;
	}

	/* Get the mime type data of the current node */
	QString mimeType = getMimeType( currentNode );

	/* For directories we use the inbuild previewer */
	if ( isDir( currentNode ) ) {
		NBDebugMsg( DbgMsgPart::ONESHOT, "Previewing folder: %s", currentNode.toLocal8Bit().data() );
		NBFolderFlash *previewer = new NBFolderFlash( currentNode );

		connect( previewer, SIGNAL( loadFolder( QString ) ), this, SLOT( doOpen( QString ) ) );
		previewer->show();

		return;
	}

	/* Other mimetypes, we depend on the PluginManager */
	/* If the PluginManager returns a valid path, we use it */
	else if ( NBPluginManager::instance()->hasPluginForMimeType( mimeType ) ) {

		QPluginLoader loader( NBPluginManager::instance()->pluginForMimeType( mimeType ) );
		QObject *pObj = loader.instance();
		NBPreviewInterface *plugin = 0;

		if ( pObj ) {
			plugin = qobject_cast<NBPreviewInterface*>( pObj );

			QDialog *previewer = plugin->getPreviewWidget( currentNode );
			previewer->setWindowFlags( previewer->windowFlags() | Qt::FramelessWindowHint );
			previewer->exec();

			return;
		}
	}

	// Custom Peeking
	NBDebugMsg( DbgMsgPart::ONESHOT, "Previewing file: %s", currentNode.toLocal8Bit().data() );
	NBCustomPeek *previewer = new NBCustomPeek( currentNode );
	previewer->show();

	currentWidget()->setFocus();
};

void NBFolderView::doReload() {

	NBDebugMsg( DbgMsgPart::ONESHOT, "Reloading..." );
	fsModel->reload();
};

void NBFolderView::doToggleHidden() {

	if ( fsModel->showHidden() ) {
		NBDebugMsg( DbgMsgPart::ONESHOT, "Hiding dot files..." );
		fsModel->setShowHidden( false );
	}

	else {
		NBDebugMsg( DbgMsgPart::ONESHOT, "Showing dot files..." );
		fsModel->setShowHidden( true );
	}

	Settings->General.ShowHidden = fsModel->showHidden();
};

void NBFolderView::prepareCopy() {

	if ( not IconView->selection().count() )
		return;

	moveItems = false;
	QModelIndexList copyList = getSelection();

	QList<QUrl> urlList;
	foreach( QModelIndex item, copyList )
		urlList << QUrl::fromLocalFile( fsModel->nodePath( item.data().toString() ) );

	QMimeData *mData = new QMimeData();
	mData->setUrls( urlList );

	clipBoard->setMimeData( mData );
};

void NBFolderView::prepareMove() {

	if ( not IconView->selection().count() )
		return;

	moveItems = true;
	QModelIndexList copyList = getSelection();

	QList<QUrl> urlList;
	foreach( QModelIndex item, copyList )
		urlList << QUrl::fromLocalFile( fsModel->nodePath( item.data().toString() ) );

	QMimeData *mData = new QMimeData();
	mData->setUrls( urlList );

	clipBoard->setMimeData( mData );
};

void NBFolderView::copy( QStringList sources, QString tgt ) {

	if ( not sources.count() )
		return;

	NBProcess::Progress *progress = new NBProcess::Progress;
	progress->sourceDir = dirName( sources.value( 0, QDir::currentPath() ) );
	progress->targetDir = fsModel->currentDir();

	QStringList srcList;
	foreach( QString path, sources )
		srcList << path.replace( progress->sourceDir, "" );

	progress->type = NBProcess::Copy;

	NBIOProcess *proc = new NBIOProcess( srcList, progress );
	pMgr->addProcess( progress, proc );

	progress->startTime = QTime::currentTime();

	proc->start();
};

void NBFolderView::move( QStringList sources, QString tgt ) {

	if ( not sources.count() )
		return;

	NBProcess::Progress *progress = new NBProcess::Progress;
	progress->sourceDir = dirName( sources.value( 0, QDir::currentPath() ) );
	progress->targetDir = fsModel->currentDir();

	QStringList srcList;
	foreach( QString path, sources )
		srcList << path.replace( progress->sourceDir, "" );

	progress->type = NBProcess::Move;

	NBIOProcess *proc = new NBIOProcess( srcList, progress );
	pMgr->addProcess( progress, proc );

	progress->startTime = QTime::currentTime();

	proc->start();
};

void NBFolderView::link( QStringList linkList, QString path ) {

	foreach( QString node, linkList )
		QFile::link( node, QDir( path ).filePath( baseName( node ) ) );
};

void NBFolderView::prepareIO() {

	const QMimeData *mimeData = clipBoard->mimeData();

	if ( mimeData->hasUrls() ) {
		NBProcess::Progress *progress = new NBProcess::Progress;
		progress->sourceDir = dirName( mimeData->urls().at( 0 ).toLocalFile() );
		progress->targetDir = fsModel->currentDir();

		QStringList srcList;
		foreach( QUrl url, mimeData->urls() )
			srcList << url.toLocalFile().replace( progress->sourceDir, "" );

		if ( moveItems )
			progress->type = NBProcess::Move;

		else
			progress->type = NBProcess::Copy;

		NBIOProcess *proc = new NBIOProcess( srcList, progress );
		pMgr->addProcess( progress, proc );

		progress->startTime = QTime::currentTime();

		proc->start();
	}

	else {

		NBNewNodeDialog *newFolder = new NBNewNodeDialog( "else", fsModel->currentDir(), mimeData->text() );
		newFolder->exec();
	}
};

void NBFolderView::doSendToTrash() {

	if ( !IconView->selection().count() ) {
		return;
	}

	QList<QModelIndex> selectedList = getSelection();

	/* Check if we have protection set */
	QSettings nbSettings( "NewBreeze", "NewBreeze" );
	QStringList safeNodes = nbSettings.value( "ProtectedNodes" ).toStringList();

	NBProcess::Progress *progress = new NBProcess::Progress;
	progress->sourceDir = fsModel->currentDir();
	progress->targetDir = QString();
	progress->type = NBProcess::Trash;

	QStringList toBeDeleted;
	Q_FOREACH( QModelIndex idx, selectedList ) {
		QString path = QDir( fsModel->rootPath() ).filePath( idx.data().toString() );
		if ( not safeNodes.contains( path ) )
			toBeDeleted << path.replace( progress->sourceDir, "" );
	}

	/* If some files have protection inform the user */
	if ( toBeDeleted.count() != selectedList.count() ) {
		NBMessageDialog::warning( NULL,
			"Error deleting protected files",
			"You have enabled <b><tt>Accidental Delete Protection</tt></b> for some of the files or folders. "
			"Only the files without accidental protection will be deleted. If you really want to delete protected "
			"files, please remove the protection and try again."
		);
	}

	NBDeleteProcess *proc = new NBDeleteProcess( toBeDeleted, progress );
	pMgr->addProcess( progress, proc );

	progress->startTime = QTime::currentTime();

	proc->start();
};

void NBFolderView::doDelete() {

	if ( not IconView->selection().count() ) {
		return;
	}

	QList<QModelIndex> selectedList = getSelection();

	/* Check if we have protection set */
	QSettings nbSettings( "NewBreeze", "NewBreeze" );
	QStringList safeNodes = nbSettings.value( "ProtectedNodes" ).toStringList();

	NBProcess::Progress *progress = new NBProcess::Progress;
	progress->sourceDir = fsModel->currentDir();
	progress->targetDir = QString();
	progress->type = NBProcess::Delete;

	QStringList toBeDeleted;
	Q_FOREACH( QModelIndex idx, selectedList ) {
		QString path = QDir( fsModel->rootPath() ).filePath( idx.data().toString() );
		if ( not safeNodes.contains( path ) )
			toBeDeleted << path.replace( progress->sourceDir, "" );
	}

	NBConfirmDeleteDialog *delDlg = new NBConfirmDeleteDialog( toBeDeleted, this );

	if ( not delDlg->exec() )
		return;

	/* If some files have protection inform the user */
	if ( toBeDeleted.count() != selectedList.count() ) {
		NBMessageDialog::warning( NULL,
			"Error deleting protected files",
			"You have enabled <b><tt>Accidental Delete Protection</tt></b> for some of the files or folders. "
			"Only the files without accidental protection will be deleted. If you really want to delete protected "
			"files, please remove the protection and try again."
		);
	}

	NBDeleteProcess *proc = new NBDeleteProcess( toBeDeleted, progress );
	pMgr->addProcess( progress, proc );

	progress->startTime = QTime::currentTime();

	proc->start();
};

void NBFolderView::doRename() {

	QModelIndex curIndex = IconView->currentIndex();

	if ( !curIndex.isValid() )
		return;

	QString curFile = curIndex.data().toString();
	NBRenameDialog *renamer = new NBRenameDialog( curFile, QDir( fsModel->currentDir() ), this );
	renamer->exec();

	currentWidget()->setFocus();

	if ( !renamer->canRename() ) {
		qDebug() << "Renaming" << curFile << "[Canceled]";
		return;
	}

	QString opath = QDir( fsModel->currentDir() ).filePath( curFile );
	QString npath = QDir( fsModel->currentDir() ).filePath( renamer->newName() );

	NBDebugMsg( DbgMsgPart::HEAD, "Renaming %s to %s... ", opath.toLocal8Bit().data(), npath.toLocal8Bit().data() );
	if ( rename( opath.toLocal8Bit().data(), npath.toLocal8Bit().data() ) ) {

		NBDebugMsg( DbgMsgPart::TAIL, "[Failed]" );
	}

	else {

		NBDebugMsg( DbgMsgPart::TAIL, "[Done]" );
		// To spare the trouble with NBFileSystemWatcher
		fsModel->rename( opath, npath );
	}
};

void NBFolderView::sortByName() {

	if ( fsModel->isCategorizationEnabled() )
		fsModel->sort( 0, Qt::CaseInsensitive, true );

	else
		fsModel->sort( 0, Qt::CaseInsensitive, false );

	QSettings sett( fsModel->nodePath( ".directory" ), QSettings::NativeFormat );
	sett.setValue( "NewBreeze/SortColumn", 0 );
};

void NBFolderView::sortBySize() {

	if ( fsModel->isCategorizationEnabled() )
		fsModel->sort( 1, Qt::CaseInsensitive, true );

	else
		fsModel->sort( 1, Qt::CaseInsensitive, false );

	QSettings sett( fsModel->nodePath( ".directory" ), QSettings::NativeFormat );
	sett.setValue( "NewBreeze/SortColumn", 1 );
};

void NBFolderView::sortByType() {

	if ( fsModel->isCategorizationEnabled() )
		fsModel->sort( 2, Qt::CaseInsensitive, true );

	else
		fsModel->sort( 2, Qt::CaseInsensitive, false );

	QSettings sett( fsModel->nodePath( ".directory" ), QSettings::NativeFormat );
	sett.setValue( "NewBreeze/SortColumn", 2 );
};

void NBFolderView::sortByDate() {

	if ( fsModel->isCategorizationEnabled() )
		fsModel->sort( 4, Qt::CaseInsensitive, true );

	else
		fsModel->sort( 4, Qt::CaseInsensitive, false );

	QSettings sett( fsModel->nodePath( ".directory" ), QSettings::NativeFormat );
	sett.setValue( "NewBreeze/SortColumn", 4 );
};

void NBFolderView::selectAll() {

	IconView->selectAll();
};

void NBFolderView::openTerminal() {

	NBDebugMsg( DbgMsgPart::HEAD, "Opening the console at %s... ", fsModel->currentDir().toLocal8Bit().data() );
	QStringList commandList = getTerminal();
	QString command = commandList.takeFirst();

	if ( command == QString( "xterm" ) )
		commandList[ 1 ] = QString( "cd %1 && /bin/bash" ).arg( termFormatString( fsModel->currentDir() ) );

	else {
		commandList[ 1 ] = fsModel->currentDir();
		commandList[ 3 ] = "/bin/bash";
	}

	NBDebugMsg( DbgMsgPart::TAIL, ( QProcess::startDetached( command, commandList ) ? "[DONE]" : "[FAILED]" ) );
};

void NBFolderView::setFocus() {

	currentWidget()->setFocus();
};

void NBFolderView::handleWatchDogBark( QString path ) {

	QFileInfo info( path );
	if ( !info.exists() ) {
		QString title = QString( "Error" );
		QString text = QString( "This directory has been deleted by an external process. Loading home dir." );

		NBMessageDialog::error( this, title, text );
		fsModel->goHome();
	}

	else {
		doReload();
	}
};

void NBFolderView::compress( QStringList archiveList ) {

	NBArchive *arc = NBArchiveDialog::newArchive( this );
	if ( arc == NULL )
		return;

	arc->setWorkingDir( fsModel->currentDir() );
	arc->updateInputFiles( archiveList );
	QtConcurrent::run( arc, &NBArchive::create );
};

void NBFolderView::extract( QString archive ) {

	QString dest = fsModel->nodePath( QFileInfo( archive ).baseName() );
	// Create the dest folder if it does nor exist
	if ( not exists( dest ) )
		QDir::current().mkdir( dest );

	NBArchive arc( archive );
	arc.setDestination( dest );
	QtConcurrent::run( arc, &NBArchive::extract );
};

void NBFolderView::updateProgress( QString nodePath, float fileCopied, float totalCopied ) {

	Q_UNUSED( fileCopied );
	Q_UNUSED( totalCopied );

	if ( dirName( nodePath ) == fsModel->currentDir() ) {
		fsModel->updateNode( baseName( nodePath ) );
	}
};

void NBFolderView::handleDeleteFailure( QStringList files, QStringList dirs ) {

	if ( ( files.count() ) or ( dirs.count() ) ) {
		NBDeleteErrorsDialog *delErrDlg = new NBDeleteErrorsDialog( files, dirs, this );
		delErrDlg->exec();
	}
};