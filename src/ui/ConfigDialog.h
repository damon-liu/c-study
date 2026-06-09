#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include "ConfigStore.h"

class ConfigDialog : public QDialog {
    Q_OBJECT
public:
    explicit ConfigDialog(const MqttConfig& config, QWidget* parent = nullptr);

    MqttConfig getConfig() const;

private:
    QLineEdit* mHostEdit;
    QSpinBox*  mPortSpin;
    QLineEdit* mUsernameEdit;
    QLineEdit* mPasswordEdit;
};

#endif // CONFIGDIALOG_H
