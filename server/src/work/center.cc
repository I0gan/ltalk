#include "center.hh"

extern MYSQL Database::global_mysql;

std::unordered_map<std::string, Data::User> Data::map_user;
std::unordered_map<std::string, Data::Group> Data::map_group;
std::string Data::web_root;
std::string Data::web_page;
std::string Data::web_404_page;

Work::Center::Center(const std::map<std::string, std::string> &map_header_info, std::string &content, std::string &http_uid, std::string &http_platform) :
    map_header_info_(map_header_info),
    content_(content),
    send_file_handler_(nullptr),
    send_data_handler_(nullptr),
    http_uid_(http_uid),
    http_platform_(http_platform) {

}

Work::Center::~Center() {

}

void Work::Center::set_fd(int fd) {
    fd_ = fd;
}
void Work::Center::set_send_file_handler(Util::CallBack1 send_file_handler) {
    send_file_handler_ = send_file_handler;
}

void Work::Center::set_send_data_handler(Util::CallBack2 send_data_handler) {
    send_data_handler_ = send_data_handler;
}

void Work::Center::set_http(const ::Net::SPHttp &http) {
    wp_http_ = http;
}

void Work::Center::Process() {
    std::string http_method;
    try {
        http_method = map_header_info_.at("method");
    } catch (std::out_of_range e) { std::cout << "no method\n"; }

    if(ParseUrl() == false) {
        Response(ResponseCode::ERROR_PARSING_URL);
        return;
    }
    try {
        platform_ = map_url_value_info_.at("platform");
        request_ = map_url_value_info_.at("request");
    } catch (std::out_of_range e) {}

    //std::cout << "method: "<< http_method << " url:[" << map_url_info_["url"] << "]\n";
    if(http_method == "get") {
        HandleGet();
    }else if(http_method == "put") {

    }else if(http_method == "post") {
        HandlePost();
    }
}

void Work::Center::HandleGet() {
    bool error = false;
    std::string path = map_url_info_["path"];
    std::string web_path = Data::web_root;
    RequestType request_type = Request::toEnum(request_);
    bool web_page_get = false;
    do {
        if(path == "/" && platform_.empty()) {
            wp_http_.lock()->Redirect("/main/");
            break;
        }
        switch (request_type) {
        case RequestType::register_page: {
            web_path += "/register/";
            web_page_get = true;
        } break;
        case RequestType::register_success_page: {
            web_path += "/register/success/";
            web_page_get = true;
        } break;
        case RequestType::main_page: {
            web_path += "/main/";
            web_page_get = true;
        } break;
        case RequestType::keep_connect: {
            DealWithKeepConnect();
        }break;
        case RequestType::get_user_info: {
            DealWithGetUserInfo();
        } break;
        case RequestType::get_public_file: {
            DealWithGetUserPublicFile();
        } break;
        case RequestType::get_private_file: {
            DealWithGetUserPrivateFile();
        } break;
        case  RequestType::get_profile_image: {
            DealWithGetProfileImage();
        } break;
        case  RequestType::search_user: {
            DealWithSearchUser();
        } break;
        case RequestType::get_friend_list: {
            DealWithGetFriendList();
        } break;
        default: {
            web_path += path;
            web_page_get = true;
        } break;
        }
    } while(false);
    // Send get file
    if(error) {
        std::cout << "error\n";
        HandleNotFound();
    }else {
        if(web_page_get) {
            if(IsDir(web_path)) {
                web_path += "index.html";
            }
            SendFile(web_path);
        }
    }
}

void Work::Center::HandlePost() {
    RequestType request_type = Request::toEnum(request_);
    //std::cout << "post: " << request_ << '\n';
    switch (request_type) {
    case RequestType::register_page: {
        if(platform_ == "web")
            DealWithRegisterUser();
    } break;
    case RequestType::login: {
        DealWithLogin();
    } break;
    case RequestType::upload_profile_image: {
        DealWithUploadProfileImage();
    } break;
    case RequestType::add_user: {
        DealWithAddUser();
    } break;
    default: {
        Response(ResponseCode::NO_ACCESS);
    } break;
    }
}

bool Work::Center::IsDir(const std::string &path) {
    if(path.size() < 1)
        return false;
    else if(path.at(path.size() - 1) == '/')
        return true;
    return false;
}

