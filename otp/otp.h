#ifndef OTP_H
#define OTP_H

#include <QDebug>
#include <QFile>
#include <QRandomGenerator>
#include <QString>

#include "configmanager.h"
#include "otp_labels.h"


#include "../external/simple-mail/src/SimpleMail"

class OTP : public QObject {

public:

    explicit OTP(QObject *parent = nullptr, ConfigManager *configman = nullptr);
    ~OTP();

    void sendMessage(QString recepientEmail);

    QString generateCode();

    void deleteCode();

    bool otpCodeAccepted(QString userEnteredCode);

    QString getCode() const;

private:

    QString host, sender, password;
    int port;

    ConfigManager *configman = nullptr;

    SimpleMail::Server *server = nullptr;
    std::pair<QString, QString> smtpAuthData;

    QString otpCode; // 4-digit temporary code

};

#endif // OTP_H
