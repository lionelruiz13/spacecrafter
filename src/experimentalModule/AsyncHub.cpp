#include "AsyncHub.hpp"
#include "coreModule/core.hpp"
#include "coreModule/coreLink.hpp"

AsyncHub *AsyncHub::instance = nullptr;

AsyncHub::AsyncHub()
{
    instance = this;
}

AsyncHub::~AsyncHub()
{
    instance = nullptr;
}
