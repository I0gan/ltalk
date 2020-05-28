#include "startup.hh"

Ltalk::StartUp::StartUp() {

}

Ltalk::StartUp::~StartUp() {

}

bool Ltalk::StartUp::Init(int argv, char **argc) {

    if(argv < 2) {
        std::cout << "-h get more info" << std::endl;
        return false;
    }

    std::string arg = argc[1];
    if(arg == "-h" || arg == "--help") {
        std::cout << "Usage: ./ltalks [OPTION...] [SECTION] PAGE...\n"
                     "-r, --run    run ltalk server\n"
                     "-s, --stop   stop ltalk server\n"
                     "-h, --help   help of ltalk server\n"
                     ;
    }else if(arg == "-r" || arg == "--run") {
        this->Run();
    }else if(arg == "-s" || arg == "--stop") {

    }else if(arg == "-p" || arg == "--print") {

    }else {
        std::cout << "-h get more info" << std::endl;
    }
    return true;
}

bool Ltalk::StartUp::Run() {

    std::cout << "Load config file" << std::endl;
    this->LoadConfig();
    std::cout << "init network" << std::endl;

    if(false == this->InitNetwork()) {
        std::cout << "init network fail!\n";
        abort();
    }

    std::cout << "Init database" << std::endl;
    this->InitDatabase();
    std::cout << "Init log" << std::endl;
    this->InitLog();

    return true;
}

// 加载配置文件
bool Ltalk::StartUp::LoadConfig() {
    using json = nlohmann::json;
    struct stat sbuf;
    if(stat(config_file_.c_str(), &sbuf) < 0) {
        d_cout("Config file [" + config_file_ + "] is not existed!");
        abort();
    }

    int fd = open(config_file_.c_str(), O_RDONLY);
    if(-1 == fd) {
        d_cout("Open Config file failed! ");
        abort();
    }
    void *mmap_ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if(mmap_ptr == (void *)-1) {
        d_cout("mmap Config file failed!");
        abort();
    }
    std::string file_json = static_cast<char *>(mmap_ptr);
    munmap(mmap_ptr, sbuf.st_size);

    json obj;
    try{
        obj = json::parse(file_json);
    } catch(json::parse_error &e) {
        d_cout(e.what());
        abort();
    }
    //解析json
    try {
       std::string tcp_port = obj["server"]["tcp port"];
       tcp_port_ = atoi(tcp_port.c_str());

       std::string udp_port = obj["server"]["udp port"];
       udp_port_ = atoi(udp_port.c_str());

       std::string thread_num = obj["server"]["thread number"];
       thread_num_ = atoi(thread_num.c_str());

       log_path_ = obj["server"]["log path"];


       db_host_ = obj["database"]["host"];

       std::string db_port = obj["database"]["port"];
       db_port_ = atoi(db_port.c_str());
       db_user_ = obj["database"]["user"];
       db_password_ = obj["database"]["password"];
       db_name_ = obj["database"]["name"];

    }  catch (json::exception &e) {
        d_cout(e.what());
        abort();
    }

    //std::cout << "http_port " << http_port_ << std::endl;
    return true;
}
bool Ltalk::StartUp::InitNetwork() {
    Ltalk::Net net;
    net.Init(tcp_port_);
    return net.Listen();
}
bool Ltalk::StartUp::InitDatabase() {
    return true;
}
bool Ltalk::StartUp::InitLog() {
    return true;
}

//int socket_bind_listen(int port) {
//    if(port < 1024 || port > 65535)
//        return -1;
//    int listen_fd = 0;
//    //if(listen_fd = socket(A))
//}