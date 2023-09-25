#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

#include <sstream>

using namespace transport_catalogue;
using namespace transport_catalogue::detail::json;
using namespace map_renderer;

namespace request_handler {

using BusPoints = std::vector<std::pair<std::string_view, std::vector<geo::Coordinates>>>;

class RequestHandler {
public:
	RequestHandler() = default;

	std::vector<geo::Coordinates> GetStopsCoordinates(TransportCatalogue& catalogue) const;

	BusQuery GetBusQuery(TransportCatalogue& catalogue, std::string_view query);
	StopQuery GetStopQuery(TransportCatalogue& catalogue, std::string_view query);

	Node MakeStopNode(int id, StopQuery query);
	Node MakeBusNode(int id, BusQuery query);
	Node MakeMapNode(int id, TransportCatalogue& catalogue, RenderSettings& render_settings);

	void HandleRequestType(TransportCatalogue& catalogue, std::vector<Stat>& stats, RenderSettings& render_settings);
	void Render(MapRenderer& map_renderer, TransportCatalogue& catalogue) const;

	const Document& GetDocument();

private:
	std::vector<std::string_view> GetSortedBusesNames(TransportCatalogue& catalogue) const;

	Document document_;
};

} // namespace request_handler