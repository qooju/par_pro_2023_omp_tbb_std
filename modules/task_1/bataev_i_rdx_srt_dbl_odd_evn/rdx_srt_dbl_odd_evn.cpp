// Copyright 2023 Bataev Ivan
#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <utility>
#include <string>
#include <limits>
#include "../../../modules/task_1/bataev_i_rdx_srt_dbl_odd_evn/rdx_srt_dbl_odd_evn.h"

using std::uint8_t;
using std::uint64_t;

void printVector(const std::vector<double>& v, const std::string& prefix) {
    std::cout << prefix << "[";
    for (int i = 0; i < v.size() - 1; i++) { std::cout << v[i] << ", "; }
    std::cout << v[v.size() - 1] << "]\n";
}

void dblRdxSrt(uint64_t*& inOutBuf, uint64_t*& tmpBuf, const int size) {
    int countBytes[256];
    int offsets[256];

    for (int pass = 0; pass < 7; pass++) {
        // count number of each byte in input buffer
        std::memset(countBytes, 0, 256*sizeof(int));
        for (int i = 0; i < size; i++) {
            uint8_t rdxVal = (inOutBuf[i] >> (pass << 3)) & 0xFF;
            countBytes[rdxVal]++;
        }

        // calc index offsets in output buffer for each byte
        offsets[0] = 0;
        for (int i = 1; i < 256; i++)
            offsets[i] = offsets[i - 1] + countBytes[i - 1];

        for (int i = 0; i < size; i++) {
            uint8_t rdxVal = (inOutBuf[i] >> (pass << 3)) & 0xFF;
            tmpBuf[offsets[rdxVal]++] = inOutBuf[i];
            // increment to keep correct order when identical byte
        }

        std::swap(tmpBuf, inOutBuf);  // swap ptrs
    }

    // the last pass = 7
    std::memset(countBytes, 0, 256*sizeof(int));
    for (int i = 0; i < size; i++) {
        uint8_t rdxVal = (inOutBuf[i] >> 56) & 0xFF;
        countBytes[rdxVal]++;
    }

    int countNegatives = 0; 
    for (int i = 128; i < 256; i++)
        countNegatives += countBytes[i];
    offsets[0] = countNegatives;
    offsets[255] = 0;
    for (int i = 1; i < 128; i++) {
        offsets[i] = offsets[i - 1] + countBytes[i - 1];  // for positive numbers
        offsets[255 - i] = offsets[256 - i] + countBytes[256 - i];  // for negative numbers
    }
    for (int i = 128; i < 256; i++)
        offsets[i] += countBytes[i];
    // for negative numbers to keep correct order when identical byte 
    // (+ prefix decrement below)

    for (int i = 0; i < size; i++) {
        uint8_t rdxVal = (inOutBuf[i] >> 56) & 0xFF;
        if (rdxVal < 128)  // for positive numbers 
            tmpBuf[offsets[rdxVal]++] = inOutBuf[i];
        else  // for negative numbers
            tmpBuf[--offsets[rdxVal]] = inOutBuf[i];
    }

    std::swap(tmpBuf, inOutBuf);  // swap ptrs
}

bool isLittleEndian() {
    std::uint16_t a = 0x0001;
    uint8_t *bldNet = (uint8_t *)&a;
    return bldNet[0];
    // return (std::endian::native == std::endian::little);  // C++20
}

