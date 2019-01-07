#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "ds18b20.h"

int ds18b20_read_data(char *dev_addr, float *ftemp) {
   DIR *dir;
   struct dirent *dirent;
   char dev[16];      // Dev ID
   char devPath[128]; // Path to device
   char buf[256];     // Data from device
   char tmpData[6];   // Temp C * 1000 reported by device
   char path[] = "/sys/bus/w1/devices";
   ssize_t numRead;
   int found;

   dir = opendir (path);
   if (dir != NULL)
   {
      found=0;
      while ((dirent = readdir (dir)))
      {
         // 1-wire devices are links beginning with 28-
         if (dirent->d_type == DT_LNK &&
             strstr(dirent->d_name, "28-") != NULL)
         {
            strcpy(dev, dirent->d_name);
            // printf("\nDevice: %s\n", dev);
            if (strcmp(dev, dev_addr) == 0) {
               found=1;
            }
         }
      }
      (void) closedir (dir);
   }
   else
   {
      fprintf(stderr, "Couldn't open the w1 devices directory\n");
      return -1;
   }
   if (found==0) {
      fprintf(stderr, "Device not found\n");
      return -2;
   }

   // Assemble path to OneWire device
   sprintf(devPath, "%s/%s/w1_slave", path, dev_addr);
   // Read temp continuously
   // Opening the device's file triggers new reading
   int fd = open(devPath, O_RDONLY);
   if(fd == -1) {
      fprintf(stderr, "Couldn't open the w1 device.\n");
      return -3;
   }
   while((numRead = read(fd, buf, 256)) > 0) {
      strncpy(tmpData, strstr(buf, "t=") + 2, 5);
      // issue #1
      tmpData[5]=0;
      float tempC = strtof(tmpData, NULL);
      *ftemp = tempC / 1000;
      //printf("Device: %s  - ", dev);
      //printf("Temp: %3.1f C\n\n", *ftemp);
   }
   close(fd);

   return 0;
} // ds18b20_read_data

