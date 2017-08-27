
#ifndef __CODE_H__
#define __CODE_H__

namespace bas{

enum LogLevel
{
    L_DT = 600,   ///DEBUG DETAILS
    L_DB = 500,   ///DEBUG
    L_IF = 400,   ///INFO
    L_RP = 350,   ///REPORT
    L_WA = 300,   ///WARING
    L_ER = 200    ///ERROR
};

enum CodeNum{
    OK = 0,
    KO = 1,
    NO_USER
};

} /* bas */

#endif /* CODE_H_ */
