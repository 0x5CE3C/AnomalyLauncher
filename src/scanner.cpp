#include "main.h"
#include "scanner.h"

bool scanner_anomaly_enable = false;
bool module_anomaly_damaged = false;

bool scanner_mo2_enable = false;
bool module_mo2_damaged = false;

bool scanner_be_enable = false;
bool module_be_damaged = false;

bool scanner_extractor_enable = false;
bool module_extractor_damaged = false;

int getNextId_() {
    int maxId = 0;
    for (int i = 0; i < iFileListCount; i++) {
        if (filelist.fileList[i].id > maxId) {
            maxId = filelist.fileList[i].id;
        }
    }
    return maxId + 1;
}

static int ids_size;

void ScanModule(int m_type)
{
    traceLastFunc("void ScanModule(int m_type) #start");
    switch (m_type)
    {
        case SCAN_MODULE_TYPE_ANOMALY:
        {
            static int anomaly_files_found = 0;
            static int anomaly_files_not_found = 0;
            static int anomaly_files_match = 0;
            static int anomaly_files_no_match = 0;

            static int anomaly_dir_found = 0;
            static int anomaly_dir_not_found = 0;
            static int anomaly_dir_match = 0;
            static int anomaly_dir_no_match = 0;

            sn->con.AddLog("[Scanner][Anomaly] > Started");
            Log("[Scanner][Anomaly] > Started");

            if (!scanner_anomaly_enable) return;

            sn->con.AddLog("[Scanner][Anomaly] Checking Path: %s", GetExePath().c_str());
            Log("[Scanner][Anomaly] Checking Path: %s", GetExePath().c_str());
                for (int i = 0; i < iFileListCount; i++)
                {
                    if (!scanner_anomaly_enable)
                    {
                        StopScanning(m_type, anomaly_dir_not_found, anomaly_dir_no_match, anomaly_files_not_found, anomaly_files_no_match);
                        break;
                    }

                    if (filelist.fileList[i].m_type == SUBMODULE_TYPE_ANOMALY)
                    {
                        if (filelist.fileList[i].f_type == FILE_TYPE_DIR)
                        {
                            CheckDirectory(i, anomaly_dir_found, anomaly_dir_not_found, anomaly_dir_match, anomaly_dir_no_match);                        
                        }
                        else if (filelist.fileList[i].f_type == FILE_TYPE_FILE)
                        {
                            CheckFile(i, anomaly_files_found, anomaly_files_not_found, anomaly_files_match, anomaly_files_no_match);
                        }
                    }
                }

            LogSummary(m_type, anomaly_dir_found, anomaly_dir_not_found, anomaly_dir_match, anomaly_dir_no_match, anomaly_files_found, anomaly_files_not_found, anomaly_files_match, anomaly_files_no_match);

            ResetCounters(m_type, anomaly_dir_found, anomaly_dir_not_found, anomaly_dir_match, anomaly_dir_no_match, anomaly_files_found, anomaly_files_not_found, anomaly_files_match, anomaly_files_no_match);
            break;
        }

        case SCAN_MODULE_TYPE_MO2:
        {
            static int mo2_files_found = 0;
            static int mo2_files_not_found = 0;
            static int mo2_files_match = 0;
            static int mo2_files_no_match = 0;

            static int mo2_dir_found = 0;
            static int mo2_dir_not_found = 0;
            static int mo2_dir_match = 0;
            static int mo2_dir_no_match = 0;

            sn->con.AddLog("[Сканер][MO2] > Запущен");
            Log("[Scanner][MO2] > Started");

            if (!scanner_mo2_enable) return;

            sn->con.AddLog("[Сканер][MO2] > Проверяемый путь: %s", GetExePath().c_str());
            Log("[Scanner][MO2] Checking Path: %s", GetExePath().c_str());
            for (int i = 0; i < iFileListCount; i++)
            {
                if (!scanner_mo2_enable)
                {
                    StopScanning(m_type, mo2_dir_not_found, mo2_dir_no_match, mo2_files_not_found, mo2_files_no_match);
                    break;
                }

                if (filelist.fileList[i].m_type == SUBMODULE_TYPE_MO2)
                {
                    if (filelist.fileList[i].f_type == FILE_TYPE_DIR)
                    {
                        CheckDirectory(i, mo2_dir_found, mo2_dir_not_found, mo2_dir_match, mo2_dir_no_match);
                    }
                    else if (filelist.fileList[i].f_type == FILE_TYPE_FILE)
                    {
                        CheckFile(i, mo2_files_found, mo2_files_not_found, mo2_files_match, mo2_files_no_match);
                    }
                }
            }

            LogSummary(m_type, mo2_dir_found, mo2_dir_not_found, mo2_dir_match, mo2_dir_no_match, mo2_files_found, mo2_files_not_found, mo2_files_match, mo2_files_no_match);

            ResetCounters(m_type, mo2_dir_found, mo2_dir_not_found, mo2_dir_match, mo2_dir_no_match, mo2_files_found, mo2_files_not_found, mo2_files_match, mo2_files_no_match);
            break;
        }

        case SCAN_MODULE_TYPE_BE:
        {
            static int be_files_found = 0;
            static int be_files_not_found = 0;
            static int be_files_match = 0;
            static int be_files_no_match = 0;

            static int be_dir_found = 0;
            static int be_dir_not_found = 0;
            static int be_dir_match = 0;
            static int be_dir_no_match = 0;

            ids_size = getNextId_();

            sn->con.AddLog("[Scanner][ModPack] > Started");
            Log("[Scanner][ModPack] > Started");

            if (!scanner_be_enable) return;

            sn->con.AddLog("[Scanner][ModPack] Checking Path: %s", GetExePath().c_str());
            Log("[Scanner][ModPack] Checking Path: %s", GetExePath().c_str());
            for (int i = 0; i < iFileListCount; i++)
            {
                if (!scanner_be_enable)
                {
                    StopScanning(m_type, be_dir_not_found, be_dir_no_match, be_files_not_found, be_files_no_match);
                    break;
                }

                if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE)
                {
                    if (filelist.fileList[i].f_type == FILE_TYPE_DIR)
                    {
                        CheckDirectory(i, be_dir_found, be_dir_not_found, be_dir_match, be_dir_no_match);
                    }
                    else if (filelist.fileList[i].f_type == FILE_TYPE_FILE)
                    {
                        CheckFile(i, be_files_found, be_files_not_found, be_files_match, be_files_no_match);                        
                    }
                }
            }

            LogSummary(m_type, be_dir_found, be_dir_not_found, be_dir_match, be_dir_no_match, be_files_found, be_files_not_found, be_files_match, be_files_no_match);

            ResetCounters(m_type, be_dir_found, be_dir_not_found, be_dir_match, be_dir_no_match, be_files_found, be_files_not_found, be_files_match, be_files_no_match);
            break;
        }

        case SCAN_MODULE_TYPE_EXTRACTOR:
        {
            static int extractor_files_found = 0;
            static int extractor_files_not_found = 0;
            static int extractor_files_match = 0;
            static int extractor_files_no_match = 0;

            static int extractor_dir_found = 0;
            static int extractor_dir_not_found = 0;
            static int extractor_dir_match = 0;
            static int extractor_dir_no_match = 0;

            sn->con.AddLog("[Scanner][Extractor] > Started");
            Log("[Scanner][Extractor] > Started");

            if (!scanner_extractor_enable) return;

            sn->con.AddLog("[Scanner][Extractor] Checking Path: %s", GetExePath().c_str());
            Log("[Scanner][Extractor] Checking Path: %s", GetExePath().c_str());
            for (int i = 0; i < iFileListCount; i++)
            {
                if (!scanner_extractor_enable)
                {
                    StopScanning(m_type, extractor_dir_not_found, extractor_dir_no_match, extractor_files_not_found, extractor_files_no_match);
                    break;
                }

                if (filelist.fileList[i].m_type == SUBMODULE_TYPE_EXTRACTOR)
                {                    
                    if (filelist.fileList[i].f_type == FILE_TYPE_FILE)
                    {
                        CheckFile(i, extractor_files_found, extractor_files_not_found, extractor_files_match, extractor_files_no_match);
                    }
                }
            }

            LogSummary(m_type, extractor_dir_found, extractor_dir_not_found, extractor_dir_match, extractor_dir_no_match, extractor_files_found, extractor_files_not_found, extractor_files_match, extractor_files_no_match);

            ResetCounters(m_type, extractor_dir_found, extractor_dir_not_found, extractor_dir_match, extractor_dir_no_match, extractor_files_found, extractor_files_not_found, extractor_files_match, extractor_files_no_match);
            break;
        }
    }
}

