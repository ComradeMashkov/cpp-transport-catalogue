#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace transport_catalogue {

constexpr static size_t STOP_NAME_POS = 5u;
constexpr static size_t BUS_NAME_POS = 4u;

struct Bus;

struct Stop {
    std::string name;

    double latitude;
    double longtitude;

    std::vector<Bus*> buses;
};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
};

struct Distance {
    const Stop* from;
    const Stop* to;

    int distance;
};

struct DistanceHasher {
public:
    size_t operator()(const std::pair<const Stop*, const Stop*> dest_pair) const {
        const auto from_hash = static_cast<const void*>(dest_pair.first);
        const auto to_hash = static_cast<const void*>(dest_pair.second);

        return v_hasher_(from_hash) * 13 + v_hasher_(to_hash);
    }

private:
    std::hash<const void*> v_hasher_;
};

using StopDict = std::unordered_map<std::string_view, Stop*>;
using BusDict = std::unordered_map<std::string_view, Bus*>;
using DistanceDict = std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher>;

class TransportCatalogue {
public:
    void AddBus(Bus&& bus);
    void AddStop(Stop&& stop);
    void AddDistance(const std::vector<Distance>& distances);

    Bus* GetBus(std::string_view bus_name);
    Stop* GetStop(std::string_view stop_name);

    std::unordered_set<const Stop*> GetUniqueStops(const Bus* bus) const;
    std::unordered_set<const Bus*> GetUniqueBuses(const Stop* stop) const;

    double GetLength(const Bus* bus) const;

    size_t GetDistanceBetweenStops(const Stop* from, const Stop* to) const;
    size_t GetRouteDistance(const Bus* bus) const;

private:
    std::deque<Stop> stops_;
    StopDict stops_associative_;

    std::deque<Bus> buses_;
    BusDict buses_associative_;

    DistanceDict distances_;
};

} // namespace transport_catalogue