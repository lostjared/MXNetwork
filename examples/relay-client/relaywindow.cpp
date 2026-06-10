#include "relaywindow.hpp"
#include<poll.h>

RelayWindow::RelayWindow(QWidget *parent) : QWidget(parent) {
    containerWidget = new QWidget(this);
    layout = new QVBoxLayout(containerWidget);

    messages = new QTextEdit(this);
    inputField = new QLineEdit(this);
    sendButton = new QPushButton("Send", this);

    layout->addWidget(messages);
    layout->addWidget(inputField);
    layout->addWidget(sendButton);

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->addWidget(containerWidget);

    connect(inputField, &QLineEdit::returnPressed, this, &RelayWindow::sendMessage);
    connect(sendButton, &QPushButton::clicked, this, &RelayWindow::sendMessage);
    setGeometry(100, 100, 640, 480);
}

RelayWindow::~RelayWindow() {

}

bool RelayWindow::makeConnection(std::string_view cuser_name, std::string_view cip, std::string_view cport) {
    if(!sockfd.connect(std::string(cip), std::string(cport))) {
        return false;
    }
    sockfd.setblocking(false);
    ip = std::string(cip);
    port = std::string(cport);
    user_name = cuser_name;
    messages->append("<span style='color: 0000FF;'>connected.</span>");
    rtimer = new QTimer(this);
    rtimer->setInterval(300);
    connect(rtimer, &QTimer::timeout, this, &RelayWindow::readData);
    rtimer->start();
    return true;
}

static constexpr size_t BUFFER_SIZE = 1024 * 8;

void RelayWindow::readData() {

    pollfd pfd{};
    pfd.fd = sockfd.sockfd();
    pfd.events = POLLIN;


    int value = poll(&pfd, 1, 100);
    if (value <= 0)
        return;

    char buffer[BUFFER_SIZE] = {};
    ssize_t bytes = sockfd.read(buffer, BUFFER_SIZE - 1, 0);

    if (bytes < 0) {
        if (errno != EAGAIN || errno != EWOULDBLOCK) {
                sockfd.close();
                std::cout << "closing connection.\n";
                exit(0);
        }
    }
    buffer[bytes] = '\0';
    messages->append("<span style='color: #FF0000;'>" + QString(buffer) + "</span>");
}

void RelayWindow::sendMessage() {
    QString message = inputField->text();
    if (!message.isEmpty()) {
        QString final_message = QString(user_name.c_str()) + ": " + message;
        messages->append("<span style='color: #00FF00;'>" + final_message + "</span>");
        sockfd.write_all(final_message.toStdString().c_str(), final_message.toStdString().length());
        emit messageSent(final_message);
        inputField->clear();
    }
}
