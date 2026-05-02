#include "otp.h"

/**
 * @brief OTP::OTP
 */
OTP::OTP(QObject *parent, ConfigManager *configman) : QObject(parent), otpCode("") {

    this -> configman = configman;

}

/**
 * @brief OTP::~OTP
 */
OTP::~OTP() {

    delete server;

}

/**
 * @brief OTP::sendMessage
 * @param recepientEmail
 * @param code
 */
void OTP::sendMessage(QString recepientEmail) {

    // creating server object
    server = new SimpleMail::Server;

    // generating temporary code for two-step authentication
    QString codeToSend = generateCode();
    this -> otpCode = codeToSend;

    // compiling a message to send:
    QString messageToSend = OTP_CODE_MESSAGE + codeToSend;

    // declaring a string with subject
    QString subject = OTP_MESSAGE_SUBJECT;

    qDebug() << "sending email called";

    // using gmail smtp server

    qDebug() << configman -> getSmtpHost();

    server -> setHost((configman -> getSmtpHost().toLatin1()));
    server -> setPort(configman -> getSmtpPort());
    server -> setConnectionType(SimpleMail::Server::SslConnection);

    // getting the username and password
    server -> setUsername(configman -> getSmtpSender().toLatin1());
    server -> setPassword(configman -> getSmtpPassword().toLatin1());

    // creating MimeMessage object
    SimpleMail::MimeMessage message;
    message.setSender(SimpleMail::EmailAddress(configman -> getSmtpSender().toLatin1(),
                                               QLatin1String("ENCODER")));

    message.addTo(SimpleMail::EmailAddress(recepientEmail.toLatin1()));
    message.setSubject(subject.toLatin1());

    // creating MimeText object
    auto text = std::make_shared<SimpleMail::MimeText>();

    text -> setText(messageToSend.toLatin1()/*QLatin1String("Hi!\n This is your message from Encoder. \n")*/);

    message.addPart(text);

    // sending an email:
    SimpleMail::ServerReply *reply = server -> sendMail(message);

    QObject::connect(reply, &SimpleMail::ServerReply::finished,
                     [reply] {qDebug() << "Server reply finished" << reply -> error() << reply -> responseText();
                              reply -> deleteLater();
                              });


}

/**
 * @brief OTP::generateCode
 * @return generated 4-digits code
 */
QString OTP::generateCode() {

    int code = QRandomGenerator::global() -> bounded(1000, 10000);
    return QString::number(code);

}

/**
 * @brief OTP::deleteCode - function that deletes code
 * @return
 */
void OTP::deleteCode() {

    this -> otpCode = "";

}

/**
 * @brief OTP::otpCodeAccepted
 * @param userEnteredCode
 * @param actualCode
 * @return
 */
bool OTP::otpCodeAccepted(QString userEnteredCode) {

    if (this -> otpCode == userEnteredCode)
        return true;

    return false;

}

QString OTP::getCode() const {

    return this -> otpCode;

}
