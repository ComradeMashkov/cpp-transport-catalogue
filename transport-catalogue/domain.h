#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace domain {

struct Stat {
	int id;
	std::string type;
	std::string name;
};

struct Bus;

struct Stop {
    std::string name;

    transport_catalogue::detail::geo::Coordinates coords;

    std::vector<Bus*> buses;
};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
    bool is_roundtrip;
};

struct Distance {
    const Stop* from;
    const Stop* to;

    int distance;
};

struct BusQuery {
    std::string_view name;
    bool query_exists;
    int route_stops;
    int unique_stops;
    int route_length;
    double curvature;
};

struct StopQuery {
    std::string_view name;
    bool query_exists;
    std::vector<std::string> buses;
};

} // namespace domain