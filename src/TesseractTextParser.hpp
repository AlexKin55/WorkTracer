#pragma once
#include "ITextParser.hpp"
#include <tesseract/baseapi.h>
#include <cmath>

class TesseractTextParser : public ITextParser {
private:
    tesseract::TessBaseAPI* ocr;

public:
    TesseractTextParser();
    ~TesseractTextParser() override;

    std::string parse(const cv::Mat& image) override;
};
