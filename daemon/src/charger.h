#ifndef CHARGER_H
#define CHARGER_H

long chargerGetTemperature();
int chargerGetCapacity();
long long chargerGetVoltage();
long long chargerGetCurrent();
int chargerGetPowerMW();

#endif // CHARGER_H
