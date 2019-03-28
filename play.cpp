#include <windows.h>
#include <mmsystem.h>
#include <cstdio>

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        printf("Usage: play sound.wav\n");
        return 0;
    }

    BOOL ret = PlaySoundA(argv[1], NULL, SND_FILENAME | SND_NODEFAULT | SND_SYNC);
    if (!ret)
    {
        printf("GetLastError(): %ld\n", GetLastError());
        return 1;
    }
    return 0;
}