bool Work::Center::ParseUrl() {
    std::string url;
    std::string value_url;
    std::string path;
    try {
        url = map_header_info_.at("url");
    } catch (std::out_of_range e) {
        std::cout << "map_header_info_[url]" << e.what() << '\n';
        return false;
    }

    map_url_info_["url_orignal"] = url;
    url = Crypto::Url::Decode(url);
    map_url_info_["url"] = url;
    int first_value_pos = url.find("?");
    if(first_value_pos > 0) {
        value_url = url.substr(first_value_pos + 1);
        path = url.substr(0, first_value_pos);
        if(path != "/" && path.size() > 0) {
            path.pop_back();
        }
    }else {
        path = url;
    }
    map_url_info_["path"] = path;
    // check is have value
    if(value_url.empty()) return true;
    value_url += '&';
    // Get value
    while(true) {
        int key_pos = value_url.find("&");
        if(key_pos < 0) {
            break;
        }
        std::string get_one = value_url.substr(0, key_pos);
        value_url = value_url.substr(key_pos + 1);
        if(get_one.empty()) continue;

        int value_pos = get_one.find('=');
        if(value_pos < 0) continue;
        std::string key = get_one.substr(0, value_pos);
        std::string value = get_one.substr(value_pos + 1);
        if(key.empty() || value.empty()) continue;
        map_url_value_info_[key] = value;
        //std::cout << "get_one: [" << get_one << "] key: [" << key << "] value: [" << map_url_value_info_[key] << "]\n";
    }
    return true;
}

void Work::Center::SendFile(std::string file_name) {
    if(send_file_handler_)
        send_file_handler_(file_name);
}

void Work::Center::SendData(const std::string &suffix, const std::string &content) {
    if(send_data_handler_)
        send_data_handler_(suffix, content);
}

void Work::Center::HandleNotFound() {
    SendFile(Data::web_404_page);
}

void Work::Center::Response(ResponseCode code) {
    Json json_obj = {
        { "server", SERVER_NAME },
        { "request", request_ },
        { "code", code },
        { "datetime" , GetDateTime() }
    };
    SendJson(json_obj);
}

void Work::Center::SendJson(Json &json_obj) {
    std::ostringstream json_sstream;
    json_sstream << json_obj;
    std::string data = json_sstream.str();
    SendData(".json", data);
}

std::string Work::Center::GetDateTime() {
    char time_str[128] = {0};
    struct timeval tv;
    time_t time;
    gettimeofday(&tv, nullptr);
    time = tv.tv_sec;
    struct tm *p_time = localtime(&time);
    strftime(time_str, 128, "%Y-%m-%d %H:%M:%S", p_time);
    return std::string(time_str);
}

bool Work::Center::CheckJsonContentType(Json &recv_json_obj, const std::string &type) {
    bool ret = false;
    std::string recv_type;
    try {
        recv_type = recv_json_obj.at("content_type");
    } catch (std::out_of_range e) { }

    if(recv_type == type) {
        ret = true;
    }
    return ret;

}

void Work::Center::DealWithKeepConnect() {
    Response(ResponseCode::SUCCESS);
}

void Work::Center::GenerateUserPath(const std::string &uid) {
    std::string base_path = "data/user/" + uid;
    mkdir(base_path.c_str(), S_IRWXU);
    mkdir((base_path + "/public").c_str(), S_IRWXU);
    mkdir((base_path + "/public/profile").c_str(), S_IRWXU);
    mkdir((base_path + "/public/files").c_str(), S_IRWXU);
    mkdir((base_path + "/private").c_str(), S_IRWXU);
    mkdir((base_path + "/private/chat").c_str(), S_IRWXU);
}

