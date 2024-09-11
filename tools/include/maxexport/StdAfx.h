#include <lang/internalError.h>
#include <max.h>
#include <stdmat.h>
#include <assert.h>
#include <algorithm>


#undef assert
#define assert(E) if (E) {} else {lang::internalError(__FILE__,__LINE__,#E);}

#define MAXEXPORT_SPLIT_BIPED
