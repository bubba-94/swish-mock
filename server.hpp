#ifndef SERVER_HPP
#define SERVER_HPP

#include "types.hpp"

namespace Payments{

    using PaymentsMap = std::unordered_map<uint16_t, Payment>;
    
    class Controller{
    public:
        explicit Controller();
        ~Controller();
        const PaymentsMap& getMap();
        void load(const std::string& path);
        
        // Endpoint specifics
        void getAllPayments();
        json getStatusById(int id);
        json getAllPaymentsJson();
        bool findMapId(int id);
        void patchStatusById(int id, CReqRef req);
        uint16_t postNewPayment(CReqRef res);
        
        // Helpers for endpoints
        void printPayments();
        void flushPayment(int key);
        int generateId();
        const std::string statusToStr(Status status);
        Status strToStatus(const std::string& str);
        PaymentsMap payMap;
    private:
        uint16_t nextId = 1;
        static json load_json(const std::string& path);
    };
};

class Routes{
    public: 
        explicit Routes(Payments::Controller& controller);
        ~Routes();
        void addRoute(HttpMethod method, const std::string& endpoint, Handler handler);
        RouteMap& getRoutes();
        void getHello(CReqRef req, ResRef res);
        void getAllPayments(CReqRef req, ResRef res);
        void postPayment(CReqRef req, ResRef res);
        void patchPaymentStatusById(CReqRef req, ResRef res);
        void getPaymentStatusById(CReqRef req, ResRef res);


        void paymentProcess(int id);
        bool checkCallbackURL(int id);
    private:
        std::atomic<bool>processPayment = false;
        std::thread worker;
        RouteMap routes;
        Payments::Controller& controller;
};

class Server{
    public:
    
        Server(const std::string& path, int port);
        ~Server();
        void run();

        std::string printHttpMethod(HttpMethod method);
        
    private:
        const std::string host;
        int port = 0;

        Payments::Controller controller;
        Routes routes;

        httplib::Server server;
};
#endif