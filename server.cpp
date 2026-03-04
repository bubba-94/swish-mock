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

std::string Server::printHttpMethod(HttpMethod method){
    switch(method){
        case HttpMethod::GET: return "GET";
        case HttpMethod::PATCH: return "PATCH";
        case HttpMethod::POST: return "POST";
        default: return " ";
    }
}

void Routes::addRoute(HttpMethod method, const std::string& endpoint, Handler handler){
    routes[method][endpoint] = handler;
}

RouteMap& Routes::getRoutes() { return routes; }

// ENDPOINTS

Routes::Routes(Payments::Controller& controller): controller(controller){
    // Load one payment from payment.json
    controller.load("payment.json");

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
    addRoute(HttpMethod::PATCH, "/paymentRequest/:id", 
        [this](const httplib::Request& req, httplib::Response& res){
            Routes::patchPaymentStatusById(req, res); 
        });
    addRoute(HttpMethod::GET, "/paymentRequest/:id", 
        [this](const httplib::Request& req, httplib::Response& res){
            Routes::getPaymentStatusById(req, res);
        });
}

Routes::~Routes(){}

void Routes::getHello(CReqRef req, ResRef res){
    res.set_content("HELLOOOO", "text/plain");
}

void Routes::getAllPayments(CReqRef req, ResRef res){ 
    controller.printPayments();

    json response = controller.getAllPaymentsJson();

    res.set_content(response.dump(), "application/json");
}

// Create a new payment (start process) 
void Routes::postPayment(CReqRef req, ResRef res){
    int id = controller.postNewPayment(req);
    
    // Random id from 1 - 101
    if(id > 1){
        processPayment = true;
        worker = std::thread(&Routes::paymentProcess, this, id);
    }
    if (worker.joinable()){
        worker.join();
    }

    json response;
    response["id"] = id;

    res.set_content(response.dump(), "application/json");
}

// Patch the Payment to any valid value. 
void Routes::patchPaymentStatusById(CReqRef req, ResRef res){
    auto paymentId = req.path_params.at("id");
    uint16_t id = std::stoi(paymentId);

    try{
        controller.patchStatusById(id, req);
        res.set_content("Updated status of payment: " + paymentId, "text/plain");
    }
    catch(const std::exception& e){
        res.status = 400;
        res.set_content("Failed to update: " + paymentId, "text/plain");
    }
}

// Check status of payment, evaluate on client side.
void Routes::getPaymentStatusById(CReqRef req, ResRef res){
    auto paymentId = req.path_params.at("id");
    uint16_t id = std::stoi(paymentId);
    json response = controller.getStatusById(id);

    // Content should be the current status of the Payment
    res.set_content(response.dump(), "application/json");
}

void Routes::paymentProcess(int id){
    bool callback = checkCallbackURL(id);
    if (!callback){
        controller.flushPayment(id);
    } 
}

bool Routes::checkCallbackURL(int id){
    const std::string validCallback = "http://localhost:8443/callback";
    std::cout << "Processing payment registration....\n";

    for (auto& [key, val] : controller.payMap){
        if (id == key){
            if (val.callbackUrl == validCallback){
                std::cout << "Payment: " << id << " has valid callback\n";
                return true;
            }
        }
    }
    return false;
}

// CONTROLLER 

Payments::Controller::Controller(){}
Payments::Controller::~Controller(){}

bool Payments::Controller::findMapId(int id){
    if(auto search = payMap.find(id); search != payMap.end()){
        return true;
    }
    return false;
}

json Payments::Controller::getAllPaymentsJson() {
    json response = json::array();
    for (const auto& payment : payMap) {
        json obj;
        obj["id"] = payment.first;
        obj["status"] = statusToStr(payment.second.status);

        response.push_back(obj);
    }

    return response;
}

json Payments::Controller::getStatusById(int id){
    json response; 
    bool checkId = findMapId(id);
    if (!checkId) return response;

    response["id"] = id;
    response["status"] = statusToStr(payMap.at(id).status);

    return response;
}
void Payments::Controller::patchStatusById(int id, CReqRef req){
    json body = json::parse(req.body);
    
    if(auto search = payMap.find(id); search != payMap.end()){
        if (!body.at("status").is_null()){
            payMap.at(id).status = strToStatus(body.at("status"));
        }
    }
}

uint16_t Payments::Controller::postNewPayment(CReqRef req){
    Payment p;
    try {
            // Parse JSON body
            json body = json::parse(req.body);

            p.amount = body.at("amount");
            p.payeeAlias = body.at("payeeAlias");
            p.currency = body.at("currency");
            p.callbackUrl = body.at("callbackUrl");
            p.message = body.at("message");
            p.status = Status::PENDING;

            uint16_t key = generateId();

            payMap[key] = p;
            return key;
        }
        catch (const std::exception& e) {
            return 0;
        }
}

void Payments::Controller::flushPayment(int id){
    payMap.erase(id);
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

const std::string Payments::Controller::statusToStr(Status status){
    switch (status){
        case Status::VALID: return "VALID";
        case Status::PENDING: return "PENDING";
        case Status::DECLINED: return "DECLINED";
        case Status::ERROR: return "ERROR";
        case Status::CANCELLED: return "CANCELLED";
        default: return " ";
    }
}

Payments::Status Payments::Controller::strToStatus(const std::string& str) {
    if (str == "VALID") return Status::VALID;
    if (str == "PENDING") return Status::PENDING;
    if (str == "DECLINED") return Status::DECLINED;
    if (str == "ERROR") return Status::ERROR;
    if (str == "CANCELLED") return Status::CANCELLED;
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
                  << "  Status:       " << statusToStr((p.status)) << "\n"
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
    if (random == 0){
        return generateId();
    }
    return random;
}

json Payments::Controller::load_json(const std::string& path){
    std::ifstream f(path);
    return json::parse(f);
}