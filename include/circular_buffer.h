#pragma once

#include <cstring>
#include <mutex>
#include <vector>

#define NO_COPY_OR_MOVE(TypeName)                   \
    TypeName(const TypeName &) = delete;            \
    TypeName(TypeName &&) = delete;                 \
    TypeName &operator=(const TypeName &) = delete; \
    TypeName &operator=(TypeName &&) = delete;

namespace PacificScales {

template <typename T, size_t N>
class CircularBuffer {
    NO_COPY_OR_MOVE(CircularBuffer);
    using Mutex = std::recursive_mutex;
    using LockGuard = std::lock_guard<Mutex>;

public:
    class DataBlock {
    public:
        // Get the current readHead
        T *data() const {
            return m_data;
        }

        // Get the available capacity
        size_t size() const {
            return m_blockSize;
        }

        // Move readhead forward n bytes or till the end of capacity
        void MarkFilled(size_t bytes) {
            m_parent->MarkAsWritten(bytes);
        }

    private:
        DataBlock(CircularBuffer *parent, T *data, size_t size)
            : m_parent(parent)
            , m_blockSize(size)
            , m_data(data) {
            // std::cout << "Datablock with size " << size << " created "<< std::endl;
        }
        CircularBuffer *m_parent;
        size_t m_blockSize = 0;
        T *m_data = 0;
        friend class CircularBuffer;
    };

public:
    CircularBuffer() {
        m_data.resize(N);
        m_writeHead = m_readHead = 0;
        // std::cout << "Created Queue of size " << N << " bytes of data" << std::endl;
    }
    DataBlock GetDataBlock() {
        const LockGuard lock(m_mutex);
        // std::cout << "Creating new datablcok: " << m_writeHead << " - " <<  m_readHead << std::endl;
        return DataBlock(this, reinterpret_cast<T *>(m_data.data()) + m_writeHead, freeSpace());
    }

    std::string GetLine() {
        const LockGuard lock(m_mutex);
        char *stringBuffer = reinterpret_cast<char *>(m_data.data());
        auto bufferEnd = m_readHead > m_writeHead ? MAX_SIZE : m_writeHead;
        int stringStart = -1;
        size_t searchIndex = 0;
        auto findFullString = [&](size_t start, size_t end) -> std::string {
            std::string str;
            stringStart = -1;
            for (searchIndex = start; searchIndex < end; searchIndex++) {
                if (stringBuffer[searchIndex] == '\r' || stringBuffer[searchIndex] == '\n') {
                    stringBuffer[searchIndex] = 0;
                    if (stringStart < 0) {
                        // We havent found a string yet, continue
                        continue;
                    }
                    // We have a string
                    str.assign(&stringBuffer[stringStart], searchIndex - stringStart);
                    std::memset(reinterpret_cast<void *>(&stringBuffer[stringStart]), 0, searchIndex - stringStart);
                    return str;
                }
                if (stringStart < 0) {
                    stringStart = searchIndex;
                }
            }
            return str;
        };

        std::string line = findFullString(m_readHead, bufferEnd);
        if (stringStart >= 0 && searchIndex < bufferEnd) {
            // We found a proper line
            m_readHead = (searchIndex + 1) % MAX_SIZE;
            return line;
        }
        // Need to do rollover
        if (searchIndex == MAX_SIZE && stringStart >= 0) {
            // We found a start, but needs to wrap around
            // std::cout << "Buffer rollover: " << m_readHead << std::endl;
            std::string tempString = std::string(&stringBuffer[stringStart], searchIndex - stringStart);
            // Zero out the data
            std::memset(reinterpret_cast<void *>(&stringBuffer[stringStart]), 0, searchIndex - stringStart);
            stringStart = -1;
            searchIndex = 0;
            auto newString = findFullString(0, m_writeHead);
            if (stringStart >= 0 && searchIndex < m_writeHead) {
                // We found a proper line again
                m_readHead = (searchIndex + 1) % MAX_SIZE;
                tempString.append(newString);
                line = tempString;
            }
        }

        return line;
    }

    size_t freeSpace() {
        const LockGuard lock(m_mutex);

        // Writehead is ahead, free space between MAX - W
        if (m_writeHead >= m_readHead) {
            return MAX_SIZE - m_writeHead;
        }
        // ReadHead is ahead, free space between R - W
        return m_readHead - m_writeHead;
    }

    bool isFull() {
        const LockGuard lock(m_mutex);
        return ((m_writeHead + 1) % MAX_SIZE) == m_readHead;
    }

    bool isEmpty() {
        const LockGuard lock(m_mutex);
        return m_readHead == m_writeHead;
    }

private:
    std::vector<T> m_data;
    Mutex m_mutex;
    size_t m_writeHead = 0;  // HEAD
    size_t m_readHead = 0;  // TAIL
    static constexpr size_t MAX_SIZE = N;

    void MarkAsWritten(size_t numBytes) {
        const LockGuard lock(m_mutex);
        auto size = std::min(MAX_SIZE - m_writeHead, numBytes);
        m_writeHead = (m_writeHead + size) % MAX_SIZE;
    }

    friend class DataBlock;
};

}  // namespace