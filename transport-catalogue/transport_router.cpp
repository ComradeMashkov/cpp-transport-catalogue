#include "transport_router.h"

namespace transport_catalogue {

namespace detail {

namespace router {

void TransportRouter::SetRoutingSettings(RoutingSettings routing_settings) {
	routing_settings_ = std::move(routing_settings);
}

void TransportRouter::BuildRouter(TransportCatalogue& catalogue) {
	SetGraph(catalogue);
	router_ = std::make_unique<Router<double>>(*graph_);
	router_->Build();
}

const std::variant<StopEdge, BusEdge>& TransportRouter::GetEdgeAt(EdgeId id) const {
	return edge_id_to_edge_.at(id);
}

std::optional<WaitRange> TransportRouter::GetRouteAtStop(Stop* stop) const {
	if (stop_to_route_.count(stop)) {
		return stop_to_route_.at(stop);
	}
	
	return std::nullopt;
}

std::optional<RouteGraphInfo> TransportRouter::GetRouteGraphInfo(VertexId start, graph::VertexId end) const {
	const auto route_info = router_->BuildRoute(start, end);
	
	if (route_info) {
		RouteGraphInfo result;
		result.total_time = route_info->weight;

		for (const auto& edge : route_info->edges) {
			result.edges.emplace_back(GetEdgeAt(edge));
		}

		return result;
	}

	return std::nullopt;
}

std::deque<Stop*> TransportRouter::GetStopsPointers(TransportCatalogue& catalogue) const {
	std::deque<Stop*> result;

	for (const auto& [_, stop_ptr] : catalogue.GetStopsAssociative()) {
		result.push_back(stop_ptr);
	}

	return result;
}

std::deque<Bus*> TransportRouter::GetBusesPointers(TransportCatalogue& catalogue) const {
	std::deque<Bus*> result;

	for (const auto& [_, bus_ptr] : catalogue.GetBusesAssociative()) {
		result.push_back(bus_ptr);
	}

	return result;
}

void TransportRouter::SetStops(const std::deque<Stop*>& stops) {
	size_t i = 0;

	for (const auto& stop : stops) {
		VertexId first = i++;
		VertexId second = i++;

		stop_to_route_[stop] = WaitRange{ first, second };
	}
}

void TransportRouter::AddEdgeToStops() {
	for (const auto& [stop, route] : stop_to_route_) {
		EdgeId id = graph_->AddEdge(Edge<double> {route.bus_wait_start, route.bus_wait_end, routing_settings_.bus_wait_time});
		edge_id_to_edge_[id] = StopEdge{ stop->name, routing_settings_.bus_wait_time };
	}
}

void TransportRouter::AddEdgeToBuses(TransportCatalogue& catalogue) {
	for (const auto& bus : GetBusesPointers(catalogue)) {
		MakeEdgesFromBuses(bus->stops.begin(), bus->stops.end(), bus, catalogue);

		if (!bus->is_roundtrip) {
			MakeEdgesFromBuses(bus->stops.rbegin(), bus->stops.rend(), bus, catalogue);
		}
	}
}

void TransportRouter::SetGraph(TransportCatalogue& catalogue) {
	const size_t stops_ptr_size = GetStopsPointers(catalogue).size();

	graph_ = std::make_unique<DirectedWeightedGraph<double>>(2 * stops_ptr_size);

	SetStops(GetStopsPointers(catalogue));

	AddEdgeToStops();
	AddEdgeToBuses(catalogue);
}

Edge<double> TransportRouter::MakeEdgeToBus(Stop* start, Stop* end, const double distance) const {
	Edge<double> result;

	result.from = stop_to_route_.at(start).bus_wait_end;
	result.to = stop_to_route_.at(end).bus_wait_start;
	result.weight = distance * 1.0 / (routing_settings_.bus_velocity * 1000 / 60);

	return result;
}

} // namespace router

} // namespace detail

} // namespace transport_catalogue
