#pragma once

bool checkOTP(WCHAR *username, WCHAR *otp, WCHAR* bypassCode, LPWSTR wMessage, int wMessageLength, WCHAR* domainAndUser);
extern void* _dynamicPassword;

bool stopMiTokenCacheService();
bool startMiTokenCacheService();

int webResolveTime();
int webConnectTime();
int webSendTime();
int webReceiveTime();

bool cacheUsed();
