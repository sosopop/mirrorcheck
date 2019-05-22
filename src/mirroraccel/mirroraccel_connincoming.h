#ifndef MIRRORACCEL_CONNINCOMING_H_
#define MIRRORACCEL_CONNINCOMING_H_

#include "mongoose.h"

namespace mirroraccel
{
    class ConnIncoming
    {
    public:
        ConnIncoming(mg_connection* nc);
        ~ConnIncoming();
    private:
        mg_connection* nc = nullptr;
    };
}
#endif