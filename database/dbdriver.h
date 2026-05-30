#ifndef DBDRIVER_H
#define DBDRIVER_H

#include <string>

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QObject>

class DatabaseDriver : public QObject {

Q_OBJECT

public:

    bool connectionOk;

    explicit DatabaseDriver(QString dbName,
                   QString hostName,
                   QString userName,
                            QString password, QObject *parent = nullptr);

    void setConnection();
    void closeConnection();
    bool userExists(const QString& userLogin, const QString& userPassword);

    void addNewUser(const QString& userLogin, const QString& userPassword, const QString& role);

private:

    QSqlDatabase databaseObj;

};

#endif // DBDRIVER_H
