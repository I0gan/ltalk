#pragma once
#include "../ltalk.hh"

enum class Process::RequestType {
    error = 0,
    main_page,
    register_page,
    register_success_page,
    login,
    register_,
    keep_connect,
    get_user_info,
    get_public_file,
    get_private_file,
    upload_profile_image,
    get_profile_image
};

enum class Process::Platform {
    unknown = 0,
    linux_,
    windows,
    android,
};

namespace Process {
class Request {
public:
    static RequestType toEnum(std::string request);

private:
    static void Init();
    static std::unordered_map<std::string, RequestType> umap_type_;
    static pthread_once_t once_control_;
};
}