string GetDirectoryPath(int i)
{
    if (strcmp(filelist.fileList[i].dir, "\"\"") == 0)
    {
        return GetExePath() + "\\" + filelist.fileList[i].name;
    }
    return GetExePath() + "\\" + filelist.fileList[i].dir + "\\" + filelist.fileList[i].name;
}

string GetFilePath(int i)
{
    if (strcmp(filelist.fileList[i].dir, "\"\"") == 0)
    {
        return GetExePath() + "\\" + filelist.fileList[i].name;
    }
    return GetExePath() + "\\" + filelist.fileList[i].dir + "\\" + filelist.fileList[i].name;
}



void CheckDirectory(int i, int& dirs_found, int& dirs_not_found, int& dirs_match, int& dirs_no_match)
{
    string pathToCheck = GetDirectoryPath(i);
    if (fsm->DirectoryExists(pathToCheck.c_str()))
    {
        if (sn->scanhashes_dirs.enabled)
        {
            string hash = md5_files_in_directory(pathToCheck.c_str());
            if (strcmp(filelist.fileList[i].hash, hash.c_str()) == 0)
            {
                if (filelist.fileList[i].m_type == SUBMODULE_TYPE_ANOMALY)
                {
                    sn->con.AddLog("[Сканер][Anomaly][Каталог][%i] > {00BB00}%s > совпадение", dirs_match, filelist.fileList[i].name);
                    Log("[Scanner][Anomaly][Dir][%i] > %s > match", dirs_match, filelist.fileList[i].name);
                    dirs_match++;
                    dirs_found++;
                }
                else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_MO2)
                {
                    sn->con.AddLog("[Сканер][MO2][Каталог][%i] > {00BB00}%s > совпадение", dirs_match, filelist.fileList[i].name);
                    Log("[Scanner][MO2][Dir][%i] > %s > match", dirs_match, filelist.fileList[i].name);
                    dirs_match++;
                    dirs_found++;
                }
                else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE)
                {
                    sn->con.AddLog("[Сканер][ModPack][Каталог][%i] > {00BB00}%s > совпадение", dirs_match, filelist.fileList[i].name);
                    Log("[Scanner][ModPack][Dir][%i] > %s > match", dirs_match, filelist.fileList[i].name);
                    dirs_match++;
                    dirs_found++;
                }                
            }
            else
            {                
                if (filelist.fileList[i].m_type == SUBMODULE_TYPE_ANOMALY)
                {
                    sn->con.AddLog("[Сканер][Anomaly][Каталог][%i] > {AAFFFF}%s > хэши не совпадают", dirs_no_match, filelist.fileList[i].name);
                    Log("[Scanner][Anomaly][Dir][%i] > %s > hashes do not match\n[Valid hash] > %s\n[Invalid hash] > %s", dirs_no_match, filelist.fileList[i].name, filelist.fileList[i].hash, hash.c_str());
                    dirs_no_match++;
                    dirs_found++;
                    module_anomaly_damaged = true;
                    ProcessDamagedModule(i);
                }
                else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_MO2)
                {
                    sn->con.AddLog("[Сканер][MO2][Каталог][%i] > {AAFFFF}%s > хэши не совпадают", dirs_no_match, filelist.fileList[i].name);
                    Log("[Scanner][MO2][Dir][%i] > %s > hashes do not match\n[Valid hash] > %s\n[Invalid hash] > %s", dirs_no_match, filelist.fileList[i].name, filelist.fileList[i].hash, hash.c_str());
                    dirs_no_match++;
                    dirs_found++;
                    module_mo2_damaged = true;
                    ProcessDamagedModule(i);
                }
                else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE)
                {                    
                   /* sn->con.AddLog("[Сканер][ModPack][Каталог][%i] > {AAFFFF}%s > хэши не совпадают", dirs_no_match, filelist.fileList[i].name);*/
                    switch (sn->umode.project_mode) {
                    case PROJECTMODES::PM_Anomaly_PZ:
                        sn->con.AddLog("%i \"%s\" \"MO2\\\\mods\" \"pz\" \"dir\" \"%s\" \"\" \"\"", ids_size, filelist.fileList[i].name, hash.c_str());                        
                        break;
                    case PROJECTMODES::PM_Anomaly_MP:
                        sn->con.AddLog("%i \"%s\" \"MO2\\\\mods\" \"mp\" \"dir\" \"%s\" \"\" \"\"", ids_size, filelist.fileList[i].name, hash.c_str());
                        break;
                    default:
                        break;
                    }                    
                    Log("[Scanner][ModPack][Dir][%i] > %s > hashes do not match\n[Valid hash] > %s\n[Invalid hash] > %s", dirs_no_match, filelist.fileList[i].name, filelist.fileList[i].hash, hash.c_str());
                    dirs_no_match++;
                    dirs_found++;       
                    ids_size++;
                    module_be_damaged = true;
                    ProcessDamagedModule(i);
                }
            }
        }
        else
        {
            if (filelist.fileList[i].m_type == SUBMODULE_TYPE_ANOMALY)
            {
                sn->con.AddLog("[Сканер][Anomaly][Каталог][%i] > {00BB00}%s > совпадение", dirs_match, filelist.fileList[i].name);
                Log("[Scanner][Anomaly][dir][%i] > %s > match", dirs_match, filelist.fileList[i].name);
                dirs_match++;
                dirs_found++;
            }
            else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_MO2)
            {
                sn->con.AddLog("[Сканер][MO2][Каталог][%i] > {00BB00}%s > совпадение", dirs_match, filelist.fileList[i].name);
                Log("[Scanner][MO2][dir][%i] > %s > match", dirs_match, filelist.fileList[i].name);
                dirs_match++;
                dirs_found++;
            }
            else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE)
            {
                sn->con.AddLog("[Сканер][ModPack][Каталог][%i] > {00BB00}%s > совпадение", dirs_match, filelist.fileList[i].name);
                Log("[Scanner][ModPack][dir][%i] > %s > match", dirs_match, filelist.fileList[i].name);
                dirs_match++;
                dirs_found++;
            }
        }
    }
    else
    {
        if (filelist.fileList[i].m_type == SUBMODULE_TYPE_ANOMALY)
        {
            sn->con.AddLog("[Сканер][Anomaly][каталог][%i] > {ff0000}%s каталог не найден", dirs_not_found, filelist.fileList[i].name);
            Log("[Scanner][Anomaly][dir][%i] > %s > dir not found", dirs_not_found, filelist.fileList[i].name);
            dirs_not_found++;
            module_anomaly_damaged = true;
            ProcessDamagedModule(i);
        }
        else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_MO2)
        {
            sn->con.AddLog("[Сканер][MO2][каталог][%i] > {ff0000}%s каталог не найден", dirs_not_found, filelist.fileList[i].name);
            Log("[Scanner][MO2][dir][%i] > %s > dir not found", dirs_not_found, filelist.fileList[i].name);
            dirs_not_found++;
            module_mo2_damaged = true;
            ProcessDamagedModule(i);
        }
        else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE)
        {
            sn->con.AddLog("[Сканер][ModPack][каталог][%i] > {ff0000}%s каталог не найден", dirs_not_found, filelist.fileList[i].name);
            Log("[Scanner][ModPack][dir][%i] > %s > dir not found", dirs_not_found, filelist.fileList[i].name);
            dirs_not_found++;
            module_be_damaged = true;
            ProcessDamagedModule(i);
        }
    }
}

