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

transport_catalogue_protobuf::TransportCatalogue GetTransportCatalogueSerialized(const transport_catalogue::TransportCatalogue& catalogue);
transport_catalogue::TransportCatalogue GetTransportCatalogueDeserialized(const transport_catalogue_protobuf::TransportCatalogue& catalogue_serialized);

transport_catalogue_protobuf::Color GetColorSerialized(const svg::Color& color);
svg::Color GetColorDeserealized(const transport_catalogue_protobuf::Color& color_serialized);

transport_catalogue_protobuf::RenderSettings GetRenderSettingsSerialized(const map_renderer::RenderSettings& render_settings);
map_renderer::RenderSettings GetRenderSettingsDeserialized(const transport_catalogue_protobuf::RenderSettings& render_settings_serialized);

transport_catalogue_protobuf::RoutingSettings GetRoutingSettingsSerialized(const domain::RoutingSettings& routing_settings);
domain::RoutingSettings GetRoutingSettingsDeserialized(const transport_catalogue_protobuf::RoutingSettings& routing_settings_serialized);

void SerializeTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue, map_renderer::RenderSettings& render_settings, const domain::RoutingSettings& routing_settings, std::ostream& os);    
TransportCatalogueUnion DeserializeTransportCatalogue(std::istream& is);

} // namespace serialization