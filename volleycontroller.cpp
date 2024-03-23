/*
 *
Copyright (C) 2024  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include <QSettings>
#include <QGridLayout>
#include <QMessageBox>
#include <QResizeEvent>
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QScreen>
#include <QDateTime>

#include "volleycontroller.h"
#include "generalsetupdialog.h"
#include "edit.h"
#include "button.h"
#include "utility.h"

#ifdef Q_OS_ANDROID
    #include <QCoreApplication>
    #include <QJniObject>
#endif

VolleyController::VolleyController(QFile *myLogFile, QWidget *parent)
    : BtScoreController(myLogFile, parent)
    , bFontBuilt(false)
    , pScoreFile(nullptr)
{
    setWindowTitle("Bluetooth Score Controller - © Gabriele Salvato (2024)");
    setWindowIcon(QIcon(":/Logo.ico"));

    panelPalette = QWidget::palette();
    panelGradient = QLinearGradient(0.0, 0.0, 0.0, height());
    panelGradient.setColorAt(0, QColor(0, 0, START_GRADIENT));
    panelGradient.setColorAt(1, QColor(0, 0, END_GRADIENT));
    panelBrush = QBrush(panelGradient);
    panelPalette.setBrush(QPalette::Active,   QPalette::Window, panelBrush);
    panelPalette.setBrush(QPalette::Inactive, QPalette::Window, panelBrush);

    panelPalette.setColor(QPalette::WindowText,      Qt::yellow);
    panelPalette.setColor(QPalette::Base,            Qt::black);
    panelPalette.setColor(QPalette::AlternateBase,   Qt::blue);
    panelPalette.setColor(QPalette::Text,            Qt::yellow);
    panelPalette.setColor(QPalette::BrightText,      Qt::white);
    panelPalette.setColor(QPalette::HighlightedText, Qt::gray);
    panelPalette.setColor(QPalette::Highlight,       Qt::transparent);

    setPalette(panelPalette);

    GetSettings();

    buildControls();
    setWindowLayout();

    pService[iServizio ? 1 : 0]->setChecked(true);
    pService[iServizio ? 0 : 1]->setChecked(false);
    pService[iServizio ? 0 : 1]->setFocus();

#ifndef Q_OS_ANDROID
    prepareScoreFile();
#endif

    setEventHandlers();

#ifdef Q_OS_ANDROID
    keepScreenOn();
#endif
}


void
VolleyController::closeEvent(QCloseEvent *event) {
    if(pScoreFile) {
        pScoreFile->close();
        delete pScoreFile;
        pScoreFile = nullptr;
    }
    SaveSettings();
    BtScoreController::closeEvent(event);
    event->accept();
}


#ifdef Q_OS_ANDROID
void
VolleyController::keepScreenOn() {
    using namespace QNativeInterface;
    QJniObject activity = QAndroidApplication::context();
    if(activity.isValid()) {
        QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

        if (window.isValid()) {
            const int FLAG_KEEP_SCREEN_ON = 128;
            window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
        }
    }
}
#endif


void
VolleyController::resizeEvent(QResizeEvent *event) {
    if(!bFontBuilt) {
        bFontBuilt = true;
        buildFontSizes();
        event->setAccepted(true);
    }
}


bool
VolleyController::prepareScoreFile() {
    QString sPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/";
    sScoreFileName = "Volley_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm");
    sScoreFileName = sPath + sScoreFileName + ".txt";
    QFileInfo checkFile(sScoreFileName);
    if(checkFile.exists() && checkFile.isFile()) {
        QDir renamed;
        renamed.remove(sScoreFileName+QString(".bkp"));
        renamed.rename(sScoreFileName, sScoreFileName+QString(".bkp"));
    }
    if(pScoreFile) {
        pScoreFile->close();
        delete pScoreFile;
        pScoreFile = nullptr;
    }
    pScoreFile = new QFile(sScoreFileName);
    if (!pScoreFile->open(QIODevice::WriteOnly)) {
        QMessageBox::information(Q_NULLPTR, "Segnapunti Volley",
                                 QString("Impossibile aprire il file %1: %2.")
                                     .arg(sScoreFileName, pScoreFile->errorString()));
        delete pScoreFile;
        pScoreFile = nullptr;
    }
    return true;
}


void
VolleyController::logScore() {
    if(pScoreFile) {
        if(pScoreFile->isOpen()) {
            // Set, Team0Name, Team0Score, Team1Name, Team1Score, Time
            QString sMessage = QString("%1, %2, %3, %4, %5, %6\n")
                                   .arg(iSet[0]+iSet[1]+1)
                                   .arg(pTeamName[0]->text())
                                   .arg(iScore[0])
                                   .arg(pTeamName[1]->text())
                                   .arg(iScore[1])
                                   .arg(QTime::currentTime().toString("hh:mm:ss"));
            pScoreFile->write(sMessage.toLatin1()); // toLatin1() --> converted to ASCII
            pScoreFile->flush();
        }
    }
}


void
VolleyController::GeneralSetup() {
    GeneralSetupDialog* pGeneralSetupDialog = new GeneralSetupDialog(&gsArgs);
    connect(pGeneralSetupDialog, SIGNAL(changeOrientation(PanelOrientation)),
            this, SLOT(onChangePanelOrientation(PanelOrientation)));
    int iResult = pGeneralSetupDialog->exec();
    if(iResult == QDialog::Accepted) {
        SaveSettings();
    }
    delete pGeneralSetupDialog;
    pGeneralSetupDialog = nullptr;
}


void
VolleyController::buildFontSizes() {
    QFont font;
    int iFontSize;
    int hMargin, vMargin;
    QMargins margins;

    font = pTeamName[0]->font();
    font.setCapitalization(QFont::Capitalize);
    margins = pTeamName[0]->contentsMargins();
    vMargin = margins.bottom() + margins.top();
    hMargin = margins.left() + margins.right();
    iFontSize = qMin((pTeamName[0]->width()/pTeamName[0]->maxLength())-2*hMargin,
                     pTeamName[0]->height()-vMargin);

    font.setPixelSize(iFontSize);

    font.setPixelSize(iFontSize*0.75);
    pTimeoutLabel->setFont(font);
    pSetsLabel->setFont(font);
    pServiceLabel->setFont(font);
    pScoreLabel->setFont(font);

    font.setWeight(QFont::Black);

    font.setPixelSize(iFontSize);
    pTeamName[0]->setFont(font);
    pTeamName[1]->setFont(font);
    pSetsEdit[0]->setFont(font);
    pSetsEdit[1]->setFont(font);
    pTimeoutEdit[0]->setFont(font);
    pTimeoutEdit[1]->setFont(font);

    font.setPixelSize(2*iFontSize);
    pScoreEdit[0]->setFont(font);
    pScoreEdit[1]->setFont(font);
}



void
VolleyController::setWindowLayout() {
    QWidget *widget = new QWidget();
    QGridLayout* pMainLayout = new QGridLayout();

    int gamePanelWidth  = 15;
    int gamePanelHeigth =  8;

    pMainLayout->addLayout(CreateGamePanel(),
                          0,
                          0,
                          gamePanelHeigth,
                          gamePanelWidth);

    pMainLayout->addLayout(CreateGameButtons(),
                          gamePanelHeigth,
                          0,
                          2,
                          8);

    pMainLayout->addLayout(pSpotButtonsLayout,
                          gamePanelHeigth,
                          8,
                          2,
                          gamePanelWidth-5);

    widget->setLayout(pMainLayout);
    setCentralWidget(widget);
    setTabOrder(pTeamName[0], pTeamName[1]);
}


QGridLayout*
VolleyController::CreateGamePanel() {
    auto* gamePanel = new QGridLayout();
    // Team
    int iRow;
    for(int iTeam=0; iTeam<2; iTeam++) {
        // Matrice x righe e 8 colonne
        iRow = 0;
        gamePanel->addWidget(pTeamName[iTeam], iRow, iTeam*4, 1, 4);
        int iCol = iTeam*5;
        iRow += 1;
        gamePanel->addWidget(pScoreDecrement[iTeam], iRow, iCol,   2, 1, Qt::AlignRight);
        gamePanel->addWidget(pScoreEdit[iTeam],      iRow, iCol+1, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);
        gamePanel->addWidget(pScoreIncrement[iTeam], iRow, iCol+2, 2, 1, Qt::AlignLeft);
        gamePanel->addItem(new QSpacerItem(10,10),     iRow, iCol+3, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);
        iRow += 2;
        gamePanel->addWidget(pService[iTeam],   iRow, iCol, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);
        iRow += 1;
        gamePanel->addWidget(pSetsDecrement[iTeam], iRow, iCol,   1, 1, Qt::AlignRight);
        gamePanel->addWidget(pSetsEdit[iTeam],      iRow, iCol+1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
        gamePanel->addWidget(pSetsIncrement[iTeam], iRow, iCol+2, 1, 1, Qt::AlignLeft);
        iRow += 1;
        gamePanel->addWidget(pTimeoutDecrement[iTeam], iRow, iCol,   1, 1, Qt::AlignRight);
        gamePanel->addWidget(pTimeoutEdit[iTeam],      iRow, iCol+1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
        gamePanel->addWidget(pTimeoutIncrement[iTeam], iRow, iCol+2, 1, 1, Qt::AlignLeft);
    }
    iRow += 1;
    QFrame* myFrame = new QFrame();
    myFrame->setFrameShape(QFrame::HLine);
    gamePanel->addWidget(myFrame, iRow, 0, 1, 10);

    iRow = 1;
    gamePanel->addWidget(pScoreLabel,   iRow, 3, 2, 2);
    iRow += 2;
    gamePanel->addWidget(pServiceLabel, iRow, 3, 1, 2);
    iRow += 1;
    gamePanel->addWidget(pSetsLabel,    iRow, 3, 1, 2);
    iRow += 1;
    gamePanel->addWidget(pTimeoutLabel, iRow, 3, 1, 2);
    //    iRow += 1;

    return gamePanel;
}


QHBoxLayout*
VolleyController::CreateGameButtons() {
    auto* gameButtonLayout = new QHBoxLayout();
    QSize iconSize = QSize(48,48);

    QPixmap* pPixmap = new QPixmap(":/resources/buttonIcons/ExchangeVolleyField.png");
    pChangeFieldButton = new QPushButton(QIcon(*pPixmap), "");
    pChangeFieldButton->setIconSize(iconSize);
    pChangeFieldButton->setFlat(true);
    pChangeFieldButton->setToolTip("Inverti Campo");

    pPixmap->load(":/resources/buttonIcons/New-Set-Volley.png");
    pNewSetButton  = new QPushButton(*pPixmap, "");
    pNewSetButton->setIconSize(iconSize);
    pNewSetButton->setFlat(true);
    pNewSetButton->setToolTip("Nuovo Set");

    delete pPixmap;

    gameButtonLayout->addStretch();
    gameButtonLayout->addWidget(pNewSetButton);
    gameButtonLayout->addStretch();
    gameButtonLayout->addWidget(pChangeFieldButton);
    gameButtonLayout->addStretch();
    return gameButtonLayout;
}


void
VolleyController::GetSettings() {
    gsArgs.maxTimeout           = pSettings->value("volley/maxTimeout", 2).toInt();
    gsArgs.maxSet               = pSettings->value("volley/maxSet", 3).toInt();
    gsArgs.iTimeoutDuration     = pSettings->value("volley/TimeoutDuration", 30).toInt();
    gsArgs.isPanelMirrored      = pSettings->value("panel/orientation",  true).toBool();
    gsArgs.sTeamLogoFilePath[0] = pSettings->value("panel/logo0", ":/Logo_UniMe.png").toString();
    gsArgs.sTeamLogoFilePath[1] = pSettings->value("panel/logo1", ":/Logo_SSD_UniMe.png").toString();
    gsArgs.sTeam[0]             = pSettings->value("team1/name", QString(tr("Locali"))).toString();
    gsArgs.sTeam[1]             = pSettings->value("team2/name", QString(tr("Ospiti"))).toString();
    // qCritical() << "GetSettings()" << gsArgs.sTeam[0] << gsArgs.sTeam[1];

    iTimeout[0] = pSettings->value("team1/timeouts", 0).toInt();
    iTimeout[1] = pSettings->value("team2/timeouts", 0).toInt();
    iSet[0]     = pSettings->value("team1/sets", 0).toInt();
    iSet[1]     = pSettings->value("team2/sets", 0).toInt();
    iScore[0]   = pSettings->value("team1/score", 0).toInt();
    iScore[1]   = pSettings->value("team2/score", 0).toInt();
    iServizio   = pSettings->value("set/service", 0).toInt();
    lastService = pSettings->value("set/lastservice", 0).toInt();

    // Check Stored Values vs Maximum Values
    for(int i=0; i<2; i++) {
        if(iTimeout[i] > gsArgs.maxTimeout)
            iTimeout[i] = gsArgs.maxTimeout;
        if(iSet[i] > gsArgs.maxSet)
            iSet[i] = gsArgs.maxSet;
    }
}


void
VolleyController::SaveStatus() {
    // Save Present Game Values
    pSettings->setValue("team1/name", gsArgs.sTeam[0]);
    pSettings->setValue("team2/name", gsArgs.sTeam[1]);
    pSettings->setValue("team1/timeouts", iTimeout[0]);
    pSettings->setValue("team2/timeouts", iTimeout[1]);
    pSettings->setValue("team1/sets", iSet[0]);
    pSettings->setValue("team2/sets", iSet[1]);
    pSettings->setValue("team1/score", iScore[0]);
    pSettings->setValue("team2/score", iScore[1]);
    pSettings->setValue("set/service", iServizio);
    pSettings->setValue("set/lastservice", lastService);
}


void
VolleyController::SaveSettings() { // Save General Setup Values
    pSettings->setValue("volley/maxTimeout",      gsArgs.maxTimeout);
    pSettings->setValue("volley/maxSet",          gsArgs.maxSet);
    pSettings->setValue("volley/TimeoutDuration", gsArgs.iTimeoutDuration);
    pSettings->setValue("panel/orientation",      gsArgs.isPanelMirrored);
    pSettings->setValue("panel/logo0",            gsArgs.sTeamLogoFilePath[0]);
    pSettings->setValue("panel/logo1",            gsArgs.sTeamLogoFilePath[1]);
    pSettings->setValue("team1/name",             pTeamName[0]->text());
    pSettings->setValue("team2/name",             pTeamName[1]->text());
}


void
VolleyController::buildControls() {
    QString sString;

    QPixmap plusPixmap, minusPixmap;
    QIcon plusButtonIcon, minusButtonIcon;
    plusPixmap.load(":/resources/buttonIcons/Plus.png");
    plusButtonIcon.addPixmap(plusPixmap);
    minusPixmap.load(":/resources/buttonIcons/Minus.png");
    minusButtonIcon.addPixmap(minusPixmap);

    QPalette pal = panelPalette;
    pal.setColor(QPalette::Text, Qt::white);
    for(int iTeam=0; iTeam<2; iTeam++){
        // Teams
        pTeamName[iTeam] = new Edit(gsArgs.sTeam[iTeam], iTeam);
        pTeamName[iTeam]->setAlignment(Qt::AlignHCenter);
        pTeamName[iTeam]->setMaxLength(MAX_NAMELENGTH);
        pal.setColor(QPalette::Text, Qt::white);
        pTeamName[iTeam]->setPalette(pal);
        pTeamName[iTeam]->setReadOnly(true);
        // Timeout
        sString = QString("%1").arg(iTimeout[iTeam], 1);
        pTimeoutEdit[iTeam] = new Edit(sString, iTeam);
        pTimeoutEdit[iTeam]->setAlignment(Qt::AlignHCenter);
        pTimeoutEdit[iTeam]->setMaxLength(1);
        pal.setColor(QPalette::Text, Qt::yellow);
        pTimeoutEdit[iTeam]->setPalette(pal);
        pTimeoutEdit[iTeam]->setReadOnly(true);
        // Timeout buttons
        pTimeoutIncrement[iTeam] = new Button("", iTeam);
        pTimeoutIncrement[iTeam]->setIcon(plusButtonIcon);
        pTimeoutIncrement[iTeam]->setIconSize(plusPixmap.rect().size());
        pTimeoutDecrement[iTeam] = new Button("", iTeam);
        pTimeoutDecrement[iTeam]->setIcon(minusButtonIcon);
        pTimeoutDecrement[iTeam]->setIconSize(minusPixmap.rect().size());
        if(iTimeout[iTeam] == 0)
            pTimeoutDecrement[iTeam]->setEnabled(false);
        if(iTimeout[iTeam] == gsArgs.maxTimeout) {
            pTimeoutIncrement[iTeam]->setEnabled(false);
            pTimeoutEdit[iTeam]->setStyleSheet("background:rgba(0, 0, 0, 0);color:red; border: none");
        }
        // Sets
        sString = QString("%1").arg(iSet[iTeam], 1);
        pSetsEdit[iTeam] = new Edit(sString, iTeam);
        pSetsEdit[iTeam]->setAlignment(Qt::AlignHCenter);
        pSetsEdit[iTeam]->setMaxLength(1);
        pSetsEdit[iTeam]->setPalette(pal);
        pSetsEdit[iTeam]->setReadOnly(true);
        // Set buttons
        pSetsIncrement[iTeam] = new Button("", iTeam);
        pSetsIncrement[iTeam]->setIcon(plusButtonIcon);
        pSetsIncrement[iTeam]->setIconSize(plusPixmap.rect().size());
        pSetsDecrement[iTeam] = new Button("", iTeam);
        pSetsDecrement[iTeam]->setIcon(minusButtonIcon);
        pSetsDecrement[iTeam]->setIconSize(minusPixmap.rect().size());
        if(iSet[iTeam] == 0)
            pSetsDecrement[iTeam]->setEnabled(false);
        if(iSet[iTeam] == gsArgs.maxSet)
            pSetsIncrement[iTeam]->setEnabled(false);
        // Service
        QPixmap pixmap(QString(":/resources/ball%1.png").arg(iTeam));
        QIcon ButtonIcon(pixmap);
        pService[iTeam] = new Button("", iTeam);
        pService[iTeam]->setIcon(ButtonIcon);
        auto const rec = QApplication::primaryScreen()->availableSize();
        auto const height = rec.height();
        pService[iTeam]->setIconSize(QSize(height/16,height/16));
        pService[iTeam]->setCheckable(true);
        pService[iTeam]->setStyleSheet("QPushButton:checked { background-color: rgb(128, 128, 255); border:none }");
        // Score
        pScoreLabel = new QLabel(tr("Punti"));
        pScoreLabel->setAlignment(Qt::AlignRight|Qt::AlignHCenter);
        sString = QString("%1").arg(iScore[iTeam], 2);
        pScoreEdit[iTeam] = new Edit(sString, iTeam);
        pScoreEdit[iTeam]->setAlignment(Qt::AlignHCenter);
        pScoreEdit[iTeam]->setMaxLength(2);
        pScoreEdit[iTeam]->setPalette(pal);
        pScoreEdit[iTeam]->setReadOnly(true);
        // Score buttons
        pScoreIncrement[iTeam] = new Button("", iTeam);
        pScoreIncrement[iTeam]->setIcon(plusButtonIcon);
        pScoreIncrement[iTeam]->setIconSize(plusPixmap.rect().size());
        pScoreDecrement[iTeam] = new Button("", iTeam);
        pScoreDecrement[iTeam]->setIcon(minusButtonIcon);
        pScoreDecrement[iTeam]->setIconSize(minusPixmap.rect().size());
        if(iScore[iTeam] == 0)
            pScoreDecrement[iTeam]->setEnabled(false);
    }
    // Timeout
    pTimeoutLabel = new QLabel(tr("Timeout"));
    pTimeoutLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    // Set
    pSetsLabel = new QLabel(tr("Set"));
    pSetsLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    // Service
    pServiceLabel = new QLabel(tr("Servizio"));
    pServiceLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    // Score
    pScoreLabel = new QLabel(tr("Punti"));
    pScoreLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
}


void
VolleyController::setEventHandlers() {
    for(int iTeam=0; iTeam <2; iTeam++) {
        connect(pTimeoutIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onTimeOutIncrement(int)));
        connect(pTimeoutDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onTimeOutDecrement(int)));
        connect(pSetsIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onSetIncrement(int)));
        connect(pSetsDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onSetDecrement(int)));
        connect(pService[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onServiceClicked(int)));
        connect(pScoreIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onScoreIncrement(int)));
        connect(pScoreDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onScoreDecrement(int)));
    }
    // New Set
    connect(pNewSetButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonNewSetClicked()));
    // Exchange Field Position
    connect(pChangeFieldButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonChangeFieldClicked()));
}


// =========================
// Event management routines
// =========================

void
VolleyController::onTimeOutIncrement(int iTeam) {
    QString sText = QString("<inctimeout>%1</inctimeout>")
                        .arg(iTeam,1);
    sendMessage(sText);
}


void
VolleyController::onTimeOutDecrement(int iTeam) {
    QString sText = QString("<dectimeout>%1</dectimeout>")
                        .arg(iTeam,1);
    sendMessage(sText);
}


void
VolleyController::onSetIncrement(int iTeam) {
    QString sText = QString("<incset>%1</incset>")
                        .arg(iTeam,1);
    sendMessage(sText);
}


void
VolleyController::onSetDecrement(int iTeam) {
    QString sText = QString("<decset>%1</decset>")
                        .arg(iTeam,1);
    sendMessage(sText);
}


void
VolleyController::onServiceClicked(int iTeam) {
    QString sMessage = QString("<servizio>%1</servizio>")
                           .arg(iTeam, 1);
    sendMessage(sMessage);
}


void
VolleyController::onScoreIncrement(int iTeam) {
    QString sText = QString("<incscore>%1</incscore>")
                    .arg(iTeam,1);
    sendMessage(sText);
}


void
VolleyController::onScoreDecrement(int iTeam) {
    QString sText = QString("<decscore>%1</decscore>")
                        .arg(iTeam,1);
    sendMessage(sText);
}


void
VolleyController::onButtonChangeFieldClicked() {
    int iRes = QMessageBox::question(this, tr("Volley_Controller"),
                                     tr("Scambiare il campo delle squadre ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iRes != QMessageBox::Yes) return;

    QString sText = QString("<fieldExchange>1</fieldExchange>");
    sendMessage(sText);
}


void
VolleyController::onButtonNewSetClicked() {
    int iRes = QMessageBox::question(this, tr("Volley_Controller"),
                                     tr("Vuoi davvero iniziare un nuovo Set ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iRes != QMessageBox::Yes) return;

    QString sText = QString("<newSet>1</newSet>");
    sendMessage(sText);
}


void
VolleyController::onChangePanelOrientation(PanelOrientation orientation) {
    Q_UNUSED(orientation)
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Direction %1")
                   .arg(static_cast<int>(orientation)));
#endif
    QString sMessage = QString("<setOrientation>%1</setOrientation>")
                           .arg(static_cast<int>(orientation));
    sendMessage(sMessage);
}


void
VolleyController::processTextMessage(QString sMessage) {
    QString sToken;
    bool ok;
    int iVal;
    QString sNoData = QString("NoData");

    sToken = XML_Parse(sMessage, "team0");
    if(sToken != sNoData){
        pTeamName[0]->setText(sToken.left(maxTeamNameLen));
    }// team0

    sToken = XML_Parse(sMessage, "team1");
    if(sToken != sNoData){
        pTeamName[1]->setText(sToken.left(maxTeamNameLen));
    }// team1

    sToken = XML_Parse(sMessage, "set0");
    if(sToken != sNoData){
        iVal = sToken.toInt(&ok);
        if(!ok || iVal<0 || iVal>3)
            iVal = 8;
        pSetsEdit[0]->setText(QString("%1").arg(iVal));
        pSetsDecrement[0]->setEnabled((iVal > 0));
        pSetsIncrement[0]->setEnabled((iVal != gsArgs.maxSet));
    }// set0

    sToken = XML_Parse(sMessage, "set1");
    if(sToken != sNoData){
        iVal = sToken.toInt(&ok);
        if(!ok || iVal<0 || iVal>3)
            iVal = 8;
        pSetsEdit[1]->setText(QString("%1").arg(iVal));
        pSetsDecrement[1]->setEnabled((iVal > 0));
        pSetsIncrement[1]->setEnabled((iVal != gsArgs.maxSet));
    }// set1

    sToken = XML_Parse(sMessage, "timeout0");
    if(sToken != sNoData){
        iVal = sToken.toInt(&ok);
        if(!ok || iVal<0 || iVal>2)
            iVal = 8;
        pTimeoutEdit[0]->setText(QString("%1"). arg(iVal));
        pTimeoutDecrement[0]->setEnabled((iVal > 0));
        pTimeoutIncrement[0]->setEnabled((iVal != gsArgs.maxTimeout));
    }// timeout0

    sToken = XML_Parse(sMessage, "timeout1");
    if(sToken != sNoData){
        iVal = sToken.toInt(&ok);
        if(!ok || iVal<0 || iVal>2)
            iVal = 8;
        pTimeoutEdit[1]->setText(QString("%1"). arg(iVal));
        pTimeoutDecrement[1]->setEnabled((iVal > 0));
        pTimeoutIncrement[1]->setEnabled((iVal != gsArgs.maxTimeout));
    }// timeout1

    sToken = XML_Parse(sMessage, "score0");
    if(sToken != sNoData) {
        iVal = sToken.toInt(&ok);
        if(!ok || iVal<0 || iVal>99)
            iVal = 99;
        pScoreEdit[0]->setText(QString("%1").arg(iVal));
        pScoreDecrement[0]->setEnabled((iVal > 0));
        pScoreIncrement[0]->setEnabled((iVal != 99));
    }// score0

    sToken = XML_Parse(sMessage, "score1");
    if(sToken != sNoData){
        iVal = sToken.toInt(&ok);
        if(!ok || iVal<0 || iVal>99)
            iVal = 99;
        pScoreEdit[1]->setText(QString("%1").arg(iVal));
        pScoreDecrement[1]->setEnabled((iVal > 0));
        pScoreIncrement[1]->setEnabled((iVal != 99));
    }// score1

    sToken = XML_Parse(sMessage, "servizio");
    if(sToken != sNoData){
        iVal = sToken.toInt(&ok);
        if(!ok || iVal<-1 || iVal>1)
            iVal = 0;
        iServizio = iVal;
        lastService = iServizio;
        pService[iServizio ? 1 : 0]->setChecked(true);
        pService[iServizio ? 0 : 1]->setChecked(false);
        pSettings->setValue("set/service", iServizio);
        pSettings->setValue("set/lastservice", lastService);
    }// servizio

    // To avoid that the focus goes to unwanted objects
    pService[iServizio ? 0 : 1]->setFocus();

    BtScoreController::processGeneralMessages(sMessage);
}
