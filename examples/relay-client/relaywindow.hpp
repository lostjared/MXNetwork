#ifndef RELAYWINDOW_HPP
#define RELAYWINDOW_HPP

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>
#include "mxnetwork/socket.hpp"

class RelayWindow : public QWidget {
    Q_OBJECT
public:
    explicit RelayWindow(QWidget *parent = nullptr);
    bool makeConnection(std::string_view user_name, std::string_view ip, std::string_view port);
    ~RelayWindow();
signals:
    void messageSent(const QString &message);
private slots:
    void sendMessage();
    void readData();
private:
    QWidget *containerWidget;
    QVBoxLayout *layout;
    QTextEdit *messages;
    QLineEdit *inputField;
    QPushButton *sendButton;
    std::string user_name, ip, port;
    mxnetwork::Socket sockfd{mxnetwork::SocketType::TYPE_INET};
    QTimer *rtimer;
};

#endif
