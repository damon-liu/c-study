#include "DeviceTreeWidget.h"
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QCursor>

DeviceTreeWidget::DeviceTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setHeaderLabels({"设备"});
    header()->setStretchLastSection(true);
    setRootIsDecorated(true);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    connect(this, &QTreeWidget::itemClicked,
            this, &DeviceTreeWidget::onItemClicked);
}

void DeviceTreeWidget::rebuild(const QVector<DeviceInfo*>& topLevelDevices,
                                 const QVector<DeviceInfo*>& allDevices) {
    clear();
    mItemMap.clear();

    for (auto* dev : topLevelDevices) {
        auto* item = new QTreeWidgetItem(this);
        QString label = (dev->type == DeviceType::Dock)
            ? QString::fromUtf8("🏢 ") + dev->name
            : QString::fromUtf8("✈ ") + dev->name;
        item->setText(0, label);
        item->setData(0, Qt::UserRole, dev->sn);
        item->setData(0, Qt::UserRole + 1, static_cast<int>(dev->type));

        if (!dev->online)
            item->setForeground(0, QColor(180, 180, 180));

        mItemMap[dev->sn] = item;

        // 添加子设备（库内飞机）
        for (auto* child : allDevices) {
            if (child->parentSn == dev->sn) {
                auto* childItem = new QTreeWidgetItem(item);
                childItem->setText(0, QString::fromUtf8("✈ ") + child->name);
                childItem->setData(0, Qt::UserRole, child->sn);
                childItem->setData(0, Qt::UserRole + 1, static_cast<int>(child->type));

                if (!child->online)
                    childItem->setForeground(0, QColor(180, 180, 180));

                mItemMap[child->sn] = childItem;
            }
        }
    }

    expandAll();
}

QString DeviceTreeWidget::selectedDeviceSn() const {
    auto* item = currentItem();
    if (!item) return {};
    return item->data(0, Qt::UserRole).toString();
}

void DeviceTreeWidget::onItemClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column)
    QString sn = item->data(0, Qt::UserRole).toString();
    emit deviceSelected(sn);
}

void DeviceTreeWidget::contextMenuEvent(QContextMenuEvent* event) {
    Q_UNUSED(event)

    QMenu menu(this);
    QAction* addAct = menu.addAction("添加设备");

    QTreeWidgetItem* current = currentItem();

    if (current) {
        menu.addSeparator();
        QAction* editTopicAct = menu.addAction("编辑 Topic");
        QAction* delAct       = menu.addAction("删除设备");

        QAction* chosen = menu.exec(QCursor::pos());
        if (chosen == delAct) {
            onDeleteDevice();
        } else if (chosen == editTopicAct) {
            onEditTopic();
        } else if (chosen == addAct) {
            onAddDevice();
        }
    } else {
        QAction* chosen = menu.exec(QCursor::pos());
        if (chosen == addAct) {
            onAddDevice();
        }
    }
}

void DeviceTreeWidget::onAddDevice() {
    QString typeStr = QInputDialog::getItem(this, "添加设备", "选择设备类型:",
        {"Dock (机场)", "Pilot (手飞飞机)"}, 0, false);
    if (typeStr.isEmpty()) return;

    DeviceType type = typeStr.contains("Dock") ? DeviceType::Dock : DeviceType::Aircraft;

    QString sn = QInputDialog::getText(this, "添加设备", "设备序列号 (SN):");
    if (sn.trimmed().isEmpty()) return;

    QString name = QInputDialog::getText(this, "添加设备", "设备名称:",
        QLineEdit::Normal, sn);
    if (name.trimmed().isEmpty()) return;

    // 生成默认 topic
    QString osdTopic = QString("thing/product/%1/osd").arg(sn.trimmed());
    QStringList defaultTopics;
    defaultTopics << osdTopic;

    // 通过动态属性传递结果，由 MainWindow 消费
    setProperty("_newDeviceSn", sn.trimmed());
    setProperty("_newDeviceType", static_cast<int>(type));
    setProperty("_newDeviceName", name.trimmed());
    setProperty("_newTopics", defaultTopics);

    // 发出信号触发 MainWindow 处理
    emit deviceSelected("");
}

void DeviceTreeWidget::onDeleteDevice() {
    QString sn = selectedDeviceSn();
    if (sn.isEmpty()) return;

    auto ret = QMessageBox::question(this, "确认删除",
        "确定要删除设备 " + sn + " 及其所有 Topic？",
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        setProperty("_deleteDeviceSn", sn);
        emit deviceSelected("");
    }
}

void DeviceTreeWidget::onEditTopic() {
    QString sn = selectedDeviceSn();
    if (sn.isEmpty()) return;

    setProperty("_editTopicSn", sn);
    emit deviceSelected("");
}
