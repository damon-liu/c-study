#include "TopicEditDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>

TopicEditDialog::TopicEditDialog(const QStringList& topics,
                                   const QString& deviceName,
                                   QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("编辑 Topic — " + deviceName);
    setMinimumSize(500, 350);

    auto* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("设备: " + deviceName + "\n管理订阅的 Topic 列表:", this));

    // Topic 列表
    mTopicList = new QListWidget(this);
    for (const auto& t : topics)
        mTopicList->addItem(t);
    layout->addWidget(mTopicList);

    // 操作按钮
    auto* btnLayout = new QHBoxLayout;
    auto* addBtn    = new QPushButton("添加", this);
    auto* editBtn   = new QPushButton("编辑", this);
    auto* removeBtn = new QPushButton("删除", this);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(removeBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    connect(addBtn,    &QPushButton::clicked, this, &TopicEditDialog::onAddTopic);
    connect(editBtn,   &QPushButton::clicked, this, &TopicEditDialog::onEditTopic);
    connect(removeBtn, &QPushButton::clicked, this, &TopicEditDialog::onRemoveTopic);

    // 确定/取消
    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QStringList TopicEditDialog::topics() const {
    QStringList result;
    for (int i = 0; i < mTopicList->count(); ++i)
        result.append(mTopicList->item(i)->text());
    return result;
}

void TopicEditDialog::onAddTopic() {
    QString topic = QInputDialog::getText(this, "添加 Topic",
        "输入 Topic 字符串:", QLineEdit::Normal,
        "thing/product/{sn}/osd");
    if (!topic.trimmed().isEmpty())
        mTopicList->addItem(topic.trimmed());
}

void TopicEditDialog::onRemoveTopic() {
    auto* item = mTopicList->currentItem();
    if (!item) {
        QMessageBox::information(this, "提示", "请先选择要删除的 Topic");
        return;
    }
    delete mTopicList->takeItem(mTopicList->row(item));
}

void TopicEditDialog::onEditTopic() {
    auto* item = mTopicList->currentItem();
    if (!item) {
        QMessageBox::information(this, "提示", "请先选择要编辑的 Topic");
        return;
    }
    QString newTopic = QInputDialog::getText(this, "编辑 Topic",
        "修改 Topic:", QLineEdit::Normal, item->text());
    if (!newTopic.trimmed().isEmpty())
        item->setText(newTopic.trimmed());
}
