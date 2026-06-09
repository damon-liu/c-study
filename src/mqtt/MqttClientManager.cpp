#include "MqttClientManager.h"
#include <QDebug>
#include <QCoreApplication>

MqttClientManager::MqttClientManager(QObject* parent)
    : QObject(parent)
    , mClient(new QMqttClient(this))
    , mReconnectTimer(new QTimer(this))
    , mReconnectDelayMs(BASE_RECONNECT_MS)
{
    mReconnectTimer->setSingleShot(true);

    connect(mClient, &QMqttClient::connected,
            this, &MqttClientManager::onConnected);
    connect(mClient, &QMqttClient::disconnected,
            this, &MqttClientManager::onDisconnected);
    connect(mClient, &QMqttClient::errorChanged,
            this, &MqttClientManager::onError);
    connect(mClient, &QMqttClient::messageReceived,
            this, &MqttClientManager::onMessageReceived);
    connect(mReconnectTimer, &QTimer::timeout,
            this, &MqttClientManager::onReconnectTimer);
}

MqttClientManager::~MqttClientManager() {
    disconnectFromBroker();
}

void MqttClientManager::connectToBroker(const MqttConfig& config) {
    mConfig = config;
    mReconnectDelayMs = BASE_RECONNECT_MS;

    mClient->setHostname(config.host);
    mClient->setPort(static_cast<quint16>(config.port));
    mClient->setUsername(config.username);
    mClient->setPassword(config.password);
    mClient->setClientId("DjiOsdMonitor_" + QString::number(QCoreApplication::applicationPid()));

    qDebug() << "MQTT: connecting to" << config.host << ":" << config.port;
    mClient->connectToHost();
}

void MqttClientManager::disconnectFromBroker() {
    stopReconnect();
    mSubscribedTopics.clear();
    mClient->disconnectFromHost();
}

bool MqttClientManager::isConnected() const {
    return mClient->state() == QMqttClient::Connected;
}

void MqttClientManager::subscribeTopics(const QStringList& topics) {
    if (!isConnected() || topics.isEmpty()) return;
    for (const auto& t : topics) {
        if (mSubscribedTopics.contains(t)) continue;
        auto* sub = mClient->subscribe(t, 1);  // QoS 1
        if (sub) {
            mSubscribedTopics.append(t);
            qDebug() << "MQTT: subscribed" << t;
        } else {
            qWarning() << "MQTT: subscribe failed" << t;
        }
    }
}

void MqttClientManager::unsubscribeTopics(const QStringList& topics) {
    if (!isConnected() || topics.isEmpty()) return;
    for (const auto& t : topics) {
        mClient->unsubscribe(t);
        mSubscribedTopics.removeAll(t);
        qDebug() << "MQTT: unsubscribed" << t;
    }
}

void MqttClientManager::replaceSubscriptions(const QStringList& addTopics,
                                               const QStringList& removeTopics) {
    unsubscribeTopics(removeTopics);
    subscribeTopics(addTopics);
}

void MqttClientManager::onConnected() {
    qDebug() << "MQTT: connected";
    stopReconnect();
    emit connected();
}

void MqttClientManager::onDisconnected() {
    qDebug() << "MQTT: disconnected";
    emit disconnected();
    startReconnect();
}

void MqttClientManager::onError(QMqttClient::ClientError error) {
    Q_UNUSED(error)
    QString errStr = mClient->errorString();
    qWarning() << "MQTT error:" << errStr;
    emit connectionError(errStr);
}

void MqttClientManager::onMessageReceived(const QByteArray& message,
                                           const QMqttTopicName& topic) {
    emit messageReceived(topic.name(), message);
}

void MqttClientManager::onReconnectTimer() {
    qDebug() << "MQTT: reconnecting...";
    mClient->connectToHost();
}

void MqttClientManager::startReconnect() {
    if (mReconnectTimer->isActive()) return;
    mReconnectDelayMs = qMin(mReconnectDelayMs * 2, MAX_RECONNECT_MS);
    qDebug() << "MQTT: retry in" << mReconnectDelayMs << "ms";
    mReconnectTimer->start(mReconnectDelayMs);
}

void MqttClientManager::stopReconnect() {
    mReconnectTimer->stop();
    mReconnectDelayMs = BASE_RECONNECT_MS;
}
