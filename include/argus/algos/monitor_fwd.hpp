/// @file     argus/algos/monitor_fwd.hpp
/// @brief    Forward declaration of monitoring classes.

#pragma once
#ifndef ARGUS_ALGOS_MONITOR
#define ARGUS_ALGOS_MONITOR

#include <memory>

namespace argus {
struct Monitor;
using MonitorPtr = std::shared_ptr<Monitor>;
} // namespace argus

#endif /* end of include guard: ARGUS_ALGOS_MONITOR */
