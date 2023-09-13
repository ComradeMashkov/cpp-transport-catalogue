#include "stat_reader.h"

#include <algorithm>
#include <iomanip>

namespace transport_catalogue {

namespace detail {

namespace iostream {

void PrintBusQuery(TransportCatalogue& catalogue, std::string_view query, std::ostream& os) {
	using namespace std::string_literals;

	query = query.substr(BUS_NAME_POS);

	const Bus* bus = catalogue.GetBus(query);

	if (bus != nullptr) {
		os << 
			"Bus "s << bus->name << ": "s << 
			bus->stops.size() << " stops on route, "s << 
			catalogue.GetRouteInfo(bus).unique_stops.size() << " unique stops, "s << 
			catalogue.GetRouteInfo(bus).distance << " route length, "s << 
			std::setprecision(PRECISION) << catalogue.GetRouteInfo(bus).distance / catalogue.GetRouteInfo(bus).length << " curvature"s 
		<< std::endl;
	}
	else {
		os << "Bus "s << query << ": not found"s << std::endl;
	}
}

void PrintStopQuery(TransportCatalogue& catalogue, std::string_view query, std::ostream& os) {
	using namespace std::string_literals;

	query = query.substr(STOP_NAME_POS);

	std::unordered_set<const Bus*> unique_buses;
	std::vector<std::string_view> bus_names;

	Stop* stop = catalogue.GetStop(query);

	if (stop != nullptr) {
		unique_buses = catalogue.GetUniqueBuses(stop);

		if (unique_buses.size() == 0u) {
			os << "Stop "s << query << ": no buses"s << std::endl;
		}

		else {
			os << "Stop "s << query << ": buses "s;

			for (const Bus* bus : unique_buses) {
				bus_names.push_back(bus->name);
			}

			std::sort(bus_names.begin(), bus_names.end());

			for (std::string_view bus_name : bus_names) {
				os << bus_name << " "s;
			}
			os << std::endl;
		}
	}

	else {
		os << "Stop "s << query << ": not found"s << std::endl;
	}
}

void PrintQuery(TransportCatalogue& catalogue, std::string_view query, std::ostream& os) {
	using namespace std::string_literals;

	if (query.substr(0u, 3u) == "Bus"s) {
		PrintBusQuery(catalogue, query, os);
	}
	else if (query.substr(0u, 4u) == "Stop"s) {
		PrintStopQuery(catalogue, query, os);
	}
	else {
		os << "Error query"s << std::endl;
	}
}

void OutputStream(TransportCatalogue& catalogue, std::ostream& os, std::istream& is) {
	std::string count;
	std::getline(is, count);
	
	size_t size = std::stoi(count);

	std::vector<std::string> query;
	std::string buffer;

	for (size_t i = 0; i < size; ++i) {
		std::getline(is, buffer);
		query.push_back(buffer);
	}

	for (std::string str : query) {
		PrintQuery(catalogue, str, os);
	}
}

} // namespace iostream

} // namespace detail

} // namespace transport_catalogue
