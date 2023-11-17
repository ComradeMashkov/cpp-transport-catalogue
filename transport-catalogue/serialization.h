#pragma once

#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"

#include "map_renderer.h"
#include "map_renderer.pb.h"

#include "svg.pb.h"

#include <iostream>

namespace serialization {

struct SerializationSettings {
    std::string file_name;
};

struct TransportCatalogueUnion {
    transport_catalogue::TransportCatalogue transport_catalogue_;
    map_renderer::RenderSettings render_settings_;
    domain::RoutingSettings routing_settings_;
};

template <typename It>
uint32_t CalculateDistance(It range_begin, It range_end, std::string_view name);

transport_catalogue_protobuf::TransportCatalogue SerializeTransportCatalogue(const transport_catalogue::TransportCatalogue& catalogue);
transport_catalogue::TransportCatalogue DeserializeTransportCatalogue(const transport_catalogue_protobuf::TransportCatalogue& catalogue_serialized);

transport_catalogue_protobuf::Color SerializeColor(const svg::Color& color);
svg::Color DeserializeColor(const transport_catalogue_protobuf::Color& color_serialized);

transport_catalogue_protobuf::RenderSettings SerializeRenderSettings(const map_renderer::RenderSettings& render_settings);
map_renderer::RenderSettings DeserializeRenderSettings(const transport_catalogue_protobuf::RenderSettings& render_settings_serialized);

transport_catalogue_protobuf::RoutingSettings SerializeRoutingSettings(const domain::RoutingSettings& routing_settings);
domain::RoutingSettings DeserializeRoutingSettings(const transport_catalogue_protobuf::RoutingSettings& routing_settings_serialized);

void SerializeTransportCatalogueUnion(transport_catalogue::TransportCatalogue& catalogue, map_renderer::RenderSettings& render_settings, const domain::RoutingSettings& routing_settings, std::ostream& os);    
TransportCatalogueUnion DeserializeTransportCatalogueUnion(std::istream& is);

} // namespace serialization