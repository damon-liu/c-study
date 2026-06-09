#ifndef PUBLISHPANEL_H
#define PUBLISHPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class PublishPanel : public QWidget {
    Q_OBJECT
public:
    explicit PublishPanel(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        // 目标 Topic 选择
        auto* topicLayout = new QHBoxLayout;
        topicLayout->addWidget(new QLabel("Target Topic:"));
        mTopicCombo = new QComboBox(this);
        mTopicCombo->setEditable(true);
        mTopicCombo->setMinimumWidth(300);
        topicLayout->addWidget(mTopicCombo, 1);
        layout->addLayout(topicLayout);

        // JSON 编辑区
        mEditor = new QPlainTextEdit(this);
        mEditor->setFont(QFont("Consolas", 10));
        mEditor->setLineWrapMode(QPlainTextEdit::NoWrap);
        mEditor->setPlaceholderText("输入要发送的 JSON...");
        layout->addWidget(mEditor);

        // 发送按钮
        auto* btnLayout = new QHBoxLayout;
        btnLayout->addStretch();
        mSendBtn = new QPushButton("发送", this);
        mSendBtn->setEnabled(false);  // v1.0 占位
        btnLayout->addWidget(mSendBtn);
        layout->addLayout(btnLayout);

        connect(mSendBtn, &QPushButton::clicked, this, [this]() {
            // TODO v1.1: 实现 MQTT publish
        });
    }

    void setTopics(const QStringList& topics) {
        mTopicCombo->clear();
        mTopicCombo->addItems(topics);
    }

private:
    QComboBox*      mTopicCombo;
    QPlainTextEdit* mEditor;
    QPushButton*    mSendBtn;
};

#endif // PUBLISHPANEL_H
