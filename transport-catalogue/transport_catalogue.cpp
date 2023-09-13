#include <numeric>
#include <stdexcept>

#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddBus(const Bus& bus) {
	buses_.push_back(bus);

	Bus* last_bus = &buses_.back();
	buses_associative_.insert(BusDict::value_type(last_bus->name, last_bus));

	for (Stop* stop : last_bus->stops) {
		stop->buses.push_back(last_bus);
	}
}

void TransportCatalogue::AddStop(const Stop& stop) {
	stops_.push_back(stop);

	Stop* last_stop = &stops_.back();
	stops_associative_.insert(StopDict::value_type(last_stop->name, last_stop));
}

void TransportCatalogue::AddDistance(const Distance& distance) {
	distances_.insert(DistanceDict::value_type(std::make_pair(distance.from, distance.to), distance.distance));
}

Bus* TransportCatalogue::GetBus(std::string_view bus_name) {
	if (buses_associative_.empty()) {
		return nullptr;
	}

	try {
		return buses_associative_.at(bus_name);
	}
	catch (const std::out_of_range& e) {
		return nullptr;
	}
}

Stop* TransportCatalogue::GetStop(std::string_view stop_name) {
	if (stops_associative_.empty()) {
		return nullptr;
	}

	try {
		return stops_associative_.at(stop_name);
	}
	catch (const std::out_of_range& e) {
		return nullptr;
	}
}

Route TransportCatalogue::GetRouteInfo(const Bus* bus) const {
	Route result;

	result.unique_stops = this->GetUniqueStops(bus);
	result.length = this->GetRouteLength(bus);
	result.distance = this->GetRouteDistance(bus);

	return result;
}

std::unordered_set<const Bus*> TransportCatalogue::GetUniqueBuses(const Stop* stop) const {
	std::unordered_set<const Bus*> unique_buses;

	unique_buses.insert(stop->buses.begin(), stop->buses.end());

	return unique_buses;
}

std::unordered_set<const Stop*> TransportCatalogue::GetUniqueStops(const Bus* bus) const {
	std::unordered_set<const Stop*> unique_stops;

	unique_stops.insert(bus->stops.begin(), bus->stops.end());

	return unique_stops;
}

size_t TransportCatalogue::GetRouteDistance(const Bus* bus) const {
	const size_t STOPS_SIZE = bus->stops.size();
	size_t distance = 0u;

	for (size_t i = 0u; i < STOPS_SIZE - 1u; ++i) {
		distance += GetDistanceBetweenStops(bus->stops[i], bus->stops[i + 1u]);
	}

	return distance;
}

double TransportCatalogue::GetRouteLength(const Bus* bus) const {
	auto compute_distance = [](const Stop* lhs, const Stop* rhs) {
		return detail::geo::ComputeDistance({ lhs->coords.lat, lhs->coords.lng }, { rhs->coords.lat, rhs->coords.lng });
		};

	return std::transform_reduce(
		next(bus->stops.begin()),
		bus->stops.end(),
		bus->stops.begin(),
		0.0,
		std::plus<>{},
		compute_distance
	);
}

size_t TransportCatalogue::GetDistanceBetweenStops(const Stop* from, const Stop* to) const {
	if (distances_.empty()) {
		return 0u;
	}

	try {
		return distances_.at(std::make_pair(from, to));
	}
	catch (const std::out_of_range& e) {
		try {
			return distances_.at(std::make_pair(to, from));
		}
		catch (const std::out_of_range& e) {
			return 0u;
		}
	}
}

} // namespace transport_catalogue