void dblRdxSrt(uint8_t*& inOutBuf, uint8_t*& tmpBuf, const int sizeBuf) {
    const int dblBytes = sizeof(double);
    const bool isLtlEnd = isLittleEndian();
    int countBytes[256];
    int offsets[256];

    int pass;
    for (pass = 0; pass < dblBytes - 1; pass++) {
        std::memset(countBytes, 0, 256*sizeof(int));
        for (int i = 0; i < sizeBuf; i+=dblBytes) {
            uint8_t rdxVal = (isLtlEnd) ? inOutBuf[i + pass] : inOutBuf[i + dblBytes - 1 - pass];
            countBytes[rdxVal]++;
        }

        offsets[0] = 0;
        for (int i = 1; i < 256; i++)
            offsets[i] = offsets[i - 1] + countBytes[i - 1];

        for (int i = 0; i < sizeBuf; i+=dblBytes) {
            uint8_t rdxVal = (isLtlEnd) ? inOutBuf[i + pass] : inOutBuf[i + dblBytes - 1 - pass];
            std::memcpy(tmpBuf + dblBytes*(offsets[rdxVal]++), inOutBuf + i, dblBytes);
        }

        std::swap(tmpBuf, inOutBuf);  // swap ptrs
    }

    // the last pass
    pass = dblBytes - 1;
    std::memset(countBytes, 0, 256*sizeof(int));
    for (int i = 0; i < sizeBuf; i+=dblBytes) {
        uint8_t rdxVal = (isLtlEnd) ? inOutBuf[i + pass] : inOutBuf[i + dblBytes - 1 - pass];
        countBytes[rdxVal]++;
    }

    int countNegatives = 0;
    for (int i = 128; i < 256; i++)
        countNegatives += countBytes[i];
    offsets[0] = countNegatives;
    offsets[255] = 0;
    for (int i = 1; i < 128; i++) {
        offsets[i] = offsets[i - 1] + countBytes[i - 1];
        offsets[255 - i] = offsets[256 - i] + countBytes[256 - i];
    }
    for (int i = 128; i < 256; i++)
        offsets[i] += countBytes[i];

    for (int i = 0; i < sizeBuf; i+=dblBytes) {
        uint8_t rdxVal = (isLtlEnd) ? inOutBuf[i + pass] : inOutBuf[i + dblBytes - 1 - pass];
        if (rdxVal < 128)
            std::memcpy(tmpBuf + dblBytes*(offsets[rdxVal]++), inOutBuf + i, dblBytes);
        else
            std::memcpy(tmpBuf + dblBytes*(--offsets[rdxVal]), inOutBuf + i, dblBytes);
    }

    if (pass%2 == 0)  // if result is in tmpBuf copy it to inOutBuf
        std::memcpy(inOutBuf, tmpBuf, sizeBuf);
    else  // if result is in inOutBuf - swap ptrs
        std::swap(tmpBuf, inOutBuf);
}

// pair of parts for compare-exchange
struct Comparator {
    int part1;
    int part2;
};

void mrgNets(std::vector<int>& partsUp, std::vector<int>& partsDown, std::vector<Comparator>& comprtrs) {
    size_t sumSize = partsUp.size() + partsDown.size();  // n + m
    if (sumSize == 1) {  // network is empty
        return;
    } else if (sumSize == 2) {  // network contains only one comparator
        comprtrs.push_back({ partsUp[0], partsDown[0] });
        return;
    }
    
    std::vector<int> partsUpOdd;
    std::vector<int> partsUpEven;
    for (int i = 0; i < partsUp.size(); i++) {
        if (i % 2)
            partsUpEven.push_back(partsUp[i]);
        else
            partsUpOdd.push_back(partsUp[i]);
    }
    std::vector<int> partsDownOdd;
    std::vector<int> partsDownEven;
    for (int i = 0; i < partsDown.size(); i++) {
        if (i % 2)
            partsDownEven.push_back(partsDown[i]);
        else
            partsDownOdd.push_back(partsDown[i]);
    }
    
    mrgNets(partsUpOdd, partsDownOdd, comprtrs);
    mrgNets(partsUpEven, partsDownEven, comprtrs);
    
    std::vector<int> sumParts(sumSize);
    std::memcpy(sumParts.data(), partsUp.data(), partsUp.size()*sizeof(int));
    std::memcpy(sumParts.data() + partsUp.size(), partsDown.data(), partsDown.size()*sizeof(int));

    for (int i = 1; i + 1 < sumSize; i += 2)
        comprtrs.push_back({ sumParts[i], sumParts[i + 1] });
}

void bldNet(std::vector<int>& parts, std::vector<Comparator>& comprtrs) {
    if (parts.size() == 1)
        return;
    
    std::vector<int> partsUp(parts.begin(), parts.begin() + parts.size() / 2);
    std::vector<int> partsDown(parts.begin() + parts.size() / 2, parts.end());

    bldNet(partsUp, comprtrs);
    bldNet(partsDown, comprtrs);
    mrgNets(partsUp, partsDown, comprtrs);
}

