#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHttpServer>
#include <QHttpServerResponse>
#include <QTcpServer>

#include "configmanager.h"
#include "dbdriver.h"
#include "otp.h"

#include <memory>

// declaring function prototypes
std::shared_ptr<DatabaseDriver> initDatabase(ConfigManager *configManager, QObject &parent);
std::shared_ptr<OTP> initOtp(ConfigManager *configManager, QObject &parent);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "Starting application";

    // read config file
    ConfigManager *configManager = new ConfigManager("config.ini", &a);

    // init database driver with settings from configManager
    std::shared_ptr<DatabaseDriver> databaseDriver = initDatabase(configManager, a);

    // init OTP sender
    std::shared_ptr<OTP> otpDriver = initOtp(configManager, a);

    // setup HTTP server
    QHttpServer httpServer;

    // adding routes
    // returning hello statement
    httpServer.route("/", [](){

        return "Welcome to Encoder server. Please see documentation for API description.";

    });

    // register route that sends
    httpServer.route("/send_code", QHttpServerRequest::Method::Post, [otpDriver]
                     (const QHttpServerRequest &request){

        if(otpDriver == nullptr)
            return QHttpServerResponse("Internal error: null", QHttpServerResponder::StatusCode::InternalServerError);

        // getting json body from the request
        const auto jsonBodyDoc = QJsonDocument::fromJson(request.body());

        // checking if jsonBodyDoc is appropriate json document
        if(!jsonBodyDoc.isObject()) {

            return QHttpServerResponse("Invalid json");

        }

        const QJsonObject jsonObj = jsonBodyDoc.object();

        // getiing values from json body
        const QString email = jsonObj.value("login").toString();

        otpDriver -> sendMessage(email);

        // all good. Return json with OTP code
        QJsonObject otpJson;
        otpJson["code"] = otpDriver -> getCode();
        return QHttpServerResponse("application/json", QJsonDocument(otpJson).toJson(QJsonDocument::Compact), QHttpServerResponder::StatusCode::Ok);

    });

    // checking existence of the user route
    httpServer.route("/login", QHttpServerRequest::Method::Post,
                     [databaseDriver](const QHttpServerRequest &request) {

        // getting json body from the request
        const auto jsonBodyDoc = QJsonDocument::fromJson(request.body());

        // checking if jsonBodyDoc is appropriate json document
        if(!jsonBodyDoc.isObject()) {

            return QHttpServerResponse("Invalid json");

        }

        const QJsonObject jsonObj = jsonBodyDoc.object();

        // getiing values from json body
        const QString loginToCheck = jsonObj.value("login").toString();
        const QString passwordToCheck = jsonObj.value("password").toString();
        if(databaseDriver == nullptr) {

            return QHttpServerResponse("Internal error: null", QHttpServerResponder::StatusCode::Forbidden);

        }

        // health check of database
        if(!databaseDriver -> connectionOk) {

            return QHttpServerResponse("Inernal error: lost DB connection", QHttpServerResponder::StatusCode::Forbidden);

        }

        // checking if this user exists in database
        if (!databaseDriver -> userExists(loginToCheck, passwordToCheck)) {

            return QHttpServerResponse("Invalid user", QHttpServerResponder::StatusCode::Forbidden);

        }

        // authorized succesfully: return "Valid"
        return QHttpServerResponse("Valid", QHttpServerResponder::StatusCode::Ok);

    });

    // adding new user to database route
    httpServer.route("/new_user", QHttpServerRequest::Method::Post, [databaseDriver](const QHttpServerRequest &request){

        const auto jsonBodyDoc = QJsonDocument::fromJson(request.body());

        if(!jsonBodyDoc.isObject()) {

            return QHttpServerResponse("Invalid", QHttpServerResponder::StatusCode::BadRequest);

        }

        const QJsonObject jsonObj = jsonBodyDoc.object();

        // getting valuse from josn object:
        const QString login    = jsonObj.value("login").toString();
        const QString password = jsonObj.value("password").toString();
        const QString role     = jsonObj.value("role").toString();

        if(databaseDriver == nullptr)
            return QHttpServerResponse("Internal error: nullptr", QHttpServerResponder::StatusCode::InternalServerError);

        if(!databaseDriver -> connectionOk)
            return QHttpServerResponse("Internal error: lost DB connection");

        databaseDriver -> addNewUser(login, password, role);

        return QHttpServerResponse("Success", QHttpServerResponder::StatusCode::Ok);

    });

    // binding QTcpServer to QHttpServer
    auto tcpServer = std::make_unique<QTcpServer>();

    if(!tcpServer -> listen(QHostAddress::Any, 8080)) {

        qDebug() << "Failed to start a server";
        return -1;

    }

    httpServer.bind(tcpServer.get());

    qDebug() << "Server running";

    return a.exec();
}

/**
 * @brief initDatabase
 * @param configManager
 * @param parent
 * @return databaseDriver
 */
std::shared_ptr<DatabaseDriver> initDatabase(ConfigManager *configManager, QObject &parent) {

    // establish a database connection
    std::shared_ptr<DatabaseDriver> databaseDriver = std::make_shared<DatabaseDriver>(
        configManager -> getDatabaseName(),
        configManager -> getDatabaseHostname(),
        configManager -> getDatabaseUsername(),
        configManager -> getDatabasePassword(),
        &parent);

    databaseDriver -> setConnection();
    qDebug() << "db connection status: " << databaseDriver -> connectionOk;

    return databaseDriver;

}

/**
 * @brief initOtp
 * @param configManager
 * @param parent
 * @return otpDriver
 */
std::shared_ptr<OTP> initOtp(ConfigManager *configManager, QObject &parent) {

    std::shared_ptr<OTP> otpDriver = std::make_shared<OTP>(&parent, configManager);

    return otpDriver;

}