void CheckFile(int i, int& files_found, int& files_not_found, int& files_match, int& files_no_match)
{
    string pathToCheck = GetFilePath(i);    
    if (fsm->FileExists(pathToCheck.c_str()))
    {
       if(sn->scanhashes.enabled)
       {
            string hash = md5file(pathToCheck.c_str());
            if (strcmp(filelist.fileList[i].hash, hash.c_str()) == 0)
            {
                if (filelist.fileList[i].m_type == SUBMODULE_TYPE_ANOMALY)
                {
                    sn->con.AddLog("[Сканер][Anomaly][файл][%i] > {00BB00}%s > совпадение", files_match, filelist.fileList[i].name);
                    Log("[Scanner][Anomaly][File][%i] > %s > match", files_match, filelist.fileList[i].name);
                }
                else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_EXTRACTOR)
                {
                    sn->con.AddLog("[Сканер][Extractor][файл][%i] > {00BB00}%s > совпадение", files_match, filelist.fileList[i].name);
                    Log("[Scanner][Extractor][File][%i] > %s > match", files_match, filelist.fileList[i].name);
                }     
                else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_MO2)
                {
                    sn->con.AddLog("[Сканер][MO2][файл][%i] > {00BB00}%s > совпадение", files_match, filelist.fileList[i].name);
                    Log("[Scanner][MO2][File][%i] > %s > match", files_match, filelist.fileList[i].name);
                }
                else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE)
                {
                    sn->con.AddLog("[Сканер][ModPack][файл][%i] > {00BB00}%s > совпадение", files_match, filelist.fileList[i].name);
                    Log("[Scanner][ModPack][File][%i] > %s > match", files_match, filelist.fileList[i].name);
                }
                files_match++;
                files_found++;
            }
            else
            {
                
                if (filelist.fileList[i].m_type == SUBMODULE_TYPE_ANOMALY)
                {
                    sn->con.AddLog("[Сканер][Anomaly][файл][%i] > {AAFFFF}%s > хэши не совпадают", files_no_match, filelist.fileList[i].name);
                    Log("[Scanner][Anomaly][File][%i] > %s > hashes do not match\n[Valid hash] > %s\n[Invalid hash] > %s", files_no_match, filelist.fileList[i].name, filelist.fileList[i].hash, hash.c_str());
                    files_no_match++;
                    files_found++;
                    module_anomaly_damaged = true;
                }
                else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_EXTRACTOR)
                {
                    sn->con.AddLog("[Сканер][Extractor][файл][%i] > {AAFFFF}%s > хэши не совпадают", files_no_match, filelist.fileList[i].name);
                    Log("[Scanner][Extractor][File][%i] > %s > hashes do not match\n[Valid hash] > %s\n[Invalid hash] > %s", files_no_match, filelist.fileList[i].name, filelist.fileList[i].hash, hash.c_str());
                    files_no_match++;
                    files_found++;
                }
                else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_MO2)
                {
                    sn->con.AddLog("[Сканер][MO2][файл][%i] > {AAFFFF}%s > хэши не совпадают", files_no_match, filelist.fileList[i].name);                    
                    Log("[Scanner][MO2][File][%i] > %s > hashes do not match\n[Valid hash] > %s\n[Invalid hash] > %s", files_no_match, filelist.fileList[i].name, filelist.fileList[i].hash, hash.c_str());
                    files_no_match++;
                    files_found++;
                    module_mo2_damaged = true;
                }
                else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE)
                {
                   /* sn->con.AddLog("[Сканер][ModPack][файл][%i] > {AAFFFF}%s > хэши не совпадают", files_no_match, filelist.fileList[i].name);*/
                    switch (sn->umode.project_mode) {
                    case PROJECTMODES::PM_Anomaly_PZ:
                        sn->con.AddLog("%i \"%s\" \"MO2\\\\mods\" \"pz\" \"file\" \"%s\" \"\" \"\"", ids_size, filelist.fileList[i].name, hash.c_str());
                        break;
                    case PROJECTMODES::PM_Anomaly_MP:
                        sn->con.AddLog("%i \"%s\" \"MO2\\\\mods\" \"mp\" \"file\" \"%s\" \"\" \"\"", ids_size, filelist.fileList[i].name, hash.c_str());
                        break;
                    default:
                        break;
                    }
                    Log("[Scanner][ModPack][File][%i] > %s > hashes do not match\n[Valid hash] > %s\n[Invalid hash] > %s", files_no_match, filelist.fileList[i].name, filelist.fileList[i].hash, hash.c_str());
                    files_no_match++;
                    files_found++;                    
                    ids_size++;
                    module_be_damaged = true;
                }
                
                ProcessDamagedModule(i);
            }
       }
       else
       {
           if (filelist.fileList[i].m_type == SUBMODULE_TYPE_ANOMALY)
           {
               sn->con.AddLog("[Сканер][Anomaly][файл][%i] > {00BB00}%s > совпадение", files_match, filelist.fileList[i].name);
               Log("[Scanner][Anomaly][File][%i] > %s > match", files_match, filelist.fileList[i].name);
               files_match++;
               files_found++;
           }
           else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_EXTRACTOR)
           {
               sn->con.AddLog("[Сканер][Extractor][файл][%i] > {00BB00}%s > совпадение", files_match, filelist.fileList[i].name);
               Log("[Scanner][Extractor][File][%i] > %s > match", files_match, filelist.fileList[i].name);
               files_match++;
               files_found++;
           }
           else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_MO2)
           {
               sn->con.AddLog("[Сканер][MO2][файл][%i] > {00BB00}%s > совпадение", files_match, filelist.fileList[i].name);
               Log("[Scanner][MO2][File][%i] > %s > match", files_match, filelist.fileList[i].name);
               files_match++;
               files_found++;
           }
           else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE)
           {
               sn->con.AddLog("[Сканер][ModPack][файл][%i] > {00BB00}%s > совпадение", files_match, filelist.fileList[i].name);
               Log("[Scanner][ModPack][File][%i] > %s > match", files_match, filelist.fileList[i].name);
               files_match++;
               files_found++;
           }
       }
    }
    else
    {
        if (filelist.fileList[i].m_type == SUBMODULE_TYPE_ANOMALY)
        {
            sn->con.AddLog("[Сканер][Anomaly][файл][%i] > {ff0000}%s файл не найден", files_not_found, filelist.fileList[i].name);
            Log("[Scanner][Anomaly][File][%i] > %s > file not found", files_not_found, filelist.fileList[i].name);
            module_anomaly_damaged = true;
            ProcessDamagedModule(i);
        }
        else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_EXTRACTOR)
        {
            sn->con.AddLog("[Сканер][Extractor][файл][%i] > {ff0000}%s файл не найден", files_not_found, filelist.fileList[i].name);
            Log("[Scanner][Extractor][File][%i] > %s > file not found", files_not_found, filelist.fileList[i].name);
            files_not_found++;
            ProcessDamagedModule(i);
        }         
        else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_MO2)
        {
            sn->con.AddLog("[Сканер][MO2][файл][%i] > {ff0000}%s файл не найден", files_not_found, filelist.fileList[i].name);
            Log("[Scanner][MO2][File][%i] > %s > file not found", files_not_found, filelist.fileList[i].name);
            files_not_found++;
            ProcessDamagedModule(i);
            module_mo2_damaged = true;
        }
        else if (filelist.fileList[i].m_type == SUBMODULE_TYPE_BE)
        {
            sn->con.AddLog("[Сканер][ModPack][файл][%i] > {ff0000}%s файл не найден", files_not_found, filelist.fileList[i].name);
            Log("[Scanner][ModPack][File][%i] > %s > file not found", files_not_found, filelist.fileList[i].name);
            files_not_found++;
            ProcessDamagedModule(i);
            module_be_damaged = true;
        }
         
    }
}

