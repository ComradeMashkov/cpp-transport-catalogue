#pragma once

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <sstream>

using namespace transport_catalogue;
using namespace transport_catalogue::detail::json;
using namespace map_renderer;
using namespace router;

namespace request_handler {

class RequestHandler {
public:
	RequestHandler() = default;

	Document HandleRequest(TransportCatalogue& catalogue, std::vector<Stat>& stats, RenderSettings& render_settings, RoutingSettings& routing_settings);

private:
	Reader reader_;
};

} // namespace request_handler