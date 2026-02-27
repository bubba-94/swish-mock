#include "server.hpp"

int main (){
    
    Server server("0.0.0.0", 8443);

    server.addRoute(HttpMethod::GET, "/hi", 
        [](const httplib::Request& req, httplib::Response& res){
            Controller::getHello(req, res); // Server reference captured
        });
    server.addRoute(HttpMethod::POST, "/paymentRequest", 
        [&](const httplib::Request& req, httplib::Response& res){
            Controller::postPayment(server, req, res); // Server reference captured
        });
    server.addRoute(HttpMethod::PATCH, "/paymentRequests/:id", 
        [&](const httplib::Request& req, httplib::Response& res){
             Controller::patchPaymentStatusById(req, res); 
        });
    server.addRoute(HttpMethod::GET, "/paymentRequests/:id", 
        [&](const httplib::Request& req, httplib::Response& res){
            Controller::getPaymentStatusById(req, res);
        });

    server.run();
    std::cout << server.startPayment << "\n";
    return 0;
}
