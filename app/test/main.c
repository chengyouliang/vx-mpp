#include<stdio.h>
#include "vx_log.h"
int main()
{
    vx_log("test log\n");
    vx_log_f("%s %d\n",__FUNCTION__,__LINE__);
    vx_err("test error");
}