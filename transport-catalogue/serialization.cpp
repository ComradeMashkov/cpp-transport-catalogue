#include "serialization.h"

namespace serialization {

template <typename It>
uint32_t CalculateDistance(It range_begin, It range_end, std::string_view name) {
    auto it = std::find_if(range_begin, range_end, [&name] (const domain::Stop& stop) {
        return stop.name == name;
    });

    return std::distance(range_begin, it);
}

transport_catalogue_protobuf::TransportCatalogue SerializeTransportCatalogue(const transport_catalogue::TransportCatalogue& catalogue) {
    transport_catalogue_protobuf::TransportCatalogue catalogue_serialized;

    const auto& stops = catalogue.GetStops();
    const auto& buses = catalogue.GetBuses();
    const auto& distances = catalogue.GetDistances();

    int id = 0;
    for (const auto& stop : stops) {
        transport_catalogue_protobuf::Stop stop_serialized;

        stop_serialized.set_id(id);
        stop_serialized.set_name(stop.name);
        stop_serialized.set_latitude(stop.coords.lat);
        stop_serialized.set_longtitude(stop.coords.lng);

        *catalogue_serialized.add_stops() = std::move(stop_serialized);

        ++id;
    }

    for (const auto& bus : buses) {
 
        transport_catalogue_protobuf::Bus bus_serialized;
 
        bus_serialized.set_name(bus.name);
 
        for (auto stop : bus.stops) {
            uint32_t stop_id = CalculateDistance(stops.cbegin(), stops.cend(), stop->name);
            bus_serialized.add_stops(stop_id);
        }
 
        bus_serialized.set_is_roundtrip(bus.is_roundtrip);
        bus_serialized.set_route_length(bus.route_length);
 
        *catalogue_serialized.add_buses() = std::move(bus_serialized);
    }
    
    for (const auto& [pair_stops, pair_distance] : distances) {
 
        transport_catalogue_protobuf::Distance distance_serialized;
 
        distance_serialized.set_start(CalculateDistance(stops.cbegin(), stops.cend(), pair_stops.first->name));
                                              
        distance_serialized.set_end(CalculateDistance(stops.cbegin(), 
                                            stops.cend(), 
                                            pair_stops.second->name));
                                            
        distance_serialized.set_distance(pair_distance);
 
        *catalogue_serialized.add_distances() = std::move(distance_serialized);
    }
 
    return catalogue_serialized;
}

transport_catalogue::TransportCatalogue DeserializeTransportCatalogue(const transport_catalogue_protobuf::TransportCatalogue& catalogue_serialized) {
    transport_catalogue::TransportCatalogue catalogue;
    
    const auto& stops_serialized = catalogue_serialized.stops();
    const auto& buses_serialized = catalogue_serialized.buses();
    const auto& distances_serialized = catalogue_serialized.distances();
    
    for (const auto& stop : stops_serialized) {
        
        domain::Stop stop_tmp;
        
        stop_tmp.name = stop.name();
        stop_tmp.coords.lat = stop.latitude();
        stop_tmp.coords.lng = stop.longtitude();
        
        catalogue.AddStop(std::move(stop_tmp));
    }
    
    const auto& stops_tmp = catalogue.GetStops(); 
    
    std::vector<domain::Distance> distances;
    for (const auto& distance : distances_serialized) {
        
        domain::Distance distance_tmp;
        
        distance_tmp.from = catalogue.GetStop(stops_tmp[distance.start()].name);
        distance_tmp.to = catalogue.GetStop(stops_tmp[distance.end()].name);
        
        distance_tmp.distance = distance.distance();
        
        distances.push_back(distance_tmp);
    }
    
    for (const auto& distance : distances) {
        catalogue.AddDistance(distance);
    }     
    
    for (const auto& bus_proto : buses_serialized) {  
    
        domain::Bus bus_tmp;
        
        bus_tmp.name = bus_proto.name();
 
        for (auto stop_id : bus_proto.stops()) {
            auto name = stops_tmp[stop_id].name;            
            bus_tmp.stops.push_back(catalogue.GetStop(name));
        }
 
        bus_tmp.is_roundtrip = bus_proto.is_roundtrip();
        bus_tmp.route_length = bus_proto.route_length();
        
        catalogue.AddBus(std::move(bus_tmp));
    }   
    
    return catalogue;
}

transport_catalogue_protobuf::Color SerializeColor(const svg::Color& color) {
    transport_catalogue_protobuf::Color color_serialized;

    if (std::holds_alternative<std::monostate>(color)) {
        color_serialized.set_none(true);
    }

    else if (std::holds_alternative<svg::Rgb>(color)) {
        svg::Rgb rgb = std::get<svg::Rgb>(color);

        color_serialized.mutable_rgb()->set_red(rgb.red);
        color_serialized.mutable_rgb()->set_green(rgb.green);
        color_serialized.mutable_rgb()->set_blue(rgb.blue);
    }

    else if (std::holds_alternative<svg::Rgba>(color)) {
        svg::Rgba rgba = std::get<svg::Rgba>(color);

        color_serialized.mutable_rgba()->set_red(rgba.red);
        color_serialized.mutable_rgba()->set_green(rgba.green);
        color_serialized.mutable_rgba()->set_blue(rgba.blue);
        color_serialized.mutable_rgba()->set_opacity(rgba.opacity);
    }

    else if (std::holds_alternative<std::string>(color)) {
        color_serialized.set_string_color(std::get<std::string>(color));
    }

    return color_serialized;
}

svg::Color DeserializeColor(const transport_catalogue_protobuf::Color& color_serialized) {
    svg::Color color;
    
    if (color_serialized.has_rgb()) {
        svg::Rgb rgb;
        
        rgb.red = color_serialized.rgb().red();
        rgb.green = color_serialized.rgb().green();
        rgb.blue = color_serialized.rgb().blue();
        
        color = rgb;
        
    } 

    else if (color_serialized.has_rgba()) {
        svg::Rgba rgba;
        
        rgba.red = color_serialized.rgba().red();
        rgba.green = color_serialized.rgba().green();
        rgba.blue = color_serialized.rgba().blue();
        rgba.opacity = color_serialized.rgba().opacity();
        
        color = rgba;
        
    }
    
    else {
        color = color_serialized.string_color();
    }
    
    return color;
}

transport_catalogue_protobuf::RenderSettings SerializeRenderSettings(const map_renderer::RenderSettings& render_settings) {
    transport_catalogue_protobuf::RenderSettings render_settings_serialized;
    
    render_settings_serialized.set_width(render_settings.width);
    render_settings_serialized.set_height(render_settings.height);
    render_settings_serialized.set_padding(render_settings.padding);
    render_settings_serialized.set_line_width(render_settings.line_width);
    render_settings_serialized.set_stop_radius(render_settings.stop_radius);
    render_settings_serialized.set_bus_label_font_size(render_settings.bus_label_font_size);
 
    transport_catalogue_protobuf::Point bus_label_offset_serialized;   
    bus_label_offset_serialized.set_x(render_settings.bus_label_offset.first);
    bus_label_offset_serialized.set_y(render_settings.bus_label_offset.second);
    
    *render_settings_serialized.mutable_bus_label_offset() = std::move(bus_label_offset_serialized);
 
    render_settings_serialized.set_stop_label_font_size(render_settings.stop_label_font_size);
 
    transport_catalogue_protobuf::Point stop_label_offset_serialized;
    stop_label_offset_serialized.set_x(render_settings.stop_label_offset.first);
    stop_label_offset_serialized.set_y(render_settings.stop_label_offset.second);
    
    *render_settings_serialized.mutable_stop_label_offset() = std::move(stop_label_offset_serialized);
    *render_settings_serialized.mutable_underlayer_color() = std::move(SerializeColor(render_settings.underlayer_color));
    render_settings_serialized.set_underlayer_width(render_settings.underlayer_width);
    
    const auto& colors = render_settings.color_palette;
    for (const auto& color : colors) {
        *render_settings_serialized.add_color_palette() = std::move(SerializeColor(color));
    }
 
    return render_settings_serialized;
}

map_renderer::RenderSettings DeserializeRenderSettings(const transport_catalogue_protobuf::RenderSettings& render_settings_serialized) {
    map_renderer::RenderSettings render_settings;
    
    render_settings.width = render_settings_serialized.width();
    render_settings.height = render_settings_serialized.height();
    render_settings.padding = render_settings_serialized.padding();
    render_settings.line_width = render_settings_serialized.line_width();
    render_settings.stop_radius = render_settings_serialized.stop_radius();
    render_settings.bus_label_font_size = render_settings_serialized.bus_label_font_size();
    
    render_settings.bus_label_offset.first = render_settings_serialized.bus_label_offset().x();
    render_settings.bus_label_offset.second = render_settings_serialized.bus_label_offset().y();
    
    render_settings.stop_label_font_size = render_settings_serialized.stop_label_font_size();
    
    render_settings.stop_label_offset.first = render_settings_serialized.stop_label_offset().x();
    render_settings.stop_label_offset.second = render_settings_serialized.stop_label_offset().y();
    
    render_settings.underlayer_color = DeserializeColor(render_settings_serialized.underlayer_color());
    render_settings.underlayer_width = render_settings_serialized.underlayer_width();
    
    for (const auto& color_proto : render_settings_serialized.color_palette()) {
        render_settings.color_palette.push_back(DeserializeColor(color_proto));
    }
    
    return render_settings;
}

transport_catalogue_protobuf::RoutingSettings SerializeRoutingSettings(const domain::RoutingSettings& routing_settings) {
    transport_catalogue_protobuf::RoutingSettings routing_settings_serialized;
    
    routing_settings_serialized.set_bus_wait_time(routing_settings.bus_wait_time);
    routing_settings_serialized.set_bus_velocity(routing_settings.bus_velocity);
 
    return routing_settings_serialized;
}

domain::RoutingSettings DeserializeRoutingSettings(const transport_catalogue_protobuf::RoutingSettings& routing_settings_serialized) {
    domain::RoutingSettings routing_settings;
    
    routing_settings.bus_wait_time = routing_settings_serialized.bus_wait_time();
    routing_settings.bus_velocity = routing_settings_serialized.bus_velocity();
    
    return routing_settings;
}

void SerializeTransportCatalogueUnion(transport_catalogue::TransportCatalogue& catalogue, map_renderer::RenderSettings& render_settings, const domain::RoutingSettings& routing_settings, std::ostream& os) {
    transport_catalogue_protobuf::TransportCatalogueUnion transport_catalogue_union_serialized;
 
    transport_catalogue_protobuf::TransportCatalogue transport_catalogue_serialized = SerializeTransportCatalogue(catalogue);
    transport_catalogue_protobuf::RenderSettings render_settings_serialized = SerializeRenderSettings(render_settings);
    transport_catalogue_protobuf::RoutingSettings routing_settings_serialized = SerializeRoutingSettings(routing_settings);
 
    *transport_catalogue_union_serialized.mutable_transport_catalogue() = std::move(transport_catalogue_serialized);
    *transport_catalogue_union_serialized.mutable_render_settings() = std::move(render_settings_serialized);
    *transport_catalogue_union_serialized.mutable_routing_settings() = std::move(routing_settings_serialized);
 
    transport_catalogue_union_serialized.SerializePartialToOstream(&os);
}    

TransportCatalogueUnion DeserializeTransportCatalogueUnion(std::istream& is) {
    transport_catalogue_protobuf::TransportCatalogueUnion transport_catalogue_union_serialized;

    if (!transport_catalogue_union_serialized.ParseFromIstream(&is)) {
        throw std::runtime_error("Failed to parse serialized file");
    }
 
    return { DeserializeTransportCatalogue(transport_catalogue_union_serialized.transport_catalogue()), DeserializeRenderSettings(transport_catalogue_union_serialized.render_settings()), DeserializeRoutingSettings(transport_catalogue_union_serialized.routing_settings()) };
}

} // namespace serialization