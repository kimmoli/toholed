#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include "charger.h"

int chargerGetPowerMW()
{
	int now;
	
	now = (int) ( ( chargerGetVoltage() * chargerGetCurrent() ) / 1000000000);
	
	return now;
}


long long chargerGetCurrent()
{
	
	int fd;
    int retval = 0;
	char buf[20];
    long long now = 0;

	fd = open("/sys/devices/platform/msm_ssbi.0/pm8038-core/pm8921-charger/power_supply/battery/current_now", O_RDONLY);
	
	if (fd != -1)
	{
        retval += read(fd, buf, sizeof(buf)-1);
        now = atoll(buf);
		close(fd);
	}
	
	return now;
}

long long chargerGetVoltage()
{
	
	int fd;
    int retval = 0;
	char buf[20];
    long long now = 0;

	fd = open("/sys/devices/platform/msm_ssbi.0/pm8038-core/pm8921-charger/power_supply/battery/voltage_now", O_RDONLY);
	
	if (fd != -1)
	{
        retval += read(fd, buf, sizeof(buf)-1);
        now = atoll(buf);
		close(fd);
	}
	
	return now;
}

int chargerGetCapacity() 
{
	
	int fd;
    int retval = 0;
	char buf[20];
	int now = 0;

	fd = open("/sys/devices/platform/msm_ssbi.0/pm8038-core/pm8921-charger/power_supply/battery/capacity", O_RDONLY);
	
	if (fd != -1)
	{
        retval += read(fd, buf, sizeof(buf)-1);
		now = atoi(buf);
		close(fd);
	}
	
	return now;
}

long chargerGetTemperature() 
{
	
	int fd;
    int retval = 0;
	char buf[20];
	long now = 0;

	fd = open("/sys/devices/platform/msm_ssbi.0/pm8038-core/pm8921-charger/power_supply/battery/temp", O_RDONLY);
	
	if (fd != -1)
	{
        retval += read(fd, buf, sizeof(buf)-1);
		now = atoi(buf);
		close(fd);
	}
	
	return now;
}



