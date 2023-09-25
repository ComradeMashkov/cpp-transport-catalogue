#include "map_renderer.h"

namespace map_renderer {

MapRenderer::MapRenderer(RenderSettings& render_settings)
	: render_settings_(render_settings) {
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
	return { (coords.lng - min_lon_) * zoom_coeff_ + padding_, (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
}

SphereProjector MapRenderer::GetSphereProjector(const std::vector<geo::Coordinates>& points) const {
	return SphereProjector(points.begin(), points.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
}

void MapRenderer::InitSphereProjector(std::vector<geo::Coordinates> points) {
	sphere_projector_ = SphereProjector(points.begin(), points.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
}

RenderSettings MapRenderer::GetRenderSettings() const {
	return render_settings_;
}

size_t MapRenderer::GetPaletteSize() const {
	return render_settings_.color_palette.size();
}

svg::Color MapRenderer::GetColor(int line) const {
	return render_settings_.color_palette[line];
}

void MapRenderer::SetLineProperties(svg::Polyline& polyline, int line) const {
	polyline.SetStrokeColor(GetColor(line));
	polyline.SetFillColor("none");
	polyline.SetStrokeWidth(render_settings_.line_width);
	polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
	polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}

void MapRenderer::SetRouteTextCommonProperties(svg::Text& text, svg::Point position, const std::string& data) const {
	text.SetPosition(position);
	text.SetOffset({ render_settings_.bus_label_offset.first, render_settings_.bus_label_offset.second });
	text.SetFontSize(render_settings_.bus_label_font_size);
	text.SetFontFamily("Verdana");
	text.SetFontWeight("bold");
	text.SetData(data);
}

void MapRenderer::SetRouteTextAdditionalProperties(svg::Text& text, svg::Point position, const std::string& data) const {
	SetRouteTextCommonProperties(text, position, data);
	text.SetFillColor(render_settings_.underlayer_color);
	text.SetStrokeColor(render_settings_.underlayer_color);
	text.SetStrokeWidth(render_settings_.underlayer_width);
	text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
}


void MapRenderer::SetRouteTextColorProperties(svg::Text& text, svg::Point position, const std::string& data, int color) const {
	SetRouteTextCommonProperties(text, position, data);
	text.SetFillColor(GetColor(color));
}

void MapRenderer::SetStopCirclesProperties(svg::Circle& circle, svg::Point position) const {
	circle.SetCenter(position);
	circle.SetRadius(render_settings_.stop_radius);
	circle.SetFillColor("white");
}

void MapRenderer::SetStopTextCommonProperties(svg::Text& text, svg::Point position, const std::string& data) const {
	text.SetPosition(position);
	text.SetOffset({ render_settings_.stop_label_offset.first, render_settings_.stop_label_offset.second });
	text.SetFontSize(render_settings_.stop_label_font_size);
	text.SetFontFamily("Verdana");
	text.SetData(data);
}

void MapRenderer::SetStopTextAdditionalProperties(svg::Text& text, svg::Point position, const std::string& data) const {
	SetStopTextCommonProperties(text, position, data);
	text.SetFillColor(render_settings_.underlayer_color);
	text.SetStrokeColor(render_settings_.underlayer_color);
	text.SetStrokeWidth(render_settings_.underlayer_width);
	text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
}

void MapRenderer::SetStopTextColorProperties(svg::Text& text, svg::Point position, const std::string& data) const {
	SetStopTextCommonProperties(text, position, data);
	text.SetFillColor("black");
}

void MapRenderer::RenderLine(BusPalette& bus_palette) {
	std::vector<geo::Coordinates> stops_coords;

	for (auto [bus, palette] : bus_palette) {
		for (auto& stop : bus->stops) {
			geo::Coordinates coords;
			coords.lat = stop->coords.lat;
			coords.lng = stop->coords.lng;

			stops_coords.push_back(coords);
		}

		svg::Polyline polyline;
		bool flag = true;

		for (auto& coord : stops_coords) {
			flag = false;
			polyline.AddPoint(sphere_projector_(coord));
		}

		if (!flag) {
			SetLineProperties(polyline, palette);
			map_.Add(polyline);
		}
		stops_coords.clear();
	}
}

void MapRenderer::RenderBusesNames(BusPalette& bus_palette) {
	std::vector<geo::Coordinates> stops_coords;
	bool flag = true;

	for (auto [bus, palette] : bus_palette) {
		for (auto& stop : bus->stops) {
			geo::Coordinates coords;
			coords.lat = stop->coords.lat;
			coords.lng = stop->coords.lng;

			stops_coords.push_back(coords);

			if (flag) {
				flag = false;
			}
		}

		struct RouteData {
			svg::Text name;
			svg::Text title;
		};

		RouteData roundtrip;
		RouteData not_roundtrip;

		if (!flag) {
			if (bus->is_roundtrip) {
				SetRouteTextAdditionalProperties(roundtrip.name, sphere_projector_(stops_coords[0]), bus->name);
				map_.Add(roundtrip.name);
				SetRouteTextColorProperties(roundtrip.title, sphere_projector_(stops_coords[0]), bus->name, palette);
				map_.Add(roundtrip.title);
			}
			else {
				SetRouteTextAdditionalProperties(roundtrip.name, sphere_projector_(stops_coords[0]), bus->name);
				map_.Add(roundtrip.name);
				SetRouteTextColorProperties(roundtrip.title, sphere_projector_(stops_coords[0]), bus->name, palette);
				map_.Add(roundtrip.title);

				if (stops_coords[0] != stops_coords[stops_coords.size() / 2]) {
					SetRouteTextAdditionalProperties(not_roundtrip.name, sphere_projector_(stops_coords[stops_coords.size() / 2]), bus->name);
					map_.Add(not_roundtrip.name);
					SetRouteTextColorProperties(not_roundtrip.title, sphere_projector_(stops_coords[stops_coords.size() / 2]), bus->name, palette);
					map_.Add(not_roundtrip.title);
				}
			}
		}

		flag = false;
		stops_coords.clear();
	}
}

void MapRenderer::RenderStopsCircles(StopsNames& stops_names) {
	std::vector<geo::Coordinates> stops_coords;
	svg::Circle circle;

	for (auto stop : stops_names) {
		if (stop) {
			geo::Coordinates coords;
			coords.lat = stop->coords.lat;
			coords.lng = stop->coords.lng;

			SetStopCirclesProperties(circle, sphere_projector_(coords));
			map_.Add(circle);
		}
	}
}

void MapRenderer::RenderStopsNames(StopsNames& stops_names) {
	std::vector<geo::Coordinates> stops_coords;

	struct StopData {
		svg::Text name;
		svg::Text title;
	};

	StopData stop_data;

	for (auto stop : stops_names) {
		if (stop) {
			geo::Coordinates coords;
			coords.lat = stop->coords.lat;
			coords.lng = stop->coords.lng;

			SetStopTextAdditionalProperties(stop_data.name, sphere_projector_(coords), stop->name);
			map_.Add(stop_data.name);

			SetStopTextColorProperties(stop_data.title, sphere_projector_(coords), stop->name);
			map_.Add(stop_data.title);
		}
	}
}

void MapRenderer::RenderMap(std::ostream& os) {
	map_.Render(os);
}

} // namespace map_renderer