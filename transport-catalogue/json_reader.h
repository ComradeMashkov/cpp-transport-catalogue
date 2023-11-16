#pragma once

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <sstream>

namespace transport_catalogue {

namespace detail {

namespace json {
	
class Reader {
public:
	Reader() = default;
	Reader(Document document);
	Reader(std::istream& is);

public:
	void ParseQuery(TransportCatalogue& catalogue, std::vector<Stat>& stats, map_renderer::RenderSettings& render_settings, router::RoutingSettings& routing_settings);
	void ParseNodeMakeBase(TransportCatalogue& catalogue, map_renderer::RenderSettings& render_settings, router::RoutingSettings& routing_settings, serialization::SerializationSettings& serialization_settings);
	void ParseNodeProcessRequests(std::vector<Stat>& stats, serialization::SerializationSettings& serialization_settings);

private:
	void ParseNode(const Node& root, TransportCatalogue& catalogue, std::vector<Stat>& stats, map_renderer::RenderSettings& render_settings, router::RoutingSettings& routing_settings);
	void ParseNodeBase(const Node& root, TransportCatalogue& catalogue);
	void ParseNodeStat(const Node& root, std::vector<Stat>& stats);

private:
	void ParseNodeRenderGeneric(map_renderer::RenderSettings& render_settings, Dict render_map);
	void ParseNodeRenderColor(map_renderer::RenderSettings& render_settings, Dict render_map);
	void ParseNodeRender(const Node& node, map_renderer::RenderSettings& render_settings);
	void ParseNodeRoute(const Node& node, router::RoutingSettings& routing_settings);
	void ParseNodeSerialization(const Node& node, serialization::SerializationSettings& serialization_settings);

public:
	Node MakeStopNode(int id, StopQuery query);
	Node MakeBusNode(int id, BusQuery query);
	Node MakeMapNode(int id, TransportCatalogue& catalogue, map_renderer::RenderSettings& render_settings);
	Node MakeRouteNode(const Stat& stat, TransportCatalogue& catalogue, router::TransportRouter& router);

	void FillMap(map_renderer::MapRenderer& map_renderer, TransportCatalogue& catalogue) const;

private:
	Stop ParseNodeStop(Node& node);
	Bus ParseNodeBus(Node& node, TransportCatalogue& catalogue);
	std::vector<Distance> ParseNodeDistances(Node& node, TransportCatalogue& catalogue);

private:
	Document document_;

private:
	struct EdgeGetter {
		Node operator()(const StopEdge& edge_info) {
			return Builder{}.StartDict().Key("type").Value("Wait").Key("stop_name").Value(std::string(edge_info.name)).Key("time").Value(edge_info.time).EndDict().Build();
		}

		Node operator()(const BusEdge& edge_info) {
			return Builder{}.StartDict().Key("type").Value("Bus").Key("bus").Value(std::string(edge_info.name)).Key("span_count").Value(static_cast<int>(edge_info.span_count)).Key("time").Value(edge_info.time).EndDict().Build();
		}
	};
};

} // namespace json

} // namespace detail

} // namespace transport_catalogue