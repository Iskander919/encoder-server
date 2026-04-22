#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#define SETTINGS_PATH "../../config/config.ini"

#include <QSettings>
#include <QFIle>

class ConfigManager : public QObject {

Q_OBJECT

public:
    explicit ConfigManager(QString path, QObject *parent = nullptr);
    ~ConfigManager();

    QString getSmtpHost()     const;
    int     getSmtpPort()     const;
    QString getSmtpSender()   const;
    QString getSmtpPassword() const;

    QString getDatabaseName()     const;
    QString getDatabaseHostname() const;
    QString getDatabaseUsername() const;
    QString getDatabasePassword() const;

private:

    void getSmtpSettings();
    void getDatabaseSettings();

    QSettings settings;

    QSettings readFromConfig();

    QString smtpHost;
    QString smtpPort;
    QString smtpSender;
    QString smtpPassword;

    QString databaseName;
    QString databaseHostname;
    QString databaseUsername;
    QString databasePassword;

};

#endif // CONFIGMANAGER_H
