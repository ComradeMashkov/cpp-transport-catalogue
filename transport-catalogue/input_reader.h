#pragma once

#include "transport_catalogue.h"

namespace transport_catalogue {

namespace detail {

namespace iostream {

Stop GetStopQuery(std::string_view query);

Bus GetBusQuery(TransportCatalogue& catalogue, std::string_view query);

std::vector<Distance> GetDistanceQuery(TransportCatalogue& catalogue, std::string_view query);

void InputStream(TransportCatalogue& catalogue, std::istream& is = std::cin);

} // namespace iostream

} // namespace detail

} // namespace transport_catalogue