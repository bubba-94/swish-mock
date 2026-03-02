#ifndef SERVER_HPP
#define SERVER_HPP

#include "types.hpp"

namespace Payments{
    enum class Status: uint8_t{
        PAID,
        PENDING,
        DECLINED,
        ERROR,
        CANCELLED
    };
    
    typedef struct {
        uint16_t id;
        uint16_t amount;
        Status status;
        std::string payeeAlias;
        std::string currency;
        std::string callbackUrl;
        std::string message;
    }Payment;

    using PaymentsMap = std::unordered_map<uint16_t, Payment>;
    
    class Controller{
    public:
        explicit Controller();
        ~Controller();
        const PaymentsMap& getMap();
        void load(const std::string& path);
        uint16_t createPayment(const Payment& p);
        void postNewPayment(int paymentId);
        void getAllPayments();
        void getStatusById();
        void patchStatusById();
        void printPayments();
        void flushMap();
        int generateId();
        const std::string convert(Status status);
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
    private:
        RouteMap routes;
        Payments::Controller& controller;
};

class Server{
    public:
    
        Server(const std::string& path, int port);
        ~Server();
        void run();
        bool processPayment();
        std::string printHttpMethod(HttpMethod method);
        
        bool startPayment = false;
    private:
        std::thread worker;
        const std::string host;
        int port = 0;
        Payments::Controller controller;
        Routes routes;
        httplib::Server server;
};
#endif