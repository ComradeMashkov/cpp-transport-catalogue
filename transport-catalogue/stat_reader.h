#pragma once

#include "transport_catalogue.h"

namespace transport_catalogue {

namespace detail {

namespace iostream {

constexpr static int PRECISION = 6;

void PrintBusQuery(TransportCatalogue& catalogue, std::string_view query, std::ostream& os);
void PrintStopQuery(TransportCatalogue& catalogue, std::string_view query, std::ostream& os);
void PrintQuery(TransportCatalogue& catalogue, std::string_view query, std::ostream& os);

void OutputStream(TransportCatalogue& catalogue, std::ostream& os = std::cout, std::istream& is = std::cin);

} // namespace iostream

} // namespace detail

} // namespace transport_catalogue