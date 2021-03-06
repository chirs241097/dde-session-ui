/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INHIBITWARNVIEW_H
#define INHIBITWARNVIEW_H

#include "warningview.h"
#include "common.h"
#include "rounditembutton.h"

#include <QWidget>

class InhibitWarnView : public WarningView
{
    Q_OBJECT

public:
    explicit InhibitWarnView(QWidget *parent = 0);
    ~InhibitWarnView();

    void setInhibitReason(const QString &reason);
    void setAcceptReason(const QString &reason) override;
    void setAction(const Actions action);
    void setAcceptVisible(const bool acceptable);

    void toggleButtonState() Q_DECL_OVERRIDE;
    void buttonClickHandle() Q_DECL_OVERRIDE;

signals:
    void cancelled() const;
    void actionInvoked(const Actions action) const;

private:
    void onOtherPageDataChanged(const QVariant &value);

private:
    Actions m_action;

    QLabel *m_reasonLbl;
    RoundItemButton *m_acceptBtn;
    RoundItemButton *m_cancelBtn;
    RoundItemButton *m_currentBtn;
    int m_dataBindIndex;
};

#endif // INHIBITWARNVIEW_H