/*void ProcessDamagedModule(int i)
{        
    if (sn->download_system.enabled && (filelist.fileList[i].m_type == SUBMODULE_TYPE_ANOMALY || filelist.fileList[i].m_type == SUBMODULE_TYPE_BE || filelist.fileList[i].m_type == SUBMODULE_TYPE_MO2) && filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_ZIP)
    {                    

                executor->AddTask([i]() {
                    filedownload(filelist.fileList[i].url, filelist.fileList[i].downloadfilename);
                    }, TaskType::FILE_DOWNLOAD);

                if (!is_extracting)
                {

                    executor->AddTask([i]() {
                        zipextractor::HandleFileExtraction(i);
                        }, TaskType::UNPACK_ZIP);
                }            
    }


    if (sn->download_system.enabled && (filelist.fileList[i].m_type == SUBMODULE_TYPE_EXTRACTOR && filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_EXE))
    {        
            executor->AddTask([i]() {
                filedownload(filelist.fileList[i].url, GetExePath() + "\\" + PROJECT_FOLDER + "\\" + filelist.fileList[i].downloadfilename);
                }, TaskType::FILE_DOWNLOAD);                    
    }
}*/


void ProcessDamagedModule(int i)
{
    if (sn->download_system.enabled && (filelist.fileList[i].m_type == SUBMODULE_TYPE_ANOMALY || filelist.fileList[i].m_type == SUBMODULE_TYPE_BE || filelist.fileList[i].m_type == SUBMODULE_TYPE_MO2) && filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_ZIP)
    {

        executor->AddTask([i]() {
            filedownload(filelist.fileList[i].url, filelist.fileList[i].downloadfilename);
            }, TaskType::FILE_DOWNLOAD);

        if (!is_extracting)
        {

            executor->AddTask([i]() {
                zipextractor::HandleFileExtraction(i);
                }, TaskType::UNPACK_ZIP);
        }
    }


    if (sn->download_system.enabled && (filelist.fileList[i].m_type == SUBMODULE_TYPE_EXTRACTOR && filelist.fileList[i].dl_type == DL_SUB_MODULE_TYPE_EXE))
    {
        executor->AddTask([i]() {
            filedownload(filelist.fileList[i].url, GetExePath() + "\\" + PROJECT_FOLDER + "\\" + filelist.fileList[i].downloadfilename);
            }, TaskType::FILE_DOWNLOAD);
    }
}