void Work::Center::DealWithRegisterUser() {
    std::string content_type;
    try {
        content_type = map_header_info_.at("content-type");
    } catch(std::out_of_range e) {
        std::cout << "no a content-type\n";
        Response(ResponseCode::ERROR_HTTP_CONTENT);
        return;
    }

    if(content_type != "application/json") {
        Response(ResponseCode::ERROR_HTTP_CONTENT);
        return;
    }

    Json recv_json_obj;
    try {
        recv_json_obj = Json::parse(content_);
    }  catch (Json::parse_error e) {
        Response(ResponseCode::ERROR_PARSING_CONTENT);
        return;
    }

    if(!CheckJsonContentType(recv_json_obj, "register_info")) {
        Response(ResponseCode::ERROR_JSON_CONTENT_TYPE);
        return;
    }

    std::string json_name;
    std::string json_email;
    std::string json_phone_number;
    std::string json_address;
    std::string json_occupation;
    std::string json_password;

    //std::cout << "json [" << recv_json_obj << "]\n";
    try {
        json_name = recv_json_obj["content"]["name"];
        json_email = recv_json_obj["content"]["email"];
        json_phone_number = recv_json_obj["content"]["phone_number"];
        json_address = recv_json_obj["content"]["address"];
        json_occupation = recv_json_obj["content"]["occupation"];
        json_password = recv_json_obj["content"]["password"];
    } catch (Json::type_error e) {
        //std::cout << "Register type_error: " << e.what() << '\n';
        Response(ResponseCode::ERROR_JSON_CONTENT);
        return;
    }

    // Filter char
    Database::MysqlQuery::Escape(json_name);
    Database::MysqlQuery::Escape(json_email);
    Database::MysqlQuery::Escape(json_phone_number);
    Database::MysqlQuery::Escape(json_address);
    Database::MysqlQuery::Escape(json_occupation);

    // check size
    bool is_valid = false;
    do {
        if(json_name.size() < 1 || json_name.size() >= 255)
            break;
        else if(json_email.size() < 4 || json_email.size() >= 255)
            break;
        else if(json_phone_number.size() < 4 || json_phone_number.size() >= 255)
            break;
        else if(json_occupation.size() >= 255)
            break;
        else if(json_password.size() < 6 || json_password.size() >= 255)
            break;
        else
            is_valid = true;
    } while(false);
    if(!is_valid) {
        Response(ResponseCode::ERROR_JSON_CONTENT);
        return;
    }

    // check is have this eamil
    Database::MysqlQuery sql_query;
    sql_query.Select("user_", "email", "email = '" + json_email + '\'');
    if(sql_query.Next()) {
        Response(ResponseCode::EXIST);
        return;
    }
    // check is have this phone number
    sql_query.Select("user_", "phone_number", "phone_number = '" + json_phone_number + '\'');
    if(sql_query.Next()) {
        Response(ResponseCode::EXIST);
        return;
    }

    std::string uid = MakeUid(json_email);
    std::string encode_password = Crypto::MD5(json_password).toString();

    // Insert into database
    std::string key_sql = "uid, account, email, nickname, name, signature, qq, phone_number,"
                          "address, hometown, occupation, created_time, network_state, last_login,"
                          "head_image, profile_image_1, profile_image_2, profile_image_3, profile_image_4, password";

    std::string value_sql = '\'' + uid + "',"; // set uid
    value_sql += '\'' + json_email + "',";     // account
    value_sql += '\'' + json_email + "',";     // email
    value_sql += '\'' + json_name + "',";      // nickname
    value_sql += '\'' + json_name + "',";      // name
    value_sql += "'unset',";        // signature
    value_sql += "'unset',";        // qq
    value_sql += '\'' + json_phone_number + "',"; // phone_number
    value_sql += '\'' + json_address + "',"; // address
    value_sql += "'unset',"; // hometown
    value_sql += '\'' + json_occupation + "',"; // occupation
    value_sql += '\'' + GetDateTime() + "',"; // created_time
    value_sql += "'offline',";        // network_state
    value_sql += "'unlogin',";        // last_login
    value_sql += "'unset',";          // head_image
    value_sql += "'unset',";          // profile_image_1
    value_sql += "'unset',";          // profile_image_2
    value_sql += "'unset',";          // profile_image_3
    value_sql += "'unset',";          // profile_image_4
    value_sql += '\'' + encode_password + "'"; // password

    if (!sql_query.Insert("user_", key_sql, value_sql)) {
        Response(ResponseCode::FAILURE);
        return;
    }
    Json send_json = {
        { "server", SERVER_NAME },
        { "code", ResponseCode::SUCCESS },
        { "datetime" , GetDateTime() },
        { "access_url", "/?request=register_success_page&platform=web"},
        { "uid" , uid},
        { "token", MakeToken(uid) }
    };

    GenerateUserPath(uid);
    SendJson(send_json);
}

std::string Work::Center::MakeToken(const std::string &uid) {
    time_t t = time(nullptr);
    return Crypto::MD5(std::to_string(t) + uid).toString();
}

std::string Work::Center::MakeUid(const std::string &str) {
    //    ::Database::MysqlQuery query;
    //    query.Select("user_", "uid", "uid=" )
    return Crypto::MD5(str).toString();
}

void Work::Center::DealWithRegisterGroup() {

}

