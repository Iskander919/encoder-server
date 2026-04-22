#include "configmanager.h"

/**
 * @brief ConfigManager::ConfigManager
 * @param path
 */
ConfigManager::ConfigManager(QString path, QObject *parent) : settings{path, QSettings::IniFormat},
    QObject{parent} {

    if(!QFile::exists(path))
        qDebug() << "config file does not exist: " << path;

    getSmtpSettings();
    getDatabaseSettings();

    qDebug() << "Config: ";
    qDebug() << "SMTP: " << this -> smtpHost;
    qDebug() << "Database: " << this -> databaseName;

}

/**
 * @brief ConfigManager::~ConfigManager
 */
ConfigManager::~ConfigManager() {


}

/**
 * @brief ConfigManager::getSmtpSettings
 */
void ConfigManager::getSmtpSettings() {

    settings.beginGroup("SMTP");

    this -> smtpHost   = settings.value("host").toString();
    this -> smtpPort   = settings.value("port").toString();
    this -> smtpSender = settings.value("sender").toString();
    this -> smtpPassword = settings.value("password").toString();

    settings.endGroup();

}

/**
 * @brief ConfigManager::getDatabaseSettings
 */
void ConfigManager::getDatabaseSettings() {

    settings.beginGroup("DATABASE");

    this -> databaseName = settings.value("name").toString();
    this -> databaseHostname = settings.value("hostname").toString();
    this -> databaseUsername = settings.value("username").toString();
    this -> databasePassword = settings.value("password").toString();

    settings.endGroup();

}

/**
 * @brief ConfigManager::getSmtpHost
 * @return
 */
QString ConfigManager::getSmtpHost() const {

    return this ->smtpHost;

}

/**
 * @brief ConfigManager::getSmtpHost
 * @return
 */
int ConfigManager::getSmtpPort() const {

    bool ok = false;

    int result = (this ->smtpPort).toInt(&ok);

    if(ok == true)
        return result;

    else
        return 0;

}

/**
 * @brief ConfigManager::getSmtpSender
 * @return
 */
QString ConfigManager::getSmtpSender() const {

    return this -> smtpSender;

}

/**
 * @brief ConfigManager::getSmtpPassword
 * @return
 */
QString ConfigManager::getSmtpPassword() const {

    return this -> smtpPassword;

}

/**
 * @brief ConfigManager::getDatabaseName
 * @return
 */
QString ConfigManager::getDatabaseName() const {

    return this -> databaseName;

}

/**
 * @brief ConfigManager::getDatabaseHostname
 * @return
 */
QString ConfigManager::getDatabaseHostname() const {

    return this -> databaseHostname;

}

/**
 * @brief ConfigManager::getDatabaseUsername
 * @return
 */
QString ConfigManager::getDatabaseUsername() const {

    return this -> databaseUsername;

}

/**
 * @brief ConfigManager::getDatabasePassword
 * @return
 */
QString ConfigManager::getDatabasePassword() const {

    return this -> databasePassword;

}


