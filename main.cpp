#include <QCoreApplication>
#include <QCommandLineParser>
#include <QtConcurrentRun>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHttpServer>
#include <QHttpServerResponse>
#include <QTcpServer>

#include "configmanager.h"
#include "dbdriver.h"
#include "otp.h"
#include "rsa.h"
#include "grasshopper.h"
#include "stribog.h"

#include <memory>

// declaring function prototypes
std::shared_ptr<DatabaseDriver> initDatabase(ConfigManager *configManager, QObject &parent);
std::shared_ptr<OTP>            initOtp     (ConfigManager *configManager, QObject &parent);

std::shared_ptr<Grasshopper> initGrasshopper();
std::shared_ptr<Stribog>     initStribog();
std::shared_ptr<RSA>         initRsa();

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

    // init crypto algorithms:
    std::shared_ptr<Grasshopper> grasshopper = initGrasshopper();
    std::shared_ptr<Stribog>     stribog     = initStribog();
    std::shared_ptr<RSA>         rsa         = initRsa();

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

    // encrypting a message with Grasshopper algorithm
    // this method gets a raw message and a key and returns encrypted message
    httpServer.route("/grasshopper_encrypt", QHttpServerRequest::Method::Post, [grasshopper](const QHttpServerRequest &request) {

        const auto jsonBodyDoc = QJsonDocument::fromJson(request.body());

        if(!jsonBodyDoc.isObject())
            return QHttpServerResponse("Invalid", QHttpServerResponder::StatusCode::BadRequest);

        const QJsonObject jsonObj = jsonBodyDoc.object();

        // getting valuse from josn object:
        const QString message    = jsonObj.value("message").toString();
        // const QString key        = jsonObj.value("key").toString();

        // making a message vector out of message from json
        std::vector<char> msgVec = {0};

        for(QChar i : message) {

            msgVec.push_back(i.toLatin1());

        }

        std::vector<char> encrypted = grasshopper -> grasshopperEncrypt(msgVec);

        // making QString to return
        QString result = "";
        for(char i : encrypted) {

            result.append(QChar(i));

        }
        // making json to return
        QJsonObject jsonGrasshopperEncrypted;
        jsonGrasshopperEncrypted["encrypted"] = result;

        return QHttpServerResponse("application/json", QJsonDocument(jsonGrasshopperEncrypted).toJson(QJsonDocument::Compact),
                                   QHttpServerResponder::StatusCode::Ok);

    });

    // encrypting a message with RSA-8 algorithm
    // this method gets a raw message and a key and returns encrypted message
    // we run this asynchronously. So we need to set the retrun type as
    // QFuture<QHttpServerResponse>
    httpServer.route("/rsa_encrypt", QHttpServerRequest::Method::Post, [rsa](const QHttpServerRequest &request) -> QFuture<QHttpServerResponse> {

        return QtConcurrent::run([=](){

            const auto jsonBodyDoc = QJsonDocument::fromJson(request.body());

            if(!jsonBodyDoc.isObject())
                return QHttpServerResponse("Invalid", QHttpServerResponder::StatusCode::BadRequest);

            const QJsonObject jsonObj = jsonBodyDoc.object();

            // getting message to encrypt from josn object:
            const QString message          = jsonObj.value("message").toString();
            const std::string msgStdStr    = message.toStdString();

            // getting key from json object:
            // const QString key              = jsonObj.value("key").toString();
            // const std::string keyStdStr    = key.toStdString();

            std::vector<int> encrypted = rsa -> encrypt(msgStdStr);

            // making result string from the encrypted vector
            QString result = "";

            for (int i : encrypted) {

                result.append(QString::number(i));

            }

            // making json to return
            QJsonObject jsonRsaEncrypted;
            jsonRsaEncrypted["encrypted"] = result;

            return QHttpServerResponse("application/json", QJsonDocument(jsonRsaEncrypted).toJson(QJsonDocument::Compact),
                                       QHttpServerResponder::StatusCode::Ok);

        });

    });

    // encrypting a message with Grasshopper route
    // this method gets a raw message and a key and returns encrypted message
    httpServer.route("/stribog_encrypt", QHttpServerRequest::Method::Post, [stribog](const QHttpServerRequest &request) {

        const auto jsonBodyDoc = QJsonDocument::fromJson(request.body());

        if(!jsonBodyDoc.isObject())
            return QHttpServerResponse("Invalid", QHttpServerResponder::StatusCode::BadRequest);

        const QJsonObject jsonObj = jsonBodyDoc.object();

        // getting valuse from josn object:
        const QString message    = jsonObj.value("message").toString();
        const std::string msg    = message.toStdString();

        // getting hash:
        std::vector<uint8_t> resVect = stribog -> getHash(stribog->hex_to_vec8(msg));

        // converting result to QString:
        QString resString = "";

        for(uint8_t val : resVect) {

            resString.append(QString::number(val));

        }

        // making json object to response with

        QJsonObject jsonObjHash;
        jsonObjHash["hash"] = resString;

        return QHttpServerResponse("application/json", QJsonDocument(jsonObjHash).toJson(QJsonDocument::Compact), QHttpServerResponder::StatusCode::Ok);

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

/**
 * @brief initGrasshopper
 * @return
 */
std::shared_ptr<Grasshopper> initGrasshopper() {

    return std::make_shared<Grasshopper>();

}

/**
 * @brief initStribog
 * @return
 */
std::shared_ptr<Stribog> initStribog() {

    return std::make_shared<Stribog>();

}

/**
 * @brief initRsa
 * @return
 */
std::shared_ptr<RSA> initRsa() {

    return std::make_shared<RSA>();

}