void Work::Center::DealWithLogin() {
    std::string content_type;
    try {
        content_type = map_header_info_.at("content-type");
    } catch(std::out_of_range e) {
        std::cout << "no a content-type\n";
        Response(ResponseCode::ERROR_HTTP_CONTENT);
        return;
    }

    if(content_type != "application/json") {
        Response(ResponseCode::ERROR_HTTP_CONTENT);
        return;
    }

    Json recv_json_obj;
    try {
        recv_json_obj = Json::parse(content_);
    }  catch (Json::parse_error e) {
        Response(ResponseCode::ERROR_PARSING_CONTENT);
        return;
    }

    if(!CheckJsonContentType(recv_json_obj, "login_info")) {
        Response(ResponseCode::ERROR_JSON_CONTENT_TYPE);
        return;
    }

    std::string json_account;
    std::string json_password;
    std::string json_platform;
    try {
        json_account = recv_json_obj["content"]["account"];
        json_password = recv_json_obj["content"]["password"];
        json_platform = recv_json_obj["platform"];
    }  catch (Json::type_error) {
        Response(ResponseCode::ERROR_JSON_CONTENT_TYPE);
        return;
    }

    // filter
    Database::MysqlQuery::Escape(json_account);
    Database::MysqlQuery::Escape(json_platform);
    Database::MysqlQuery sql_query;

    sql_query.Select("user_", "account", "account = '" + json_account + '\'');

    if(!sql_query.Next()) {
        Response(ResponseCode::NOT_EXIST);
        return;
    }

    sql_query.Select("user_", "password", "account = '" + json_account + '\'');
    if(!sql_query.Next()) {
        Response(ResponseCode::FAILURE);
        return;
    }

    std::string encode_password = Crypto::MD5(json_password).toString();
    std::string db_password = sql_query.Value(0);

    if(encode_password != db_password) {
        Response(ResponseCode::FAILURE);
        return;
    }

    sql_query.Select("user_", "uid", "account = '" + json_account + '\'');
    if(!sql_query.Next()) {
        Response(ResponseCode::FAILURE);
        return;
    }

    std::string db_uid = sql_query.Value(0);
    std::string token = MakeToken(db_uid);

    http_platform_ = json_platform;
    platform_ = json_platform;

    if(CheckIsLogined(db_uid)) {
        Response(ResponseCode::LOGINED);
        return;
    }

    if(!UpdateUserInfo(db_uid, token, platform_)) {
        Response(ResponseCode::FAILURE);
        return;
    }
    sql_query.Update("user_", "last_login, network_state", GetDateTime() + ',' + json_platform, "uid='" + db_uid + '\'');

    Json send_json = {
        { "server", SERVER_NAME },
        { "code", ResponseCode::SUCCESS },
        { "request", request_},
        { "datetime" , GetDateTime() },
        { "access_url", "none"},
        { "uid" , db_uid},
        { "token",  token }
    };
    SendJson(send_json);
}

bool Work::Center::CheckIsLogined(const std::string &uid) {
    Data::User user_info;
    bool ret_result = false;
    do {
        if(Data::map_user.find(uid) != Data::map_user.end()) {
            user_info = Data::map_user[uid];
        }else {
            std::cout << "not exited!\n";
            std::cout << "check uid[" << uid << "]\n";
            break;
        }
        if(platform_ == "linux" && user_info.linux_fd != -1) {
            ret_result =  true;
            break;
        }else if(platform_ == "windows" && user_info.windows_fd != -1) {
            ret_result =  true;
            break;
        }else if(platform_ == "android" && user_info.android_fd != - 1) {
            ret_result =  true;
            break;
        }else if(platform_ == "web" && user_info.web_fd != - 1) {
            ret_result =  true;
            break;
        }else {
            ret_result =  false;
            break;
        }
    } while(false);
    return ret_result;
}

bool Work::Center::UpdateUserInfo(const std::string &uid, const std::string &token, const std::string &platform) {
    http_uid_ = uid;
    Data::User user;
    user.linux_fd = -1;
    user.windows_fd = -1;
    user.android_fd = -1;
    user.web_fd = -1;
    user.uid = uid;
    //std::cout << "update uid[" << uid << "]\n";
    if(Data::map_user.find(uid) != Data::map_user.end()) {
        user = Data::map_user[uid];
    }

    if(platform == "linux") {
        user.linux_fd = fd_;
        user.linux_token = token;
        user.linux_http = wp_http_.lock();
        std::cout << "Update ..";
    }else if(platform == "windows") {
        user.windows_fd = fd_;
        user.windows_token = token;
        user.windows_http = wp_http_.lock();
    }else if(platform == "android") {
        user.android_fd = fd_;
        user.android_token = token;
        user.android_http = wp_http_.lock();
    }else if(platform == "web") {
        user.web_fd = fd_;
        user.web_token = token;
        user.web_http = wp_http_.lock();
    }else {
        http_uid_ = "";
        return false;
    }
    Data::map_user[uid] = user;

    return true;
}

