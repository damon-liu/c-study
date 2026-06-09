#ifndef RAWJSONPANEL_H
#define RAWJSONPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QClipboard>
#include <QApplication>
#include <QFont>

class RawJsonPanel : public QWidget {
    Q_OBJECT
public:
    explicit RawJsonPanel(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        auto* header = new QHBoxLayout;
        auto* title  = new QLabel("原始 JSON (只读)");
        auto* copyBtn = new QPushButton("📋 一键复制");
        copyBtn->setFixedWidth(110);
        header->addWidget(title);
        header->addStretch();
        header->addWidget(copyBtn);
        layout->addLayout(header);

        mEditor = new QPlainTextEdit(this);
        mEditor->setReadOnly(true);
        mEditor->setFont(QFont("Consolas", 10));
        mEditor->setLineWrapMode(QPlainTextEdit::NoWrap);
        mEditor->setPlaceholderText("选中设备后显示原始 JSON...");
        layout->addWidget(mEditor);

        connect(copyBtn, &QPushButton::clicked, this, [this]() {
            QClipboard* clip = QApplication::clipboard();
            clip->setText(mEditor->toPlainText());
        });
    }

    void setJson(const QString& json) {
        mEditor->setPlainText(json);
    }

    void clear() {
        mEditor->clear();
    }

private:
    QPlainTextEdit* mEditor;
};

#endif // RAWJSONPANEL_H
