/*
 * Business.cpp
 *
 */

#include "Business.hpp"
#include "Process.h"
namespace bus{
void BusInf::db_done(const int, void *)
{
    if (_tools == 0)
        return;
    _tools->deal_done();
}
} /* namespace BUS */
