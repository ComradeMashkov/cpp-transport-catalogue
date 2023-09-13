#pragma once

#include "transport_catalogue.h"

namespace transport_catalogue {

namespace detail {

namespace iostream {

constexpr static int PRECISION = 6;

void PrintBusQuery(TransportCatalogue& cat, std::string_view query);
void PrintStopQuery(TransportCatalogue& cat, std::string_view query);
void PrintQuery(TransportCatalogue& cat, std::string_view query);

void OutputStream(TransportCatalogue& cat);

} // namespace iostream

} // namespace detail

} // namespace transport_catalogue