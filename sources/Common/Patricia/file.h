#ifndef __FILE_H__
#define __FILE_H__
#include <string>

namespace Trees {
    using namespace std;
    class IFile
    {
    public:
        virtual bool isOpen() = 0;
        virtual bool open(string filename, bool forceNew = false) = 0;
        virtual bool close() = 0;
        virtual bool write(const char* buf, size_t size) = 0;
        virtual bool read(char* buf, size_t size) = 0;
        virtual bool seek(size_t newPos) = 0;
        virtual bool seekToEnd() = 0;

        //currentPos and tell are the same function
        virtual size_t currentPos() = 0;
        virtual size_t tell() = 0;
        virtual size_t fileSize() = 0;
    };

    class CFile: public IFile
    {
    private:
        FILE *file;
        bool opened = false;
    public:
        bool isOpen() override;
        bool open(string filename, bool forceNew = false) override;
        bool close() override;
        bool write(const char* buf, size_t size) override;
        bool read(char* buf, size_t size) override;
        bool seek(size_t newPos) override;
        bool seekToEnd() override;
        //tell and currentPos is the same function
        size_t currentPos() override;
        size_t tell() override;
        size_t fileSize() override;
    };
};

#endif
