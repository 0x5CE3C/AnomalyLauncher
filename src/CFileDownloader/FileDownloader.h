#pragma once


#include "..\CTaskManager.h"
#include <atomic>

void filedownload(string url, string staticfilename = "");
void ProgressBarInfo(const char* info, ...)IM_FMTARGS(2);

string GetConvertedLinkbyYandex(string input_url);
string GetFileNamebyYandex(string input_url);

extern double progress_;
extern double lastReportedProgress; // Выносим наружу, чтобы сбрасывать перед каждой загрузкой!
extern string download_info;

extern std::atomic<Task*> g_activeDownloadTask;
