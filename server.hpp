// Standard library
#include <iostream>
#include <thread>
#include <unordered_map>
#include <functional>
#include <fstream>

#include <ctime>
#include <chrono>

// External
#include "httplib.h"
#include <nlohmann/json.hpp>

enum class HttpMethod{
    GET, POST, PATCH
};

using ResRef = httplib::Response&;
using CReqRef = const httplib::Request&;
using json = nlohmann::json;
using SystemClock = std::chrono::time_point<std::chrono::system_clock>;
using Handler = std::function<void(const httplib::Request&, httplib::Response&)>;
using RouteMap = std::unordered_map<HttpMethod, std::unordered_map<std::string, Handler>>;

class Server{
public:

    Server(const std::string& path, int port);
    ~Server();
    void run();
    void addRoute(HttpMethod method, const std::string& endpoint, Handler handler);
    bool processPayment();
    std::string printHttpMethod(HttpMethod method);

    bool startPayment = false;
private:
    std::thread worker;
    const std::string host;
    int port = 0;
    httplib::Server server;
    RouteMap routes;
};

class Controller{
public: 
    static json load_json(const std::string& path);
    static void getHello(CReqRef req, ResRef res);
    static void postPayment(Server& server, CReqRef req, ResRef res);
    static void patchPaymentStatusById(CReqRef req, ResRef res);
    static void getPaymentStatusById(CReqRef req, ResRef res);
};