void StopScanning(int m_type, int& dirs_not_found, int& dirs_no_match, int& files_not_found, int& files_no_match)
{
    dirs_not_found = 0;
    dirs_no_match = 0;
    files_not_found = 0;
    files_no_match = 0;

    if (m_type == SCAN_MODULE_TYPE_ANOMALY)
    {
        sn->con.AddLog("[Сканер][Anomaly] > Остановлен | ожидание команды...");
        Log("[Scanner][Anomaly] > Stopped | waiting command...");
    } 
    else if (m_type == SCAN_MODULE_TYPE_MO2)
    {
        sn->con.AddLog("[Сканер][MO2] > Остановлен | ожидание команды...");
        Log("[Scanner][MO2] > Stopped | waiting command...");
    }
    else if (m_type == SCAN_MODULE_TYPE_BE)
    {
        sn->con.AddLog("[Сканер][ModPack] > Остановлен | ожидание команды...");
        Log("[Scanner][ModPack] > Stopped | waiting command...");        
    }
    else if (m_type == SCAN_MODULE_TYPE_EXTRACTOR)
    {
        sn->con.AddLog("[Сканер][Extractor] > Остановлен | ожидание команды...");
        Log("[Scanner][Extractor] > Stopped | waiting command...");
    }
}

void LogSummary(int m_type, int dirs_found, int dirs_not_found, int dirs_match, int dirs_no_match, int files_found, int files_not_found, int files_match, int files_no_match)
{
    if (m_type == SCAN_MODULE_TYPE_ANOMALY)
    {
        sn->con.AddLog("[Сканер][Anomaly] > Остановка | каталогов найдено: %d\\%d, не найдено: %d, совпадений: %d, не совпадений: %d", dirs_found, dirs_found + dirs_no_match + dirs_not_found, dirs_not_found, dirs_match, dirs_no_match);

        sn->con.AddLog("[Сканер][Anomaly] > Остановка | файлов найдено: %d\\%d, не найдено: %d, совпадений: %d, не совпадений: %d", files_found, files_found + files_no_match + files_not_found, files_not_found, files_match, files_no_match);

        if (dirs_not_found == 0 && dirs_no_match == 0 && files_not_found == 0 && files_no_match == 0)
        {
            module_anomaly_damaged = false;
            scanner_anomaly_enable = false;
        }
    }
    else if (m_type == SCAN_MODULE_TYPE_MO2)
    {
        sn->con.AddLog("[Сканер][MO2] > Остановка | каталогов найдено: %d\\%d, не найдено: %d, совпадений: %d, не совпадений: %d", dirs_found, dirs_found + dirs_no_match + dirs_not_found, dirs_not_found, dirs_match, dirs_no_match);

        sn->con.AddLog("[Сканер][MO2] > Остановка | файлов найдено: %d\\%d, не найдено: %d, совпадений: %d, не совпадений: %d", files_found, files_found + files_no_match + files_not_found, files_not_found, files_match, files_no_match);

        if (dirs_not_found == 0 && dirs_no_match == 0 && files_not_found == 0 && files_no_match == 0)
        {
            module_mo2_damaged = false;
            scanner_mo2_enable = false;
        }
    }
    else if (m_type == SCAN_MODULE_TYPE_BE)
    {
        sn->con.AddLog("[Сканер][ModPack] > Остановка | каталогов найдено: %d\\%d, не найдено: %d, совпадений: %d, не совпадений: %d", dirs_found, dirs_found + dirs_no_match + dirs_not_found, dirs_not_found, dirs_match, dirs_no_match);

        sn->con.AddLog("[Сканер][ModPack] > Остановка | файлов найдено: %d\\%d, не найдено: %d, совпадений: %d, не совпадений: %d", files_found, files_found + files_no_match + files_not_found, files_not_found, files_match, files_no_match);

        if (dirs_not_found == 0 && dirs_no_match == 0 && files_not_found == 0 && files_no_match == 0)
        {
            module_be_damaged = false;
            scanner_be_enable = false;
        }
    }
    else if (m_type == SCAN_MODULE_TYPE_EXTRACTOR)
    {
        sn->con.AddLog("[Сканер][Extractor] > Остановка | файлов найдено: %d\\%d, не найдено: %d, совпадений: %d, не совпадений: %d", files_found, files_found + files_no_match + files_not_found, files_not_found, files_match, files_no_match);

        if (files_not_found == 0 && files_no_match == 0)
        {
            module_extractor_damaged = false;
            scanner_extractor_enable = false;
        }
    }
}

void ResetCounters(int m_type, int& dirs_found, int& dirs_not_found, int& dirs_match, int& dirs_no_match, int& files_found, int& files_not_found, int& files_match, int& files_no_match)
{
    dirs_found = 0;
    dirs_not_found = 0;
    dirs_match = 0;
    dirs_no_match = 0;
    files_found = 0;
    files_not_found = 0;
    files_match = 0;
    files_no_match = 0;
    ids_size = getNextId_();
    if (module_anomaly_damaged)
    {
        scanner_anomaly_enable = false;
    }
    if (module_mo2_damaged)
    {
        scanner_mo2_enable = false;
    }
    if (module_be_damaged)
    {
        scanner_be_enable = false;
    }
    if (module_extractor_damaged)
    {
        scanner_extractor_enable = false;
    }
}