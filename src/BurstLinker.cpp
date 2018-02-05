//
// Created by succlz123 on 2018/1/30.
//

#include "BurstLinker.h"
#include "GifAnalyzer.h"

using namespace blk;

BurstLinker::~BurstLinker() {
    delete gifEncoder;
}

bool BurstLinker::init(const char *path, uint16_t width, uint16_t height, uint32_t loopCount, uint32_t threadNum) {
    gifEncoder = new GifEncoder();
    return gifEncoder->init(path, width, height, 0, threadNum);
}

bool BurstLinker::connect(uint32_t *imagePixel, uint32_t delay,
                          QuantizerType quantizerType, DitherType ditherType,
                          uint16_t left, uint16_t top) {
    if (gifEncoder == nullptr) {
        return false;
    }
    std::vector<uint8_t> content;
    gifEncoder->addImage(imagePixel, delay, quantizerType,
                         ditherType, left, top, content);
    delete[] imagePixel;
    gifEncoder->flush(content);
    return true;
}

bool BurstLinker::connect(std::vector<uint32_t *> imagePixels, uint32_t delay,
                          QuantizerType quantizerType, DitherType ditherType,
                          uint16_t left, uint16_t top) {
    if (gifEncoder == nullptr) {
        return false;
    }
    size_t size = imagePixels.size();
    std::vector<std::future<std::vector<uint8_t>>> tasks;
    for (int k = 0; k < size; ++k) {
        auto result = gifEncoder->threadPool->enqueue([=]() {
            std::vector<uint8_t> content;
            uint32_t *imagePixel = imagePixels[k];
            gifEncoder->addImage(imagePixel, delay, quantizerType,
                                 ditherType, left, top, content);
            delete[] imagePixel;
            return content;
        });
        tasks.emplace_back(std::move(result));
    }
    for (auto &task : tasks) {
        std::vector<uint8_t> result = task.get();
        gifEncoder->flush(result);
    }
    return true;
}

void BurstLinker::release() {
    if (gifEncoder != nullptr) {
        gifEncoder->finishEncoding();
    }
}

void BurstLinker::analyzerGifInfo(const char *path) {
    GifAnalyzer gifAnalyzer;
    gifAnalyzer.showGifInfo(path);
}