void compExch(double*& bufUp, double*& bufDown, double*& tmpBufUp, double*& tmpBufDown, int lSize) {
    // std::cout << "bufUp: ";
    // for(int i = 0; i < lSize; ++i) {std::cout << bufUp[i] << ", ";}
    // std::cout << "\n";
    // std::cout << "bufDown: ";
    // for(int i = 0; i < lSize; ++i) {std::cout << bufDown[i] << ", ";}
    // std::cout << "\n";
    
    for (int i = 0, j = 0, k = 0; k < lSize; k++) {
        if (bufUp[i] < bufDown[j])
            tmpBufUp[k] = bufUp[i++];
        else
            tmpBufUp[k] = bufDown[j++];
    }
    // for(int i = 0; i < lSize; ++i) {std::cout << tmpBufUp[i] << ", ";}
    // std::cout << "\n";

    for (int i = lSize - 1, j = lSize - 1, k = lSize - 1; k >= 0; k--) {
        if (bufDown[i] > bufUp[j])
            tmpBufDown[k] = bufDown[i--];
        else
            tmpBufDown[k] = bufUp[j--];
    }
    // for(int i = 0; i < lSize; ++i) {std::cout << tmpBufDown[i] << ", ";}
    // std::cout << "\n";

    //std::cout <<"buf: " << bufUp << " tmp: " << tmpBufUp << "\n";
    //std::cout <<"buf: " << bufDown << " tmp: " << tmpBufDown << "\n";
    std::swap(tmpBufUp, bufUp);  // swap ptrs
    std::swap(tmpBufDown, bufDown);  // swap ptrs
    //std::cout <<"buf: " << bufUp << " tmp: " << tmpBufUp << "\n";
    //std::cout <<"buf: " << bufDown << " tmp: " << tmpBufDown << "\n";
}

// void printVector(const std::vector<Comparator>& v, const std::string& prefix) {
//     std::cout << prefix << "[";
//     for (int i = 0; i < v.size() - 1; i++) { std::cout << "(" << v[i].part1 << ", " << v[i].part2 << "), "; }
//     std::cout << "(" << v[v.size() - 1].part1 << ", " << v[v.size() - 1].part2 << ")]\n";
// }

void oddEvnMerge(std::vector<double>& buf, std::vector<double>& tmpBuf, std::vector<double*> partsPtrs, std::vector<double*> tmpPartsPtrs, int numParts, int lSize) {
    std::vector<Comparator> comprtrs;
    std::vector<int> parts;
    for (int i = 0; i < numParts; i++)
        parts.push_back(i);

    // build merge network for ordered parts of buffer
    bldNet(parts, comprtrs);
    //printVector(comprtrs, "");

    // use the network to merge these parts (block sorting)
    for (int i = 0; i < comprtrs.size(); ++i)
        compExch(partsPtrs[comprtrs[i].part1], partsPtrs[comprtrs[i].part2], tmpPartsPtrs[comprtrs[i].part1], tmpPartsPtrs[comprtrs[i].part2], lSize);
    // compare-exchange for each comparator from merge network

    // if partPtr points to tmpBuf copy this part to buf 
    for (int i = 0; i < numParts; i++)
        if (partsPtrs[i] != buf.data() + i*lSize) {
            //std::cout << "buf + shift: " << buf.data() + i * lSize << " tmp + shift: " << tmpBuf.data() + i * lSize << "\n";
            //std::cout << "partsPtrs: " << partsPtrs[i] << " tmpPtrs: " << tmpPartsPtrs[i] << "\n";
            std::memcpy(buf.data() + i*lSize, tmpBuf.data() + i*lSize, lSize*sizeof(double));
            //std::cout << "buf + shift: " << buf.data() + i * lSize << " tmp + shift: " << tmpBuf.data() + i * lSize << "\n";
            //std::cout << "partsPtrs: " << partsPtrs[i] << " tmpPtrs: " << tmpPartsPtrs[i] << "\n";
        }
}

void seqRdxSrt(std::vector<double>& buf, int size, const int numParts) {
    // all parts must be the same size for batcher merge
    while (buf.size()%numParts)
        buf.push_back(std::numeric_limits<double>::max());

    std::vector<double> tmpBuf(buf.size());

    int lSize = (int)buf.size()/numParts;
    std::vector<double*> partsPtrs;
    std::vector<double*> tmpPartsPtrs;
    for (int shift = 0; shift < buf.size(); shift += lSize) {
        partsPtrs.push_back(buf.data() + shift);
        tmpPartsPtrs.push_back(tmpBuf.data() + shift);
    }
    
    if (sizeof(double) == sizeof(uint64_t))
        for (int i = 0; i < numParts; i++)
            dblRdxSrt(reinterpret_cast<uint64_t*&>(partsPtrs[i]), reinterpret_cast<uint64_t*&>(tmpPartsPtrs[i]), lSize);
    else  // C++ standard guarantees only a minimum size of double
        for (int i = 0; i < numParts; i++)
            dblRdxSrt(reinterpret_cast<uint8_t*&>(partsPtrs[i]), reinterpret_cast<uint8_t*&>(tmpPartsPtrs[i]), lSize*sizeof(double));

    printVector(buf, "");
    oddEvnMerge(buf, tmpBuf, partsPtrs, tmpPartsPtrs, numParts, lSize);

    while (buf.size() - size)
        buf.pop_back();
}

