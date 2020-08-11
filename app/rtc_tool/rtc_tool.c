
#include "tool.h"
#include "cJSON.h"
#include "read_file.h"
#include "str2digit.h"
#include "rtc_ops.h"
#include "cmd_line.h"


const char* weekday_str[] = 
{ 
    "Sunday",
	"Monday",
    "Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
};

const char* months_str[] =
{
	"Jan.",
	"Feb.",
	"Mar.",
	"Apr.",
	"May.",
	"Jun.",
	"Jul.",
	"Aug.",
	"Sept.",
	"Oct.",
	"Nov.",
	"Dec.",
};

typedef struct rtc_time_t{
	int years;
	int months;
	int weekdays;
	int days;
	int hours;
	int minutes;
	int seconds;
}rtc_time_t;

rtc_time_t* get_rtc_time(){
	rtc_time_t *time = (rtc_time_t*)xzalloc(sizeof(rtc_time_t));

	// 2020
	time->years = get_years();

	// 01 ~ 12
	time->months = get_months();

	// 0 SunDay ~ 6 Saturday
	time->weekdays = get_weekdays();

	// 01 ~ 31 or 28 or 29 or 30 leap year
	time->days = get_days();

	// 00 ~ 23
	time->hours = get_hours();
	// 0-59
	time->minutes = get_minutes();

	// 0-59
	time->seconds = get_seconds();

	return time;
}

int set_rtc_time(rtc_time_t* time){
	stop_clock();

	set_weekdays(time->weekdays);
	set_years(time->years);
	set_months(time->months);
	set_days(time->days);
	set_hours(time->hours);
	set_minutes(time->minutes);
	set_seconds(time->seconds);

	start_clock();

	return 0;
}

//weekday : 0 , calendar : 20200729 , time : 100652 
rtc_time_t* parse_args(g_args_para* g_args){
	rtc_time_t *rtc = (rtc_time_t*)xzalloc(sizeof(rtc_time_t));
	rtc->weekdays = g_args->weekday;
	
	// parse calendar
	uint32_t calendar = g_args->calendar;
	rtc->days   = calendar - (calendar / 100) * 100;
	calendar = calendar / 100 ;
	rtc->months = calendar - (calendar / 100) * 100;
	calendar = calendar / 100 ;
	rtc->years  = calendar;
	printf("days : %d , months : %d , years : %d \n", rtc->days, rtc->months, rtc->years);
	
	// parse time
	uint32_t time = g_args->time;
	rtc->seconds = time - (time / 100) * 100;
	time = time / 100;
	rtc->minutes = time - (time / 100) * 100;
	time = time / 100;
	rtc->hours   = time;

	printf("hours : %d , minutes : %d , seconds : %d \n", rtc->hours, rtc->minutes, rtc->seconds);

	return rtc;
}

void small_int2str(int val, char* str){
	char tmp[3];
	sprintf(tmp, "%d", val);
	if(strlen(tmp) == 1){
		memcpy(str + 1, tmp, strlen(str) + 1);
	}else{
		memcpy(str,tmp,strlen(str) + 1);
	}
}

void print_rtc(rtc_time_t *time){

	char str[128];
    char hours[3] = {'0'};
	char minutes[3] = {'0'};
	char seconds[3] = {'0'};

	small_int2str(time->hours, hours);
	small_int2str(time->minutes, minutes);
	small_int2str(time->seconds, seconds);

	sprintf(str,"%s %s %d -- %s:%s:%s %d \n", weekday_str[time->weekdays], months_str[time->months-1], time->days,
							 hours, minutes, seconds, time->years);
	printf(str);
}

void date_cmd(rtc_time_t* time )
{
	//date -s "2020-07-30 15:00:00"
    char command[128];
	char hours[3]   = {'0'};
	char minutes[3] = {'0'};
	char seconds[3] = {'0'};
	char months[3]  = {'0'};
	char days[3]    = {'0'};
	uint32_t calendar;

	small_int2str(time->hours, hours);
	small_int2str(time->minutes, minutes);
	small_int2str(time->seconds, seconds);
	small_int2str(time->months, months);
	small_int2str(time->days, days);

	calendar = time->years * 10000 + time->months * 100 + time->days ;

    sprintf(command, "date -s \"%d-%s-%s %s:%s:%s\"", time->years, months,days ,hours, minutes, seconds);
	system(command);
}

int main(int argc, char *argv[])
{
	// read hw time
	if(argc == 1){
		rtc_time_t* time = get_rtc_time();
		date_cmd(time);
		print_rtc(time);
		free(time);
		return 0;
	}

	g_args_para g_args;
	if(parse_cmd_line(argc, argv, &g_args)){
		return 0;
	}

	printf("weekday : %d , calendar : %d , time : %d \n", g_args.weekday, g_args.calendar, g_args.time );

	rtc_time_t* time = parse_args(&g_args);

	set_rtc_time(time);

	print_rtc(time);
	free(time);

	return 0;
}

