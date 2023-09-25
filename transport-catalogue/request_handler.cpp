#include "request_handler.h"

using namespace std::string_literals;

namespace request_handler {

	std::vector<geo::Coordinates> RequestHandler::GetStopsCoordinates(TransportCatalogue& catalogue) const {
	std::vector<geo::Coordinates> result;
	auto buses = catalogue.GetBusesAssociative();

	for (auto& [name, bus] : buses) {
		for (auto& stop : bus->stops) {
			geo::Coordinates coordinates;
			coordinates.lat = stop->coords.lat;
			coordinates.lng = stop->coords.lng;

			result.push_back(coordinates);
		}
	}

	return result;
}

std::vector<std::string_view> RequestHandler::GetSortedBusesNames(TransportCatalogue& catalogue) const {
	std::vector<std::string_view> buses_names;

	auto buses = catalogue.GetBusesAssociative();
	if (buses.size() > 0u) {
		for (auto& [bus_name, bus] : buses) {
			buses_names.push_back(bus_name);
		}

		std::sort(buses_names.begin(), buses_names.end());

		return buses_names;
	}

	return {};
}

BusQuery RequestHandler::GetBusQuery(TransportCatalogue& catalogue, std::string_view query) {
	BusQuery result;
	Bus* bus = catalogue.GetBus(query);

	if (bus) {
		result.name = bus->name;
		result.query_exists = true;
		result.route_stops = bus->stops.size();
		result.unique_stops = catalogue.GetRouteInfo(bus).unique_stops.size();
		result.route_length = catalogue.GetRouteInfo(bus).distance;
		result.curvature = static_cast<double>(catalogue.GetRouteInfo(bus).distance) / catalogue.GetRouteInfo(bus).length;
		return result;
	}

	result.name = query;
	result.query_exists = false;
	return result;
}

StopQuery RequestHandler::GetStopQuery(TransportCatalogue& catalogue, std::string_view query) {
	std::unordered_set<const Bus*> unique_buses;

	StopQuery result;

	Stop* stop = catalogue.GetStop(query);

	if (stop) {
		result.name = stop->name;
		result.query_exists = true;
		unique_buses = catalogue.GetUniqueBuses(stop);

		if (unique_buses.size() > 0) {
			for (auto bus : unique_buses) {
				result.buses.push_back(bus->name);
			}

			std::sort(result.buses.begin(), result.buses.end());
		}

		return result;
	}

	result.name = query;
	result.query_exists = false;
	return result;
}

Node RequestHandler::MakeStopNode(int id, StopQuery query) {
	Dict result;
	Array buses;

	if (!query.query_exists) {
		result.emplace("request_id", Node{ id });
		result.emplace("error_message", Node{ "not found"s });
	}

	else {
		result.emplace("request_id", Node{ id });

		for (std::string bus_name : query.buses) {
			buses.push_back(Node{ bus_name });
		}

		result.emplace("buses", Node{ buses });
	}

	return Node{ result };
}

Node RequestHandler::MakeBusNode(int id, BusQuery query) {
	Dict result;

	if (!query.query_exists) {
		result.emplace("request_id", Node{ id });
		result.emplace("error_message", Node{ "not found"s });

	}

	else {
		result.emplace("request_id", Node{ id });
		result.emplace("curvature", Node{ query.curvature });
		result.emplace("route_length", Node{ query.route_length });
		result.emplace("stop_count", Node{ query.route_stops });
		result.emplace("unique_stop_count", Node{ query.unique_stops });

	}

	return Node{ result };
}

Node RequestHandler::MakeMapNode(int id, TransportCatalogue& catalogue, RenderSettings& render_settings) {
	Dict result;
	MapRenderer map_renderer(render_settings);
	std::ostringstream map_stream;
	std::string map_string;

	map_renderer.InitSphereProjector(GetStopsCoordinates(catalogue));
	Render(map_renderer, catalogue);
	map_renderer.RenderMap(map_stream);
	map_string = map_stream.str();

	result.emplace("request_id", Node(id));
	result.emplace("map", Node(map_string));

	return result;
}

void RequestHandler::HandleRequestType(TransportCatalogue& catalogue, std::vector<Stat>& stats, RenderSettings& render_settings) {
	std::vector<Node> result;

	for (auto stat : stats) {
		if (stat.type == "Stop") {
			result.push_back(MakeStopNode(stat.id, GetStopQuery(catalogue, stat.name)));
		}
		else if (stat.type == "Bus") {
			result.push_back(MakeBusNode(stat.id, GetBusQuery(catalogue, stat.name)));
		}
		else if (stat.type == "Map") {
			result.push_back(MakeMapNode(stat.id, catalogue, render_settings));
		}
	}

	document_ = Document{ Node(result) };
}

void RequestHandler::Render(MapRenderer& map_renderer, TransportCatalogue& catalogue) const {
	MapRenderer::BusPalette bus_palette;
	MapRenderer::StopsNames stops_names_sorted;

	int palette_size = 0u;
	int palette_index = 0u;

	palette_size = map_renderer.GetPaletteSize();
	if (palette_size == 0u) {
		std::cout << "Color palette is empty";
		return;
	}

	auto buses = catalogue.GetBusesAssociative();
	if (buses.size() > 0u) {
		for (auto bus_name : GetSortedBusesNames(catalogue)) {
			Bus* bus = catalogue.GetBus(bus_name);

			if (bus) {
				if (bus->stops.size() > 0u) {
					bus_palette.push_back(std::make_pair(bus, palette_index));
					++palette_index;

					if (palette_index == palette_size) {
						palette_index = 0u;
					}
				}
			}
		}

		if (bus_palette.size() > 0u) {
			map_renderer.RenderLine(bus_palette);
			map_renderer.RenderBusesNames(bus_palette);
		}
	}

	auto stops = catalogue.GetStopsAssociative();
	if (stops.size() > 0u) {
		std::vector<std::string_view> stops_names;

		for (auto& [stop_name, stop] : stops) {
			if (stop->buses.size() > 0u) {
				stops_names.push_back(stop_name);
			}
		}

		std::sort(stops_names.begin(), stops_names.end());

		for (auto stop_name : stops_names) {
			Stop* stop = catalogue.GetStop(stop_name);

			if (stop) {
				stops_names_sorted.push_back(stop);
			}
		}

		if (stops_names_sorted.size() > 0u) {
			map_renderer.RenderStopsCircles(stops_names_sorted);
			map_renderer.RenderStopsNames(stops_names_sorted);
		}
	}
}

const Document& RequestHandler::GetDocument() {
	return document_;
}

} // namespace request_handler