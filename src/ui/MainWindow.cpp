#include "MainWindow.h"
#include "ConfigDialog.h"
#include "TopicEditDialog.h"
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSet>

MainWindow::MainWindow(DeviceManager* devMgr, QWidget* parent)
    : QMainWindow(parent), mDevMgr(devMgr)
{
    setWindowTitle("DJI MQTT OSD 监控客户端");
    resize(1100, 700);
    setupMenuBar();
    setupLayout();
    setupStatusBar();
    connectSignals();
}

void MainWindow::setupMenuBar() {
    auto* connectMenu = menuBar()->addMenu("连接");

    auto* configAct = connectMenu->addAction("MQTT 配置...");
    connect(configAct, &QAction::triggered, this, [this]() {
        ConfigDialog dlg(mDevMgr->mqttConfig(), this);
        if (dlg.exec() == QDialog::Accepted) {
            MqttConfig cfg = dlg.getConfig();
            mDevMgr->setMqttConfig(cfg);
        }
    });

    connectMenu->addSeparator();

    auto* connectAct = connectMenu->addAction("连接");
    connect(connectAct, &QAction::triggered, this, &MainWindow::onConnectAction);

    auto* disconnectAct = connectMenu->addAction("断开");
    connect(disconnectAct, &QAction::triggered, this, &MainWindow::onDisconnectAction);

    connectMenu->addSeparator();

    auto* exitAct = connectMenu->addAction("退出");
    connect(exitAct, &QAction::triggered, qApp, &QApplication::quit);
}

void MainWindow::setupLayout() {
    // 左侧设备树
    mDeviceTree = new DeviceTreeWidget(this);
    mDeviceTree->setFixedWidth(220);

    // 右侧面板
    mOsdPanel     = new OsdPanel(this);
    mRawJsonPanel = new RawJsonPanel(this);
    mPublishPanel = new PublishPanel(this);
    mPublishPanel->setVisible(false);  // 默认隐藏

    // 分割器: OSD | 原始JSON | Topic下发
    mRightSplitter = new QSplitter(Qt::Vertical, this);
    mRightSplitter->addWidget(mOsdPanel);
    mRightSplitter->addWidget(mRawJsonPanel);
    mRightSplitter->addWidget(mPublishPanel);
    mRightSplitter->setStretchFactor(0, 3);  // OSD 占 3
    mRightSplitter->setStretchFactor(1, 2);  // JSON 占 2
    mRightSplitter->setStretchFactor(2, 0);  // Publish 不拉伸（隐藏时无影响）

    // 展开/折叠按钮
    auto* rightContainer = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    auto* toggleBtn = new QPushButton("▶ Topic 下发", this);
    toggleBtn->setCheckable(true);
    toggleBtn->setStyleSheet("QPushButton { text-align: left; padding: 4px 8px; }");
    connect(toggleBtn, &QPushButton::toggled, this, [this, toggleBtn](bool checked) {
        mPublishPanel->setVisible(checked);
        toggleBtn->setText(checked ? "◢ Topic 下发" : "▶ Topic 下发");
    });
    rightLayout->addWidget(mRightSplitter);
    rightLayout->addWidget(toggleBtn);

    // 主分割器: 设备树 | 右侧面板
    auto* mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->addWidget(mDeviceTree);
    mainSplitter->addWidget(rightContainer);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);

    setCentralWidget(mainSplitter);
}

void MainWindow::setupStatusBar() {
    mStatusLabel      = new QLabel("🔴 未连接");
    mDeviceCountLabel = new QLabel("设备: 0");
    statusBar()->addWidget(mStatusLabel);
    statusBar()->addPermanentWidget(mDeviceCountLabel);
}

