#pragma once

int CharToBase64(const unsigned char* pszData, int length, unsigned char* ppszOutput, unsigned short *outputLength);
int Base64ToChar(const unsigned char* pszData, int length, unsigned char* ppszOutput, unsigned short *outputLength);

