/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Project:     
 * Purpose:     
 * Author:      
 * License:     GPL (see file 'COPYING' in the project root for more details)
 * Comments:    
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __QUCS_ACTIVEFILTER_FILTER_EXCEPTIONS_H_INCLUDED__
#define __QUCS_ACTIVEFILTER_FILTER_EXCEPTIONS_H_INCLUDED__

#include <exception>

namespace QUCS
{
namespace Exception
{

class FilterError: public ::std::runtime_error
{
public:
    FilterError(const char * msg):
        ::std::runtime_error(msg)
    {
    }

}; // class FilterError

}
}

#endif /* __QUCS_ACTIVEFILTER_FILTER_EXCEPTIONS_H_INCLUDED__ */

/********************************** End of file *******************************/
