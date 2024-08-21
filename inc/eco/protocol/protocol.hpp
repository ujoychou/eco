#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-08-21] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <stdint.h>


namespace eco {
namespace protocol {
////////////////////////////////////////////////////////////////////////////////
// app protocol
class being;
class rtps;
class someip;
class ftdc;
class mqtt;

// app protocol: rpc
class grpc;
class srpc;

// app protocol: middleware
class mysql;
class kafka;
class redis;

////////////////////////////////////////////////////////////////////////////////
} // namespace protocol
} // namespace eco