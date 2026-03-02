#ifndef TYPES_HPP
#define TYPES_HPP

// Standard library
#include <iostream>
#include <thread>
#include <unordered_map>
#include <functional>
#include <fstream>

#include <ctime>
#include <chrono>

// External
#include <nlohmann/json.hpp>
#include "server.hpp"
#include "httplib.h"

enum class HttpMethod{
    GET, POST, PATCH
};

using ResRef = httplib::Response&;
using CReqRef = const httplib::Request&;
using json = nlohmann::json;
using SystemClock = std::chrono::time_point<std::chrono::system_clock>;
using Handler = std::function<void(const httplib::Request&, httplib::Response&)>;
using RouteMap = std::unordered_map<HttpMethod, std::unordered_map<std::string, Handler>>;

#endif