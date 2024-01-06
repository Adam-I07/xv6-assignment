#include "types.h"
#include "user.h"
int main(int argc, char* argv[])
{
    setvideomode(0x13);
    getch();
    setvideomode(0x03);
    exit();
}
