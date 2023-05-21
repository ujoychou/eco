#pragma once
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-17.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2023 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Object.h>


ECO_NS_BEGIN(eco);
class TaskGraph;
////////////////////////////////////////////////////////////////////////////////
class Task
{
public:
    // task state 4
    enum 
    {
        state_none = 0,
        state_exec = 1,
        state_done = 2,
        state_fail = 3,
    };

    // task owner: task graph
    TaskGraph* graph;

    int work_load;

    virtual void async() = 0;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco)