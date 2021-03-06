#include "center.h"

Center::Center(QObject *parent) : QObject(parent) {

}

Center::~Center() {

}

void Center::init() {
    login_page_ = new LoginPage();
    main_page_ = new MainPage();
    login_page_->init();
    connect(login_page_, &LoginPage::login, this, &Center::requestLogin);
    connect(login_page_, &LoginPage::logined, this, &Center::dealWithLogined);
    connect(login_page_, &LoginPage::localCmd, this, &Center::dealWithLocalCmd);
    connect(main_page_, &MainPage::localCmd, this, &Center::dealWithLocalCmd);
    connect(main_page_, &MainPage::openUserChatPage, this, &Center::dealWithOpenUserChatPage);
    change_theme_page_ = new ChangeThemePage();
    change_theme_page_->init();
    connect(change_theme_page_, &ChangeThemePage::changed, this, &Center::changeTheme);

    about_page_ = new AboutPage();
    about_page_->init();

    profile_page_ = new ProfilePage();
    profile_page_->init();

    request_step_timer_ = new QTimer(this);
    connect(request_step_timer_, &QTimer::timeout, this, &Center::requestStep);
    add_user_page_ = new AddUserPage();
    add_user_page_->init();
    connect(add_user_page_, &AddUserPage::addUser, this, &Center::requestAddUser);

    // 连接服务器
    http_.connect(SERVER_DOMAIN);
    connect(&http_, &Http::finished, this, &Center::requestReply);
    http_.setRawHeader("Accept", "application/json");
}

void Center::test() {

}

void Center::testRecv() {
    qDebug() << "recv::: " << http_.data();
}


void Center::requestAddUser(QString target_account, QString target_uid, QString verify_message) {
    QJsonDocument json_document;
    QJsonObject json_object;
    json_object.insert("platform", "linux");
    json_object.insert("client_version", "linux 0.1");
    json_object.insert("datetime", QDateTime::currentDateTime().toString("yy-MM-dd dd:mm:ss"));
    json_object.insert("content_type", "add_info");
    json_object.insert("token", user_.token.toUtf8().data());
    json_object.insert("uid", user_.uid.toUtf8().data());
    QJsonObject json_object_content;
    json_object_content.insert("target_account", target_account.toUtf8().data());
    json_object_content.insert("target_uid", target_uid.toUtf8().data());
    json_object_content.insert("account", user_.account.toUtf8().data());
    json_object_content.insert("verify_message", verify_message.toUtf8().data());
    json_object.insert("content", json_object_content);
    json_document.setObject(json_object);
    QByteArray byte_array = json_document.toJson(QJsonDocument::Compact);
    //qDebug() << byte_array;
    QString url = "/?request=add_user&platform=linux";

    http_.post(url, byte_array);
}

void Center::start() {
    login_page_->show();
}

void Center::requestLogin(QString account, QString password) {
    //mannager->post()
    QJsonDocument json_document;
    QJsonObject json_object;
    json_object.insert("platform", "linux");
    json_object.insert("client_version", "linux 0.1");
    json_object.insert("datetime", QDateTime::currentDateTime().toString("yy-MM-dd dd:mm:ss"));
    json_object.insert("content_type", "login_info");
    QJsonObject json_object_content;
    json_object_content.insert("account", account.toUtf8().data());
    json_object_content.insert("password", password.toUtf8().toBase64().data());
    json_object.insert("content", json_object_content);
    json_document.setObject(json_object);
    QByteArray byte_array = json_document.toJson(QJsonDocument::Compact);
    //qDebug() << byte_array;
    QString url;
    url = "/?request=login&platform=linux";
    http_.setRawHeader("Content-Type", "application/json");
    http_.post(url, byte_array);
}

void Center::requestReply() {
    qDebug() << "my_http recv: " << http_.data();
    QJsonDocument json_document;
    QJsonObject json_object;
    QJsonParseError json_error;
    do {
        if(http_.rawHeader("Content-Type") == "application/json") {
            recved_data_ = http_.data();
            //qDebug() << recv_data;
            json_document = QJsonDocument::fromJson(recved_data_, &json_error);
            if(json_error.error != QJsonParseError::NoError) {
                qDebug() << "json 文件解析失败";
                break;
            }
        }else {
            qDebug() << "接收类型不对";
            break;
        }
        //qDebug() << "recv: " << recved_data_;
        json_object = json_document.object();
        QJsonValue json_value_request = json_object.value("request");
        if(!json_value_request.isString()) {
            qDebug() << "Json类型不对";
            break;
        }
        QString request = json_value_request.toString();
        if(request == "keep_connect") {
            dealWithKeepConnectReply();
        } else if(request == "login") {
            login_page_->dealWithRecv(json_object);
            break;
        } else if(request == "get_user_info") {
            qDebug() << "get_user_info: " << recved_data_;
            handleGetUserInfoReply(json_object);
            break;
        } else if(request == "add_user") {
            //qDebug() << "add_user: " << recved_data_;
            handleAddUser(json_object);
            break;
        } else if(request == "get_friend_list") {
            qDebug() << "get_friend_list: " << recved_data_;
            handleGetFriendListReply(json_object);
            break;
        } else {
            qDebug() << "收到数据出错: " << recved_data_;
            break;
        }
    } while(false);
}
void Center::dealWithKeepConnectReply() {
    request_step_ = static_cast<RequestStep>(static_cast<int>(request_step_) + 1);
    qDebug() << "keep";

}
void Center::dealWithLogined(QString account, QString uid, QString token) {
    user_.account = account;
    user_.uid = uid;
    user_.token = token;
    qDebug() << account << " uid: " << uid << "token: " << token;
    login_page_->close();
    main_page_->init();
    main_page_->show();
    generateUserPath(); //目录生成
    requestStep();
    request_step_timer_->start(3000);
}

void Center::dealWithLocalCmd(LocalCmd cmd) {
    switch (cmd) {
    case LocalCmd::exit: {
        this->exit();
    } break;
    case LocalCmd::logout: {

    } break;
    case LocalCmd::show_chnage_theme_page: {
        change_theme_page_->show();
    } break;
    case LocalCmd::show_main_page: {
        main_page_->showNormal();
        main_page_->show();
    } break;
    case LocalCmd::show_about_page: {
        about_page_->show();
    } break;
    case LocalCmd::show_profile_page : {
        profile_page_->showNormal();
        profile_page_->show();
    } break;
    case LocalCmd::show_add_user_page: {
        add_user_page_->showNormal();
        add_user_page_->show();
    }
    default: {

    } break;
    }
}

void Center::exit() {
    login_page_->close();
    main_page_->close();
    change_theme_page_->close();
    about_page_->close();
    profile_page_->close();
    add_user_page_->close();

    delete main_page_;
    delete login_page_;
    delete change_theme_page_;
    delete about_page_;
    delete profile_page_;
    delete add_user_page_;
}

void Center::keepConnect() {
    QString url = "/?request=keep_connect&platform=linux";
    http_.get(url);
}

void Center::requestGetUserInfo() {
    QString url;
    url = "/?request=get_user_info&platform=linux&account=" + user_.account + "&uid=" + user_.uid + "&token=" + user_.token;
    http_.get(url);
    qDebug() << "request user info\n";
}

void Center::requestGetFriendList() {
    QString url = "/?request=get_friend_list&platform=linux&account=" + user_.account + "&uid=" + user_.uid + "&token=" + user_.token;
    http_.get(url);
    qDebug() << "request friend list\n";
}
void Center::handleGetFriendListReply(const QJsonObject &json_obj) {
    request_step_ = static_cast<RequestStep>(static_cast<int>(request_step_) + 1);
    int code = json_obj.value("code").toInt();
    if(code != 0) {
        return;
    }
    QJsonObject friend_list_json = json_obj.value("content").toObject();
    for(auto friend_list_item_json_iter = friend_list_json.begin(); friend_list_item_json_iter != friend_list_json.end(); ++friend_list_item_json_iter) {
        QJsonObject friend_list_item = friend_list_item_json_iter.value().toObject();
        UserInfo info;
        info.uid = friend_list_item["uid"].toString();
        info.account = friend_list_item["account"].toString();
        info.email = friend_list_item["email"].toString();
        info.email = friend_list_item["address"].toString();
        info.network_state = friend_list_item["network_state"].toString();
        info.nickname = friend_list_item["nickname"].toString();
        info.head_image = friend_list_item["head_image"].toString();
        info.signature = friend_list_item["signature"].toString();
        friend_list_[info.uid] = info;
        createUserChatPage(info); // crate user chat page
    }

    for(auto info : friend_list_)
        main_page_->addUserListItem(info);

}
void Center::requestStep() {
    switch (request_step_) {
    case RequestStep::keep_connect: {
        keepConnect();
    } break;
    case RequestStep::getUserInfo: {
        if(is_request_info_) {
            requestGetUserInfo();
        }
        else
            request_step_ = RequestStep::keep_connect;
    } break;
    case RequestStep::getFriendList: {
        if(is_request_info_)
            requestGetFriendList();
        else
            request_step_ = RequestStep::keep_connect;
    } break;
    default: {
        request_step_ = RequestStep::keep_connect;
        is_request_info_ = false;
    }
        break;
    }
}

void Center::requestGetGroupList() {

}

void Center::handleGetUserInfoReply(const QJsonObject &json_obj) {
    request_step_ = static_cast<RequestStep>(static_cast<int>(request_step_) + 1);
    int code = json_obj.value("code").toInt();
    QJsonObject content;
    if(code != 0) {
        return;
    }
    if(json_obj.value("content").isObject())
        content = json_obj.value("content").toObject();
    user_.account = content.value("account").toString();
    user_.email = content.value("email").toString();
    user_.nickname = content.value("nickname").toString();
    user_.name = content.value("name").toString();
    user_.phone_number = content.value("phone_number").toString();
    user_.address = content.value("address").toString();
    user_.creation_time = content.value("created_time").toString();
    user_.ocupation = content.value("occupation").toString();
    user_.network_state = content.value("network_state").toString();
    user_.last_login = content.value("last_login").toString();
    user_.head_image = content.value("head_image").toString();
    user_.profile_image_1 = content.value("profile_image_1").toString();
    user_.profile_image_2 = content.value("profile_image_2").toString();
    user_.profile_image_3 = content.value("profile_image_3").toString();
    user_.profile_image_4 = content.value("profile_image_4").toString();
    // 设置信息
    main_page_->setUserInfo(user_);
    profile_page_->setUserInfo(user_);
    //qDebug() << "email" << user_.creation_time ;//json_obj;
}

void Center::handleAddUser(const QJsonObject &json_obj) {
    qDebug() << "OKKK";
}

void Center::generateUserPath() {
    QString user_path =DATA_PATH + ('/' + user_.account);
    QDir dir;
    dir.cd(dir.homePath());
    if(!dir.exists(user_path)) {
        dir.mkpath(user_path);
    }
}

void Center::changeTheme(QString theme) {
    theme_ = theme;
    change_theme_page_->setTheme(theme);
    login_page_->setTheme(theme);
    main_page_->setTheme(theme);
    profile_page_->setTheme(theme);
    about_page_->setTheme(theme);
    add_user_page_->setTheme(theme);
}
void Center::createUserChatPage(const UserInfo &info) {
    auto iter = friend_window_list_.find(info.uid);
    if(iter != friend_window_list_.end()){ //存在, 更新信息
        UserChatPage *user = *iter;
        user->setInfo(info);
        return;
    }
    UserChatPage *user = new UserChatPage();
    user->init();
    connect(user, &UserChatPage::sendMessage, this, &Center::requestSendMessage);
    friend_window_list_[info.uid] = user;
}

void Center::requestSendMessage(QString message) {

}

void Center::dealWithOpenUserChatPage(QString uid) {
    qDebug() << "open chat page: " << uid;
    auto iter = friend_window_list_.find(uid);
    if(iter != friend_window_list_.end()){ //存在, 更新信息
        UserChatPage *p = *iter;
        p->show();
    }
}
