#include "dbdriver.h"
/**
 * @brief DatabaseDriver::DatabaseDriver
 * @param dbName
 * @param hostName
 * @param userName
 * @param password
 */
DatabaseDriver::DatabaseDriver (QString dbName,
                                QString hostName,
                                QString userName,
                                QString password,
                               QObject *parent) : QObject{parent} {

    this -> databaseObj = QSqlDatabase::addDatabase("QPSQL");

    databaseObj.setHostName(hostName);
    databaseObj.setDatabaseName(dbName);
    databaseObj.setUserName(userName);
    databaseObj.setPassword(password);

    this -> connectionOk = false;

}

/**
 * @brief DatabaseDriver::setConnection
 */
void DatabaseDriver::setConnection() {

    this -> connectionOk = databaseObj.open();

}

/**
 * @brief DatabaseDriver::userExists
 * @param userLogin - user's login to check if exists in database
 * @param userPassword - password to check
 * @return
 */
bool DatabaseDriver::userExists(const QString& userLogin, const QString& userPassword) {

    bool loginValid = false, passwordValid = false;

    if (!databaseObj.isOpen() && !databaseObj.isValid()) {

        return false;

    }

    QSqlQuery query(databaseObj);

    // getting user's login
    query.prepare("SELECT user_login FROM users WHERE user_login = :user_login_requested");

    // binding userLogin to query
    query.bindValue(":user_login_requested", userLogin);

    // checking if user exists
    if (query.exec() && query.next()) {


        qDebug() << "User exists: login ok";
        /*return true;*/
        loginValid = true;

    }

    else {

        qDebug() << "Query failed to execute: " << query.lastError().text();
        return false;

    };

    // checking user's password
    query.prepare("SELECT user_password FROM users WHERE user_login = :user_login_requested");
    query.bindValue(":user_login_requested", userLogin);

    if (query.exec() && query.next() && loginValid) {

        if(query.value(0) == userPassword) {

            qDebug() << "User exists: password ok";
            passwordValid = true;
            return true;

        }

    }

    return false;

}

/**
 * @brief DatabaseDriver::addNewUser adds new user to database via query
 * @param userLogin user's email
 * @param userPassword user's password
 */
void DatabaseDriver::addNewUser(const QString& userLogin, const QString& userPassword, const QString& role) {

    if (!databaseObj.isOpen() && !databaseObj.isValid()) {

        qDebug() << "Error adding user: database error";
        return;

    }

    QSqlQuery addingQuery(databaseObj);

    addingQuery.prepare("INSERT INTO users (user_login, user_password, users_role) VALUES (:userLogin, :userPassword, :role)");

    addingQuery.bindValue(":userLogin", userLogin);
    addingQuery.bindValue(":userPassword", userPassword);
    addingQuery.bindValue(":role", role);

    if(!addingQuery.exec())
        qDebug() << "Failed to add user to database: " << addingQuery.lastError().text();

    else
        qDebug() << "Added user to database";
        return;

}



