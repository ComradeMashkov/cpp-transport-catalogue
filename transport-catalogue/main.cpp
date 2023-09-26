#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <fstream>

using namespace std;
using namespace transport_catalogue;
using namespace map_renderer;
using namespace request_handler;
using namespace transport_catalogue::detail::json;

int main() {

    std::vector<Stat> stat_request;
    map_renderer::RenderSettings render_settings;
    TransportCatalogue catalogue;

    std::ifstream file("input1.json");

    RequestHandler request_handler;

    Reader reader;
    reader = Reader(file);
    reader.ParseQuery(catalogue, stat_request, render_settings);

    Print(request_handler.HandleRequest(catalogue, stat_request, render_settings), cout);
}