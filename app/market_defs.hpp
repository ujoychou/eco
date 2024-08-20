#ifndef ECO_MARKET_DEFS_HPP
#define ECO_MARKET_DEFS_HPP
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ history v1.0
[ujoy 2024-06-29]
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include "app_framework.hpp"


////////////////////////////////////////////////////////////////////////////////
enum
{
    domain_market  = 1,
    domain_trade   = 2,
    domain_monitor   = 3,
};
typedef int domain_id;

enum
{
    service_realtime    = 1,
    service_history     = 2,
    service_download    = 3, 
    service_monitor     = 4, 
};
typedef int service_id;


// realtime service
enum
{
    method_get_tick     = 1,
    method_get_target   = 2,
    method_get_product  = 3,
    method_sub_tick     = 4,
    method_sub_tick_all = 5,
};
typedef int method_id;

class Realtime
{
public:
    Method::ref get_tick;
    Method::ref get_target;
    Method::ref get_product;
    Method::ref sub_tick;
    Method::ref sub_tick_all;
};

namespace market {
////////////////////////////////////////////////////////////////////////////////
struct tick
{
    int a;
    int b;
    float c;
    float d;
    uint64_t target_id;
    std::string target_name;
    std::string target_show;
};

struct kbar
{
    int a;
    int b;
    float c;
    float d;
    uint64_t target_id;
    std::string target_name;
    std::string target_show;
};

struct product
{
    int a;
    int b;
    float c;
    float d;
    uint64_t target_id;
    std::string target_name;
    std::string target_show;
};

////////////////////////////////////////////////////////////////////////////////
} // eco
#endif
