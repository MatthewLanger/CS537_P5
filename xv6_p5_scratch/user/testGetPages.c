#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) 
{
    printf(1, "Num free pages: %d\n", getFreePagesCount());
    exit();
} 