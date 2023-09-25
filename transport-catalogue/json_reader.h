#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace transport_catalogue {

namespace detail {

namespace json {
	
class Reader {
public:
	Reader() = default;
	Reader(Document document);
	Reader(std::istream& is);

	void ParseQuery(TransportCatalogue& catalogue, std::vector<Stat>& stats, map_renderer::RenderSettings& render_settings);
	void ParseNode(const Node& root, TransportCatalogue& catalogue, std::vector<Stat>& stats, map_renderer::RenderSettings& render_settings);
	void ParseNodeBase(const Node& root, TransportCatalogue& catalogue);
	void ParseNodeStat(const Node& root, std::vector<Stat>& stats);
	void ParseNodeRender(const Node& node, map_renderer::RenderSettings& render_settings);
	
	Stop ParseNodeStop(Node& node);
	Bus ParseNodeBus(Node& node, TransportCatalogue& catalogue);
	std::vector<Distance> ParseNodeDistances(Node& node, TransportCatalogue& catalogue);

private:
	Document document_;
};

} // namespace json

} // namespace detail

} // namespace transport_catalogue