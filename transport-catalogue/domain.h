#pragma once

#include "geo.h"
#include "graph.h"

#include <string>
#include <variant>
#include <vector>

namespace domain {

struct Stat {
	int id;
	std::string type;
	std::string name;
    std::string from;
    std::string to;
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

struct BusEdge {
    std::string_view name;
    size_t span_count = 0;
    double time = 0;
};

struct StopQuery {
    std::string_view name;
    bool query_exists;
    std::vector<std::string> buses;
};

struct StopEdge {
    std::string_view name;
    double time = 0;
};

struct RoutingSettings {
    double bus_wait_time = 0;
    double bus_velocity = 0;
};

struct WaitRange {
    graph::VertexId bus_wait_start;
    graph::VertexId bus_wait_end;
};

struct RouteGraphInfo {
    double total_time;
    std::vector<std::variant<StopEdge, BusEdge>> edges;
};

} // namespace domain