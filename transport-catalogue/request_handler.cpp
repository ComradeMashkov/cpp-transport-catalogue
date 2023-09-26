#include "request_handler.h"

using namespace std::string_literals;

namespace request_handler {

Document RequestHandler::HandleRequest(TransportCatalogue& catalogue, std::vector<Stat>& stats, RenderSettings& render_settings) {
	std::vector<Node> result;

	for (auto stat : stats) {
		if (stat.type == "Stop") {
			result.push_back(reader_.MakeStopNode(stat.id, catalogue.GetStopQuery(stat.name)));
		}
		else if (stat.type == "Bus") {
			result.push_back(reader_.MakeBusNode(stat.id, catalogue.GetBusQuery(stat.name)));
		}
		else if (stat.type == "Map") {
			result.push_back(reader_.MakeMapNode(stat.id, catalogue, render_settings));
		}
	}

	return Document{ Node(result) };
}

} // namespace request_handler