void Work::Center::DealWithGetUserInfo() {
    std::string account;
    std::string uid;
    std::string token;
    try {
        account = map_url_value_info_.at("account");
        uid = map_url_value_info_.at("uid");
        token = map_url_value_info_.at("token");
    }  catch (std::out_of_range e) {
        std::cout << "NO_access\n";
        Response(ResponseCode::NO_ACCESS);
        return;
    }

    if(!CheckToken(uid, token)) {
        Response(ResponseCode::FAILURE);
        return;
    }

    Database::MysqlQuery query;
    Database::MysqlQuery::Escape(account);
    query.Select("user_",
                 "account, email, nickname, name, phone_number, address, occupation, created_time, "
                 "network_state, last_login, head_image, profile_image_1, profile_image_2, profile_image_3, "
                 "profile_image_4"
                 ,"uid = '" + uid + '\'');
    if(!query.Next()) {
        Response(ResponseCode::FAILURE);
        return;
    }
    std::string db_account, db_email, db_nickname, db_name, db_phone_number
            , db_address, db_occupation, db_created_time, db_network_state, db_last_login,
            db_head_image, db_profile_image_1, db_profile_image_2, db_profile_image_3, db_profile_image_4;

    try {
        db_account = query.Value(0);
        db_email = query.Value(1);
        db_nickname = query.Value(2);
        db_name = query.Value(3);
        db_phone_number = query.Value(4);
        db_address = query.Value(5);
        db_occupation = query.Value(6);
        db_created_time = query.Value(7);
        db_network_state = query.Value(8);
        db_last_login = query.Value(9);
        db_head_image = query.Value(10);
        db_profile_image_1 = query.Value(11);
        db_profile_image_2 = query.Value(12);
        db_profile_image_3 = query.Value(13);
        db_profile_image_4 = query.Value(14);
    }  catch (Database::mysql_out_of_range e) {
        std::cout << e.what();
        Response(ResponseCode::FAILURE);
        return;
    }

    Json send_json = {
        { "server", SERVER_NAME },
        { "code", ResponseCode::SUCCESS },
        { "request", request_},
        { "datetime" , GetDateTime() },
        { "platform", platform_ },
        { "content-type", "user_info"},
        { "content", {
              {"account", db_account},
              {"email", db_email},
              {"nickname", db_nickname},
              {"name", db_name},
              {"phone_number", db_phone_number},
              {"address", db_address},
              {"occupation", db_occupation},
              {"created_time", db_created_time},
              {"network_state", db_network_state},
              {"last_login", db_last_login},
              {"head_image", db_head_image},
              {"profile_image_1", db_profile_image_1},
              {"profile_image_2", db_profile_image_2},
              {"profile_image_3", db_profile_image_3},
              {"profile_image_4", db_profile_image_4},
          }}
    };
    SendJson(send_json);
}

bool Work::Center::CheckToken(const std::string &uid, const std::string &token) {
    Data::User user;
    bool ret_value = false;
    if(Data::map_user.find(uid) != Data::map_user.end()) {
        user = Data::map_user[uid];
    }else {
        return false;
    }
    if(platform_ == "linux") {
        ret_value =  user.linux_token == token;
    } else if(platform_ == "windows"){
        ret_value =  user.windows_token == token;
    } else if(platform_ == "android") {
        ret_value =  user.android_token == token;
    } else if(platform_ == "web") {
        ret_value =  user.web_token == token;
    }else
        ret_value = false;

    return ret_value;
}

void Work::Center::DealWithGetUserPublicFile() {
    std::string uid;
    std::string file_name;
    try {
        uid = map_url_value_info_.at("uid");
        file_name = map_url_value_info_.at("file_name");

    }  catch (std::out_of_range e) {
        Response(ResponseCode::FAILURE);
        return;
    }
    std::string file_path = "data/user/" + uid + "/public/files/" + file_name;
    std::cout << "get public file [" << file_path << "]\n";
    SendFile(file_path);
}

void Work::Center::DealWithGetUserPrivateFile() {

}

void Work::Center::DealWithGetGetChatFile() {

}

