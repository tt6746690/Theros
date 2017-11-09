#include <algorithm>    // reverse

#include "Utils.h"
#include "Router.h"

namespace Summer {


std::ostream& operator<<(std::ostream& os, const Handler& handler)
{
    return os << "(" << handler.handler_id_ << ")";
}

void Router::handle(RequestMethod method, const std::string& path, HandlerType handler)
{
    auto &t = routing_tables[to_underlying_t(method)];
    t.insert({path, handler});
}


template <typename RequestMethods>
void Router::handle(RequestMethods methods, const std::string& path, const HandlerType& handler)
{
    for (const auto &method : methods) handle(method, path, handler);
}

void  Router::get(const std::string& path, const HandlerType& handler) { handle(RequestMethod::GET, path, handler); }
void  Router::post(const std::string& path, const HandlerType& handler) { handle(RequestMethod::POST, path, handler); }
void  Router::put(const std::string& path, const HandlerType& handler) { handle(RequestMethod::PUT, path, handler); }
void  Router::use(const std::string& path, const HandlerType& handler)
{
    for(auto i = to_underlying_t(RequestMethod::GET); i != to_underlying_t(RequestMethod::UNDETERMINED); ++i)
        handle(static_cast<RequestMethod>(i), path, handler);
}


auto Router::resolve(RequestMethod method, std::string& path) -> RouteType
{
    auto &t = routing_tables[to_underlying_t(method)];
    auto found = t.find(path);

    RouteType route;
    while(found != t.end()) {
        route.push_back(*found);
        --found;
    }

    std::reverse(route.begin(), route.end());
    return route;
}


auto Router::resolve(Request& request) -> RouteType
{
    auto method = request.method_;
    auto path = request.uri_.abs_path_;
    auto &table = routing_tables[to_underlying_t(method)];

    return {};
}

std::ostream& operator<<(std::ostream& os, const Router& r)
{
    for(int i = 0; i < method_count; ++i) {
        auto& table =  r.routing_tables[i];
        if(table.size())
            os << Request::request_method_to_string(static_cast<RequestMethod>(i)) << eol << table << eol;
    }
    return os;
}



} // namespace Summer

// std::vector<T> resolve(Request &req)
// {
//   auto method = req.method_;
//   auto path = req.uri_.abs_path_;
//   auto &route = routes_[to_underlying_t(method)];

//   std::string param_key, param_value;
//   auto found = route.find(path, param_key, param_value);

//   if (found == route.end())
//     return {};

//   if (!param_key.empty() && !param_value.empty())
//     req.param_.insert({param_key, param_value});

//   std::vector<T> handle_sequence;

//   while (found != route.end())
//   {
//     if (*found)
//       handle_sequence.push_back(*found);
//     --found;
//   }

//   std::reverse(handle_sequence.begin(), handle_sequence.end());
//   return handle_sequence;
// }