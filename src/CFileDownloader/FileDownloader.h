#pragma once

void* filedownload(string url, string staticfilename = "");
void ProgressBarInfo(const char* info, ...)IM_FMTARGS(2);

string GetConvertedLinkbyYandex(string input_url);
string GetFileNamebyYandex(string input_url);

extern double progress_;
extern string download_info;