void Work::Center::DealWithUploadProfileImage() {
    std::cout << "upload_profile_image\n";
    std::string name;
    std::string type;
    std::string account;
    std::string uid;
    std::string token;
    try {
        name = map_url_value_info_.at("name");
        account = map_url_value_info_.at("account");
        uid = map_url_value_info_.at("uid");
        token = map_url_value_info_.at("token");
        type = map_url_value_info_.at("type");
    } catch (std::out_of_range e) {
        Response(ResponseCode::FAILURE);
        std::cout << "lack var\n";
        return;
    }

    if(!CheckToken(uid, token)) {
        Response(ResponseCode::NO_ACCESS);
        std::cout << "check token error\n";
        return;
    }

    if(type == "head_image" || type == "profile_image_1" || type == "profile_image_2"
            || type == "profile_image_3" || type == "profile_image_4") {
    }else {
        Response(ResponseCode::FAILURE);
        std::cout << "file type error\n";
        return;
    }

    std::string save_image_path = "data/user/" + uid + "/public/profile/";

    if(!name.empty()) {
        save_image_path += name;
    } else {
        Response(ResponseCode::FAILURE);
        std::cout << "path: name empty\n";
        return;
    }
    std::cout << "path: " << save_image_path << '\n';

    int fd = open(save_image_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if(fd == -1) {
        Response(ResponseCode::FAILURE);
        return;
    }
    write(fd, content_.data(), content_.size());
    close(fd);

    std::string store_url = "/?request=get_profile_image&uid=" + uid + "&name=" + name;
    std::cout << "store_url: " << store_url << '\n';
    Database::MysqlQuery query;
    query.Update("user_", type, store_url, "uid='" + uid + '\'');
    Response(ResponseCode::SUCCESS);
}

void Work::Center::DealWithGetProfileImage() {
    std::string name;
    std::string uid;
    try {
        name = map_url_value_info_.at("name");
        uid = map_url_value_info_.at("uid");
    } catch (std::out_of_range e) {
        Response(ResponseCode::FAILURE);
        return;
    }
    std::string file_path = "data/user/" + uid + "/public/profile/" + name;
    SendFile(file_path);
}

void Work::Center::DealWithSearchUser() {
    std::string search;
    std::string type;

    std::string db_uid, db_account, db_nickname, db_head_image,
            db_address, db_network_state, db_email, db_signature;

    try {
        search = map_url_value_info_.at("search");
        type = map_url_value_info_.at("type");
    } catch (std::out_of_range e) {
        Response(ResponseCode::FAILURE);
        return;
    }
    ::Database::MysqlQuery query;
    ::Database::MysqlQuery::Escape(search);

    if(type == "account") {
        query.Select("user_", "uid", "account='" + search + '\'');
        if(query.Next()) {
            db_uid = query.Value(0);
        }else {
            Response(ResponseCode::NOT_EXIST);
            return;
        }

        query.Select("user_", "account, email, nickname, signature, head_image, address, network_state", "uid='" + db_uid + '\'');
        if(query.Next()) {
            try {
                db_account = query.Value(0);
                db_email = query.Value(1);
                db_nickname = query.Value(2);
                db_signature = query.Value(3);
                db_head_image = query.Value(4);
                db_address = query.Value(5);
                db_network_state = query.Value(6);
            }  catch (::Database::mysql_out_of_range e) {
                std::cout << e.what() << '\n';
                Response(ResponseCode::FAILURE);
                return;
            }
        }else {
            Response(ResponseCode::FAILURE);
            return;
        }
    }

    //std::cout << "search " << search << " type " << type << "\n";
    Json send_json = {
        { "server", SERVER_NAME },
        { "code", ResponseCode::SUCCESS },
        { "request", request_},
        { "datetime" , GetDateTime() },
        { "platform", platform_ },
        { "content-type", "user_info"},
        { "content", {
              {"uid", db_uid},
              {"account", db_account},
              {"email", db_email},
              {"nickname", db_nickname},
              {"address", db_address},
              {"signature", db_signature},
              {"network_state", db_network_state},
              {"head_image", db_head_image},
          }}
    };
    SendJson(send_json);
}

void Work::Center::DealWithAddUser() {
    std::string content_type;
    try {
        content_type = map_header_info_.at("content-type");
    } catch(std::out_of_range e) {
        std::cout << "no a content-type\n";
        Response(ResponseCode::ERROR_HTTP_CONTENT);
        return;
    }

    if(content_type != "application/json") {
        Response(ResponseCode::ERROR_HTTP_CONTENT);
        return;
    }

    Json recv_json_obj;
    try {
        recv_json_obj = Json::parse(content_);
    }  catch (Json::parse_error e) {
        Response(ResponseCode::ERROR_PARSING_CONTENT);
        return;
    }

    if(!CheckJsonContentType(recv_json_obj, "add_info")) {
        Response(ResponseCode::ERROR_JSON_CONTENT_TYPE);
        return;
    }

    std::string target_account, tid, account, uid, token, verify_message;
    try {
        tid = recv_json_obj["content"]["target_uid"];
        target_account = recv_json_obj["content"]["target_account"];
        account = recv_json_obj["content"]["account"];
        verify_message = recv_json_obj["content"]["verify_message"];
        uid = recv_json_obj["uid"];
        token = recv_json_obj["token"];
    } catch (Json::type_error) {
        Response(ResponseCode::ERROR_JSON_CONTENT_TYPE);
        return;
    }

    if(!CheckToken(uid, token)) {
        Response(ResponseCode::NO_ACCESS);
        return;
    }

    if(verify_message.size() >= 1023) {
        Response(ResponseCode::FAILURE);
        return;
    }

    // 检查是否是自己
    if(tid == uid) {
        Response(ResponseCode::FAILURE);
        return;
    }

    ::Database::MysqlQuery::Escape(target_account);
    ::Database::MysqlQuery::Escape(tid);
    ::Database::MysqlQuery::Escape(account);
    ::Database::MysqlQuery::Escape(uid);
    ::Database::MysqlQuery::Escape(verify_message);

    ::Database::MysqlQuery query;
    // 检查对方帐号是否存在
    query.Select("user_", "uid", "uid='" + tid + '\'');
    if(!query.Next()) {
        Response(ResponseCode::NOT_EXIST);
        return;
    }

    // 检查对方帐号是否在自己的好友中
    query.Select("user_friend_", "tid", "uid='" + tid + '\'');
    if(query.Next()) {
        Response(ResponseCode::EXIST);
        return;
    }

    Json json_obj = {
        { "server", SERVER_NAME },
        { "request", request_ },
        { "code", 0 },
        { "datetime" , GetDateTime() },
        { "content-type", "message"},
        { "content" , {
              "type", "add_user",
              "uid", uid,
              "message", verify_message
          }}
    };

    /*
    `eid` varchar(256) NOT NULL,
    `uid` varchar(256) NOT NULL,
    `tid` varchar(256) NOT NULL,
    `gid` varchar(256) NOT NULL,
    `request` varchar(256) NOT NULL,
    `mid` varchar(256) NOT NULL,
    `remark` varchar(512) NOT NULL,
    `created_time`  varchar(256) NOT NULL,
    PRIMARY KEY (`id`)
     */
    std::string i_eid = MakeToken(uid), i_uid = uid, i_tid = tid, i_gid = "unset",
            i_request = "add_user", i_mid = "none", i_remark = "none", i_created_time = GetDateTime();
    std::string value_sql = '\'' + i_eid + "',"; // set eid
    value_sql += '\'' + i_uid + "',";        // uid
    value_sql += '\'' + i_tid + "',";        // tid
    value_sql += '\'' + i_gid + "',";        // gid
    value_sql += '\'' + i_request + "',";    // request
    value_sql += '\'' + i_mid + "',";        // mid
    value_sql += '\'' + i_remark + "',";     // remark
    value_sql += '\'' + i_created_time + "'";      // created time

    if(!query.Insert("event_", "eid, uid, tid, gid, request, mid, remark, created_time", value_sql)) {
        Response(ResponseCode::FAILURE);
        return;
    }

    AddTowUserInDb(uid, tid, "friend");
    //std::cout << "add_user: \n";
    ::Work::PushMessage::ToUser(tid, json_obj);
}

void Work::Center::DealWithAddUserReply() {

}

bool Work::Center::AddTowUserInDb(const std::string &uid, const std::string &tid, const std::string &remark) {
    if(uid == tid) {
        return false;
    }
    ::Database::MysqlQuery query;
    // 检查对方帐号是否存在
    query.Select("user_", "uid", "uid='" + tid + '\'');
    if(!query.Next()) {
        return false;
    }
    // 检查自己帐号是否存在
    query.Select("user_", "uid", "uid='" + uid + '\'');
    if(!query.Next()) {
        return false;
    }

    // 检查对方帐号是否在自己的好友中
    query.Select("user_friend_", "tid", "uid='" + tid + '\'');
    if(query.Next()) {
        return false;
    }
    std::string datatime = GetDateTime();
    std::string value_sql = '\'' + uid + "',";    // set eid
    value_sql += '\'' + tid + "',";               // tid
    value_sql += '\'' + remark + "',";            // remark
    value_sql += '\'' + datatime + "'";           // datetime

    if(!query.Insert("user_friend_", "uid, tid, remark, created_time", value_sql)) {
        Response(ResponseCode::FAILURE);
        return false;
    }

    value_sql = '\'' + tid + "',";                // set uid
    value_sql += '\'' + uid + "',";               // tid
    value_sql += '\'' + remark + "',";            // remark
    value_sql += '\'' + datatime + "'";           // datetime

    if(!query.Insert("user_friend_", "uid, tid, remark, created_time", value_sql)) {
        Response(ResponseCode::FAILURE);
        return false;
    }

    return true;
}

void Work::Center::DealWithSendUserMessage() {
    std::string content_type;
    try {
        content_type = map_header_info_.at("content-type");
    } catch(std::out_of_range e) {
        std::cout << "no a content-type\n";
        Response(ResponseCode::ERROR_HTTP_CONTENT);
        return;
    }

    if(content_type != "application/json") {
        Response(ResponseCode::ERROR_HTTP_CONTENT);
        return;
    }

    Json recv_json_obj;
    try {
        recv_json_obj = Json::parse(content_);
    }  catch (Json::parse_error e) {
        Response(ResponseCode::ERROR_PARSING_CONTENT);
        return;
    }

    if(!CheckJsonContentType(recv_json_obj, "send_user_message")) {
        Response(ResponseCode::ERROR_JSON_CONTENT_TYPE);
        return;
    }

    std::string target_account, target_uid, account, uid, token, message;
    try {
        target_uid = recv_json_obj["content"]["target_uid"];
        target_account = recv_json_obj["content"]["target_account"];
        account = recv_json_obj["content"]["type"];
        message = recv_json_obj["content"]["message"];
        uid = recv_json_obj["uid"];
        token = recv_json_obj["token"];
    } catch (Json::type_error) {
        Response(ResponseCode::ERROR_JSON_CONTENT_TYPE);
        return;
    }

    if(!CheckToken(uid, token)) {
        Response(ResponseCode::NO_ACCESS);
        return;
    }

    if(message.size() >= 10240) { // 限制字数
        Response(ResponseCode::FAILURE);
        return;
    }

    // 检查是否是自己
    if(target_uid == uid) {
        Response(ResponseCode::FAILURE);
        return;
    }

    ::Database::MysqlQuery::Escape(target_account);
    ::Database::MysqlQuery::Escape(target_uid);
    ::Database::MysqlQuery::Escape(account);
    ::Database::MysqlQuery::Escape(uid);
    ::Database::MysqlQuery::Escape(message);

    ::Database::MysqlQuery query;

    // 检查对方帐号是否在自己的好友中
    query.Select("user_friend_", "tid", "uid='" + target_uid + '\'');
    if(query.Next()) {
        Response(ResponseCode::EXIST);
        return;
    }

    Json json_obj = {
        { "server", SERVER_NAME },
        { "request", request_ },
        { "code", 0 },
        { "datetime" , GetDateTime() },
        { "content-type", "message"},
        { "content" , {
              "type", "send_user_message",
              "uid", uid,
              "message", message
          }}
    };

    //std::cout << "add_user: \n";
    ::Work::PushMessage::ToUser(target_uid, json_obj);
}



void Work::Center::DealWithGetFriendList() {
    std::string account;
    std::string uid;
    std::string token;
    try {
        account = map_url_value_info_.at("account");
        uid = map_url_value_info_.at("uid");
        token = map_url_value_info_.at("token");
    }  catch (std::out_of_range e) {
        std::cout << "NO_access\n";
        Response(ResponseCode::NO_ACCESS);
        return;
    }

    if(!CheckToken(uid, token)) {
        Response(ResponseCode::FAILURE);
        return;
    }

    Database::MysqlQuery query;
    Database::MysqlQuery::Escape(account);
    Database::MysqlQuery::Escape(uid);

    query.Select("user_friend_", "tid", "uid='" + uid + '\'');
    Json send_josn_friend_list;
    while(query.Next()) {
        std::string tid = query.Value(0);
        Database::MysqlQuery query_get;
        std::string db_uid, db_account, db_email, db_nickname, db_signature, db_head_image, db_network_state, db_address;
        query_get.Select("user_", "uid, account, email, nickname, signature, head_image, address, network_state", "uid='" + tid + '\'');
        if(query_get.Next()) {
            try {
                db_uid = query_get.Value(0);
                db_account = query_get.Value(1);
                db_email = query_get.Value(2);
                db_nickname = query_get.Value(3);
                db_signature = query_get.Value(4);
                db_head_image = query_get.Value(5);
                db_address = query_get.Value(6);
                db_network_state = query_get.Value(7);
            }  catch (::Database::mysql_out_of_range e) {
                std::cout << e.what() << '\n';
                Response(ResponseCode::FAILURE);
                return;
            }
            send_josn_friend_list[db_uid] = {
                {"uid", db_uid },
                {"account", db_account },
                {"email", db_email },
                {"nickname", db_nickname },
                {"signature", db_signature },
                {"head_image", db_head_image },
                {"address", db_address },
                {"network_state", db_network_state }
            };
        }else {
            Response(ResponseCode::FAILURE);
            return;
        }
    }
    Json send_json = {
        { "server", SERVER_NAME },
        { "request", request_ },
        { "code", 0 },
        { "datetime" , GetDateTime() },
        { "content-type", "friend_list"},
        { "content" , send_josn_friend_list }
    };
    SendJson(send_json);
}






