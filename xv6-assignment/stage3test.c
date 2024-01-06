#include "types.h"
#include "user.h"

int main(int argc, char* argv[])
{
    setvideomode(0x13);
    int pid = fork();
    if(pid == 0)
    {
        int hdc = beginpaint(0);
        setpencolour(16, 0, 0, 63);
        for (int i = 20; i <= 160; i+=20)
        {
            selectpen(hdc, 16);
            moveto(hdc, i, i+20);
            lineto(hdc, i, i + 40);
            lineto(hdc, i + 20, i + 40);
            lineto(hdc, i+20, i + 20);
            lineto(hdc, i, i + 20);
        }
        endpaint(hdc);
        exit();
    } else if (pid > 0)
    {
        int hdc = beginpaint(0);
        setpencolour(17, 0, 63, 0);
        for (int i = 20; i <= 160; i+=20)
        {
            selectpen(hdc, 17);
            moveto(hdc, i, i);
            lineto(hdc, i, i + 20);
            lineto(hdc, i + 20, i + 20);
            lineto(hdc, i+20, i);
            lineto(hdc, i, i);
        }
        endpaint(hdc);
        wait();
    } else {
        printf(1, "Fork failed\n");
    }
    getch();
    setvideomode(0x03);
    exit();
}