//
//  KTX.cpp
//  ktx/src/ktx
//
//  Created by Zach Pomerantz on 2/08/2017.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "KTX.h"

#include <algorithm> //min max and more

using namespace ktx;

uint32_t Header::evalPadding(size_t byteSize) {
    //auto padding = byteSize % PACKING_SIZE;
 //   return (uint32_t) (padding ? PACKING_SIZE - padding : 0);
    return (uint32_t) (3 - (byteSize + 3) % PACKING_SIZE);// padding ? PACKING_SIZE - padding : 0);
}


const Header::Identifier ktx::Header::IDENTIFIER {{
    0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
}};

Header::Header() {
    memcpy(identifier, IDENTIFIER.data(), IDENTIFIER_LENGTH);
}

uint32_t Header::evalMaxDimension() const {
    return std::max(getPixelWidth(), std::max(getPixelHeight(), getPixelDepth()));
}

uint32_t Header::evalPixelWidth(uint32_t level) const {
    return std::max(getPixelWidth() >> level, 1U);
}
uint32_t Header::evalPixelHeight(uint32_t level) const {
    return std::max(getPixelHeight() >> level, 1U);
}
uint32_t Header::evalPixelDepth(uint32_t level) const {
    return std::max(getPixelDepth() >> level, 1U);
}

size_t Header::evalPixelSize() const {
    return glTypeSize; // Really we should generate the size from the FOrmat etc
}

size_t Header::evalRowSize(uint32_t level) const {
    auto pixWidth = evalPixelWidth(level);
    auto pixSize = evalPixelSize();
    auto netSize = pixWidth * pixSize;
    auto padding = evalPadding(netSize);
    return netSize + padding;
}
size_t Header::evalFaceSize(uint32_t level) const {
    auto pixHeight = evalPixelHeight(level);
    auto pixDepth = evalPixelDepth(level);
    auto rowSize = evalRowSize(level);
    return pixDepth * pixHeight * rowSize;
}
size_t Header::evalImageSize(uint32_t level) const {
    auto faceSize = evalFaceSize(level);
    if (numberOfFaces == NUM_CUBEMAPFACES && numberOfArrayElements == 0) {
        return faceSize;
    } else {
        return (getNumberOfSlices() * numberOfFaces * faceSize);
    }
}


KeyValue::KeyValue(const std::string& key, uint32_t valueByteSize, const Byte* value) :
    _byteSize((uint32_t) key.size() + 1 + valueByteSize), // keyString size + '\0' ending char + the value size
    _key(key),
    _value(valueByteSize)
{
    if (_value.size() && value) {
        memcpy(_value.data(), value, valueByteSize);
    }
}

KeyValue::KeyValue(const std::string& key, const std::string& value) :
    KeyValue(key, (uint32_t) value.size(), (const Byte*) value.data())
{

}

uint32_t KeyValue::serializedByteSize() const {
    return (uint32_t) (sizeof(uint32_t) + _byteSize + Header::evalPadding(_byteSize));
}

uint32_t KeyValue::serializedKeyValuesByteSize(const KeyValues& keyValues) {
    uint32_t keyValuesSize = 0;
    for (auto& keyval : keyValues) {
        keyValuesSize += keyval.serializedByteSize();
    }
    return (keyValuesSize + Header::evalPadding(keyValuesSize));
}


KTX::KTX() {
}

KTX::~KTX() {
}

void KTX::resetStorage(const StoragePointer& storage) {
    _storage = storage;
}

const Header* KTX::getHeader() const {
    if (!_storage) {
        return nullptr;
    } 
    return reinterpret_cast<const Header*>(_storage->data());
}


size_t KTX::getKeyValueDataSize() const {
    if (_storage) {
        return getHeader()->bytesOfKeyValueData;
    } else {
        return 0;
    }
}

size_t KTX::getTexelsDataSize() const {
    if (_storage) {
        //return  _storage->size() - (sizeof(Header) + getKeyValueDataSize());
        return  (_storage->data() + _storage->size()) - getTexelsData();
    } else {
        return 0;
    }
}

const Byte* KTX::getKeyValueData() const {
    if (_storage) {
        return (_storage->data() + sizeof(Header));
    } else {
        return nullptr;
    }
}

const Byte* KTX::getTexelsData() const {
    if (_storage) {
        return (_storage->data() + sizeof(Header) + getKeyValueDataSize());
    } else {
        return nullptr;
    }
}

storage::StoragePointer KTX::getMipFaceTexelsData(uint16_t mip, uint8_t face) const {
    storage::StoragePointer result;
    if (mip < _images.size()) {
        const auto& faces = _images[mip];
        if (face < faces._numFaces) {
            auto faceOffset = faces._faceBytes[face] - _storage->data();
            auto faceSize = faces._faceSize;
            result = _storage->createView(faceSize, faceOffset);
        }
    }
    return result;
}
