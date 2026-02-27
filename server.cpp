#include "server.hpp"

Server::Server(const std::string& host, int port)
: host(host), port(port){}
Server::~Server(){}

void Server::run(){
    // if (startPayment){
    //     std::thread payment(handlePayment);
    // }
    std::cout << "\n"; 

    for (auto& [method, endpoints] : routes) {
        std::cout << printHttpMethod(method) << " routes: " << "\n";
        for (auto& [endpoint, _] : endpoints) {
            std::cout << " - " << endpoint << "\n";
            }
        std::cout << "\n";
    }

    // For each method and endpoint in route table
    for (auto& [method, endpoints] : routes) {
        // HTTP Methods
        for (auto& [endpoint, handler] : endpoints) {
            if (method == HttpMethod::GET) server.Get(endpoint, handler);
            else if (method == HttpMethod::POST) server.Post(endpoint, handler);
            else if (method == HttpMethod::PATCH) server.Patch(endpoint, handler);
        }
    }

    // Log events
    server.set_logger([](const httplib::Request& req, const httplib::Response& res) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        // Time and output of log
        std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " | "
        << req.version << " " << req.method << " " << req.path << " " << res.status << " " << res.body << std::endl;
    });

    server.listen(host, port);
}

void Server::addRoute(HttpMethod method, const std::string& endpoint, Handler handler){
    routes[method][endpoint] = handler;
}

json Controller::load_json(const std::string& path){
    std::ifstream f(path);
    return json::parse(f);
}

// ENDPOINTS

void Controller::getHello(CReqRef req, ResRef res){
    res.set_content("Hello", "application/json");
}

// Create a new payment (start process) 
void Controller::postPayment(Server& server, CReqRef req, ResRef res){
    auto j = load_json("payment.json");
    // if (!file){
    //     res.status = 500;
    //     res.set_content("Content not found", "application/json");
    //     return;
    // }

    server.startPayment = true;
    std::cout << (server.startPayment ? "true" : "false");
    res.set_content(j.dump(4), "application/json");
}

// Update current payment status to CREATED
void Controller::patchPaymentStatusById(CReqRef req, ResRef res){
    auto paymentId = req.path_params.at("id");

    res.set_content("Param id: " + paymentId, "application/json");
}

// Check status of payment 
void Controller::getPaymentStatusById(CReqRef req, ResRef res){
    auto paymentId = req.path_params.at("id");

    // Content should be the current status of the Payment
    res.set_content("Param id: " + paymentId, "application/json");
}

std::string Server::printHttpMethod(HttpMethod method){
    switch(method){
        case HttpMethod::GET: return "GET";
        case HttpMethod::PATCH: return "PATCH";
        case HttpMethod::POST: return "POST";
        default: return " ";
    }

}