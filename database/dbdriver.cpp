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

    if (!databaseObj.isOpen() && !databaseObj.isValid()) {

        //qdebug:: something
        return false;

    }

    QSqlQuery query(databaseObj);

    // getting user's login
    query.prepare("SELECT user_login FROM users WHERE user_login = :user_login_requested");

    // binding userLogin to query
    query.bindValue(":user_login_requested", userLogin);

    // checking if user exists
    if (query.exec() && query.next()) {


        qDebug() << "User exists";
        return true;

    }

    else {

        qDebug() << "Query failed to execute: " << query.lastError().text();
        return false;

    };

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



