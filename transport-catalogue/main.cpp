#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace transport_catalogue;

int main() {
	TransportCatalogue catalogue;
	detail::iostream::InputStream(catalogue);
	detail::iostream::OutputStream(catalogue);
}