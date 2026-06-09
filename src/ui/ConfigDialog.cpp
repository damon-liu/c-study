#include "ConfigDialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>

ConfigDialog::ConfigDialog(const MqttConfig& config, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("MQTT 连接配置");
    setMinimumWidth(380);

    auto* layout = new QVBoxLayout(this);

    auto* form = new QFormLayout;
    mHostEdit = new QLineEdit(config.host, this);
    mPortSpin = new QSpinBox(this);
    mPortSpin->setRange(1, 65535);
    mPortSpin->setValue(config.port);
    mUsernameEdit = new QLineEdit(config.username, this);
    mPasswordEdit = new QLineEdit(config.password, this);
    mPasswordEdit->setEchoMode(QLineEdit::Password);

    form->addRow("Broker IP:", mHostEdit);
    form->addRow("端口:", mPortSpin);
    form->addRow("用户名:", mUsernameEdit);
    form->addRow("密码:", mPasswordEdit);
    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

MqttConfig ConfigDialog::getConfig() const {
    MqttConfig cfg;
    cfg.host     = mHostEdit->text().trimmed();
    cfg.port     = mPortSpin->value();
    cfg.username = mUsernameEdit->text().trimmed();
    cfg.password = mPasswordEdit->text();
    return cfg;
}
