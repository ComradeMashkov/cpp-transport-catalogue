#pragma once

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <variant>

namespace transport_catalogue {

namespace detail {

namespace router {

using namespace domain;
using namespace graph;

class TransportRouter {
public:
	// Setters

	void SetRoutingSettings(RoutingSettings routing_settings);
	void SetStops(const std::deque<Stop*>& stops);
	void SetGraph(TransportCatalogue& catalogue);

	// Getters

	const std::variant<StopEdge, BusEdge>& GetEdgeAt(EdgeId id) const;
	std::optional<WaitRange> GetRouteAtStop(Stop* stop) const;
	std::optional<RouteGraphInfo> GetRouteGraphInfo(VertexId start, VertexId end) const;
	std::deque<Stop*> GetStopsPointers(TransportCatalogue& catalogue) const;
	std::deque<Bus*> GetBusesPointers(TransportCatalogue& catalogue) const;

	// Init methods

	void BuildRouter(TransportCatalogue& catalogue);

	void AddEdgeToStops();
	void AddEdgeToBuses(TransportCatalogue& catalogue);
	Edge<double> MakeEdgeToBus(Stop* start, Stop* end, const double distance) const;

	template <typename Iterator>
	void MakeEdgesFromBuses(Iterator first, Iterator last, const Bus* bus, const TransportCatalogue& catalogue);

private:
	std::unordered_map<Stop*, WaitRange> stop_to_route_;
	std::unordered_map<EdgeId, std::variant<StopEdge, BusEdge>> edge_id_to_edge_;

	std::unique_ptr<DirectedWeightedGraph<double>> graph_;
	std::unique_ptr<Router<double>> router_;

	RoutingSettings routing_settings_;
};

template <typename Iterator>
void TransportRouter::MakeEdgesFromBuses(Iterator first, Iterator last, const Bus* bus, const TransportCatalogue& catalogue) {
	for (auto it = first; it != last; ++it) {
		size_t dist = 0;
		size_t span = 0;

		for (auto it_next = std::next(it); it_next != last; ++it_next) {
			dist += catalogue.GetDistanceBetweenStops(*prev(it_next), *it_next);
			++span;

			EdgeId id = graph_->AddEdge(MakeEdgeToBus(*it, *it_next, dist));

			edge_id_to_edge_[id] = BusEdge{ bus->name, span, graph_->GetEdge(id).weight };
		}
	}
}

} // namespace router

} // namespace detail

} // namespace transport_catalogue