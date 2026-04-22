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

void initDatabase(DatabaseDriver *databaseDriver, ConfigManager *configManager, QObject &parent);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "Starting application";

    // read config file
    ConfigManager *configManager = new ConfigManager("config.ini", &a);

    // init database driver with settings from configManager
    DatabaseDriver *databaseDriver = nullptr;
    initDatabase(databaseDriver, configManager, a);

    // OTP *otpManager = new OTP(&a);

    // setup HTTP server
    QHttpServer httpServer;

    // adding routes
    // returning hello statement
    httpServer.route("/", [](){

        return "Welcome to Encoder server. Please see documentation for API description.";

    });

    // register new user route
    httpServer.route("/users", QHttpServerRequest::Method::Post, [](){

        return "Yp";

    });

    // checking existence of the user route
    httpServer.route("/login", QHttpServerRequest::Method::Post,
                     [&databaseDriver](const QHttpServerRequest &request) {

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


        /*return "Yo";*/

    });

    // binding QTcpServer to QHttpServer
    auto tcpServer = std::make_unique<QTcpServer>();

    if(!tcpServer -> listen(QHostAddress::Any, 8080) || !httpServer.bind(tcpServer.get())) {

        qDebug() << "Failed to start a server";
        return -1;

    }

    qDebug() << "Server running";

    return a.exec();
}

/**
 * @brief initDatabase
 * @param databaseDriver
 * @param configManager
 * @param parent
 */
void initDatabase(DatabaseDriver *databaseDriver, ConfigManager *configManager, QObject &parent) {

    // establish a database connection
    databaseDriver = new DatabaseDriver(
        configManager -> getDatabaseName(),
        configManager -> getDatabaseHostname(),
        configManager -> getDatabaseUsername(),
        configManager -> getDatabasePassword(),
        &parent);

    databaseDriver -> setConnection();
    qDebug() << "db connection status: " << databaseDriver -> connectionOk;

}

void startTcpServer() {


}
