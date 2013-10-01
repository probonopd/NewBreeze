/*
	*
	* NBPreviewWidget.hpp - NewBreeze PreviewWidget Classes
	*
*/

#include <NBCustomPeek.hpp>
#include <NBTools.hpp>
#include <NBThumbsCache.hpp>
#include <NBGuiWidgets.hpp>

NBCustomPeek::NBCustomPeek( QWidget *parent, QString pth ) : QWidget( parent ) {

	path = QString( pth );

	createGUI();
	setWindowProperties();
};

void NBCustomPeek::createGUI() {

	QHBoxLayout *lblBtnLyt = new QHBoxLayout();
	QVBoxLayout *widgetLyt = new QVBoxLayout();
	QVBoxLayout *baseLyt = new QVBoxLayout();

	QLabel *lbl = new QLabel( "<tt><b>" + path + "</b></tt>" );

	QToolButton *openBtn = new QToolButton();
	openBtn->setIcon( QIcon( ":/icons/maximize.png" ) );
	openBtn->setAutoRaise( true );
	openBtn->setFocusPolicy( Qt::NoFocus );

	QWidget *baseWidget = new QWidget();
	baseWidget->setObjectName( tr( "guiBase" ) );

	connect( openBtn, SIGNAL( clicked() ), this, SLOT( openInExternal() ) );

	peekWidgetBase = new QWidget();
	peekWidgetBase->setFixedSize( 700, 500 );
	peekWidgetBase->setObjectName( tr( "previewBase" ) );

	iconLbl = new QLabel();
	nameLbl = new QLabel();
	pathLbl = new QLabel();
	mimeLbl = new QLabel();
	sizeLbl = new QLabel();
	timeLbl = new QLabel();
	ugidLbl = new QLabel();

	getFileProps();

	QVBoxLayout *fLyt = new QVBoxLayout();
	fLyt->setAlignment( Qt::AlignCenter );
	fLyt->addWidget( iconLbl );
	fLyt->addWidget( nameLbl );
	fLyt->addWidget( pathLbl );
	fLyt->addWidget( Separator::horizontal() );

	fLyt->addWidget( mimeLbl );
	fLyt->addWidget( sizeLbl );
	fLyt->addWidget( Separator::horizontal() );

	fLyt->addWidget( timeLbl );
	fLyt->addWidget( Separator::horizontal() );

	fLyt->addWidget( ugidLbl );

	QFrame *frame = new QFrame();
	frame->setFrameStyle( QFrame::StyledPanel | QFrame::Plain );
	frame->setFixedWidth( 540 );
	frame->setLayout( fLyt );

	QVBoxLayout *previewLyt = new QVBoxLayout();
	previewLyt->addWidget( frame );
	previewLyt->setAlignment( Qt::AlignCenter );
	peekWidgetBase->setLayout( previewLyt );

	lblBtnLyt->addWidget( lbl );
	lblBtnLyt->addStretch( 0 );
	lblBtnLyt->addWidget( openBtn );

	widgetLyt->addLayout( lblBtnLyt );
	widgetLyt->addWidget( peekWidgetBase );

	baseWidget->setLayout( widgetLyt );
	baseLyt->addWidget( baseWidget );
	baseLyt->setContentsMargins( 0, 0, 0, 0 );

	setLayout( baseLyt );
	peekWidgetBase->setFocus();
};

void NBCustomPeek::setWindowProperties() {

	setFixedSize( 720, 540 );
	setWindowFlags( Qt::Popup );

	if ( ( Settings.General.Style == QString( "TransDark" ) ) or ( Settings.General.Style == QString( "TransLight" ) ) )
		setAttribute( Qt::WA_TranslucentBackground );

	setStyleSheet( getStyleSheet( "NBPreview", Settings.General.Style ) );

	QDesktopWidget dw;
	int hpos = (int)( ( dw.width() - 720 ) / 2 );
	int vpos = (int)( ( dw.height() - 540 ) / 2 );

	setGeometry( hpos, vpos, 720, 540 );
};

void NBCustomPeek::keyPressEvent( QKeyEvent *keyEvent ) {

	if ( keyEvent->key() == Qt::Key_Escape )
		close();

	else
		QWidget::keyPressEvent( keyEvent );
};

void NBCustomPeek::getFileProps() {

	iconLbl->setPixmap( NBIcon::icon( QFileInfo( path ), QSize( 64, 64 ) ) );
	QFileInfo info( path );

	QFontMetrics fm( nameLbl->font() );

	QString ctime = info.created().toString( "ddd, MMM dd, yyyy hh:mm:ss AP" );
	QString mtime = info.lastModified().toString( "ddd, MMM dd, yyyy hh:mm:ss AP" );
	QString atime = info.lastRead().toString( "ddd, MMM dd, yyyy hh:mm:ss AP" );

	iconLbl->setAlignment( Qt::AlignCenter );
	nameLbl->setAlignment( Qt::AlignCenter );
	pathLbl->setAlignment( Qt::AlignCenter );
	mimeLbl->setAlignment( Qt::AlignCenter );
	sizeLbl->setAlignment( Qt::AlignCenter );
	timeLbl->setAlignment( Qt::AlignCenter );
	ugidLbl->setAlignment( Qt::AlignCenter );

	nameLbl->setText( fm.elidedText( "<tt>Name:</tt> " + info.fileName(), Qt::ElideMiddle, 540 ) );
	pathLbl->setText( fm.elidedText( "<tt>Path:</tt> " + info.absolutePath(), Qt::ElideMiddle, 540 ) );
	mimeLbl->setText( "<tt>MimeType:</tt> " + getMimeTypeAlt( path ) );
	sizeLbl->setText( "<tt>Size:</tt> " + formatSize( info.size() ) );

	timeLbl->setText( QString(
		"<tt>Creation Time:</tt> %1<br>"		\
		"<tt>Modification Time:</tt> %2<br>"	\
		"<tt>Access Time:</tt> %3"				\
		).arg( ctime ).arg( mtime ).arg( atime )
	);

	ugidLbl->setText(  QString(
		"<tt>User:</tt> %1<br>"		\
		"<tt>Goup:</tt> %2<br>"		\
		).arg( info.owner() ).arg( info.group() )
	);
};


void NBCustomPeek::openInExternal() {

	QProcess::startDetached( "xdg-open " + path );
	close();
};