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
#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

#include "generalsetuparguments.h"
#include "panelorientation.h"


QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)


class GeneralSetupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GeneralSetupDialog(GeneralSetupArguments* pArguments);

signals:
    void changeOrientation(PanelOrientation newOrientation);

public slots:
    void onOk();
    void onChangePanelOrientation(int iOrientation);

private:
    QPalette    panelPalette;
    QGradient   panelGradient;
    QBrush      panelBrush;
    QPushButton buttonOk;

    QComboBox   directionCombo;
    GeneralSetupArguments* pTempArguments;
};

