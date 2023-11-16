#include <fstream>
#include <iostream>

#include "json_reader.h"
#include "request_handler.h"

using namespace std;

using namespace transport_catalogue;
using namespace transport_catalogue::detail::json;
using namespace transport_catalogue::detail::router;

using namespace map_renderer;
using namespace request_handler;
using namespace serialization;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    TransportCatalogue catalogue;

    RenderSettings render_settings;
    RoutingSettings routing_settings;

    SerializationSettings serialization_settings;

    Reader reader;
    vector<Stat> stats;

    ifstream in("test_in.json");
    ofstream out("test_out.json");

    if (mode == "make_base"sv) {

        reader = Reader(cin);
        reader.ParseNodeMakeBase(catalogue, render_settings, routing_settings, serialization_settings);
        ofstream out_file(serialization_settings.file_name, ios::binary);
        SerializeTransportCatalogue(catalogue, render_settings, routing_settings, out_file);

    } 

    else if (mode == "process_requests"sv) {

        reader = Reader(cin);

        reader.ParseNodeProcessRequests(stats, serialization_settings);

        ifstream in_file(serialization_settings.file_name, ios::binary);
        TransportCatalogueUnion catalogue_union = DeserializeTransportCatalogue(in_file);

        RequestHandler request_handler;

        Print(request_handler.HandleRequest(catalogue_union.transport_catalogue_, stats, catalogue_union.render_settings_, catalogue_union.routing_settings_), cout); 

    } else {
        PrintUsage();
        return 1;
    }
}