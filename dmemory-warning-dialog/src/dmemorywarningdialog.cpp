#include "dmemorywarningdialog.h"
#include "processinfotable.h"
#include "processinfomodel.h"
#include "processinfodelegate.h"
#include "buttondelegate.h"

#include <QKeyEvent>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

#include <ddialog.h>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

DMemoryWarningDialog *dialog = nullptr;

bool confirm(const QPixmap &icon)
{
    const QStringList btns = QStringList() << QApplication::translate("DMemoryWarningDialog", "Cancel")
//                                           << QApplication::translate("DMemoryWarningDialog", "View")
                                           << QApplication::translate("DMemoryWarningDialog", "Free");

    DDialog terminateDialog(dialog);
    terminateDialog.setMessage(QApplication::translate("DMemoryWarningDialog", "Are you sure to terminate this process?"));
    terminateDialog.setIconPixmap(icon);
    terminateDialog.addButtons(btns);

    QObject::connect(&terminateDialog, &DDialog::buttonClicked, [&](const int index) {
        if (index == 1)
            terminateDialog.accept();
    });

    if (terminateDialog.exec() == DDialog::Accepted)
        return true;
    return false;
}

DMemoryWarningDialog::DMemoryWarningDialog(QWidget *parent)
    : DAbstractDialog(parent)

    , m_startManagerInter(new StartManagerInter("com.deepin.SessionManager", "/com/deepin/StartManager", QDBusConnection::sessionBus(), this))
    , m_infoModel(new ProcessInfoModel)

    , m_icon(new QLabel)
    , m_memNeeded(new QLabel)

    , m_cancelButton(new QPushButton)
    , m_continueButton(new QPushButton)
{
    setWindowTitle(" ");

    dialog = this;

    m_cancelButton->setText(tr("Cancel"));
    m_continueButton->setText(tr("Continue"));
    m_icon->setPixmap(QIcon::fromTheme("dde").pixmap(32, 32));
    m_memNeeded->setText("dde need 100M memory");

    ProcessInfoTable *table = new ProcessInfoTable;
    table->setModel(m_infoModel);
    table->setItemDelegate(new ProcessInfoDelegate);
    table->setItemDelegateForColumn(3, new ButtonDelegate);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addWidget(m_cancelButton);
    btnsLayout->addWidget(m_continueButton);

    QLabel *icon = new QLabel;
    icon->setAlignment(Qt::AlignCenter);
    icon->setPixmap(QIcon::fromTheme("messagebox_warning").pixmap(64, 64));

    QLabel *label = new QLabel;
    label->setText(tr("Please free some memory to start new process"));
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    QFont f = label->font();
    f.setPointSize(12);
    label->setFont(f);

    QHBoxLayout *tipsLayout = new QHBoxLayout;
    tipsLayout->addStretch();
    tipsLayout->addWidget(m_icon);
    tipsLayout->addWidget(m_memNeeded);
    tipsLayout->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(icon);
    vLayout->addWidget(label);
    vLayout->addWidget(table);
    vLayout->addSpacing(20);
    vLayout->addLayout(tipsLayout);
    vLayout->addSpacing(20);
    vLayout->addLayout(btnsLayout);

    setLayout(vLayout);
    setFixedSize(400, 600);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Dialog);
    move(qApp->primaryScreen()->geometry().center() - rect().center());

    connect(m_cancelButton, &QPushButton::clicked, this, &DMemoryWarningDialog::onCancelClicked);
    connect(m_continueButton, &QPushButton::clicked, this, &DMemoryWarningDialog::onContinueClicked);
    connect(m_startManagerInter, &StartManagerInter::NeededMemoryChanged, this, &DMemoryWarningDialog::onNeedMemChanged);
    onNeedMemChanged(m_startManagerInter->neededMemory());
}

DMemoryWarningDialog::~DMemoryWarningDialog()
{
    m_startManagerInter->TryAgain(false);
}

void DMemoryWarningDialog::updateAppInfo(const QString &appInfo)
{
    if (appInfo.endsWith(".desktop"))
    {
        const int start = appInfo.lastIndexOf('/');
        const int end = appInfo.indexOf(".desktop");
        const QString &appName = appInfo.mid(start + 1, end - start - 1);

        m_icon->setPixmap(QIcon::fromTheme(appInfo.mid(start + 1, end - start - 1)).pixmap(32, 32));
        m_appName = appName;
        m_tipsType = LaunchApp;
    } else if (appInfo.contains("TabsLimit.OpenUrl")) {
        m_icon->setPixmap(QIcon::fromTheme("google-chrome").pixmap(32, 32));
        m_tipsType = OpenChromeTab;
    } else {
        m_icon->setPixmap(QIcon::fromTheme("terminal").pixmap(32, 32));
        m_tipsType = ExecuteCommand;
    }

    updateTips();
}

void DMemoryWarningDialog::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
#ifdef QT_DEBUG
    case Qt::Key_Escape:    qApp->quit();   break;
#endif
    default:;
    }
}

void DMemoryWarningDialog::showEvent(QShowEvent *e)
{
    DAbstractDialog::showEvent(e);

    QTimer::singleShot(1, this, &DMemoryWarningDialog::raise);
    QTimer::singleShot(1, this, &DMemoryWarningDialog::activateWindow);

    m_infoModel->startRefreshData();
}

void DMemoryWarningDialog::hideEvent(QHideEvent *e)
{
    DAbstractDialog::hideEvent(e);

    m_infoModel->stopRefreshData();
}

void DMemoryWarningDialog::onNeedMemChanged(const qulonglong needed)
{
    m_continueButton->setEnabled(!needed);
    m_needed = needed;
    updateTips();
}

void DMemoryWarningDialog::onContinueClicked()
{
    m_startManagerInter->TryAgain(true);
    hide();
}

void DMemoryWarningDialog::onCancelClicked()
{
    m_startManagerInter->TryAgain(false);
    hide();
}

void DMemoryWarningDialog::updateTips()
{
    if (m_needed)
    {
        const int m_bytes = m_needed / 1024;

        switch (m_tipsType) {
        case LaunchApp:
            m_memNeeded->setText(tr("%1 need extra %2M to launch").arg(m_appName).arg(m_bytes));
            break;
        case OpenChromeTab:
            m_memNeeded->setText(tr("Need extra %1M to open new tab").arg(m_bytes));
            break;
        case ExecuteCommand:
            m_memNeeded->setText(tr("Need extra %1M to execute command").arg(m_bytes));
            break;
        }
    } else {
        switch (m_tipsType) {
        case LaunchApp:
            m_memNeeded->setText(tr("Click continue to open %1").arg(m_appName));
            break;
        case OpenChromeTab:
            m_memNeeded->setText(tr("Click continue to open new tab"));
            break;
        case ExecuteCommand:
            m_memNeeded->setText(tr("Click continue to execute command"));
            break;
        }
    }
}