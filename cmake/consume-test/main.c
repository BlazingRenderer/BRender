#include <brender.h>

int main(void)
{
    if(BrBegin() != BRE_OK)
        return 1;

    BrEnd();
    return 0;
}
