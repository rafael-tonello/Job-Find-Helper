#include "file.h"

bool Trees::CFile::isOpen()
{
    return this->opened;
}

bool Trees::CFile::open(string filename, bool forceNew)
{
    if (this->opened)
        this->close();

    this->opened = false;
    if (!forceNew)
    {
        this->file = fopen(filename.c_str(), "rb+");
        if (!this->file)
            this->file = fopen(filename.c_str(), "wb+");
    }
    else
        this->file = fopen(filename.c_str(), "wb+");
    
    if (this->file)
        this->opened = true;

    return this->opened;
}

bool Trees::CFile::close()
{
    if (fclose(this->file))
        return true;

    return false;
}

bool Trees::CFile::write(const char* buf, size_t size)
{
    fwrite(buf, size, 1, this->file);
    fflush(this->file);
    return true;
}

bool Trees::CFile::read(char* buf, size_t size)
{
    fread(buf, size, 1, this->file);
    return true;
}

bool Trees::CFile::seek(size_t newPos)
{
    fseek(this->file, newPos, SEEK_SET);
    return true;
}

bool Trees::CFile::seekToEnd()
{
    fseek(this->file, 0, SEEK_END);
    return true;
}

size_t Trees::CFile::currentPos()
{
    return ftell(this->file);
    return true;
}

size_t Trees::CFile::tell()
{
    return this->currentPos();
    return true;
}

size_t Trees::CFile::fileSize()
{
    auto curr = this->currentPos();
    this->seekToEnd();
    auto ret = this->currentPos();
    this->seek(curr);
    return ret;
}     
