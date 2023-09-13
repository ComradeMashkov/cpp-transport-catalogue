#include "input_reader.h"

#include <iostream>

namespace transport_catalogue {

namespace detail {

namespace iostream {

Stop GetStopQuery(std::string_view query) {
	const auto COLON_POS = query.find(':');
	const auto COMMA_POS = query.find(',');

	Stop stop;
	stop.name = query.substr(STOP_NAME_POS, COLON_POS - STOP_NAME_POS);
	stop.coords.lat = std::stod(std::string(query).substr(COLON_POS + 2u, COMMA_POS - COLON_POS - 2u));
	stop.coords.lng = std::stod(std::string(query).substr(COMMA_POS + 2u));

	return stop;
}

Bus GetBusQuery(TransportCatalogue& catalogue, std::string_view query) {
	const auto COLON_POS = query.find(':');

	Bus bus;
	bus.name = query.substr(BUS_NAME_POS, COLON_POS - BUS_NAME_POS);

	query = query.substr(COLON_POS + 2u);

	auto GREATER_THAN_POS = query.find('>');
	if (GREATER_THAN_POS == std::string::npos) {
		auto DASH_POS = query.find('-');

		while (DASH_POS != std::string::npos) {
			bus.stops.push_back(catalogue.GetStop(query.substr(0u, DASH_POS - 1u)));
			query = query.substr(DASH_POS + 2u);
			DASH_POS = query.find('-');
		}

		bus.stops.push_back(catalogue.GetStop(query.substr(0u, DASH_POS - 1u)));

		for (size_t i = bus.stops.size() - 1u; i > 0; --i) {
			bus.stops.push_back(bus.stops[i - 1u]);
		}
	}

	else {
		while (GREATER_THAN_POS != std::string::npos) {
			bus.stops.push_back(catalogue.GetStop(query.substr(0u, GREATER_THAN_POS - 1u)));
			query = query.substr(GREATER_THAN_POS + 2u);
			GREATER_THAN_POS = query.find('>');
		}

		bus.stops.push_back(catalogue.GetStop(query.substr(0u, GREATER_THAN_POS - 1u)));
	}

	return bus;
}

std::vector<Distance> GetDistanceQuery(TransportCatalogue& catalogue, std::string_view query) {
	std::vector<Distance> result;

	const auto COLON_POS = query.find(':');

	std::string from = std::string(query).substr(STOP_NAME_POS, COLON_POS - STOP_NAME_POS);

	query = query.substr(query.find(',') + 1u);
	query = query.substr(query.find(',') + 2u);

	while (query.find(',') != std::string::npos) {
		int distance = std::stoi(std::string(query).substr(0u, query.find('m')));
		std::string to = std::string(query).substr(query.find('m') + STOP_NAME_POS);
		to = to.substr(0u, to.find(','));

		result.push_back({ catalogue.GetStop(from), catalogue.GetStop(to), distance });

		query = query.substr(query.find(',') + 2u);
	}

	int distance = std::stoi(std::string(query).substr(0u, query.find('m')));
	std::string to = std::string(query).substr(query.find('m') + STOP_NAME_POS);
	
	result.push_back({ catalogue.GetStop(from), catalogue.GetStop(to), distance });
	return result;
}

void InputStream(TransportCatalogue& catalogue, std::istream& is) {
	std::string count;
	std::getline(is, count);

	if (!count.empty()) {
		std::string query;
		std::vector<std::string> buses;
		std::vector<std::string> stops;
		size_t size = std::stoi(count);

		for (size_t i = 0u; i < size; ++i) {
			std::getline(is, query);

			if (!query.empty()) {
				const auto SPACE_POS = query.find_first_not_of(' ');
				query = query.substr(SPACE_POS);

				if (query.substr(0u, 3u) != "Bus") {
					stops.push_back(query);
				}
				else {
					buses.push_back(query);
				}
			}
		}

		for (const auto& stop : stops) {
			catalogue.AddStop(GetStopQuery(stop));
			
		}

		for (const auto& stop : stops) {
			for (const auto& distance : GetDistanceQuery(catalogue, stop)) {
				catalogue.AddDistance(distance);
			}
		}

		for (const auto& bus : buses) {
			catalogue.AddBus(GetBusQuery(catalogue, bus));
		}
	}
}

} // namespace iostream

} // namespace detail

} // namespace transport_catalogue