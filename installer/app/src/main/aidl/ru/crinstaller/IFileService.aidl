package ru.crinstaller;

interface IFileService {
    // Shizuku calls this transaction to stop the service.
    void destroy() = 16777114;

    String install(in ParcelFileDescriptor zip) = 1;
    String uninstall() = 2;
    String status() = 3;
}
