#pragma once

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

#include <sstream>

namespace transport_catalogue {

namespace detail {

namespace json {
	
class Reader {
public:
	Reader() = default;
	Reader(Document document);
	Reader(std::istream& is);

	void ParseQuery(TransportCatalogue& catalogue, std::vector<Stat>& stats, map_renderer::RenderSettings& render_settings);

private:
	void ParseNode(const Node& root, TransportCatalogue& catalogue, std::vector<Stat>& stats, map_renderer::RenderSettings& render_settings);
	void ParseNodeBase(const Node& root, TransportCatalogue& catalogue);
	void ParseNodeStat(const Node& root, std::vector<Stat>& stats);

private:
	void ParseNodeRenderGeneric(map_renderer::RenderSettings& render_settings, Dict render_map);
	void ParseNodeRenderColor(map_renderer::RenderSettings& render_settings, Dict render_map);
	void ParseNodeRender(const Node& node, map_renderer::RenderSettings& render_settings);

public:
	Node MakeStopNode(int id, StopQuery query);
	Node MakeBusNode(int id, BusQuery query);
	Node MakeMapNode(int id, TransportCatalogue& catalogue, map_renderer::RenderSettings& render_settings);

	void FillMap(map_renderer::MapRenderer& map_renderer, TransportCatalogue& catalogue) const;

private:
	Stop ParseNodeStop(Node& node);
	Bus ParseNodeBus(Node& node, TransportCatalogue& catalogue);
	std::vector<Distance> ParseNodeDistances(Node& node, TransportCatalogue& catalogue);

private:
	Document document_;
};

} // namespace json

} // namespace detail

} // namespace transport_catalogue