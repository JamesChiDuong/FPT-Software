/*******************************************************************************
* Add public API in header files inside :
* API
******************************************************************************/
#include "app.h"
/*******************************************************************************
* Add macros, enumeration types, structure types, inside:
* Definitions
******************************************************************************/
/*******************************************************************************
* Add static API prototype in C file inside :
* Prototypes
******************************************************************************/
/*******************************************************************************
* Code
******************************************************************************/
int main()
{
    bool result = true;

    /* check input of app*/
    result = app_init();
    /* if initialize faile, user need to input again*/
    while (result != true)
    {
        result = app_init();
    }
    /* option of user*/
    app_createMenuOption();
    /* deinitialize*/
    app_deinit();

    return 0;
}