void MainWindow::connectSignals() {
    // 设备树选择变化
    connect(mDeviceTree, &DeviceTreeWidget::deviceSelected,
            this, &MainWindow::onDeviceSelected);

    // OSD 数据更新
    connect(mDevMgr, &DeviceManager::deviceOsdUpdated,
            this, &MainWindow::onOsdUpdated);

    // 连接状态
    connect(mDevMgr, &DeviceManager::brokerConnected, this, [this]() {
        mStatusLabel->setText("🟢 已连接 | " +
            mDevMgr->mqttConfig().host + ":" +
            QString::number(mDevMgr->mqttConfig().port));
        updateStatusBar();
    });
    connect(mDevMgr, &DeviceManager::brokerDisconnected, this, [this]() {
        mStatusLabel->setText("🔴 未连接");
    });
    connect(mDevMgr, &DeviceManager::brokerError, this, [this](const QString& err) {
        mStatusLabel->setText("⚠ " + err);
        statusBar()->showMessage("MQTT 错误: " + err, 5000);
    });

    // 设备增删 → 刷新树
    connect(mDevMgr, &DeviceManager::deviceAdded, this, [this]() {
        mDeviceTree->rebuild(mDevMgr->topLevelDevices(), mDevMgr->allDevices());
        updateStatusBar();
    });
    connect(mDevMgr, &DeviceManager::deviceRemoved, this, [this]() {
        mDeviceTree->rebuild(mDevMgr->topLevelDevices(), mDevMgr->allDevices());
        mOsdPanel->clear();
        mRawJsonPanel->clear();
        updateStatusBar();
    });

    // 设备在线状态变化
    connect(mDevMgr, &DeviceManager::deviceOnlineChanged,
            this, [this](const QString& sn, bool online) {
        Q_UNUSED(sn) Q_UNUSED(online)
        mDeviceTree->rebuild(mDevMgr->topLevelDevices(), mDevMgr->allDevices());
    });

    // 设备树右键菜单操作透传
    connect(mDeviceTree, &DeviceTreeWidget::deviceSelected,
            this, &MainWindow::handleDeviceTreeActions);

    // 初始化设备树
    mDeviceTree->rebuild(mDevMgr->topLevelDevices(), mDevMgr->allDevices());
    updateStatusBar();
}

void MainWindow::onDeviceSelected(const QString& sn) {
    if (sn.isEmpty()) {
        // 可能是右键操作信号，检查属性
        handleDeviceTreeActions();
        return;
    }

    DeviceInfo* dev = mDevMgr->device(sn);
    if (!dev) return;

    // OSD
    const AircraftOsd* airOsd = mDevMgr->latestAircraftOsd(sn);
    const DockOsd* dockOsd   = mDevMgr->latestDockOsd(sn);
    mOsdPanel->showOsd(dev, airOsd, dockOsd, mDevMgr->latestRawJson(sn));

    // JSON
    mRawJsonPanel->setJson(mDevMgr->latestRawJson(sn));

    // 更新 publish panel 的 topic 列表
    mPublishPanel->setTopics(mDevMgr->topicsForDevice(sn));
}

void MainWindow::onOsdUpdated(const QString& sn, const QString& rawJson) {
    // 如果当前选中的设备就是被更新的设备，刷新面板
    if (mDeviceTree->selectedDeviceSn() == sn) {
        onDeviceSelected(sn);
    }
}

void MainWindow::onConnectAction() {
    mDevMgr->connectBroker();
}

void MainWindow::onDisconnectAction() {
    mDevMgr->disconnectBroker();
}

void MainWindow::updateStatusBar() {
    mDeviceCountLabel->setText("设备: " +
        QString::number(mDevMgr->allDevices().size()));
}

void MainWindow::handleDeviceTreeActions() {
    // 检查 DeviceTreeWidget 上的属性标记
    QString newSn = mDeviceTree->property("_newDeviceSn").toString();
    if (!newSn.isEmpty()) {
        DeviceInfo info;
        info.sn   = newSn;
        info.name = mDeviceTree->property("_newDeviceName").toString();
        info.type = static_cast<DeviceType>(
            mDeviceTree->property("_newDeviceType").toInt());
        QStringList topics = mDeviceTree->property("_newTopics").toStringList();
        mDevMgr->addDevice(info, topics);

        // 清除属性标记
        mDeviceTree->setProperty("_newDeviceSn", QVariant());
        return;
    }

    QString delSn = mDeviceTree->property("_deleteDeviceSn").toString();
    if (!delSn.isEmpty()) {
        mDevMgr->removeDevice(delSn);
        mDeviceTree->setProperty("_deleteDeviceSn", QVariant());
        return;
    }

    QString editSn = mDeviceTree->property("_editTopicSn").toString();
    if (!editSn.isEmpty()) {
        DeviceInfo* dev = mDevMgr->device(editSn);
        if (dev) {
            QStringList currentTopics = mDevMgr->topicsForDevice(editSn);
            TopicEditDialog dlg(currentTopics, dev->name, this);
            if (dlg.exec() == QDialog::Accepted) {
                QStringList newTopics = dlg.topics();
                // 计算差异并应用
                QSet<QString> oldSet(currentTopics.begin(), currentTopics.end());
                QSet<QString> newSet(newTopics.begin(), newTopics.end());

                // 移除已删除的
                for (const auto& t : currentTopics) {
                    if (!newSet.contains(t))
                        mDevMgr->removeTopic(editSn, t);
                }
                // 添加新增的
                for (const auto& t : newTopics) {
                    if (!oldSet.contains(t))
                        mDevMgr->addTopic(editSn, t);
                }
            }
        }
        mDeviceTree->setProperty("_editTopicSn", QVariant());
        return;
    }
}
