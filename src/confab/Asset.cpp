#include "Asset.hpp"

#include "Constants.hpp"

// Flatbuffer includes generated by calling flatc on the Flatbuffer schema (.fbs) files.
#include "schemas/FlatAsset_generated.h"

#include <glog/logging.h>

namespace Confab {

// static
Asset::Type Asset::typeStringToEnum(const std::string& assetType) {
    if (assetType == "snippet") {
        return kSnippet;
    } else if (assetType == "image") {
        return kImage;
    } else if (assetType == "yaml") {
        return kYAML;
    } else if (assetType == "sample") {
        return kSample;
    }
    return kInvalid;
}

Asset::Asset(Asset::Type type) :
    m_record(),
    m_flatAsset(nullptr),
    m_builder(new flatbuffers::FlatBufferBuilder(kDataChunkSize)),
    m_assetBuilder(new Data::FlatAssetBuilder(*m_builder)),
    m_type(type) {
    m_assetBuilder->add_type(static_cast<Data::Type>(m_type));
}

void Asset::setKey(uint64_t key) {
    CHECK(m_assetBuilder);
    m_assetBuilder->add_key(key);
}

uint8_t* Asset::setInlineData(size_t size) {
    CHECK(m_builder);
    CHECK(m_assetBuilder);
    CHECK(size <= kSingleChunkDataSize) << "inline data size over capacity.";

    uint8_t* space = nullptr;
    auto inlineData = m_builder->CreateUninitializedVector(size, &space);
    m_assetBuilder->add_inlineData(inlineData);
    return space;
}

void Asset::setFileExtension(const std::string& fileExtension) {
    CHECK(m_builder);
    CHECK(m_assetBuilder);

    auto extension = m_builder->CreateString(fileExtension);
    m_assetBuilder->add_fileExtension(extension);
}

void Asset::setSalt(uint64_t salt) {
    CHECK(m_assetBuilder);

    m_assetBuilder->add_salt(salt);
}

const SizedPointer Asset::flatten() {
    auto asset = m_assetBuilder->Finish();
    m_builder->Finish(asset);
    return SizedPointer(m_builder->GetBufferPointer(), m_builder->GetSize());
}

// static
const Asset Asset::LoadAsset(const Database::Record& record, uint64_t key) {
    auto flatAsset = Data::GetFlatAsset(record.data().data());
    return Asset(record, flatAsset, key);
}

Asset::Asset(const Database::Record& record, const Data::FlatAsset* flatAsset, uint64_t key) :
    m_record(record),
    m_flatAsset(flatAsset),
    m_type(static_cast<Asset::Type>(m_flatAsset->type())) {
    if (m_flatAsset->key() != 0) {
        m_key = m_flatAsset->key();
    } else {
        m_key = key;
    }
}

}  // namespace Confab

