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

void Reader::ParseQuery(TransportCatalogue& catalogue, std::vector<Stat>& stats, map_renderer::RenderSettings& render_settings) {
	ParseNode(document_.GetRoot(), catalogue, stats, render_settings);
}

void Reader::ParseNode(const Node& root, TransportCatalogue& catalogue, std::vector<Stat>& stats, map_renderer::RenderSettings& render_settings) {
	Dict dict;

	if (root.IsMap()) {
		dict = root.AsMap();

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
			if (base.IsMap()) {
				dict = base.AsMap();

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

			if (node.IsMap()) {
				dict = node.AsMap();
				stat_node.id = dict.at("id").AsInt();
				stat_node.type = dict.at("type").AsString();

				if (stat_node.type != "Map") {
					stat_node.name = dict.at("name").AsString();
				}
				else {
					stat_node.name = "";
				}

				stats.push_back(stat_node);
			}
		}

	}

	else {
		std::cout << "Failed to parse a query: base_requests is not an array-type"sv;
	}
}

void Reader::ParseNodeRender(const Node& node, map_renderer::RenderSettings& render_settings) {
	Dict render_map;
	Array bus_labels_offset;
	Array stop_labels_offset;
	Array colors;
	Array color_palette;

	uint8_t red;
	uint8_t green;
	uint8_t blue;
	double opacity;

	if (node.IsMap()) {
		render_map = node.AsMap();

		try {
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

		catch (...) {
			std::cout << "Failed to parse a query: incorrect render settings"sv;
		}
	}

	else {
		std::cout << "Failed to parse a query: render_settings is not a map-type"sv;
	}
}

Stop Reader::ParseNodeStop(Node& node) {
	Stop stop;
	Dict dict;

	if (node.IsMap()) {
		dict = node.AsMap();
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

	if (node.IsMap()) {
		dict = node.AsMap();
		bus.name = dict.at("name").AsString();
		bus.is_roundtrip = dict.at("is_roundtrip").AsBool();

		try {
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

		catch (...) {
			std::cout << "Failed to parse a query: \"stops\" parameter is empty"sv << std::endl;
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

	if (node.IsMap()) {
		stop = node.AsMap();
		from = stop.at("name").AsString();

		try {
			road_map = stop.at("road_distances").AsMap();

			for (auto [key, value] : road_map) {
				to = key;
				distance = value.AsInt();
				distances.push_back({ catalogue.GetStop(from), catalogue.GetStop(to), distance });
			}
		}

		catch (...) {
			std::cout << "Failed to parse a query: invalid route"sv << std::endl;
		}
	}

	return distances;
}

} // namespace json

} // namespace detail

} // namespace transport_catalogue