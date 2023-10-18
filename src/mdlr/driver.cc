#include "mdlr/driver.h"

#if defined(MDLR_USE_MINIAUDIO)

#include "mdlr/miniaudio.cc.inl"

#endif // defined(MDLR_USE_MINIAUDIO)

namespace mdlr
{
    std::unique_ptr<Driver> Driver::create(DriverBackend backend)
    {
        switch (backend)
        {
        #if defined(MDLR_USE_MINIAUDIO)
            case DriverBackend::Miniaudio:
                return std::unique_ptr<Driver>(new MiniaudioDriver());
        #endif // defined(MDLR_USE_MINIAUDIO)
            default:
                break;
        }
        return nullptr;
    }
}