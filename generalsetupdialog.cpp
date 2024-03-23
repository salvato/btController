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
#include "generalsetupdialog.h"
#include "utility.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QDir>
#include <QApplication>
#include <QLabel>
#include <QFrame>
#include <QFileDialog>
#include <QStandardPaths>


GeneralSetupDialog::GeneralSetupDialog(GeneralSetupArguments* pArguments)
    : QDialog()
    , pTempArguments(pArguments)
{
    setWindowTitle("General Setup");
    setWindowIcon(QIcon(":/resources/buttonIcons/PanelSetup.png"));

    panelPalette = QWidget::palette();
    panelGradient = QLinearGradient(0.0, 0.0, 0.0, height());
    panelGradient.setColorAt(0, QColor(0, 0, START_GRADIENT));
    panelGradient.setColorAt(1, QColor(0, 0, END_GRADIENT));
    panelBrush = QBrush(panelGradient);
    panelPalette.setBrush(QPalette::Active,   QPalette::Window, panelBrush);
    panelPalette.setBrush(QPalette::Inactive, QPalette::Window, panelBrush);
    panelPalette.setColor(QPalette::WindowText,    Qt::yellow);
    panelPalette.setColor(QPalette::Base,          Qt::black);
    panelPalette.setColor(QPalette::AlternateBase, Qt::blue);
    panelPalette.setColor(QPalette::Text,          Qt::yellow);
    panelPalette.setColor(QPalette::BrightText,    Qt::white);
    setPalette(panelPalette);

    buttonOk.setText("Fatto");
    connect(&buttonOk, SIGNAL(clicked()), this, SLOT(onOk()));

    QLabel* pLabelDirection = new QLabel(("Orientamento"));
    directionCombo.addItem(tr("Normale"));
    directionCombo.addItem(tr("Riflesso"));
    if(pArguments->isPanelMirrored)
        directionCombo.setCurrentText(tr("Riflesso"));
    else
        directionCombo.setCurrentText(tr("Normale"));

    // Signals from Dialog Elements
    connect(&directionCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onChangePanelOrientation(int)));

    directionCombo.setStyleSheet("background:white;color:black");

    QGridLayout* pMainLayout = new QGridLayout;

    pMainLayout->addWidget(pLabelDirection,         0, 0, 1, 2);
    pMainLayout->addWidget(&directionCombo,         0, 2, 1, 6);

    pMainLayout->addWidget(&buttonOk,               1, 7, 1, 1);

    setLayout(pMainLayout);
}


void
GeneralSetupDialog::onChangePanelOrientation(int iOrientation) {
    pTempArguments->isPanelMirrored = false;
    PanelOrientation newOrientation = PanelOrientation::Normal;
    if(iOrientation == 1) {
        newOrientation = PanelOrientation::Reflected;
        pTempArguments->isPanelMirrored = true;
    }
    emit changeOrientation(newOrientation);
}


void
GeneralSetupDialog::onOk() {
    accept();
}

