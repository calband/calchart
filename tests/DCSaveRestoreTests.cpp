#include "DCSaveRestore.h"
#include <catch2/catch_test_macros.hpp>
#include <wx/dcmemory.h>

TEST_CASE("DCSaveRestoreOrigin", "wxDC")
{
#ifdef __WXMAC__
    wxMemoryDC dc;
    wxPoint orig = dc.GetDeviceOrigin();

    {
        auto _ = SaveAndRestore::DeviceOrigin(dc);
        dc.SetDeviceOrigin(orig.x + 1, orig.y + 2);
        wxPoint newOrigin = dc.GetDeviceOrigin();
        CHECK_FALSE(orig == newOrigin);
    }
    wxPoint newOrigin = dc.GetDeviceOrigin();
    CHECK(orig == newOrigin);
#endif
}
