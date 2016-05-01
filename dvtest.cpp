///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dvcontroller.h"

int exitflag;

static void usage ();
static void sigfun (int sig);

void usage()
{
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  dvtest [options] Encode/decode test loop\n");
    fprintf(stderr, "  dvtest -h        Show help\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Input/Output options:\n");
    fprintf(stderr, "  -i <device>   Audio input device or file with 8 kS/s S16LE audio samples (default is /dev/audio, - for piped stdin)\n");
    fprintf(stderr, "  -o <device>   Audio output device or file with 8 kS/s S16LE audio samples (default is /dev/audio, - for stdout)\n");
    fprintf(stderr, "  -D <device>   Use DVSI AMBE3000 based device for AMBE decoding (e.g. ThumbDV)\n");
    fprintf(stderr, "                Device name is the corresponding TTY USB device e.g /dev/ttyUSB0\n");
    fprintf(stderr, "Decoder options:\n");
    fprintf(stderr, "  -f <num>      Format index\n");
    fprintf(stderr, "     0:         None (does nothing - default)\n");
    fprintf(stderr, "     1:         3600x2400 (e.g. D-Star)\n");
    fprintf(stderr, "     2:         3600x2450 (e.g. DMR)\n");
    fprintf(stderr, "\n");
}

void sigfun(int sig)
{
    exitflag = 1;
    signal(SIGINT, SIG_DFL);
}

int main(int argc, char **argv)
{
    int c;
    extern char *optarg;
    extern int optind, opterr, optopt;
    char in_file[1023];
    int  in_file_fd = -1;
    char out_file[1023];
    int  out_file_fd = -1;
    char serialDevice[16];
    std::string dvSerialDevice;
    SerialDV::DVRate dvRate = SerialDV::DVRateNone;

    while ((c = getopt(argc, argv,
            "hi:o:f:D:")) != -1)
    {
        opterr = 0;
        switch (c)
        {
        case 'h':
            usage();
            exit(0);
        case 'i':
            strncpy(in_file, (const char *) optarg, 1023);
            in_file[1023] = '\0';
            break;
        case 'o':
            strncpy(out_file, (const char *) optarg, 1023);
            out_file[1023] = '\0';
            break;
        case 'D':
            strncpy(serialDevice, (const char *) optarg, 16);
            serialDevice[16] = '\0';
            dvSerialDevice = serialDevice;
            break;
        case 'f':
            int formatNum;
            sscanf(optarg, "%d", &formatNum);
            if ((formatNum >= 0) && (formatNum <= 2))
            {
                dvRate = (SerialDV::DVRate) formatNum;
            }
            break;
        default:
            usage();
            exit(0);
        }
    }

    if (strncmp(in_file, (const char *) "-", 1) == 0)
    {
        in_file_fd = STDIN_FILENO;
    }
    else
    {
        in_file_fd = open(in_file, O_RDONLY);
    }

    if (in_file_fd > -1)
    {
        fprintf(stderr, "Opened %s for input.\n", in_file);
    }
    else
    {
        fprintf(stderr, "Cannot open %s for input. Aborting\n", in_file);
        return 0;
    }

    if (strncmp(out_file, (const char *) "-", 1) == 0)
    {
        out_file_fd = STDOUT_FILENO;
    }
    else
    {
        out_file_fd = open(out_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    }

    if (out_file_fd > -1)
    {
        fprintf(stderr, "Opened %s for output.\n", out_file);
    }
    else
    {
        fprintf(stderr, "Cannot open %s for output %s. Aborting\n", out_file, strerror(errno));
        return 0;
    }

    SerialDV::DVController dvController;
    short dvAudioSamples[SerialDV::MBE_AUDIO_BLOCK_SIZE];
    unsigned char dvMbeSamples[SerialDV::MBE_FRAME_LENGTH_BYTES];

    if (!dvSerialDevice.empty())
    {
        if (dvController.open(dvSerialDevice))
        {
            fprintf(stderr, "Opened DV serial device %s\n", dvSerialDevice.c_str());
        }
        else
        {
            fprintf(stderr, "Failed to open DV serial device %s. Aborting\n", dvSerialDevice.c_str());
            return 0;
        }
    }
    else
    {
        fprintf(stderr, "No DV serial device specified. Aborting\n");
        return 0;
    }

    while (exitflag == 0)
    {
        short sample;
        int nbAudioSamples;
        short *audioSamples;

        int result = read(in_file_fd, (void *) dvAudioSamples, SerialDV::MBE_AUDIO_BLOCK_BYTES);

        if (result == 0)
        {
            fprintf(stderr, "No more input. Terminating\n");
            break;
        }
        else if (result != SerialDV::MBE_AUDIO_BLOCK_BYTES)
        {
            fprintf(stderr, "Incomplete audio frame. Terminating\n");
            break;
        }

        if (!dvController.encode(dvAudioSamples, dvMbeSamples, dvRate))
        {
            fprintf(stderr, "Encoding failure. Terminating\n");
            break;
        }

        if (!dvController.decode(dvAudioSamples, dvMbeSamples, dvRate))
        {
            fprintf(stderr, "Decoding failure. Terminating\n");
            break;
        }

        result = write(out_file_fd, (const void *) dvAudioSamples, SerialDV::MBE_AUDIO_BLOCK_BYTES);

        if (result == -1)
        {
            fprintf(stderr, "Error writing to output\n");
        }
        else if (result != SerialDV::MBE_AUDIO_BLOCK_BYTES)
        {
            fprintf(stderr, "Written %d out of %d audio samples\n", result/2, SerialDV::MBE_AUDIO_BLOCK_SIZE);
        }
    }

    fprintf(stderr, "End of process\n");

    dvController.close();

    if ((out_file_fd > -1) && (out_file_fd != STDOUT_FILENO)) {
        close(out_file_fd);
    }

    if ((in_file_fd > -1) && (in_file_fd != STDIN_FILENO)) {
        close(in_file_fd);
    }

    return 0;
}

