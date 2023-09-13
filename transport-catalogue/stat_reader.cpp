#include "stat_reader.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace transport_catalogue {

namespace detail {

namespace iostream {

void PrintBusQuery(TransportCatalogue& cat, std::string_view query) {
	using namespace std::string_literals;

	query = query.substr(BUS_NAME_POS);

	const Bus* bus = cat.GetBus(query);

	if (bus != nullptr) {
		std::cout << "Bus "s << bus->name << ": "s << bus->stops.size() << " stops on route, "s << cat.GetUniqueStops(bus).size() << " unique stops, "s << cat.GetRouteDistance(bus) << " route length, "s << std::setprecision(PRECISION) << cat.GetRouteDistance(bus) / cat.GetLength(bus) << " curvature"s << std::endl;
	}
	else {
		std::cout << "Bus "s << query << ": not found"s << std::endl;
	}
}

void PrintStopQuery(TransportCatalogue& cat, std::string_view query) {
	using namespace std::string_literals;

	query = query.substr(STOP_NAME_POS);

	std::unordered_set<const Bus*> unique_buses;
	std::vector<std::string_view> bus_names;

	Stop* stop = cat.GetStop(query);

	if (stop != nullptr) {
		unique_buses = cat.GetUniqueBuses(stop);

		if (unique_buses.size() == 0u) {
			std::cout << "Stop "s << query << ": no buses"s << std::endl;
		}

		else {
			std::cout << "Stop "s << query << ": buses "s;

			for (const Bus* bus : unique_buses) {
				bus_names.push_back(bus->name);
			}

			std::sort(bus_names.begin(), bus_names.end());

			for (std::string_view bus_name : bus_names) {
				std::cout << bus_name << " "s;
			}
			std::cout << std::endl;
		}
	}

	else {
		std::cout << "Stop "s << query << ": not found"s << std::endl;
	}
}

void PrintQuery(TransportCatalogue& cat, std::string_view query) {
	using namespace std::string_literals;

	if (query.substr(0u, 3u) == "Bus"s) {
		PrintBusQuery(cat, query);
	}
	else if (query.substr(0u, 4u) == "Stop"s) {
		PrintStopQuery(cat, query);
	}
	else {
		std::cout << "Error query"s << std::endl;
	}
}

void OutputStream(TransportCatalogue& cat) {
	std::string count;
	std::getline(std::cin, count);
	
	size_t size = std::stoi(count);

	std::vector<std::string> query;
	std::string buffer;

	for (size_t i = 0; i < size; ++i) {
		std::getline(std::cin, buffer);
		query.push_back(buffer);
	}

	for (std::string str : query) {
		PrintQuery(cat, str);
	}
}

} // namespace iostream

} // namespace detail

} // namespace transport_catalogue
