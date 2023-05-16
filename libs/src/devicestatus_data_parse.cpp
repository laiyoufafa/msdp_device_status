/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "devicestatus_data_parse.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr int32_t FILE_SIZE_MAX = 0x5000;
constexpr int32_t READ_DATA_BUFF_SIZE = 256;
constexpr int32_t INVALID_FILE_SIZE = -1;
const std::string MSDP_DATA_PATH = "/data/msdp/device_status_data.json";
const std::string MSDP_DATA_DIR = "/data/msdp";
} // namespace

std::vector<int32_t> DeviceStatusDataParse::tempcount_ =
    std::vector<int32_t> (static_cast<int32_t>(Type::TYPE_MAX), static_cast<int32_t>(TypeValue::INVALID));

int32_t DeviceStatusDataParse::CreateJsonFile()
{
    int32_t fd = open(MSDP_DATA_PATH.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP);
    if (fd < 0) {
        DEV_HILOGE(SERVICE, "open failed");
        return DEVICESTATUS_FAILED;
    }
    close(fd);

    struct stat buf;
    if (stat(MSDP_DATA_DIR.c_str(), &buf) != 0) {
        DEV_HILOGE(SERVICE, "stat folder path is invalid %{public}d", errno);
        return DEVICESTATUS_FAILED;
    }
    if (chown(MSDP_DATA_PATH.c_str(), buf.st_uid, buf.st_gid) != 0) {
        DEV_HILOGE(SERVICE, "chown failed, errno is %{public}d", errno);
        return DEVICESTATUS_FAILED;
    }

    return DEVICESTATUS_OK;
}

bool DeviceStatusDataParse::ParseDeviceStatusData(Data& data, Type type)
{
    std::string jsonBuf = ReadJsonFile(MSDP_DATA_PATH.c_str());
    if (jsonBuf.empty()) {
        DEV_HILOGE(SERVICE, "read json failed, errno is %{public}d", errno);
        data.type = type;
        data.value = OnChangedValue::VALUE_INVALID;
        return false;
    }
    return DeviceStatusDataInit(jsonBuf, true, type, data);
}

bool DeviceStatusDataParse::DeviceStatusDataInit(const std::string& fileData, bool logStatus, Type& type,
    Data& data)
{
    DEV_HILOGD(SERVICE, "Enter");
    JsonParser parser;
    parser.json_ = cJSON_Parse(fileData.c_str());
    data.type = type;
    data.value = OnChangedValue::VALUE_INVALID;
    if (cJSON_IsArray(parser.json_)) {
        DEV_HILOGE(SERVICE, "parser is array");
        return false;
    }

    if (type < Type::TYPE_ABSOLUTE_STILL || type >= Type::TYPE_MAX) {
        DEV_HILOGE(SERVICE, "Type error");
        return false;
    }

    cJSON* mockarray = cJSON_GetObjectItem(parser.json_, DeviceStatusJson[type].json.c_str());
    int32_t jsonsize = cJSON_GetArraySize(mockarray);
    if (jsonsize == 0) {
        return false;
    }
    tempcount_[type] = tempcount_[type] % jsonsize;
    cJSON* mockvalue = cJSON_GetArrayItem(mockarray, tempcount_[type]);
    tempcount_[type]++;
    data.type = type;
    data.value = static_cast<OnChangedValue>(mockvalue->valueint);
    DEV_HILOGD(SERVICE, "type:%{public}d, status:%{public}d", data.type, data.value);
    return true;
}

bool DeviceStatusDataParse::DisableCount(const Type type)
{
    DEV_HILOGD(SERVICE, "Enter");
    tempcount_[static_cast<int32_t>(type)] = static_cast<int32_t>(TypeValue::INVALID);
    return true;
}

std::string DeviceStatusDataParse::ReadJsonFile(const std::string &filePath)
{
    if (filePath.empty()) {
        DEV_HILOGE(SERVICE, "Path is empty");
        return "";
    }
    char realPath[PATH_MAX] = {};
    if (realpath(filePath.c_str(), realPath) == nullptr) {
        DEV_HILOGE(SERVICE, "Path is error, %{public}d", errno);
        return "";
    }
    if (!CheckFileDir(realPath, MSDP_DATA_DIR)) {
        DEV_HILOGE(SERVICE, "File dir is invalid");
        return "";
    }
    if (!CheckFileExtendName(realPath, "json")) {
        DEV_HILOGE(SERVICE, "Unable to parse files other than json format");
        return "";
    }
    if (!IsFileExists(filePath)) {
        DEV_HILOGE(SERVICE, "File not exist");
        return "";
    }
    if (!CheckFileSize(filePath)) {
        DEV_HILOGE(SERVICE, "File size out of read range");
        return "";
    }
    return ReadFile(realPath);
}

int32_t DeviceStatusDataParse::GetFileSize(const std::string& filePath)
{
    struct stat statbuf = { 0 };
    if (stat(filePath.c_str(), &statbuf) != 0) {
        DEV_HILOGE(SERVICE, "Get file size error");
        return INVALID_FILE_SIZE;
    }
    return statbuf.st_size;
}

bool DeviceStatusDataParse::CheckFileDir(const std::string& filePath, const std::string& dir)
{
    if (filePath.compare(0, MSDP_DATA_DIR.size(), MSDP_DATA_DIR) != 0) {
        DEV_HILOGE(SERVICE, "FilePath dir is invalid");
        return false;
    }
    return true;
}

bool DeviceStatusDataParse::CheckFileSize(const std::string& filePath)
{
    int32_t fileSize = GetFileSize(filePath);
    if ((fileSize <= 0) || (fileSize > FILE_SIZE_MAX)) {
        DEV_HILOGE(SERVICE, "File size out of read range");
        return false;
    }
    return true;
}

bool DeviceStatusDataParse::CheckFileExtendName(const std::string& filePath, const std::string& checkExtension)
{
    std::string::size_type pos = filePath.find_last_of('.');
    if (pos == std::string::npos) {
        DEV_HILOGE(SERVICE, "File is not found extension");
        return false;
    }
    return (filePath.substr(pos + 1, filePath.npos) == checkExtension);
}

bool DeviceStatusDataParse::IsFileExists(const std::string& fileName)
{
    return (access(fileName.c_str(), F_OK) == 0);
}

std::string DeviceStatusDataParse::ReadFile(const std::string &filePath)
{
    DEV_HILOGD(SERVICE, "Enter");
    FILE* fp = fopen(filePath.c_str(), "r");
    if (fp == nullptr) {
        DEV_HILOGE(SERVICE, "Open failed");
        return "";
    }
    std::string dataStr;
    char buf[READ_DATA_BUFF_SIZE] = {};
    while (fgets(buf, sizeof(buf), fp) != nullptr) {
        dataStr += buf;
    }
    if (fclose(fp) != 0) {
        DEV_HILOGW(SERVICE, "Close file failed");
    }
    DEV_HILOGD(SERVICE, "Exit");
    return dataStr;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
