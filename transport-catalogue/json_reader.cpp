#include "json_reader.h"

using namespace std::literals;

namespace transport_catalogue {

namespace detail {

namespace json {

Reader::Reader(Document document)
	: document_(std::move(document)) {
}

Reader::Reader(std::istream& is)
	: document_(Load(is)) {
}

void Reader::ParseQuery(TransportCatalogue& catalogue, std::vector<Stat>& stats, map_renderer::RenderSettings& render_settings, router::RoutingSettings& routing_settings) {
	ParseNode(document_.GetRoot(), catalogue, stats, render_settings, routing_settings);
}

void Reader::ParseNode(const Node& root, TransportCatalogue& catalogue, std::vector<Stat>& stats, map_renderer::RenderSettings& render_settings, router::RoutingSettings& routing_settings) {
	Dict dict;

	if (root.IsDict()) {
		dict = root.AsDict();

		try {
			ParseNodeBase(dict.at("base_requests"), catalogue);
		}
		catch (...) {
			std::cout << "Failed to parse a query: base_requests is empty"sv;
		}

		try {
			ParseNodeStat(dict.at("stat_requests"), stats);
		}
		catch (...) {

		}

		try {
			ParseNodeRender(dict.at("render_settings"), render_settings);
		}
		catch (...) {
			std::cout << "Failed to parse a query: render settings are empty"sv;
		}

		try {
			ParseNodeRoute(dict.at("routing_settings"), routing_settings);
		}
		catch (...) {
			std::cout << "Failed to parse a query: route settings are empty";
		}
	}

	else {
		std::cout << "Failed to parse a query: root is not a map-type"sv;
	}
}

void Reader::ParseNodeBase(const Node& root, TransportCatalogue& catalogue) {
	Array base_requests;
	Dict dict;
	Node node;

	std::vector<Node> buses;
	std::vector<Node> stops;

	if (root.IsArray()) {
		base_requests = root.AsArray();

		for (auto base : base_requests) {
			if (base.IsDict()) {
				dict = base.AsDict();

				try {
					node = dict.at("type");
					if (node.IsString()) {
						if (node.AsString() == "Bus") {
							buses.push_back(dict);
						}
						else if (node.AsString() == "Stop") {
							stops.push_back(dict);
						}
						else {
							std::cout << "Failed to parse a query: base_requests have a wrong type"sv;
						}
					}
				}

				catch (...) {
					std::cout << "Failed to parse a query: base_requests don't have a \"type\" value"sv;
				}
			}
		}

		for (auto stop : stops) {
			catalogue.AddStop(ParseNodeStop(stop));
		}

		for (auto stop : stops) {
			for (const auto& distance : ParseNodeDistances(stop, catalogue)) {
				catalogue.AddDistance(distance);
			}
		}

		for (auto bus : buses) {
			catalogue.AddBus(ParseNodeBus(bus, catalogue));
		}
	}

	else {
		std::cout << "Failed to parse a query: base_requests is not an array-type"sv;
	}
}

void Reader::ParseNodeStat(const Node& node, std::vector<Stat>& stats) {
	Array array;
	Dict dict;
	Stat stat_node;

	if (node.IsArray()) {
		array = node.AsArray();

		for (auto node : array) {

			if (node.IsDict()) {
				dict = node.AsDict();
				stat_node.id = dict.at("id").AsInt();
				stat_node.type = dict.at("type").AsString();

				if ((stat_node.type == "Bus") || (stat_node.type == "Stop")) {
					stat_node.name = dict.at("name").AsString();
					stat_node.from = "";
					stat_node.to = "";
				}

				else {
					stat_node.name = "";
					if (stat_node.type == "Route") {
						stat_node.from = dict.at("from").AsString();
						stat_node.to = dict.at("to").AsString();
					}
					else {
						stat_node.from = "";
						stat_node.to = "";
					}
				}

				stats.push_back(stat_node);
			}
		}

	}

	else {
		std::cout << "Failed to parse a query: base_requests is not an array-type"sv;
	}
}

void Reader::ParseNodeRenderGeneric(map_renderer::RenderSettings& render_settings, Dict render_map) {
	Array bus_labels_offset;
	Array stop_labels_offset;

	render_settings.width = render_map.at("width").AsDouble();
	render_settings.height = render_map.at("height").AsDouble();
	render_settings.padding = render_map.at("padding").AsDouble();
	render_settings.line_width = render_map.at("line_width").AsDouble();
	render_settings.stop_radius = render_map.at("stop_radius").AsDouble();
	render_settings.bus_label_font_size = render_map.at("bus_label_font_size").AsInt();

	if (render_map.at("bus_label_offset").IsArray()) {
		bus_labels_offset = render_map.at("bus_label_offset").AsArray();
		render_settings.bus_label_offset = std::make_pair(bus_labels_offset[0].AsDouble(), bus_labels_offset[1].AsDouble());
	}

	render_settings.stop_label_font_size = render_map.at("stop_label_font_size").AsInt();

	if (render_map.at("stop_label_offset").IsArray()) {
		stop_labels_offset = render_map.at("stop_label_offset").AsArray();
		render_settings.stop_label_offset = std::make_pair(stop_labels_offset[0].AsDouble(), stop_labels_offset[1].AsDouble());
	}
}

void Reader::ParseNodeRenderColor(map_renderer::RenderSettings& render_settings, Dict render_map) {
	Array colors;
	Array color_palette;

	uint8_t red;
	uint8_t green;
	uint8_t blue;
	double opacity;

	if (render_map.at("underlayer_color").IsString()) {
		render_settings.underlayer_color = svg::Color(render_map.at("underlayer_color").AsString());
	}
	else if (render_map.at("underlayer_color").IsArray()) {
		colors = render_map.at("underlayer_color").AsArray();
		red = colors[0].AsInt();
		green = colors[1].AsInt();
		blue = colors[2].AsInt();

		if (colors.size() == 4u) {
			opacity = colors[3].AsDouble();
			render_settings.underlayer_color = svg::Color(svg::Rgba(red, green, blue, opacity));
		}
		else if (colors.size() == 3u) {
			render_settings.underlayer_color = svg::Color(svg::Rgb(red, green, blue));
		}
	}

	render_settings.underlayer_width = render_map.at("underlayer_width").AsDouble();

	if (render_map.at("color_palette").IsArray()) {
		color_palette = render_map.at("color_palette").AsArray();

		for (auto palette : color_palette) {
			if (palette.IsString()) {
				render_settings.color_palette.push_back(svg::Color(palette.AsString()));
			}
			else if (palette.IsArray()) {
				colors = palette.AsArray();
				red = colors[0].AsInt();
				green = colors[1].AsInt();
				blue = colors[2].AsInt();

				if (colors.size() == 4u) {
					opacity = colors[3].AsDouble();
					render_settings.color_palette.push_back(svg::Color(svg::Rgba(red, green, blue, opacity)));
				}
				else if (colors.size() == 3u) {
					render_settings.color_palette.push_back(svg::Color(svg::Rgb(red, green, blue)));
				}
			}
		}
	}
}

void Reader::ParseNodeRender(const Node& node, map_renderer::RenderSettings& render_settings) {
	Dict render_map = node.AsDict();

	if (node.IsDict()) {
		ParseNodeRenderGeneric(render_settings, render_map);
		ParseNodeRenderColor(render_settings, render_map);
	}

	else {
		std::cout << "Failed to parse a query: render_settings is not a map-type"sv;
	}
}

void Reader::ParseNodeRoute(const Node& node, router::RoutingSettings& routing_settings) {
	Dict route;

	if (node.IsDict()) {
		route = node.AsDict();

		try {
			routing_settings.bus_wait_time = route.at("bus_wait_time").AsDouble();
			routing_settings.bus_velocity = route.at("bus_velocity").AsDouble();
		}
		catch (...) {
			std::cout << "Failed to parse route settings";
		}
	}

	else {
		std::cout << "Failed to parse route settings: it is not a map";
	}
}

Node Reader::MakeStopNode(int id, StopQuery query) {
	Node result;
	Builder builder;

	if (!query.query_exists) {
		builder.StartDict().Key("request_id"s).Value(id).Key("error_message"s).Value("not found"s).EndDict();
		result = builder.Build();
	}

	else {
		builder.StartDict().Key("request_id"s).Value(id).Key("buses"s).StartArray();

		for (std::string bus_name : query.buses) {
			builder.Value(bus_name);
		}

		builder.EndArray().EndDict();
		result = builder.Build();
	}

	return result;
}

Node Reader::MakeBusNode(int id, BusQuery query) {
	Node result;
	Builder builder;

	if (!query.query_exists) {
		result = builder.StartDict().Key("request_id"s).Value(id).Key("error_message"s).Value("not found"s).EndDict().Build();
	}

	else {
		result = builder.StartDict().Key("request_id"s).Value(id)
		.Key("curvature"s).Value(query.curvature)
		.Key("route_length"s).Value(query.route_length)
		.Key("stop_count"s).Value(query.route_stops)
		.Key("unique_stop_count"s).Value(query.unique_stops)
		.EndDict().Build();

	}

	return result;
}

Node Reader::MakeMapNode(int id, TransportCatalogue& catalogue, map_renderer::RenderSettings& render_settings) {
	Node result;
	Builder builder;

	map_renderer::MapRenderer map_renderer(render_settings);
	std::ostringstream map_stream;
	std::string map_string;

	map_renderer.InitSphereProjector(catalogue.GetStopsCoordinates());
	FillMap(map_renderer, catalogue);
	map_renderer.RenderMap(map_stream);
	map_string = map_stream.str();

	result = builder.StartDict().Key("request_id"s).Value(id).Key("map"s).Value(map_string).EndDict().Build();

	return result;
}

Node Reader::MakeRouteNode(Stat& stat, TransportCatalogue& catalogue, router::TransportRouter& router) {
	const auto route_info = router.GetRouteGraphInfo(catalogue.GetStop(stat.from), catalogue.GetStop(stat.to));

	if (!route_info) {
		return Builder{}.StartDict().Key("request_id").Value(stat.id).Key("error_message").Value("not found").EndDict().Build();
	}

	Array items;
	for (const auto& item : route_info->edges) {
		items.emplace_back(std::visit(EdgeGetter{}, item));
	}

	return Builder{}.StartDict().Key("request_id").Value(stat.id).Key("total_time").Value(route_info->total_time).Key("items").Value(items).EndDict().Build();
}

void Reader::FillMap(map_renderer::MapRenderer& map_renderer, TransportCatalogue& catalogue) const {
	map_renderer::MapRenderer::BusPalette bus_palette;
	map_renderer::MapRenderer::StopsNames stops_names_sorted;

	int palette_size = 0u;
	int palette_index = 0u;

	palette_size = map_renderer.GetPaletteSize();
	if (palette_size == 0u) {
		std::cout << "Color palette is empty";
		return;
	}

	auto buses = catalogue.GetBusesAssociative();
	if (buses.size() > 0u) {
		for (auto bus_name : catalogue.GetSortedBusesNames()) {
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

Stop Reader::ParseNodeStop(Node& node) {
	Stop stop;
	Dict dict;

	if (node.IsDict()) {
		dict = node.AsDict();
		stop.name = dict.at("name").AsString();
		stop.coords.lat = dict.at("latitude").AsDouble();
		stop.coords.lng = dict.at("longitude").AsDouble();
	}

	return stop;
}

Bus Reader::ParseNodeBus(Node& node, TransportCatalogue& catalogue) {
	Bus bus;
	Dict dict;
	Array bus_stops;

	if (node.IsDict()) {
		dict = node.AsDict();
		bus.name = dict.at("name").AsString();
		bus.is_roundtrip = dict.at("is_roundtrip").AsBool();

		bus_stops = dict.at("stops").AsArray();

		for (auto stop : bus_stops) {
			bus.stops.push_back(catalogue.GetStop(stop.AsString()));
		}

		if (!bus.is_roundtrip) {
			size_t size = bus.stops.size() - 1u;

			for (size_t i = size; i > 0u; --i) {
				bus.stops.push_back(bus.stops[i - 1u]);
			}
		}
		
	}

	return bus;
}

std::vector<Distance> Reader::ParseNodeDistances(Node& node, TransportCatalogue& catalogue) {
	std::vector<Distance> distances;
	Dict stop;
	Dict road_map;
	std::string from;
	std::string to;
	int distance;

	if (node.IsDict()) {
		stop = node.AsDict();
		from = stop.at("name").AsString();

		road_map = stop.at("road_distances").AsDict();

		for (auto [key, value] : road_map) {
			to = key;
			distance = value.AsInt();
			distances.push_back({ catalogue.GetStop(from), catalogue.GetStop(to), distance });
		}
	}

	return distances;
}

} // namespace json

} // namespace detail

} // namespace transport_catalogue