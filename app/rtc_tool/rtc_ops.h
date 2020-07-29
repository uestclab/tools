#ifndef _RTC_OPS_H
#define _RTC_OPS_H

void stop_clock();
void start_clock();

// 00 ~ 99
//2020
void set_years(int years);

// 01 ~ 12
void set_months(int months);

// 0 SunDay ~ 6 Saturday
void set_weekdays(int weekdays);

// 01 ~ 31 or 28 or 29 or 30 leap year
void set_days(int days);

// 00 ~ 23
void set_hours(int hours);

// 0-59
void set_minutes(int minutes);

// 0-59
void set_seconds(int seconds);

// 2020
int get_years();

// 01 ~ 12
int get_months();

// 0 SunDay ~ 6 Saturday
int get_weekdays();

// 01 ~ 31 or 28 or 29 or 30 leap year
int get_days();

// 00 ~ 23
int get_hours();
// 0-59
int get_minutes();

// 0-59
int get_seconds();


#endif