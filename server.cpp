#include "server.hpp"

// SERVER

Server::Server(const std::string& host, int port):  
    host(host), 
    port(port),
    controller(),
    routes(controller){}

Server::~Server(){}

void Server::run(){
    // if (startPayment){
    //     std::thread payment(handlePayment);
    // }
    std::cout << "\n"; 

    for (auto& [method, endpoints] : routes.getRoutes()) {
        std::cout << printHttpMethod(method) << " routes: " << "\n";
        for (auto& [endpoint, _] : endpoints) {
            std::cout << " - " << endpoint << "\n";
            }
        std::cout << "\n";
        }

    // For each method and endpoint in route table
    for (auto& [method, endpoints] : routes.getRoutes()) {
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

void Routes::addRoute(HttpMethod method, const std::string& endpoint, Handler handler){
    routes[method][endpoint] = handler;
    controller.load("payment.json");
}

RouteMap& Routes::getRoutes() { return routes; }

// ENDPOINTS

Routes::Routes(Payments::Controller& controller): controller(controller){
    addRoute(HttpMethod::GET, "/hi", 
        [this](const httplib::Request& req, httplib::Response& res){
            Routes::getHello(req, res);
        });
    addRoute(HttpMethod::GET, "/allPaymentRequests",
        [this](const httplib::Request& req, httplib::Response& res){
            Routes::getAllPayments(req, res);
        });
    addRoute(HttpMethod::POST, "/paymentRequest", 
        [this](const httplib::Request& req, httplib::Response& res){
            Routes::postPayment(req, res);
        });
    addRoute(HttpMethod::PATCH, "/paymentRequests/:id", 
        [this](const httplib::Request& req, httplib::Response& res){
            Routes::patchPaymentStatusById(req, res); 
        });
    addRoute(HttpMethod::GET, "/paymentRequests/:id", 
        [this](const httplib::Request& req, httplib::Response& res){
            Routes::getPaymentStatusById(req, res);
        });
}
Routes::~Routes(){}

void Routes::getHello(CReqRef req, ResRef res){
    res.set_content(R"({"message":"Hello"})", "application/json");
}

void Routes::getAllPayments(CReqRef req, ResRef res){ 
    controller.printPayments();
}

// Create a new payment (start process) 
void Routes::postPayment(CReqRef req, ResRef res){

    // A json payment from curL???
    controller.postNewPayment(controller.generateId());

    res.set_content("Created payment", "application/json");
}

// Update current payment status to CREATED
void Routes::patchPaymentStatusById(CReqRef req, ResRef res){
    auto paymentId = req.path_params.at("id");
    res.set_content("Updated status of payment: " + paymentId, "application/json");
}

// Check status of payment 
void Routes::getPaymentStatusById(CReqRef req, ResRef res){
    auto paymentId = req.path_params.at("id");
    // Content should be the current status of the Payment
    res.set_content("Payment: " + paymentId, "application/json");
}

std::string Server::printHttpMethod(HttpMethod method){
    switch(method){
        case HttpMethod::GET: return "GET";
        case HttpMethod::PATCH: return "PATCH";
        case HttpMethod::POST: return "POST";
        default: return " ";
    }
}

// CONTROLLER 

Payments::Controller::Controller(){}
Payments::Controller::~Controller(){}

void Payments::Controller::postNewPayment(int paymentId){

}

void Payments::Controller::load(const std::string& path)
{
    auto file = load_json(path);

    payMap.clear();

    for (const auto& payment : file)
    {
        Payment p;
        p.amount = payment.at("amount").get<uint16_t>();

        // handle null status safely
        if (payment["status"].is_null())
            p.status = Status::PENDING;
        else
            p.status = static_cast<Status>(payment["status"].get<int>());

        p.payeeAlias = payment.at("payeeAlias").get<std::string>();
        p.currency = payment.at("currency").get<std::string>();
        p.callbackUrl = payment.at("callbackUrl").get<std::string>();
        p.message = payment.at("message").get<std::string>();

        payMap[generateId()] = p;
    }
}

const std::string Payments::Controller::convert(Status status){
    switch (status){
        case Status::PAID: return "PAID";
        case Status::PENDING: return "PENDING";
        case Status::DECLINED: return "DECLINED";
        case Status::ERROR: return "ERROR";
        case Status::CANCELLED: return "CANCELLED";
        default: return " ";
    }
}

void Payments::Controller::printPayments() {
    if (payMap.empty()) {
        std::cout << "No payments found.\n";
        return;
    }

    std::cout << "==== Payments ====\n";

    for (const auto&  [key, p]: payMap) {
        std::cout << "ID: " << key << "\n"
                  << "  Amount:       " << p.amount << "\n"
                  << "  Status:       " << convert(p.status) << "\n"
                  << "  Payee Alias:  " << p.payeeAlias << "\n"
                  << "  Currency:     " << p.currency << "\n"
                  << "  Callback URL: " << p.callbackUrl << "\n"
                  << "  Message:      " << p.message << "\n"
                  << "-------------------------\n";
    }
}

int Payments::Controller::generateId(){
    srand(time(0));
    int random = rand() % 101;
    if (auto search = payMap.find(random); search != payMap.end()){
        return generateId();
    }
    return random;
}

json Payments::Controller::load_json(const std::string& path){
    std::ifstream f(path);
    return json::parse(f);
}