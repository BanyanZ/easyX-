#pragma once
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cmath>
using namespace std;

class ManageTime
{
public:
	ManageTime() {}
	~ManageTime() {}

	// 将 time_t 转为连续数字字符串 YYYYMMDDHHMMSS
	void TimeToString(time_t time1, char* szTime) {
		struct tm tm1;
#if defined(_WIN32) || defined(_WIN64)
		localtime_s(&tm1, &time1);
#else
		localtime_r(&time1, &tm1);
#endif
		sprintf(szTime, "%04d%02d%02d%02d%02d%02d",
			tm1.tm_year + 1900, tm1.tm_mon + 1, tm1.tm_mday,
			tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
	}

	// 把连续数字字符串 YYYYMMDDHHMMSS 转成 time_t（失败返回 0）
	time_t StringToTime(char* szTime) {
		struct tm tm1;
		if (sscanf(szTime, "%4d%2d%2d%2d%2d%2d",
			&tm1.tm_year,
			&tm1.tm_mon,
			&tm1.tm_mday,
			&tm1.tm_hour,
			&tm1.tm_min,
			&tm1.tm_sec) != 6) {
			return 0;
		}
		tm1.tm_year -= 1900;
		tm1.tm_mon -= 1;
		tm1.tm_isdst = -1;
		return mktime(&tm1);
	}

	// 在给定 time_t 上加一个月，返回 YYYYMMDDHHMMSS（保留时分秒）
	string AddMonth(time_t& tTime) {
		struct tm t;
#if defined(_WIN32) || defined(_WIN64)
		localtime_s(&t, &tTime);
#else
		localtime_r(&tTime, &t);
#endif
		int year = t.tm_year + 1900;
		int mon = t.tm_mon + 1;
		int day = t.tm_mday;
		int hour = t.tm_hour;
		int min = t.tm_min;
		int sec = t.tm_sec;

		mon++;
		if (mon > 12) { mon = 1; year++; }

		int mdays;
		switch (mon) {
		case 1: case 3: case 5: case 7: case 8: case 10: case 12: mdays = 31; break;
		case 4: case 6: case 9: case 11: mdays = 30; break;
		case 2:
			if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) mdays = 29;
			else mdays = 28;
			break;
		default: mdays = 31; break;
		}
		if (day > mdays) day = mdays;

		char buf[32] = { 0 };
		sprintf(buf, "%04d%02d%02d%02d%02d%02d", year, mon, day, hour, min, sec);
		return string(buf);
	}

	// 计算两个字符串表示时间的天数差（期望 YYYYMMDDHHMMSS 或只数字字符串）
	int calculateDaysDifference(string& timeStr1, string& timeStr2) {
		if (timeStr1.empty() || timeStr2.empty()) return 0;
		string s1 = timeStr1;
		string s2 = timeStr2;
		time_t t1 = StringToTime(const_cast<char*>(s1.c_str()));
		time_t t2 = StringToTime(const_cast<char*>(s2.c_str()));
		return calculateDaysDifference(t1, t2);
	}

	// 计算两个 time_t 的天数差，超过0点计为一天（向上取整）
	int calculateDaysDifference(time_t time1, time_t time2) {
		if (time1 == 0 || time2 == 0) return 0;
		double diffSeconds = difftime(time2, time1);
		const double secondsPerDay = 24.0 * 60.0 * 60.0;
		double diffDays = diffSeconds / secondsPerDay;
		if (diffDays > 0) return static_cast<int>(ceil(diffDays));
		if (diffDays < 0) return static_cast<int>(floor(diffDays));
		return 0;
	}
};